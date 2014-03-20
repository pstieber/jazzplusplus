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

#include <mmsystem.h>

#include "WindowsPlayer.h"
#include "Audio.h"
#include "WindowsMidiInterface.h"

class JZSample;

//*****************************************************************************
//*****************************************************************************
class JZWindowsAudioPlayer : public JZWindowsIntPlayer
{
  friend class JZAudioListener;

  public:

    enum TEErrorCode
    {
      NoError,
      ErrOutOpen,
      ErrOutPrepare,
      ErrOutUnprepare,
      ErrInpOpen,
      ErrInpPrepare,
      ErrInpUnprepare,
      ErrCapGet,
      ErrCapSync
    };

    JZWindowsAudioPlayer(JZSong* pSong);

    virtual ~JZWindowsAudioPlayer();

    int LoadSamples(const char *filename);

    virtual void Notify();

    virtual void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);

    virtual void StopPlay();

    virtual void StartAudio();   // called async by driver

    virtual bool IsInstalled()
    {
      return mInstalled && JZWindowsIntPlayer::IsInstalled();
    }

    virtual bool GetAudioEnabled() const
    {
      return mAudioEnabled;
    }

    virtual void SetAudioEnabled(bool AudioEnabled)
    {
      mAudioEnabled = AudioEnabled;
    }

    virtual void ListenAudio(int key, int start_stop_mode = 1);

    virtual void ListenAudio(JZSample &spl, int fr_smpl, int to_smpl);

    virtual int GetListenerPlayPosition();

    virtual bool IsListening() const
    {
      return mpListener != 0;
    }

    // for recording
    int RecordMode() const;

    int PlaybackMode() const
    {
      return !RecordMode() || mCanDuplex;
    }

    TEErrorCode GetError()
    {
      return mErrorCode;
    }

    virtual void ShowError();

  private:

    // ms specific
    friend void FAR PASCAL audioInterrupt(
      HWAVEOUT,
      UINT,
      DWORD,
      DWORD,
      DWORD);

    // Description:
    //   Send the sample set to driver.
    void WriteBuffers();

    void AudioCallback(UINT msg);

    int OpenDsp();    // 0 = ok
    int CloseDsp();   // 0 = ok

  private:

    TEErrorCode mErrorCode;

    // Indicates if full duplex record/play is possible.
    bool mCanDuplex;

    // Indicates if the  exact output play position can be determined.
    bool mCanSynchronize;

    bool mInstalled;

    // A value of false means MIDI only.
    bool mAudioEnabled;

    int blocks_played;       // # of blocks written to device
    int play_buffers_needed;  // driver requests more output buffers

    int start_clock;     // when did play start
    int start_time;      // play start time (not altered by SetTempo)
    JZAudioListener* mpListener;

    HWAVEOUT hout;
    HWAVEIN hinp;
    int hout_open;        // true = playback device opended successful
    int hinp_open;        // true = recording device opended successful

    JZAudioRecordBuffer recbuffers;
    int record_buffers_needed;  // driver needs more buffers

    // a semaphor for thread synchronization. Since Notify() and
    // the audio callback are not time critical, its safe to
    // let them wait for each other.
    CRITICAL_SECTION mutex;
};
