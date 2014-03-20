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

#include "config.h"
#include "Events.h"
#include "Track.h"
#include "Song.h"
//#include "midinet.h"
#include "Audio.h"

#include <wx/timer.h>

#include <sys/types.h>
#include <time.h>

class JZRecordingInfo;

//*****************************************************************************
//*****************************************************************************
enum JZClockSource
{
  CsInt = 0,
  CsFsk,
  CsMidi,
  CsMtc
};

//*****************************************************************************
//*****************************************************************************
class JZDeviceList
{
  public:

    JZDeviceList();

    virtual ~JZDeviceList();

    unsigned GetCount() const
    {
      return mDeviceNames.size();
    }

    const wxString& GetName(unsigned i) const
    {
      if (i < mDeviceNames.size())
      {
        return mDeviceNames[i];
      }
      static wxString Unknown("Unknown");
      return Unknown;
    }

    unsigned Add(const wxString& Name)
    {
      mDeviceNames.push_back(Name);
      return mDeviceNames.size();
    }

    void Clear()
    {
      mDeviceNames.clear();
    }

  protected:

    std::vector<wxString> mDeviceNames;

  private:

    // Hidden and unimplemented to prevent accidental copy or assignment.
    JZDeviceList(const JZDeviceList &);
    JZDeviceList& operator = (const JZDeviceList &);
};

//*****************************************************************************
//*****************************************************************************
class JZPlayLoop
{
  public:

    JZPlayLoop();

    void Set(int Start, int Stop);

    void Reset();

    // external clock -> internal clock where
    //   external clock == physical clock
    //   internal clock == song position
    int Ext2IntClock(int Clock);

    // the other way round
    int Int2ExtClock(int Clock);

    void PrepareOutput(
      JZEventArray* pEventArray,
      JZSong* pSong,
      int ExtFr,
      int ExtTo,
      bool AudioMode = false);

  private:

    int mStartClock;

    int mStopClock;
};

//*****************************************************************************
//*****************************************************************************
class JZPlayer : public wxTimer
{
  public:

    JZPlayer(JZSong* pSong);

    virtual ~JZPlayer();

    void Notify();

    virtual void FlushToDevice();

    // return 0 = ok, 1 = buffer full, try again later
    virtual int OutEvent(JZEvent* pEvent) = 0;

    virtual void OutBreak() = 0;

    // Send event immediately ignoring the clock.
    void OutNow(JZTrack *t, JZEvent* pEvent)
    {
      pEvent->SetDevice(t->GetDevice());
      OutNow(pEvent);
    }

    void OutNow(int device, JZEvent* pEvent)
    {
      pEvent->SetDevice(device);
      OutNow(pEvent);
    }

    void OutNow(JZTrack* t, JZParam* r);

    // what's played right now?
    virtual int GetRealTimeClock() = 0;

