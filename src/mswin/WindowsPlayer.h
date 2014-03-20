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

#include "Player.h"

extern "C"
{
#include <mmsystem.h>
}

#include "WindowsMidiInterface.h"

//*****************************************************************************
//*****************************************************************************
class JZWindowsPlayer : public JZPlayer
{
  public:

    JZWindowsPlayer(JZSong* pSong);

    virtual bool IsInstalled();
    virtual ~JZWindowsPlayer();
    virtual int OutEvent(JZEvent* pEvent);
    virtual bool OutSysex(JZEvent* pEvent, DWORD time);
    void OutNow(JZEvent* pEvent);
    void OutNow(JZParam* pParam);
    void OutBreak();
    virtual void OutBreak(int BreakOver);
    void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);
    void StopPlay();
    virtual int GetRealTimeClock() = 0;
    virtual void FlushToDevice();
    void SetSoftThru(int on, int InputDevice, int OutputDevice);

    virtual void InitMtcRec()
    {
    }

    virtual JZMtcTime* FreezeMtcRec()
    {
      return 0;
    }

    static void SettingsDlg(int& InputDevice, int& OutputDevice);

    enum
    {
      MAX_MIDI_DEVS = 10
    };

  protected:

    tWinPlayerState* mpState;
    DWORD Event2Dword(JZEvent* pEvent);
    JZEvent* Dword2Event(DWORD dw);
    int Clock2Time(int clock);
    int Time2Clock(int time);
    void SetTempo(int bpm, int clock);
    BOOL timer_installed;
    int midiClockOut;
    int lastValidMtcClock;
    void FillMidiClocks(int to);
    void FlushToDevice(int clock);

    JZEventArray OutOfBandEvents;
    int RealTimeClock2Time(int clock);
    int Time2RealTimeClock(int time);
    void SetRealTimeTempo(int bpm, int clock);
    int real_start_time;
    int real_ticks_per_minute;

    // buffer for sysexdata
    HANDLE hSysHdr;
    MIDIHDR *pSysHdr;
    HANDLE hSysBuf;
    unsigned char *pSysBuf;
    unsigned short maxSysLen;
};

//*****************************************************************************
//*****************************************************************************
class JZWindowsIntPlayer : public JZWindowsPlayer
{
  public:

    JZWindowsIntPlayer(JZSong* pSong)
      : JZWindowsPlayer(pSong)
    {
    }

    virtual int GetRealTimeClock();
};

//*****************************************************************************
//*****************************************************************************
class JZWindowsMidiPlayer : public JZWindowsPlayer
{
  public:

    JZWindowsMidiPlayer(JZSong* pSong)
      : JZWindowsPlayer(pSong)
    {
    }

    virtual int GetRealTimeClock();
    virtual int OutEvent(JZEvent* pEvent);
    virtual void OutBreak(int clock);
};

//*****************************************************************************
//*****************************************************************************
class JZWindowsMtcPlayer : public JZWindowsPlayer
{
  public:
    JZWindowsMtcPlayer(JZSong* pSong)
      : JZWindowsPlayer(pSong)
    {
    }

    virtual int GetRealTimeClock();
    virtual void InitMtcRec();
    virtual JZMtcTime* FreezeMtcRec();
};
