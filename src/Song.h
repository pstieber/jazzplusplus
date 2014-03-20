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

#include "Track.h"
#include "Configuration.h"
#include "Globals.h"

#include <string>

class JZMetronomeInfo;
class JZSong;

//*****************************************************************************
//*****************************************************************************
class JZBarInfo
{
  public:

    JZBarInfo(const JZSong& Song);

    int GetBarIndex() const;

    void SetBar(int Bar = 0);

    int GetClock() const;

    void SetClock(int Clock = 0);

    int GetCountsPerBar() const;

    int GetTicksPerBar() const;

    void Next();

  private:

    int mBarIndex;
    int mClock;
    int mCountsPerBar;
    int mTicksPerQuarter;
    int mTicksPerBar;
    JZEventIterator mIterator;
    JZEvent* mpEvent;

};

//*****************************************************************************
//*****************************************************************************
class JZSong
{
  friend class JZBarInfo;

  public:

    JZSong();
    virtual ~JZSong();

    int GetMaxQuarters() const;

    int GetTicksPerQuarter() const;

    void SetTicksPerQuarter(int TicksPerQuarter);

    int GetTrackCount() const;

    int GetIntroLength() const;

    void SetIntroLength(int IntroLength);

    void NewUndoBuffer();
    void Undo();
    void Redo();

    void Clear();

    void Read(JZReadBase& Io, const std::string& FileName);

    void Write(JZWriteBase& Io, const std::string& FileName);

    JZTrack* GetTrack(int TrackIndex);

    int GetLastClock() const;

    int NumUsedTracks();        // number of used tracks

    int Speed();

// SN++
    void moveTrack(int from,int to);
//

    void ClockToString(int Clock, std::string& ClockString) const;

    int StringToClock(const std::string& ClockString) const;

    // Merge events from all tracks into the destination array.
    void MergeTracks(
      int FrClock,
      int ToClock,
      JZEventArray *Destin,
      const JZMetronomeInfo& MetronomeInfo,
      int DeltaClock = 0,
      bool AudioMode = false);

    void MergePlayTrackEvent(
      JZPlayTrackEvent* c,
      JZEventArray *Destin,
      int recursionDepth);

    int SetMeterChange(
      int BarNr,
      int Numerator,
      int Denomiator); //  0 = ok

  private:

    void MakeMetronome(
      int FrClock,
      int ToClock,
      JZEventArray *Destin,
      const JZMetronomeInfo& MetronomeInfo,
      int delta = 0);

  private:

    // This value sindicates the end of song in quarter notes for scrollbar
    // settings.
    int mMaxQuarters;

    int mTicksPerQuarter;

    int mIntroLength;

  public:

    int mTrackCount;
    JZTrack mTracks[eMaxTrackCount];
};

//*****************************************************************************
// Description:
//   These are the bar information class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZBarInfo::GetBarIndex() const
{
  return mBarIndex;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZBarInfo::GetClock() const
{
  return mClock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZBarInfo::GetTicksPerBar() const
{
  return mTicksPerBar;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZBarInfo::GetCountsPerBar() const
{
  return mCountsPerBar;
}

//*****************************************************************************
// Description:
//   These are the song class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZSong::GetMaxQuarters() const
{
  return mMaxQuarters;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZSong::GetTicksPerQuarter() const
{
  return mTicksPerQuarter;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZSong::GetTrackCount() const
{
  return mTrackCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZSong::GetIntroLength() const
{
  return mIntroLength;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZSong::SetIntroLength(int IntroLength)
{
  mIntroLength = IntroLength;
}
