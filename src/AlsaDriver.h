//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2010 Peter J. Stieber
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
#include "Player.h"
#include "Audio.h"
#include "AlsaPlayer.h"

#include <sys/time.h>

class JZSample;
class JZAlsaAudioListener;

class JZAlsaAudioPlayer : public JZAlsaPlayer
{
  friend class JZAlsaAudioListener;
  public:
    JZAlsaAudioPlayer(JZSong *song);
    virtual ~JZAlsaAudioPlayer();
    int LoadSamples(const char *filename);
    virtual void Notify();
    virtual void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);
    virtual void StopPlay();
    virtual bool IsInstalled()
    {
      return mInstalled && JZAlsaPlayer::IsInstalled();
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
    virtual bool IsListening() const
    {
      return mpListener != 0;
    }
    virtual int GetListenerPlayPosition();
    virtual void StartAudio();
    virtual void ResetPlay(int clock);

    enum
    {
      PLAYBACK = 0,
      CAPTURE
    };

    // for recording
    int RecordMode() const;
    int PlayBackMode() const;

  private:

    int WriteSamples();
    void ReadSamples();
    void MidiSync();
    void OpenDsp(int mode, int sync_mode);

    void CloseDsp(bool Reset);

    int GetCurrentPosition(int mode);
    int GetFreeSpace(int mode);

    // If true can do full duplex record/play.
    int mCanDuplex;

    snd_pcm_t *pcm[2];
    bool mInstalled;

    int audio_clock_offset;
    int cur_pos;
    int last_scount;
    int cur_scount;
    int running_mode;
    int  midi_speed;  // start speed in bpm
    int  curr_speed;  // actual speed in bpm

    // False means MIDI only.
    bool mAudioEnabled;

    int card; // card number in config
    std::string mDeviceNames[2]; // device names
    int frag_size[2];
    int frag_byte_size[2];
    int frame_shift[2];
    int frame_boundary[2];

    JZAlsaAudioListener* mpListener;
    JZAudioRecordBuffer recbuffers;
};