    virtual void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);
    virtual void StopPlay();
    virtual void AllNotesOff(bool Reset = false);

    virtual void SetSoftThru(int on, int idev, int odev)
    {
    }

    virtual void SetHardThru(int on, int idev, int odev)
    {
    }

    virtual void InitMtcRec()
    {
    }

    virtual JZMtcTime* FreezeMtcRec()
    {
      return 0;
    }

    // Tests if hardware found and successfully setup.
    virtual bool IsInstalled() = 0;

    // if unable to install, pop up a messagebox explaining why.
    virtual void ShowError();

    const JZEventArray& GetRecordBuffer() const
    {
      return mRecdBuffer;
    }

    void SetRecordInfo(JZRecordingInfo* pRecordingInfo)
    {
      mpRecordingInfo = pRecordingInfo;
    }

    bool IsRecordBufferEmpty() const
    {
      return mRecdBuffer.IsEmpty();
    }

    bool IsPlaying() const
    {
      return mPlaying;
    }

    virtual int FindMidiDevice()
    {
      return -1;
    }

    virtual int SupportsMultipleDevices()
    {
      return 0;
    }
    virtual JZDeviceList& GetOutputDevices()
    {
      return DummyDeviceList;
    }
    virtual JZDeviceList& GetInputDevices()
    {
      return DummyDeviceList;
    }
    virtual int GetThruInputDevice()
    {
      return 0;
    }
    virtual int GetThruOutputDevice()
    {
      return 0;
    }

    // Audio stuff
    virtual void StartAudio()
    {
    }

    virtual bool GetAudioEnabled() const
    {
      return false;
    }

    virtual void SetAudioEnabled(bool AudioEnabled)
    {
    }

    virtual void ListenAudio(int key, int start_stop_mode = 1)
    {
    }
    virtual void ListenAudio(JZSample &spl, int fr_smpl, int to_smpl)
    {
    }
    virtual bool IsListening() const
    {
      return 0;
    }

    virtual const std::string& GetSampleLabel(int Index)
    {
      return mSamples.GetSampleLabel(Index);
    }

    virtual void AdjustAudioLength(JZTrack *t)
    {
      int ticks_per_minute = mpSong->GetTicksPerQuarter() * mpSong->Speed();
      mSamples.AdjustAudioLength(t, ticks_per_minute);
    }

    void EditAudioGlobalSettings(wxWindow* pParent);

    void EditAudioSamples(wxWindow* pParent);

    void LoadSampleSet(wxWindow* pParent);

    void SaveSampleSetAs(wxWindow* pParent);

    void SaveSampleSet(wxWindow* pParent);

    void ClearSampleSet(wxWindow* pParent);

    void EditSample(int key)
    {
      mSamples.Edit(key);
    }

    virtual int GetListenerPlayPosition()
    {
      return -1;
    }

    void LoadDefaultSettings()
    {
      mSamples.LoadDefaultSettings();
    }

  protected:

    virtual void OutNow(JZEvent* pEvent) = 0;

  protected:

    int mOutClock;

    JZPlayLoop* mpPlayLoop;

    // This is the timer value for polling the record queue.
    int mPollMillisec;

     // If this value is 0, then not recording.
    JZRecordingInfo* mpRecordingInfo;

    bool mPlaying;

    JZSong* mpSong;

    JZEventArray mPlayBuffer;
    JZEventArray mRecdBuffer;

    JZEventArray* mpAudioBuffer;

    JZSampleSet mSamples;

  private:

    JZDeviceList DummyDeviceList;
};

extern char *midinethost;

//*****************************************************************************
// Roland MPU 401
//*****************************************************************************

#ifdef DEV_MPU401

#include <unistd.h>
#include <fcntl.h>

//*****************************************************************************
//*****************************************************************************
class JZBuffer : public JZWriteBase
{

    char Buffer[2000];
    int  Written, Read;

  public:

    int Clock;
    int  RunningStatus;

    void Clear()
    {
      Read = Written = 0;
      Clock = 0;
      RunningStatus = 0;
    }

    JZBuffer()
    {
      Clear();
    }

    int Put(char c)
    {
      if (Written < (int)sizeof(Buffer))
      {
        Buffer[Written++] = c;
        return 0;
      }
      return -1;
    }

    int Get(int dev)
    {
      if (Read == Written)
        ReadFile(dev);
      if (Read != Written)
        return (unsigned char)Buffer[Read++];
      return -1;
    }

    int Empty()
    {
      return Read == Written;
    }

    int FreeBytes()
    {
      return (int)sizeof(Buffer) - Written;
    }

    void PutVar(int val)
    {
      unsigned int buf;
      buf = val & 0x7f;
      while ((val >>= 7) > 0)
      {
        buf <<= 8;
        buf |= 0x80;
        buf += (val & 0x7f);
      }

      while (1)
      {
        Put((unsigned char)buf);
        if (buf & 0x80)
          buf >>= 8;
        else
          break;
      }
    }

    int GetVar(int dev)
    {
      unsigned int val;
      int c;
      val = Get(dev);
      if (val & 0x80)
      {
        val &= 0x7f;
        do
        {
          c = Get(dev);
          assert(c > 0);
          val = (val << 7) + (c & 0x7f);
        } while (c & 0x80);
      }
      return val;
    }

    int WriteFile(int dev)
    {
      int bytes;
      if (Read != Written)
      {
        bytes = write_noack_mpu(Buffer + Read, Written - Read);
        if (bytes > 0)
          Read += bytes;
      }
      if (Read != Written)
        return 0;
      Read = Written = 0;
      return 1;
    }

    int ReadFile(int dev)
    {
      int i, bytes;
      if (Read)        // move data to beginning of buffer
      {
        for (i = 0; i < Written - Read; i++)
          Buffer[i] = Buffer[i + Read];
        Written -= Read;
        Read = 0;
      }
      non_block_io( dev, 1 );
      bytes = read(dev, Buffer + Written, sizeof(Buffer) - Written);
      non_block_io( dev, 0 );
      if (bytes > 0)
      {
        Written += bytes;
        return bytes;
      }
      return 0;
    }

