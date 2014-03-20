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

#include "AudioDriver.h"

#include "RecordingInfo.h"
#include "Configuration.h"
#include "Globals.h"

#include <iostream>
#include <unistd.h>
//#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <fcntl.h>

using namespace std;

#define AUDIO_DEVICE "/dev/dsp"

//*****************************************************************************
//*****************************************************************************
class JZAudioListener : public wxTimer
{
  public:

    JZAudioListener(JZAudioPlayer* pPlayer, int key)
      : wxTimer(),
        mpPlayer(pPlayer),
        mCount(0),
        mHardExit(true)
    {
      mpPlayer->mpListener = this;

      mpPlayer->mpRecordingInfo = 0;  // not recording!

      // SYNC seems not to work?? so add 8 more silent buffers
      // to hear the end of the sample too.
      mCount = 8 + mpPlayer->mSamples.PrepareListen(key);
      mpPlayer->OpenDsp();
      Start(20);
    }

    JZAudioListener(
      JZAudioPlayer* pPlayer,
      JZSample& spl,
      int fr_smpl,
      int to_smpl)
      : wxTimer(),
        mpPlayer(pPlayer),
        mCount(0),
        mHardExit(true)
    {
      mpPlayer->mpListener = this;
      mpPlayer->mpRecordingInfo = 0;  // not recording!

      mCount = 8 + mpPlayer->mSamples.PrepareListen(&spl, fr_smpl, to_smpl);
      mpPlayer->OpenDsp();
      Start(20);
    }

    ~JZAudioListener()
    {
      Stop();
      mpPlayer->CloseDsp(mHardExit);
      mpPlayer->mpListener = 0;
    }

    virtual void Notify()
    {
      mCount -= mpPlayer->WriteSamples();
      mCount += mpPlayer->mSamples.ContinueListen();
      if (mCount <= 0)
      {
        mHardExit = false;
        delete this;
      }
    }

    int GetPlayPosition()
    {
      count_info cinfo;
      if (ioctl(mpPlayer->dev, SNDCTL_DSP_GETOPTR, &cinfo) == -1)
      {
        perror("SNDCTL_DSP_GETOPTR");
      }
      return (cinfo.bytes - cinfo.ptr) / sizeof(short);
    }

  private:

    JZAudioPlayer* mpPlayer;

    int mCount;

    int mHardExit;
};

