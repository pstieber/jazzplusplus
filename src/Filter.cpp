//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2013 Peter J. Stieber
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//*****************************************************************************

#include "Filter.h"

#include "Dialogs/FilterDialog.h"

#include "Events.h"
#include "Help.h"
#include "Song.h"

#include <cstdlib>

using namespace std;

//*****************************************************************************
//*****************************************************************************
const JZFilterEvent DefaultFilterEvents[eFilterCount] =
{
  { StatKeyOn,          "Note",               1,     0,  127},
  { StatKeyPressure,    "Poly Aftertouch",    1,     0,  127},
  { StatControl,        "Controller",         1,     0,  127},
  { StatProgram,        "Patch",              1,     0,  127},
  { StatPitch,          "Pitch",              1, -8192, 8192}
};

//*****************************************************************************
// Description:
//   This is the filter class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZFilter::JZFilter(JZSong* pSong)
  : mFilterEvents(0),
    mOtherSelected(true),
    mpSong(pSong),
    mFromClock(0),
    mToClock(120 * 4),
    mFromTrack(1),
    mToTrack(1)
{
  mFilterEvents = new JZFilterEvent [eFilterCount];
  memcpy(mFilterEvents, ::DefaultFilterEvents, sizeof(::DefaultFilterEvents));

  for (int i = 0; i < eFilterCount; ++i)
  {
    mFilterEvents[i].FromValue = DefaultFilterEvents[i].MinValue;
    mFilterEvents[i].ToValue = DefaultFilterEvents[i].MaxValue;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZFilter::JZFilter(JZFilter const& Other)
  : mFilterEvents(0),
    mOtherSelected(Other.mOtherSelected),
    mpSong(Other.mpSong),
    mFromClock(Other.mFromClock),
    mToClock(Other.mToClock),
    mFromTrack(Other.mFromTrack),
    mToTrack(Other.mToTrack)
{
  mFilterEvents = new JZFilterEvent [eFilterCount];
  memcpy(mFilterEvents, Other.mFilterEvents, sizeof(::DefaultFilterEvents));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZFilter& JZFilter::operator = (JZFilter const& Rhs)
{
  if (this != &Rhs)
  {
    memcpy(mFilterEvents, Rhs.mFilterEvents, sizeof(::DefaultFilterEvents));
    mOtherSelected = Rhs.mOtherSelected;
    mpSong         = Rhs.mpSong;
    mFromClock     = Rhs.mFromClock;
    mToClock       = Rhs.mToClock;
    mFromTrack     = Rhs.mFromTrack;
    mToTrack       = Rhs.mToTrack;
  }
  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZFilter::~JZFilter()
{
  delete mFilterEvents;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::GetFilterEvent(
  TEFilterType FilterType,
  bool& Selected,
  int& FromValue,
  int& ToValue)
{
  Selected = mFilterEvents[FilterType].Selected;
  FromValue = mFilterEvents[FilterType].FromValue;
  ToValue = mFilterEvents[FilterType].ToValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::SetFilterEvent(
  TEFilterType FilterType,
  bool Selected,
  int FromValue,
  int ToValue)
{
  mFilterEvents[FilterType].Selected = Selected;
  mFilterEvents[FilterType].FromValue = FromValue;
  mFilterEvents[FilterType].ToValue   = ToValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::SetOtherSelected(bool OtherSelected)
{
  mOtherSelected = OtherSelected;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::GenerateFromTimeString(string& FromTimeString) const
{
  if (mpSong)
  {
    mpSong->ClockToString(mFromClock, FromTimeString);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::SetFromTime(const string& FromTimeString)
{
  if (mpSong)
  {
    mFromClock  = mpSong->StringToClock(FromTimeString);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::GenerateToTimeString(string& ToTimeString) const
{
  if (mpSong)
  {
    mpSong->ClockToString(mToClock, ToTimeString);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::SetToTime(const string& ToTimeString)
{
  if (mpSong)
  {
    mToClock  = mpSong->StringToClock(ToTimeString);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZFilter::IsSelected(JZEvent* pEvent)
{
  int Value = pEvent->GetValue();
  for (int i = 0; i < eFilterCount; ++i)
  {
    if (pEvent->GetStat() == mFilterEvents[i].Stat)
    {
      // Aftertouch belongs to KeyOn events.
      if (pEvent->GetStat() == StatKeyPressure)
      {
        int aval = pEvent->IsKeyPressure()->GetKey();
        return
          mFilterEvents[i].Selected &&
          mFilterEvents[i].FromValue <= aval &&
          aval <= mFilterEvents[i].ToValue;
      }
      if (pEvent->GetStat() == StatTimeSignat)
      {
        return mFilterEvents[i].Selected;
      }
      if (pEvent->GetStat() == StatChnPressure)
      {
        return mFilterEvents[i].Selected;
      }

      if (pEvent->GetStat() == StatSysEx)
      {
        return mFilterEvents[i].Selected;
      }

      return
        mFilterEvents[i].Selected &&
        mFilterEvents[i].FromValue <= Value &&
        Value <= mFilterEvents[i].ToValue;
    }
  }
  return mOtherSelected;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilter::Dialog(wxWindow* pParent)
{
  JZFilterDialog FilterDialog(*this, pParent);
  if (FilterDialog.ShowModal() == wxID_OK)
  {
  }
}

//*****************************************************************************
// Description:
//   This is the track iterator class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
JZTrackIterator::JZTrackIterator(JZFilter* pFilter, bool Reverse)
  : mpFilter(pFilter),
    mpSong(mpFilter->GetSong()),
    mTrackIndex(0),
    mReverse(Reverse)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrack* JZTrackIterator::First()
{
  if (mReverse)
  {
    mTrackIndex = mpFilter->GetToTrack();
  }
  else
  {
    mTrackIndex = mpFilter->GetFromTrack();
  }
  return mpSong->GetTrack(mTrackIndex);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrack* JZTrackIterator::Next()
{
  if (mReverse)
  {
    --mTrackIndex;
    if (mTrackIndex < mpFilter->GetFromTrack())
    {
      return 0;
    }
  }
  else
  {
    ++mTrackIndex;
    if (mTrackIndex > mpFilter->GetToTrack())
    {
      return 0;
    }
  }
  return mpSong->GetTrack(mTrackIndex);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZTrackIterator::Count() const
{
  return mpFilter->GetToTrack() - mpFilter->GetFromTrack() + 1;
}