    int Write(JZEvent *e, unsigned char *data, int len)
    {
      int i;
      for (i = 0; i < len; i++)
        Put(data[i]);
      return 0;
    }
};


// ---------------------------- jazz-driver -----------------------------

// Define 0xfa to mean start-play-command (filtered by midinetd)
#define START_PLAY_COMMAND 0xfa

// Define 0xfb to mean stop-play-command (filtered by midinetd)
#define STOP_PLAY_COMMAND 0xfb

// 0xfb also sent by midinetd to mark start of recorded data
#define START_OF_RECORD_BUFFER 0xfb

#define ACTIVE_TRACKS 7
#define ACTIVE_TRACKS_MASK 0x7f

//*****************************************************************************
//*****************************************************************************
class JZMpuPlayer : public JZPlayer
{
    int  dev;
    JZBuffer PlyBytes;
    JZBuffer RecBytes;
    int playclock;
    int clock_to_host_counter;

    int ActiveTrack;
    int TrackClock[ACTIVE_TRACKS];
    int TrackRunningStatus[ACTIVE_TRACKS];

    JZEventArray OutOfBandEvents;

  public:

    JZMpuPlayer(JZSong *song);
    virtual ~JZMpuPlayer();
    int  OutEvent(JZEvent *e);
    void OutNow(JZEvent *e);
    void OutBreak();
    void OutBreak(int BreakOver);
    void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);
    void StopPlay();
    int GetRealTimeClock();
    virtual bool IsInstalled();
    int GetRecordedData();
    void SetHardThru(int on, int idev, int odev);

    void FlushOutOfBand( int Clock );
};

#define TRK (0<<6)
#define DAT (1<<6)
#define CMD (2<<6)
#define RES (3<<6)

#define MPUDEVICE "/dev/mpu401"
#define MIDINETSERVICE "midinet"

#endif // DEV_MPU401

//*****************************************************************************
// Description:
//   This is the null driver class declaration.
//*****************************************************************************
class JZNullPlayer : public JZPlayer
{
  public:

    JZNullPlayer(JZSong* pSong)
      : JZPlayer(pSong)
    {
    }

    virtual bool IsInstalled()
    {
      return true;
    }

    virtual ~JZNullPlayer()
    {
    }

    int OutEvent(JZEvent* pEvent)
    {
      return 0;
    }

    void OutNow(JZEvent* pEvent)
    {
    }

    void OutBreak()
    {
    }

    void StartPlay(int Clock, int LoopClock = 0, int Continue = 0)
    {
    }

    void StopPlay()
    {
    }

    int GetRealTimeClock()
    {
      return 0;
    }
};

// --------------------------- voxware driver -------------------------------

#ifdef DEV_SEQUENCER2

#include <sys/soundcard.h>

SEQ_USE_EXTBUF();
extern int seqfd;
extern int mididev;
void seqbuf_dump(void);
#define seqbuf_empty() (_seqbufptr == 0)
#define seqbuf_clear() (_seqbufptr = 0)
void seqbuf_flush_last_event();


//*****************************************************************************
//*****************************************************************************
class JZOssThru : public wxTimer
{
  public:
    virtual void Notify();
    JZOssThru();
    ~JZOssThru();
};

//*****************************************************************************
//*****************************************************************************
class JZSeq2Player : public JZPlayer
{
  public:

    friend class JZOssThru;
    JZSeq2Player(JZSong* pSong);
    virtual bool IsInstalled();
    virtual ~JZSeq2Player();
    int  OutEvent(JZEvent *e, int now);
    int  OutEvent(JZEvent *e)
    {
      OutEvent(e, 0);
      return 0;
    }
    void OutNow(JZEvent *e)
    {
      OutEvent(e, 1);
    }
    void OutBreak();
    void OutBreak(int BreakOver);
    void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);
    void StopPlay();
    int GetRealTimeClock();
    virtual void FlushToDevice();
    void SetSoftThru(int on, int idev, int odev);
    int     FindMidiDevice();

  protected:

    int    play_clock;
    int    recd_clock;
    int    start_clock;
    int    echo_clock;

    JZOssThru *through;
    int     card_id;
};

#endif // DEV_SEQUENCER2