//*****************************************************************************
//*****************************************************************************
JZAudioPlayer::JZAudioPlayer(JZSong* pSong)
  : JZSeq2Player(pSong)
{
  mpAudioBuffer = new JZEventArray();
  mInstalled = false;
  mAudioEnabled = (gpConfig->GetValue(C_EnableAudio) != 0);
  mpListener = 0;
  mCanDuplex = 0;    // no duplex yet.
  dev = -1;

  // check for device
  dev = open(AUDIO_DEVICE, O_WRONLY, 0);
  if (dev >= 0)
  {
    // check device caps
    int caps;
    ioctl(dev, SNDCTL_DSP_GETCAPS, &caps);
    if (caps & DSP_CAP_REALTIME)
    {
      ; // fprintf(stderr, AUDIO " supports REALTIME, good!\n");
    }

    if (caps & DSP_CAP_DUPLEX)
    {
      mCanDuplex = 1;   // good soundcard!
    }

    if (!(caps & DSP_CAP_TRIGGER))
    {
      cerr << "no CAP_TRIGGER support!" << endl;
    }
    else
    {
      mInstalled = true;
    }

    close(dev);
  }
  else
  {
    perror(AUDIO_DEVICE);
  }

  dev = -1;  // closed
  mAudioEnabled = mAudioEnabled && mInstalled;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZAudioPlayer::~JZAudioPlayer()
{
  delete mpListener;
  delete mpAudioBuffer;
  if (dev >= 0)
  {
    close(dev);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAudioPlayer::LoadSamples(const char* pFileName)
{
  return mSamples.Load(pFileName);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAudioPlayer::RecordMode() const
{
  return mpRecordingInfo != 0 && mpRecordingInfo->mpTrack->GetAudioMode();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::StartAudio()
{
  if (!mAudioEnabled)
  {
    return;
  }

  int ticks_per_minute = mpSong->GetTicksPerQuarter() * mpSong->Speed();
  mSamples.ResetBuffers(mpAudioBuffer, start_clock, ticks_per_minute);
  if (PlaybackMode())
  {
    mSamples.FillBuffers(mOutClock);
  }

  audio_bytes = 0;
  midi_clock  = 0;
  midi_speed  = mpSong->Speed();
  curr_speed  = midi_speed;

  OpenDsp();

  if (gpConfig->GetValue(C_OssBug1))
  {
    WriteSamples();
  }
  else
  {
    // ok, suspend the device until midi starts
    ioctl(dev, SNDCTL_DSP_SETSYNCRO, 0);
    if (PlaybackMode())
    {
      WriteSamples();
    }
    SEQ_PLAYAUDIO(0xffff);  // start all available devices
  }

  force_read = 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::OpenDsp()
{
  int tmp;

  if (!mAudioEnabled)
  {
    return;
  }

  mCanDuplex = gpConfig->GetValue(C_DuplexAudio);

  // linux driver seems to need some real free memory, which sometimes
  // is not available when operating with big samples.  So allocate
  // some memory here, touch it and free it. Hopefully libc returns it
  // to the system and the driver grabs it on open().  NOT TESTED!
  {
    const int size = 0x10000; // 64K
    char* pMemory = (char *)malloc(size);
    memset(pMemory, 0, size);
    free(pMemory);
  }

  int mode = 0;
  if (mCanDuplex)
  {
    mode = O_RDWR;
  }
  else if (RecordMode())
  {
    mode = O_RDONLY;
  }
  else
  {
    mode = O_WRONLY;
  }

  dev = open(AUDIO_DEVICE, mode, 0);
  if (dev < 0)
  {
    perror(AUDIO_DEVICE);
    mAudioEnabled = false;
    return;
  }

  if (mCanDuplex)
  {
    ioctl(dev, SNDCTL_DSP_SETDUPLEX, 0);
  }

  tmp = 0xffff0000 | FRAGBITS;
  if (ioctl(dev, SNDCTL_DSP_SETFRAGMENT, &tmp) == -1)
  {
    perror("ioctl DSP_SETFRAGMENT");
  }

  tmp = mSamples.GetBitsPerSample();
  ioctl(dev, SNDCTL_DSP_SAMPLESIZE, &tmp);
  if (tmp != mSamples.GetBitsPerSample())
  {
    cerr << "Unable to set the sample size" << endl;
  }

  tmp = (mSamples.GetChannelCount() == 1) ? 0 : 1;
  if (ioctl (dev, SNDCTL_DSP_STEREO, &tmp) == -1)
  {
    cerr << "Unable to set mono/stereo" << endl;
  }

  tmp = mSamples.GetSamplingRate();
  if (ioctl (dev, SNDCTL_DSP_SPEED, &tmp) == -1)
  {
    perror("ioctl DSP_SPEED");
  }

  // Check to see if the fragment size was OK.
  ioctl(dev, SNDCTL_DSP_GETBLKSIZE, &tmp);
  if (tmp < 1)
  {
    perror("GETBLKSIZE");
  }
  else if (tmp != FRAGBYTES)
  {
    cerr
      << "Unable to verify FRAGMENT " << tmp
      << ", fbytes = " << FRAGBYTES
      << ", fshorts = " << FRAGSHORTS
      << endl;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::CloseDsp(bool Reset)
{
  if (dev >= 0)
  {
    if (Reset)
    {
      if (ioctl(dev,  SNDCTL_DSP_RESET, 0) == -1)
      {
        perror("SNDCTL_DSP_RESET");
      }
    }
    else
    {
      if (ioctl (dev, SNDCTL_DSP_SYNC, NULL) < 0)
      {
        perror("SNDCTL_DSP_SYNC");
      }
    }
    close(dev);
    dev = -1;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::Notify()
{
  if (mAudioEnabled)
  {
    if (PlaybackMode())
    {
      WriteSamples();

      // here it may hang when swapping in pages
      mSamples.FillBuffers(mOutClock);

      WriteSamples();
    }
    if (RecordMode())
    {
      ReadSamples();
    }

    if (mSamples.GetSoftSync())
    {
      MidiSync();
    }
  }
  JZSeq2Player::Notify();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAudioPlayer::WriteSamples()
{
  if (!mAudioEnabled)
  {
    return 0;
  }

  int blocks_written = 0;

  // number of blocks to be written
  audio_buf_info info;
  if (ioctl(dev, SNDCTL_DSP_GETOSPACE, &info) == -1)
  {
    perror("SNDCTL_DSP_GETOSPACE");
  }

  // TODO:  This is a bug in the audiodriver in newer kernels (2.1.28)
  // and the OSS/Linux for 2.0.29 it should be
  // for (int i = 0; i < info.fragments; ++i)

  for (int i = 0; i < info.fragments - 1; i++)
  {
    JZAudioBuffer* buf = mSamples.mFullBuffers.Get();
    if (buf == 0)
    {
      break;
    }
    if (write(dev, buf->Data(), BUFBYTES) != BUFBYTES)
    {
      perror("write");
    }
    blocks_written ++;
    mSamples.mFreeBuffers.Put(buf);
  }

  return blocks_written;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::ReadSamples()
{
  audio_buf_info info;
  if (ioctl(dev, SNDCTL_DSP_GETISPACE, &info) == -1)
  {
    perror("SNDCTL_DSP_GETISPACE");
  }

  // a oss bug: if read is not called, there will be
  // no recording. probably recording does NOT start
  // exactly in sync with midi - but who knows.
  if (force_read && !info.fragments)
  {
    info.fragments = 1;
  }
  force_read = 0;

  for (int i = 0; i < info.fragments; ++i)
  {
    short* b = recbuffers.RequestBuffer()->data;
    if (read(dev, b, BUFBYTES) != BUFBYTES)
    {
      // OSS bug?  It returns EINTR?? on first read.
      if (errno != EINTR && errno != EAGAIN)
      {
        perror("read");
      }
      recbuffers.UndoRequest();
      break;
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::MidiSync()
{
  // OSS is buggy!  In Win32 SDK you read the docs, hack away and
  // everything works.  In OSS, there are no docs and if it works
  // with kernel x it wont with kernel y.

  if (!mAudioEnabled)
  {
    return;
  }

  int command = SNDCTL_DSP_GETOPTR;
  if (!PlaybackMode())
  {
    command = SNDCTL_DSP_GETIPTR;
  }

  // get realtime info for audio/midi sync
  count_info cinfo;
  if (ioctl(dev, command, &cinfo) == -1)
  {
    perror("SNDCTL_DSP_GETOPTR");
  }

  // search for SNDCTL_DSP_GETOPTR in linux/drivers/sound/dmabuf
  // before trying to understand the next line
  int new_bytes = cinfo.bytes - cinfo.ptr;  // info.ptr is garbage!!
  if (new_bytes != audio_bytes)
  {
    // driver has processed some bytes or whole fragment
    if (ioctl(seqfd, SNDCTL_SEQ_GETTIME, &midi_clock) < 0)
    {
      perror(
        "ioctl SNDCTL_SEQ_GETTIME failed - "
        "please get a newer kernel (2.1.28 or up)");
    }
    audio_bytes = new_bytes;

    // OSS bug?: mpu401 does not like speed changes too often
    int audio_clock = (int)mSamples.Samples2Ticks(audio_bytes / 2);
    int delta_clock = audio_clock - midi_clock;
    int new_speed = midi_speed + delta_clock;

    // limit speed changes to some reasonable values
    const int limit = 1;
    if (new_speed > midi_speed + limit)
    {
      new_speed = midi_speed + limit;
    }
    if (new_speed < midi_speed - limit)
    {
      new_speed = midi_speed - limit;
    }

    if (new_speed != curr_speed)
    {
      if (ioctl(seqfd, SNDCTL_TMR_TEMPO, &new_speed) < 0)
      {
        // Sometimes this happens with mpu-401 timer.
        ; // perror("SNDCTL_TMR_TEMPO");
      }
      else
      {
        curr_speed = new_speed;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::StartPlay(int Clock, int LoopClock, int Continue)
{
  delete mpListener;
  mSamples.StartPlay(Clock);
  JZSeq2Player::StartPlay(Clock, LoopClock, Continue);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::StopPlay()
{
  mSamples.StopPlay();
  JZSeq2Player::StopPlay();
  if (!mAudioEnabled)
  {
    return;
  }

  CloseDsp(true);
  if (RecordMode())
  {
    int frc = mpRecordingInfo->mFromClock;
    if (frc < start_clock)
    {
      frc = start_clock;
    }
    int toc = mpRecordingInfo->mToClock;
    if (toc > recd_clock)
    {
      toc = recd_clock;
    }
    mSamples.SaveRecordingDlg(frc, toc, recbuffers);
  }
  recbuffers.Clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::ListenAudio(int key, int start_stop_mode)
{
  if (!mAudioEnabled)
  {
    return;
  }

  // when already listening then stop listening
  if (mpListener)
  {
    delete mpListener;
    mpListener = 0;
    if (start_stop_mode)
    {
      return;
    }
  }
  if (key < 0)
  {
    return;
  }

  if (dev >= 0)  // device busy (playing)
  {
    return;
  }

  mpListener = new JZAudioListener(this, key);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioPlayer::ListenAudio(JZSample &spl, int fr_smpl, int to_smpl)
{
  if (!mAudioEnabled)
  {
    return;
  }

  // when already listening then stop listening
  if (mpListener)
  {
    delete mpListener;
  }

  if (dev >= 0)  // device busy (playing)
  {
    return;
  }
  mpListener = new JZAudioListener(this, spl, fr_smpl, to_smpl);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAudioPlayer::GetListenerPlayPosition()
{
  if (!mpListener)
  {
    return -1L;
  }
  return mpListener->GetPlayPosition();
}
