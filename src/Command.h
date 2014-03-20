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

#include "CommandUtilities.h"

class JZFilter;
class JZEvent;
class JZTrack;
class JZSong;
class JZEventArray;
class JZRndArray;
class JZBarInfo;
class JZKeyOnEvent;

//*****************************************************************************
//*****************************************************************************
class JZScale
{
  public:
    void Init(int ScaleNr, JZFilter* pFilter = 0);
    int ScaleKeys[12];

    int Member(int Key)
    {
      return ScaleKeys[Key % 12];
    }

    int Next(int Key);
    int Prev(int Key);
    int Transpose(int Key, int Steps);
    int FitInto(int Key);
    static int Analyze(JZFilter* pFilter);        // returns ScaleNr
};

//*****************************************************************************
//*****************************************************************************
class JZCommand
{
  public:

    JZCommand(JZFilter* pFilter);

    virtual ~JZCommand();

    virtual void Execute(int NewUndo = 1);

    virtual void ExecuteTrack(JZTrack* pTrack);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

    int Interpolate(int Clock, int vmin, int vmax);

  public:

    JZFilter* mpFilter;
    JZSong* mpSong;
    bool mReverse;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandShift : public JZCommand
{
  public:

    JZCommandShift(JZFilter* pFilter, long DeltaClock);
    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    long mDeltaClock;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandErase : public JZCommand
{
  public:
    int LeaveSpace;
    JZCommandErase(JZFilter* pFilter, int LeaveSpace = 1);
    virtual void Execute(int NewUndo = 1);
    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);
};

//*****************************************************************************
//*****************************************************************************
class JZtCommandVelocity : public JZCommand
{
  public:

    JZtCommandVelocity(
      JZFilter* pFilter,
      int From,
      int To,
      JEValueAlterationMode Mode);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    int mFromValue, mToValue;
    JEValueAlterationMode mMode;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandLength : public JZCommand
{
  public:

    JZCommandLength(
      JZFilter* pFilter,
      int FromValue,
      int ToValue,
      JEValueAlterationMode Mode);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    int mFromValue, mToValue;
    JEValueAlterationMode mMode;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandSequenceLength : public JZCommand
{
  public:
  double scale;
  long startClock;
    JZCommandSequenceLength(JZFilter* pFilter, double scale);
    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);
};

//*****************************************************************************
//*****************************************************************************
class JZCommandMidiDelay : public JZCommand
{
  public:
  double scale;
  long clockDelay;
  int repeat;

    JZCommandMidiDelay(
      JZFilter* pFilter,
      double scale,
      long clockDelay,
      int repeat);
    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);
};

//*****************************************************************************
//*****************************************************************************
class JZCommandConvertToModulation : public JZCommand
{
  public:

  JZCommandConvertToModulation(JZFilter* pFilter);
  virtual void ExecuteTrack(JZTrack* pTrack);
};

//*****************************************************************************
//*****************************************************************************
class JZCommandCleanup : public JZCommand
{
    long lengthLimit;
    int  shortenOverlaps;
    JZKeyOnEvent *prev_note[16][128];
  public:
    JZCommandCleanup(JZFilter* pFilter, long limitClocks, int shortenOverlaps);
    virtual void ExecuteTrack(JZTrack* pTrack);
    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);
};

//*****************************************************************************
//*****************************************************************************
class JZCommandSearchReplace : public JZCommand
{
  public:

    JZCommandSearchReplace(JZFilter* pFilter, short From, short To);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    short mFrom, mTo;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandQuantize : public JZCommand
{
  public:

    JZCommandQuantize(
      JZFilter* pFilter,
      int QntClocks,
      bool NoteStart,
      bool NoteLength,
      int Groove,
      int Delay);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    long Quantize(int Clock, int islen);

  private:

    int mQntClocks;
    bool mNoteStart;
    bool mNoteLength;
    int mGroove;
    int mDelay;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandTranspose : public JZCommand
{
  public:

    JZCommandTranspose(
      JZFilter* pFilter,
      int Notes,
      int ScaleIndex = 0,
      bool FitIntoScale = false);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    int mNotes;
    int mFitIntoScale;
    JZScale mScale;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandSetChannel : public JZCommand
{
  public:

    JZCommandSetChannel(JZFilter* pFilter, int NewChannel);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    int mNewChannel;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandCopyToBuffer : public JZCommand
{
  public:

    JZCommandCopyToBuffer(JZFilter* pFilter, JZEventArray *Buffer);

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:

    JZEventArray* mpBuffer;
};

//*****************************************************************************
//*****************************************************************************
class JZCommandCopy : public JZCommand
{
  public:
    int  DestTrack;
    long DestClock;

    int EraseSource;        // no
    int EraseDestin;        // yes
    int InsertSpace;        // no
    long RepeatClock;        // -1L

    JZCommandCopy(JZFilter* pFilter, long DestTrack, long DestClock);
    virtual void ExecuteTrack(JZTrack* pTrack);
};

//*****************************************************************************
//*****************************************************************************
class JZCommandExchangeLeftRight : public JZCommand
{
  public:
    JZCommandExchangeLeftRight(JZFilter* pFilter);
    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);
};

//*****************************************************************************
//*****************************************************************************
class JZCommandExchangeUpDown : public JZCommand
{
  public:
    JZCommandExchangeUpDown(JZFilter* pFilter);
    virtual void ExecuteTrack(JZTrack* pTrack);
};

//*****************************************************************************
//*****************************************************************************
class JZCommandMapper : public JZCommand
{
  public:

    enum prop
    {
      veloc,
      length,
      key,
      rhythm,
      random,
      pan,
      modul,
      cc1,
      cc2,
      pitch,
      clock
    };

    JZCommandMapper(
      JZFilter* pFilter,
      prop Destination,
      prop dst,
      JZRndArray& RandomArray,
      int BarCount,
      bool Add);

    virtual ~JZCommandMapper();

    virtual void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  private:
    int mBarCount;
    int mStartBar;
    bool mAdd;
    prop mSource;
    prop mDestination;
    JZBarInfo* mpBarInfo;
    JZRndArray& mRandomArray;
};
