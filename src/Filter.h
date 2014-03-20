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

#pragma once

#include "Events.h"

class JZSong;
class JZTrack;
class wxDialog;
class wxWindow;

//*****************************************************************************
//*****************************************************************************
enum TEFilterType
{
  eFilterKeyOn             = 0,
  eFilterKeyPressure       = 1,  // PolyAftertouch belongs to KeyOn Events.
  eFilterControl           = 2,
  eFilterProgram           = 3,
  eFilterPitch             = 4,
  eFilterCount             = 5
};

//*****************************************************************************
//*****************************************************************************
class JZFilterEvent
{
  public:
    int Stat;
    const char* mName;
    bool Selected;
    int MinValue, MaxValue;
    int FromValue, ToValue;
};

//*****************************************************************************
//*****************************************************************************
class JZFilter : public wxObject
{
  public:

    JZFilter(JZSong* pSong);

    JZFilter(const JZFilter& Other);

    JZFilter& operator = (const JZFilter& Rhs);

    virtual ~JZFilter();

    void GetFilterEvent(
      TEFilterType FilterType,
      bool& Selected,
      int& FromValue,
      int& ToValue);

    void SetFilterEvent(
      TEFilterType FilterType,
      bool Selected,
      int FromValue,
      int ToValue);

    void SetOtherSelected(bool OtherSelected);

    JZSong* GetSong() const;

    int GetFromClock() const;
    void SetFromClock(int FromClock);
    void GenerateFromTimeString(std::string& FromTimeString) const;
    void SetFromTime(const std::string& FromTimeString);

    int GetToClock() const;
    void SetToClock(int ToClock);
    void GenerateToTimeString(std::string& ToTimeString) const;
    void SetToTime(const std::string& ToTimeString);

    int GetFromTrack() const;
    void SetFromTrack(int FromTrack);

    int GetToTrack() const;
    void SetToTrack(int ToTrack);

    bool GetFilterMeter() const;
    void SetFilterMeter(bool FilterMeter);

    bool GetFilterChannelAftertouch() const;
    void SetFilterChannelAftertouch(bool FilterChannelAftertouch);

    bool GetFilterSysEx() const;
    void SetFilterSysEx(bool FilterSysEx);

    bool GetFilterOther() const;
    void SetFilterOther(bool FilterOther);

    int IsSelected(JZEvent* pEvent);

    void Dialog(wxWindow* pParent);

  private:

    JZFilterEvent* mFilterEvents;

    bool mOtherSelected;

    JZSong* mpSong;

    int mFromClock, mToClock;

    int mFromTrack, mToTrack;

    bool mFilterMeter;

    bool mFilterChannelAftertouch;

    bool mFilterSysEx;

    bool mFilterOther;

    wxDialog* mpDialogBox;
};

//*****************************************************************************
// Description:
//   This is the track iterator class declaration.
//*****************************************************************************
class JZTrackIterator
{
  public:

    JZTrackIterator(JZFilter* pFilter, bool Reverse = false);
    JZTrack* First();
    JZTrack* Next();
    int Count() const;

  private:

    JZFilter* mpFilter;
    JZSong* mpSong;
    int mTrackIndex;
    bool mReverse;
};

//*****************************************************************************
// Description:
//   These are the filter class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
JZSong* JZFilter::GetSong() const
{
  return mpSong;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZFilter::GetFromClock() const
{
  return mFromClock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetFromClock(int FromClock)
{
  mFromClock = FromClock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZFilter::GetToClock() const
{
  return mToClock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetToClock(int ToClock)
{
  mToClock = ToClock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZFilter::GetFromTrack() const
{
  return mFromTrack;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetFromTrack(int FromTrack)
{
  mFromTrack = FromTrack;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZFilter::GetToTrack() const
{
  return mToTrack;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetToTrack(int ToTrack)
{
  mToTrack = ToTrack;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
bool JZFilter::GetFilterMeter() const
{
  return mFilterMeter;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetFilterMeter(bool FilterMeter)
{
  mFilterMeter = FilterMeter;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
bool JZFilter::GetFilterChannelAftertouch() const
{
  return mFilterChannelAftertouch;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetFilterChannelAftertouch(bool FilterChannelAftertouch)
{
  mFilterChannelAftertouch = FilterChannelAftertouch;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
bool JZFilter::GetFilterSysEx() const
{
  return mFilterSysEx;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetFilterSysEx(bool FilterSysEx)
{
  mFilterSysEx = FilterSysEx;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
bool JZFilter::GetFilterOther() const
{
  return mFilterOther;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZFilter::SetFilterOther(bool FilterOther)
{
  mFilterOther = FilterOther;
}
