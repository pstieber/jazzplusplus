//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2015 Peter J. Stieber
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

#include "Player.h"

#include "ProjectManager.h"
#include "Synth.h"
#include "TrackFrame.h"
#include "TrackWindow.h"
#include "EventWindow.h"
#include "Dialogs.h"
#include "Audio.h"
#include "Globals.h"

#include <wx/choicdlg.h>
#include <wx/msgdlg.h>

//#include <unistd.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <errno.h>

#ifndef _MSC_VER
#include <sys/ioctl.h>
#endif

#define CLOCK_TO_HOST_TICKS 15  // midinetd sends clock
                                // information every 15'th
                                // tick

#define FIRST_DELTACLOCK 720    // First time send events up to
                                // 720 ticks ahead in time
                                // (often 6 quarter
                                // notes)

#define DELTACLOCK 960          // Later send events up to
                                // 960 ticks ahead in time
                                // (often 8 quarter
                                // notes)

#define ADVANCE_PLAY 480        // Send more events to midinetd
                                // 480 ticks in time ahead of last
                                // written events (often 4
                                // quarternotes, which is also
                                // often one bar).

char *midinethost = NULL;
char *midinetservice = NULL;


#include <fcntl.h>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZDeviceList::JZDeviceList()
  : mDeviceNames()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZDeviceList::~JZDeviceList()
{
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPlayLoop::JZPlayLoop()
  : mStartClock(0),
    mStopClock(0)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayLoop::Reset()
{
  mStartClock = mStopClock = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayLoop::Set(int Start, int Stop)
{
  mStartClock = Start;
  mStopClock = Stop;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPlayLoop::Ext2IntClock(int Clock)
{
  if (mStopClock)
  {
    return (Clock - mStartClock) % (mStopClock - mStartClock) + mStartClock;
  }
  return Clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPlayLoop::Int2ExtClock(int Clock)
{
  return Clock;
}

//-----------------------------------------------------------------------------
// Description:
//   Copy events from the passed song to output buffer.
//-----------------------------------------------------------------------------
void JZPlayLoop::PrepareOutput(
  JZEventArray* pEventArray,
  JZSong* pSong,
  int ExtFr,
  int ExtTo,
  bool AudioMode)
{
  if (pEventArray == 0)
  {
    return;
  }

  int From = Ext2IntClock(ExtFr);
  int Delta = ExtFr - From;
  int Size = ExtTo - ExtFr;
  while (mStopClock && From + Size > mStopClock)
  {
    pSong->MergeTracks(
      From,
      mStopClock,
      pEventArray,
      gpProject->GetMetronomeInfo(),
      Delta,
      AudioMode);

    Size  -= mStopClock - From;
    From   = mStartClock;
    Delta += mStopClock - mStartClock;
  }

  if (Size > 0)
  {
    pSong->MergeTracks(
      From,
      From + Size,
      pEventArray,
      gpProject->GetMetronomeInfo(),
      Delta,
      AudioMode);
  }
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPlayer::JZPlayer(JZSong* pSong)
  : mOutClock(0),
    mpPlayLoop(0),
    mPollMillisec(200),
    mpRecordingInfo(0),
    mPlaying(false),
    mpSong(pSong),
    mpAudioBuffer(0),
    mSamples(pSong->GetTicksPerQuarter() * pSong->Speed())
{
  DummyDeviceList.Add("default");
  mpPlayLoop = new JZPlayLoop();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPlayer::~JZPlayer()
{
  delete mpPlayLoop;
  mPlayBuffer.Clear();
  mRecdBuffer.Clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::Notify()
{
  // called by timer
  int Now = GetRealTimeClock();

#ifdef DEBUG_PLAYER_NOTIFY
  cout << "JZPlayer::Notify " << Now << endl;
#endif // DEBUG_PLAYER_NOTIFY
  if (Now < 0)
  {
    return;
  }

  // time to put more events
  if (Now >= (mOutClock - ADVANCE_PLAY))
  {
#ifdef DEBUG_PLAYER_NOTIFY
    cout << "*** Notify: more events to playbuffer" << endl;
#endif // DEBUG_PLAYER_NOTIFY

    mpPlayLoop->PrepareOutput(
      &mPlayBuffer,
      mpSong,
      mOutClock,
      Now + DELTACLOCK,
      false);
    if (mpAudioBuffer)
    {
      mpPlayLoop->PrepareOutput(
        mpAudioBuffer,
        mpSong,
        mOutClock,
        Now + DELTACLOCK,
        true);
    }
    mOutClock = Now + DELTACLOCK;
    mPlayBuffer.Length2Keyoff();
  }

  // optimization:
  //
  // if (there are some events to be played)
  //   send them to driver
  // else
  //   tell the driver that there is nothing to do at the moment
  if (mPlayBuffer.mEventCount && mPlayBuffer.mppEvents[0]->GetClock() < mOutClock)
  {
    FlushToDevice();
  }
  else
  {
    // Does nothing unless mOutClock has changed.
    OutBreak();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Try to send all events up to mOutClock to device.
//-----------------------------------------------------------------------------
void JZPlayer::FlushToDevice()
{
  int BufferFull = 0;

  JZEventIterator Iterator(&mPlayBuffer);
  JZEvent* pEvent = Iterator.Range(0, mOutClock);
  while (!BufferFull && pEvent)
  {
    if (OutEvent(pEvent) != 0)
      BufferFull = 1;
    else
    {
      pEvent->Kill();
      pEvent = Iterator.Next();
    }
  }

  if (!BufferFull)
    OutBreak();
  mPlayBuffer.Cleanup(0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::OutNow(JZTrack* pTrack, JZParam *r)
{
  OutNow(pTrack, &r->mMsb);
  OutNow(pTrack, &r->mLsb);
  OutNow(pTrack, &r->mDataMsb);
  OutNow(pTrack, &r->mResetMsb);
  OutNow(pTrack, &r->mResetLsb);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::StartPlay(int Clock, int LoopClock, int Continue)
{
#ifdef DEBUG_PLAYER_STARTPLAY
  cout << "JZPlayer::StartPlay" << endl;
#endif // DEBUG_PLAYER_STARTPLAY

  int i;

  if (LoopClock > 0)
  {
    mpPlayLoop->Set(Clock, LoopClock);
  }
  else
  {
    mpPlayLoop->Reset();
  }

  Clock = mpPlayLoop->Int2ExtClock(Clock);
  mPlayBuffer.Clear();
  mRecdBuffer.Clear();
  if (mpAudioBuffer)
  {
    mpAudioBuffer->Clear();
  }

  JZTrack* pTrack;

  if (!Continue)
  {
    if (
      gpConfig->GetValue(C_SendSynthReset) == 2 ||
      ((Clock == 0) && (gpConfig->GetValue(C_SendSynthReset) == 1)))
    {
      // fixme: we should have different synths for each device
      pTrack = mpSong->GetTrack(0);
      JZEvent* mpResetEvent = gpSynth->CreateResetEvent();
      OutNow(pTrack, mpResetEvent);
      delete mpResetEvent;
    }

    // Send Volume, Pan, Chorus, etc
    for (i = 0; i < mpSong->GetTrackCount(); ++i)
    {
      pTrack = mpSong->GetTrack(i);
      if (pTrack->mpBank)
      {
        OutNow(pTrack, pTrack->mpBank);
      }
      if (pTrack->mpBank2)
      {
        OutNow(pTrack, pTrack->mpBank2);
      }
      if (pTrack->mpPatch)
      {
        OutNow(pTrack, pTrack->mpPatch);
      }
      if (pTrack->mpVolume)
      {
        OutNow(pTrack, pTrack->mpVolume);
      }
      if (pTrack->mpPan)
      {
        OutNow(pTrack, pTrack->mpPan);
      }
      if (pTrack->mpReverb)
      {
        OutNow(pTrack, pTrack->mpReverb);
      }
      if (pTrack->mpChorus)
      {
        OutNow(pTrack, pTrack->mpChorus);
      }
      if (pTrack->mpVibRate)
      {
        OutNow(pTrack, pTrack->mpVibRate);
      }
      if (pTrack->mpVibDepth)
      {
        OutNow(pTrack, pTrack->mpVibDepth);
      }
      if (pTrack->mpVibDelay)
      {
        OutNow(pTrack, pTrack->mpVibDelay);
      }
      if (pTrack->mpCutoff)
      {
        OutNow(pTrack, pTrack->mpCutoff);
      }
      if (pTrack->mpResonance)
      {
        OutNow(pTrack, pTrack->mpResonance);
      }
      if (pTrack->mpEnvAttack)
      {
        OutNow(pTrack, pTrack->mpEnvAttack);
      }
      if (pTrack->mpEnvDecay)
      {
        OutNow(pTrack, pTrack->mpEnvDecay);
      }
      if (pTrack->mpEnvRelease)
      {
        OutNow(pTrack, pTrack->mpEnvRelease);
      }

      int j;
      if (!pTrack->mDrumParams.IsEmpty())
      {
        JZDrumInstrumentParameter *dpar = pTrack->mDrumParams.FirstElem();
        while (dpar)
        {
          for (j = drumPitchIndex; j < numDrumParameters; ++j)
          {
            if (dpar->Get(j))
            {
              OutNow(pTrack, dpar->Get(j));
            }
          }
          dpar = pTrack->mDrumParams.NextElem( dpar );
        }
      }

      if (pTrack->mpBendPitchSens)
      {
        OutNow(pTrack, pTrack->mpBendPitchSens);
      }

      for (j = mspModPitchControl; j < mspModulationSysexParameters; ++j)
      {
        if (pTrack->mpModulationSettings[j])
        {
          OutNow(pTrack, pTrack->mpModulationSettings[j]);
        }
      }

      for (j = bspBendPitchControl; j < bspBenderSysexParameters; ++j)
      {
        if (pTrack->mpBenderSettings[j])
        {
          OutNow(pTrack, pTrack->mpBenderSettings[j]);
        }
      }

      for (j = cspCAfPitchControl; j < cspCAfSysexParameters; ++j)
      {
        if (pTrack->mpCAfSettings[j])
        {
          OutNow(pTrack, pTrack->mpCAfSettings[j]);
        }
      }

      for (j = pspPAfPitchControl; j < pspPAfSysexParameters; ++j)
      {
        if (pTrack->mpPAfSettings[j])
        {
          OutNow(pTrack, pTrack->mpPAfSettings[j]);
        }
      }

      for (j = cspCC1PitchControl; j < cspCC1SysexParameters; ++j)
      {
        if (pTrack->mpCC1Settings[j])
        {
          OutNow(pTrack, pTrack->mpCC1Settings[j]);
        }
      }

      for (j = cspCC2PitchControl; j < cspCC2SysexParameters; ++j)
      {
        if (pTrack->mpCC2Settings[j])
        {
          OutNow(pTrack, pTrack->mpCC2Settings[j]);
        }
      }

      if (pTrack->mpCC1ControllerNr)
      {
        OutNow(pTrack, pTrack->mpCC1ControllerNr);
      }

      if (pTrack->mpCC2ControllerNr)
      {
        OutNow(pTrack, pTrack->mpCC2ControllerNr);
      }

      if (gpConfig->GetValue(C_UseReverbMacro))
      {
        if (pTrack->mpReverbType)
        {
          OutNow(pTrack, pTrack->mpReverbType);
        }
      }
      else
      {
        for (j = 0; j < rspReverbSysexParameters; ++j)
        {
          if (pTrack->mpReverbSettings[j])
          {
            OutNow(pTrack, pTrack->mpReverbSettings[j]);
          }
        }
      }
      if (gpConfig->GetValue(C_UseChorusMacro))
      {
        if (pTrack->mpChorusType)
        {
          OutNow(pTrack, pTrack->mpChorusType);
        }
      }
      else
      {
        for (j = 0; j < cspChorusSysexParameters; ++j)
        {
          if (pTrack->mpChorusSettings[j])
          {
            OutNow(pTrack, pTrack->mpChorusSettings[j]);
          }
        }
      }

      if (pTrack->mpEqualizerType)
      {
        OutNow(pTrack, pTrack->mpEqualizerType);
      }
      if (pTrack->mpPartialReserve)
      {
        OutNow(pTrack, pTrack->mpPartialReserve);
      }
      if (pTrack->mpMasterVol)
      {
        OutNow(pTrack, pTrack->mpMasterVol);
      }
      if (pTrack->mpMasterPan)
      {
        OutNow(pTrack, pTrack->mpMasterPan);
      }
      if (pTrack->mpRxChannel)
      {
        OutNow(pTrack, pTrack->mpRxChannel);
      }
      if (pTrack->mpUseForRhythm && *gpSynth->GetSysexValPtr(pTrack->mpUseForRhythm ))
      {
        OutNow(pTrack, pTrack->mpUseForRhythm );
      }
    } // for
  } // if !Continue

  pTrack = mpSong->GetTrack(0);
  JZEvent* pEvent = pTrack->GetCurrentTempo(Clock);
  if (pEvent)
  {
    OutNow(pEvent);
  }

  // Send songpointer?
  if (gpConfig->GetValue(C_RealTimeOut))
  {
    unsigned char s[2];
    s[0] = Clock & 0x7f;
    s[1] = (Clock & 0x3fff) >> 7;
    JZSongPtrEvent SongPtr(0, s, 2);
    OutNow(&SongPtr);
  }

  SetHardThru(
    gpConfig->GetValue(C_HardThru),
    gpConfig->GetValue(C_ThruInput),
    gpConfig->GetValue(C_ThruOutput));

  mOutClock = Clock + FIRST_DELTACLOCK;

  JZProjectManager::Instance().NewPlayPosition(
    mpPlayLoop->Ext2IntClock(Clock));

  mpPlayLoop->PrepareOutput(
    &mPlayBuffer,
    mpSong,
    Clock,
    Clock + FIRST_DELTACLOCK,
    false);

  if (mpAudioBuffer)
  {
    mpPlayLoop->PrepareOutput(
      mpAudioBuffer,
      mpSong,
      Clock,
      Clock + FIRST_DELTACLOCK,
      true);
  }
  mPlayBuffer.Length2Keyoff();

  // Notify() has to be called very often because voxware
  // midi thru is done there
  Start(mPollMillisec);        // start wxTimer

  mPlaying = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::StopPlay()
{
  // Stop the wxTimer.
  Stop();
  mPlaying = false;

  int Clock = GetRealTimeClock();

  // SN++ Patch: Notes off for not GM/GS devices
  int ii;
  JZKeyOffEvent pKeyOff(0, 0, 0);

  for (ii = 0; ii < mpSong->GetTrackCount(); ++ii)
  {
    JZTrack *Track = mpSong->GetTrack(ii);
    if (Track)
    {
      JZEventIterator Iterator(Track);
      JZEvent* pEvent = Iterator.First();
      while (pEvent && pEvent->GetClock() < Clock + 100)
      {
        JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
        if (pKeyOn)
        {
          if (pKeyOn->GetClock() + pKeyOn->GetEventLength() >= Clock - 100)
          {
            pKeyOff.SetChannel(pKeyOn->GetChannel());
            pKeyOff.SetKey(pKeyOn->GetKey());
            OutNow(&pKeyOff);
          }
        }
        pEvent = Iterator.Next();
      }
    }
  }

  JZProjectManager::Instance().NewPlayPosition(-1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::AllNotesOff(bool Reset)
{
  JZControlEvent NoteOff(0, 0, 0x78, 0);
  JZPitchEvent Pitch    (0, 0, 0);
  JZControlEvent CtrlRes(0, 0, 0x79, 0);

  JZDeviceList &devs = gpMidiPlayer->GetOutputDevices();
  for (unsigned dev = 0; dev < devs.GetCount(); ++dev)
  {
    for (int c = 0; c < 16; ++c)
    {
      NoteOff.SetChannel(c);
      NoteOff.SetDevice(dev);
      OutNow(&NoteOff);

      Pitch.SetChannel(c);
      Pitch.SetDevice(dev);
      OutNow(&Pitch);

      CtrlRes.SetChannel(c);
      CtrlRes.SetDevice(dev);
      OutNow(&CtrlRes);
    }
  }

  if (Reset)
  {
    JZEvent* mpResetEvent = gpSynth->CreateResetEvent();
    OutNow(mpResetEvent);
    delete mpResetEvent;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::ShowError()
{
  wxMessageBox("could not install driver", "Error", wxOK);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::EditAudioGlobalSettings(wxWindow* pParent)
{
  mSamples.EditAudioGlobalSettings(pParent);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::EditAudioSamples(wxWindow* pParent)
{
  mSamples.EditAudioSamples(pParent);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::LoadSampleSet(wxWindow* pParent)
{
  mSamples.LoadSampleSet(pParent);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::SaveSampleSetAs(wxWindow* pParent)
{
  mSamples.SaveSampleSetAs(pParent);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::SaveSampleSet(wxWindow* pParent)
{
  mSamples.SaveSampleSet(pParent);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPlayer::ClearSampleSet(wxWindow* pParent)
{
  mSamples.ClearSampleSet(pParent);
}

#ifdef DEV_MPU401

//*****************************************************************************
// MPU-Player
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMpuPlayer::JZMpuPlayer(JZSong* pSong)
  : JZPlayer(pSong)
{
  mPollMillisec = 25;
  midinethost = getenv("MIDINETHOST");
  if (!midinethost || !strlen(midinethost))
  {
    midinethost = "localhost";
  }
  midinetservice = getenv("MIDINETSERVICE");
  if (!midinetservice || !strlen(midinetservice))
  {
    midinetservice = MIDINETSERVICE;
  }
  dev = midinetconnect( midinethost, midinetservice );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMpuPlayer::~JZMpuPlayer()
{
  close(dev);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZMpuPlayer::IsInstalled()
{
  return dev >= 0;
}

#if DB_WRITE
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int dwrite(int dev, const char* buf, int size)
{
  int i, written;
  //dev = 2;        // stderr
  written = write(dev, buf, size);
  if (written < 0)
  {
    printf("writing failed\n");
    return written;
  }
  printf("W: ");
  for (i = 0; i < written; ++i)
  {
    printf("%02x ", (unsigned char)buf[i]);
  }
  putchar('\n');
  if (written != size)
  {
    printf("L: ");
    for (i = written; i < size; ++i)
    {
      printf("%02x ", (unsigned char)buf[i]);
    }
    putchar('\n');
  }
  fflush(stdout);
  return written;
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMpuPlayer::StartPlay(int IntClock, int LoopClock, int Continue)
{
  int ExtClock = mpPlayLoop->Int2ExtClock(IntClock);
  char *play;
  int playsize;

  static char play1[] =
  {
    CMD+1, 0x34,                    /* timing byte always */
    CMD+1, 0x8e,                /* conductor off */
    CMD+1, 0x8c,                /* don'pTrack send measures while recording */
    CMD+1, 0xe7, DAT+1, 60,        /* clock-to-host every 15'th tick (60/4) */
    CMD+1, 0x95,                 /* send clock to host instead */
    CMD+1, 0x87,                  /* pitch+controller enabled */
    CMD+1, 0x98, CMD+1, 0x9a,
    CMD+1, 0x9c, CMD+1, 0x9e,        /* channel-ref-tables off */
    CMD+1, 0xec,
    DAT+1, ACTIVE_TRACKS_MASK,        /* active tracks */
    CMD+1, 0xb8,                /* clear play counters */
    CMD+1, 0x90,                 /* real time affection off */
    CMD+1, 0x2a                  /* start record/play */
  };

  static char play2[] =
  {
    CMD+1, 0x38,                  /* common to host */
    CMD+1, 0x39,                  /* real time to host */
    CMD+1, 0x34,                    /* timing byte always */
    CMD+1, 0x8e,                /* conductor off */
    CMD+1, 0x8c,                /* don'pTrack send measures while recording */
    CMD+1, 0xe7, DAT+1, 60,        /* clock-to-host every 15'th tick (60/4) */
    CMD+1, 0x95,                 /* send clock to host instead */
    CMD+1, 0x87,                  /* pitch+controller enabled */
    CMD+1, 0x98, CMD+1, 0x9a,
    CMD+1, 0x9c, CMD+1, 0x9e,        /* channel-ref-tables off */
    CMD+1, 0xec,
    DAT+1, ACTIVE_TRACKS_MASK,        /* active tracks */
    CMD+1, 0xb8,                /* clear play counters */
    CMD+1, 0x90,                 /* real time affection off (yes!) */
    CMD+1, 0x2a                  /* stand by record */
  };

  PlyBytes.Clear();

  RecBytes.Clear();
  RecBytes.Clock = ExtClock;

  playclock = ExtClock;
  clock_to_host_counter = 0;

  ActiveTrack = 0;
  for (int i = 0; i < ACTIVE_TRACKS; ++i)
  {
    TrackClock[i] = ExtClock;
    TrackRunningStatus[i] = 0;
  }

  // Setup Timebase
  char timebase[2];
  timebase[0] = CMD+1;
  switch (mpSong->GetTicksPerQuarter())
  {
    case  48: timebase[1] = 0xc2; break;
    case  72: timebase[1] = 0xc3; break;
    case  96: timebase[1] = 0xc4; break;
    case 120: timebase[1] = 0xc5; break;
    case 144: timebase[1] = 0xc6; break;
    case 168: timebase[1] = 0xc7; break;
    case 192: timebase[1] = 0xc8; break;
    default : timebase[1] = 0xc5; break;
  }
  write_ack_mpu(timebase, 2);

  OutOfBandEvents.Clear();
  JZPlayer::StartPlay(IntClock, LoopClock, Continue);

  // Supress realtime messages to MIDI Out port?
  if (!Config(C_RealTimeOut))
  {
    char realtime[2];
    realtime[0] = CMD + 1;
    realtime[1] = 0x32;
    write_ack_mpu(realtime, 2);
  }

  // What is the clock source ?
  char clocksource[2];
  clocksource[0] = CMD+1;
  switch (Config(C_ClockSource))
  {
    case CsInt:
      clocksource[1] = 0x80;
      play = play1;
      playsize = sizeof( play1 );
      break;
    case CsFsk:
      clocksource[1] = 0x81;
      play = play1;
      playsize = sizeof( play1 );
      break;
    case CsMidi:
      clocksource[1] = 0x82;
      play = play2;
      playsize = sizeof( play2 );
      break;
    default:
      clocksource[1] = 0x80;
      play = play1;
      playsize = sizeof( play1 );
      break;
  }
  write_ack_mpu(clocksource, 2);

  JZPlayer::Notify();

  // Start play
  write_ack_mpu(play, playsize);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMpuPlayer::StopPlay()
{
  static const char stop = RES;
  JZPlayer::StopPlay();
  // Reset mpu
  write_ack_mpu( &stop, 1);
  PlyBytes.Clear();
  AllNotesOff();

  // Get record buffer
  GetRecordedData();
  mRecdBuffer.Keyoff2Length();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMpuPlayer::SetHardThru(int on, int idummy, int odummy)
{
  char midithru[2];
  midithru[0] = CMD+1;
  if (on)
  {
    midithru[1] = 0x89;
  }
  else
  {
    midithru[1] = 0x88;
  }
  write_ack_mpu( midithru, 2 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMpuPlayer::OutEvent(JZEvent* pEvent)
{
  if (!PlyBytes.WriteFile(dev))
  {
    return 1;        // buffer full
  }

  int Stat = pEvent->GetStat();

  switch (Stat)
  {
    case StatKeyOff:
    case StatKeyOn:
    case StatKeyPressure:
    case StatControl:
    case StatProgram:
    case StatChnPressure:
    case StatPitch:
    {
      JZGetMidiBytes midi;
      int i;
      JZChannelEvent *c;

      pEvent->Write(midi);
      Stat = midi.mBuffer[0]; // Status + Channel

      OutBreak(pEvent->GetClock());

      if ((c = pEvent->IsChannelEvent()) != 0)
      {
        switch (c->Channel)
        {
          case 0:
          case 3:
            ActiveTrack = 5;
            break;
          case 1:
          case 4:
            ActiveTrack = 4;
            break;
          case 2:
          case 5:
            ActiveTrack = 3;
            break;
          case 6:
          case 10:
          case 13:
            ActiveTrack = 2;
            break;
          case 9:
            ActiveTrack = 6;
            break;
          case 7:
          case 11:
          case 14:
            ActiveTrack = 1;
            break;
          case 8:
          case 12:
          case 15:
            ActiveTrack = 0;
            break;
          default:
            ActiveTrack = 6;
            break;
        }
      }
      else
      {
        // Not channel event => play on track #6
        ActiveTrack = 6;
      }

      int Time = pEvent->GetClock() - TrackClock[ActiveTrack];
      assert(Time < 240);

      if (Stat != TrackRunningStatus[ActiveTrack])
      {
        PlyBytes.Put(TRK + midi.mByteCount + 1 + 1);
        PlyBytes.Put(ActiveTrack);
        PlyBytes.Put(Time);
        PlyBytes.Put(Stat);
        TrackRunningStatus[ActiveTrack] = Stat;
      }
      else
      {
        PlyBytes.Put(TRK + midi.mByteCount + 1);
        PlyBytes.Put(ActiveTrack);
        PlyBytes.Put(Time);
      }
      for (i = 1; i < midi.mByteCount; ++i)
        PlyBytes.Put(midi.mBuffer[i]);

      TrackClock[ActiveTrack] = pEvent->GetClock();
      return 0;
    }

    case StatSetTempo:
    case StatSysEx:
    {
      if (pEvent->GetClock() > 0)
      {
        OutOfBandEvents.Put(pEvent->Copy());
      }
      return 0;
    }
   default:        // Meterchange etc
    return 0;
    break;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMpuPlayer::OutBreak()
{
  // Send a break to the driver starting at PlyBytes.GetClock() and ending at
  // mOutClock.
  if (!PlyBytes.WriteFile(dev))
  {
    return;
  }

  (void)OutBreak(mOutClock);
  PlyBytes.WriteFile(dev);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMpuPlayer::OutBreak(int BreakOver)
{
  int OverFlow = 1;

  while (OverFlow)
  {
    OverFlow = 0;
    for (int i = 0; i < ACTIVE_TRACKS; ++i)
    {
      if ((BreakOver - TrackClock[i]) >= 240)
      {
        PlyBytes.Put(TRK + 1 + 1);
        PlyBytes.Put(i);
        PlyBytes.Put(0xf8);
        TrackClock[i] += 240;
        OverFlow = 1;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMpuPlayer::OutNow(JZEvent* pEvent)
{
  // send event to driver immediately regardless of events remaining
  // in the play-queue.

  int i, n = 0;
  JZGetMidiBytes midi;
  if (pEvent->Write(midi) == 0)
  {
    char* buf = new char[midi.mByteCount + 3];
    buf[n++] = CMD+1;
    buf[n++] = 0xd7;
    buf[n++] = DAT + midi.mByteCount;
    for (i = 0; i < midi.mByteCount; ++i)
    {
      buf[n++] = midi.mBuffer[i];
    }
    write_noack_mpu(buf, n);
    delete[] buf;
  }
  else        // special event
  {
    switch (pEvent->GetStat())
    {
      case StatSetTempo:
        {
          char cmd[4];
          JZSetTempoEvent* s = (JZSetTempoEvent *)pEvent;
          int bpm = s->GetBPM();
          cmd[0] = CMD+1;
          cmd[1] = 0xE0;
          cmd[2] = DAT+1;
          cmd[3] = (char)bpm;
          write_noack_mpu(cmd, 4);
        }
        break;
      case StatSysEx:
        {
          n = 0;
          JZSysExEvent* s = (JZSysExEvent *) pEvent;
          char *sysex = new char[s->Length+4];
          sysex[n++] = CMD+1;
          sysex[n++] = 0xdf;
          sysex[n++] = DAT + s->Length + 1;
          sysex[n++] = StatSysEx;
          for (i = 0; i < s->Length; ++i)
          {
            sysex[n++] = s->Data[i];
          }
          write_noack_mpu(sysex, n);
          delete[] sysex;
        }
      case StatSongPtr:
        {
          n = 0;
          JZSongPtrEvent *s = (JZSongPtrEvent *) pEvent;
          char *common = new char[s->Length+4];
          common[n++] = CMD+1;
          common[n++] = 0xdf;
          common[n++] = DAT + s->Length + 1;
          common[n++] = StatSongPtr;
          for (i = 0; i < s->Length; ++i)
          {
            common[n++] = s->Data[i];
          }
          write_noack_mpu(common, n);
          delete[] common;
        }
        break;

      default:        // ignore others
        break;
    }
  }
}

//-----------------------------------------------------------------------------
// try to send all out of band events up to Clock to device
//-----------------------------------------------------------------------------
void JZMpuPlayer::FlushOutOfBand(int Clock)
{
  JZEventIterator Iterator(&OutOfBandEvents);
  JZEvent* pEvent = Iterator.Range(0, Clock);
  while (pEvent)
  {
    switch (pEvent->GetStat())
    {
      case StatSetTempo:
        {
          char cmd[4];
          JZSetTempoEvent *s = (JZSetTempoEvent *)pEvent;
          int bpm = s->GetBPM();
          cmd[0] = CMD+1;
          cmd[1] = 0xE0;
          cmd[2] = DAT+1;
          cmd[3] = (char)bpm;
          write_noack_mpu(cmd, 4);
        }
        break;
      case StatSysEx:
        {
           int n = 0;
           JZSysExEvent* s = (JZSysExEvent *) pEvent;
           char *sysex = new char[s->Length+4];
           sysex[n++] = CMD+1;
           sysex[n++] = 0xdf;
           sysex[n++] = DAT + s->Length + 1;
           sysex[n++] = StatSysEx;
           for (int i = 0; i < s->Length; ++i)
           {
              sysex[n++] = s->Data[i];
           }
           write_noack_mpu(sysex, n);
           delete[] sysex;
        }
      default:
        break;
    }
    pEvent->Kill();
    pEvent = Iterator.Next();
  }

  OutOfBandEvents.Cleanup(0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMpuPlayer::GetRealTimeClock()
{
  static int receiving_song_ptr = 0;
  static int d0, d1;
  int c;
  while ((c = RecBytes.Get(dev)) >= 0)
  {
    // The midinetd sends 0xfd to jazz every 15'th tick
    if (c == 0xfd)
    {
      // CLOCK_TO_HOST received
      playclock += CLOCK_TO_HOST_TICKS;
      ++clock_to_host_counter;
#ifdef SLOW_MACHINE
      // Update screen every 4 beats (120 ticks/beat).
      if ((clock_to_host_counter % 32) == 0)
      {
        JZProjectManager::Instance().NewPlayPosition(
          mpPlayLoop->Ext2IntClock(playclock));
      }
#else
      // Update screen every 8'th note (120 ticks/beat).
      if ((clock_to_host_counter % 4) == 0)
      {
        JZProjectManager::Instance().NewPlayPosition(
          mpPlayLoop->Ext2IntClock(playclock));
      }
#endif
      FlushOutOfBand(playclock);
    }
    else if (c == 0xfa)
    {
      // Start play received
    }
    else if (c == 0xfb)
    {
      // Continue play received
    }
    else if (c == 0xfc)
    {
      // Stop play received
      AllNotesOff();
      return -1;
    }
    else if (c == 0xf2 || receiving_song_ptr)
    {
      // Song pointer received
      ++receiving_song_ptr;

      int ExtClock;

      switch (receiving_song_ptr)
      {
        case 1:
          break;
        case 2:
          d0 = c;
          break;
        case 3:
          gpMidiPlayer->StopPlay();
          d1 = c;
          ExtClock = (d0 + (128 * d1)) * (mpSong->GetTicksPerQuarter() / 4);
          receiving_song_ptr = 0;
          d0 = d1 = 0;
          gpMidiPlayer->StartPlay(ExtClock, 0, 1);
          return -1;
        default:
          receiving_song_ptr = 0;
          d0 = d1 = 0;
      }
    }
  }
  return playclock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMpuPlayer::GetRecordedData()
{
  int c, i;
  unsigned char *recbuf = NULL;
  int numbytes = 0;

  numbytes = get_recbuf_mpu( &recbuf );
  if (numbytes == 0)
  {
    // No bytes in record buffer
    return 0;
  }

  // Go through the record buffer and create events
  i = 0;
  while (i < numbytes)
  {
    c = recbuf[i++];
    if (c == 0xf8)
    {
      RecBytes.Clock += 240;
    }
    else if (c < 0xf0)
    {
      // timing byte
      RecBytes.Clock += c;
      c = recbuf[i++];
      if (c < 0xf0)  // Voice-Message
      {
        unsigned char c1, c2;
        int Channel;
        JZEvent* pEvent = 0;

        if (c & 0x80)
        {
          RecBytes.RunningStatus = c;
          c1 = recbuf[i++];
        }
        else
          c1 = c;

        Channel = RecBytes.RunningStatus & 0x0f;
        switch (RecBytes.RunningStatus & 0xf0)
        {
          case StatKeyOff:
            c2 = recbuf[i++];  // SN++ added veloc
            pEvent = new JZKeyOffEvent(RecBytes.Clock, Channel, c1, c2);
            pEvent = new JZKeyOffEvent(RecBytes.Clock, Channel, c1);
            break;

          case StatKeyOn:
            c2 = recbuf[i++];
            if (!c2)
            {
              pEvent = new JZKeyOffEvent(RecBytes.Clock, Channel, c1);
            }
            else
            {
              pEvent = new JZKeyOnEvent(RecBytes.Clock, Channel, c1, c2);
            }
            break;
// #if 0
          case StatKeyPressure:
            c2 = recbuf[i++];
            pEvent = new JZKeyPressureEvent(RecBytes.Clock, Channel, c1, c2);
            break;

          case StatChnPressure:
            pEvent = new JZChnPressureEvent(RecBytes.Clock, Channel, c1);
            break;

          case StatControl:
            c2 = recbuf[i++];
            pEvent = new JZControlEvent(RecBytes.Clock, Channel, c1, c2);
            break;

          case StatProgram:
            pEvent = new JZProgramEvent(RecBytes.Clock, Channel, c1);
            break;

          case StatPitch:
            c2 = recbuf[i++];
            pEvent = new JZPitchEvent(RecBytes.Clock, Channel, c1, c2);
            break;

          default:
            printf("unrecognized MIDI Status %02x, %02x", (unsigned char)RecBytes.RunningStatus, c1);
            break;

        }
        if (pEvent)
        {
          pEvent->Clock = mpPlayLoop->Ext2IntClock(pEvent->Clock);
          mRecdBuffer.Put(pEvent);
        }
      }
      else if (c == 0xfc)
      {
        // Data end mark
      }
      else
        printf("unrecognized Status after Timing Byte: %02x\n", c);
    }
    else
      printf("unrecognized Status (no timing byte): %02x\n", c);
  }
  if (recbuf) free( recbuf );
  return( numbytes );
}

#endif // DEV_MPU401


#ifdef DEV_SEQUENCER2

//*****************************************************************************
// /dev/sequencer2
//*****************************************************************************

// SN-- SEQ_DEFINEBUF (32768);
#define sequencer_buffer     65536
SEQ_DEFINEBUF (sequencer_buffer);  // 64K

int seqfd   = -1;
int mididev = -1;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void seqbuf_dump(void)
{
  if (_seqbufptr)
  {
    int size;
    size = write (seqfd, _seqbuf, _seqbufptr);
    if (size == -1 && errno == EAGAIN)
    {
      cout << "write /dev/sequencer2: EAGAIN" << endl;
      return;
    }

    if (size < 0)
    {
      perror("write /dev/sequencer2");
      exit(-1);
    }

    _seqbufptr -= size;
    if (_seqbufptr)
      memmove(_seqbuf, _seqbuf + size, _seqbufptr);
  }
}

#define seqbuf_empty()  (_seqbufptr == 0)
#define seqbuf_clear()  (_seqbufptr = 0)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void seqbuf_flush_last_event()
{
  _seqbufptr -= 8;
  if (ioctl(seqfd, SNDCTL_SEQ_OUTOFBAND, _seqbuf + _seqbufptr) < 0)
    perror("ioctl flush_last");
}

//*****************************************************************************
// voxware midi through
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZOssThru::JZOssThru()
{
  int time_base = 120;
  int tempo     = 120;
  seqbuf_clear();
  ioctl(seqfd, SNDCTL_SEQ_RESET, 0);
  ioctl(seqfd, SNDCTL_TMR_TIMEBASE, &time_base);
  ioctl(seqfd, SNDCTL_TMR_TEMPO, &tempo);
  SEQ_START_TIMER();
  // 16-th at 120 bpm = 31 ms / beat
  Start(5);        // poll every 5 ms
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZOssThru::~JZOssThru()
{
  Stop();
  SEQ_STOP_TIMER();
  seqbuf_dump();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZOssThru::Notify()
{
  unsigned char buf[128];
  int size;
  //SEQ_ECHO_BACK(0);
  while ((size = read(seqfd, buf, sizeof(buf))) > 0)
  {
    int i = 0;
    while (i < size)
    {
      if (buf[i] == EV_CHN_COMMON || buf[i] == EV_CHN_VOICE)
      {
        buf[i+1] = mididev;
        if (ioctl(seqfd, SNDCTL_SEQ_OUTOFBAND, &buf[i]) < 0)
           perror("ioctl outofband");
      }
      i += 8;
    }
  }

  if (_seqbufptr)
  {
    seqbuf_dump();
  }
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSeq2Player::JZSeq2Player(JZSong* pSong)
  : JZPlayer(pSong)
{
  // The code has to poll fast for MIDI thru.
  mPollMillisec = 10;

  recd_clock   = 0;
  play_clock   = 0;
  echo_clock   = 0;
  start_clock  = 0;

  through      = 0;

  seqfd = open("/dev/sequencer2", O_RDWR | O_NDELAY);
  if (seqfd < 0)
    seqfd = open("/dev/music", O_RDWR | O_NDELAY);
  if (seqfd < 0)
  {
    perror("/dev/sequencer2 or /dev/music");
    return;
  }
  // from .jazz
  mididev = gpConfig->GetValue(C_Seq2Device);
  if (mididev < 0)
  {
    mididev = FindMidiDevice();
    gpConfig->Put(C_Seq2Device, mididev);

    if (mididev < 0)
    {
      return;  // IsInstalled() == FALSE
    }
  }

  synth_info sinfo;

  sinfo.device = mididev;
  if (ioctl(seqfd, SNDCTL_SYNTH_INFO, &sinfo) < 0)
  {
    perror("sndctl_synth_info");
  }

  card_id = -1;
  if (!strncmp( "MPU-401", sinfo.name, 7))
  {
    card_id = SNDCARD_MPU401;
  }

  if (gpConfig->GetValue(C_SoftThru))
  {
    through = new JZOssThru();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSeq2Player::IsInstalled()
{
  return seqfd >= 0 && mididev >= 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSeq2Player::~JZSeq2Player()
{
  delete through;
   if (seqfd > 0)
     close(seqfd);
   seqfd = -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSeq2Player::FindMidiDevice()
{
  struct synth_info si;
  int i, nrsynths, ninp;

  if (ioctl(seqfd, SNDCTL_SEQ_NRSYNTHS, &nrsynths) == -1)
  {
    perror("SNDCTL_SEQ_NRSYNTHS");
    return -1;
  }

  wxString *devs = new wxString[nrsynths];

  ninp = 0;
  for (i = 0; i < nrsynths; ++i)
  {
    si.device = i;
    if (ioctl(seqfd, SNDCTL_SYNTH_INFO, &si) == -1)
    {
      perror("SNDCTL_SYNTH_INFO");
      return -1;
    }
 //    if (si.synth_type == SYNTH_TYPE_MIDI || si.synth_type == SYNTH_TYPE_SAMPLE)
    {
      devs[ninp] = si.name;
      ++ninp;
    }
  }

  if (ninp > 0)
  {
    const char* pTitle = "MIDI Device";
    wxSingleChoiceDialog *dialog = new wxSingleChoiceDialog(
      gpTrackWindow,
      pTitle,
      pTitle,
      ninp,
      devs);

    if (mididev != -1)
    {
      dialog->SetSelection(mididev);
    }

    int res = dialog->ShowModal();
    int k = dialog->GetSelection();
    dialog->Destroy();

    delete [] devs;

    if (res == wxCANCEL)
    {
      k = -1;
    }

    return k;
  }
  else
  {
    delete [] devs;
    cerr << "no midi device found!\n";
    return -1;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSeq2Player::SetSoftThru(int on, int idummy, int odummy)
{
  gpConfig->Put(C_SoftThru, on);
  if (on)
  {
    if (!through)
    {
      through = new JZOssThru();
    }
  }
  else
  {
    if (through)
    {
      delete through;
    }
    through = 0;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSeq2Player::OutEvent(JZEvent* pEvent, int now)
{
  if (!now)
  {
    OutBreak(pEvent->GetClock());
  }

  int Stat = pEvent->GetStat();
  switch (Stat)
  {
    case StatKeyOn:
      {
        JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
        SEQ_START_NOTE(
          mididev,
          pKeyOn->GetChannel(),
          pKeyOn->GetKey(),
          pKeyOn->GetVelocity());
        if (now)
        {
          seqbuf_flush_last_event();
        }
      }
      break;

    case StatKeyOff:
      {
        JZKeyOffEvent* pKeyOff = pEvent->IsKeyOff();
        SEQ_STOP_NOTE(
          mididev,
          pKeyOff->GetChannel(),
          pKeyOff->GetKey(),
          pKeyOff->GetOffVelocity());
        if (now)
        {
          seqbuf_flush_last_event();
        }
      }
      break;
    case StatProgram:
      {
        JZProgramEvent* pProgram = pEvent->IsProgram();
        SEQ_SET_PATCH(mididev, pProgram->GetChannel(), pProgram->GetProgram());
        if (now)
        {
          seqbuf_flush_last_event();
        }
      }
      break;

// SN++ Aftertouch
    case StatKeyPressure:
      {
         JZKeyPressureEvent* pKeyPressure = pEvent->IsKeyPressure();

         SEQ_KEY_PRESSURE(
           mididev,
           pKeyPressure->GetChannel(),
           pKeyPressure->GetKey(),
           pKeyPressure->GetPressureValue());

         if (now)
         {
           seqbuf_flush_last_event();
         }
      }
      break;
//

    case StatChnPressure:
      {
         JZChnPressureEvent *k = pEvent->IsChnPressure();
         SEQ_CHN_PRESSURE(mididev, k->GetChannel(), k->Value);
         if (now)
         {
           seqbuf_flush_last_event();
         }
      }
      break;

    case StatControl:
      {
        JZControlEvent* k = pEvent->IsControl();
        SEQ_CONTROL(mididev, k->GetChannel(), k->GetControl(), k->GetControlValue());
        if (now)
        {
          seqbuf_flush_last_event();
        }
      }
      break;

    case StatPitch:
      {
        JZPitchEvent *k = pEvent->IsPitch();
        SEQ_BENDER(mididev, k->GetChannel(), k->Value + 8192);
        if (now)
        {
          seqbuf_flush_last_event();
        }
      }
      break;

    case StatSetTempo:
      {
        int bpm = pEvent->IsSetTempo()->GetBPM();
        if (now)
        {
          if (ioctl(seqfd, SNDCTL_TMR_TEMPO, &bpm) < 0)
            perror("sndctl_tmr_tempo");
        }
        else
        {
          if (!GetAudioEnabled())
            if (pEvent->GetClock() > 0)
              SEQ_SET_TEMPO(bpm);
        }
      }
      break;

    case StatSysEx:
      {
         if (now)
         {
            // todo
            JZSysExEvent* s = pEvent->IsSysEx();
            struct sysex_info *sysex = (struct sysex_info *)new char [
              sizeof(struct sysex_info) + s->GetDataLength() + 1];

            sysex->key = SYSEX_PATCH;
            sysex->device_no = mididev;
            sysex->len = s->GetDataLength() + 1;
            sysex->data[0] = 0xf0;
            memcpy(sysex->data + 1, s->GetData(), s->GetDataLength());
            SEQ_WRPATCH(sysex, sizeof(*sysex) + sysex->len - 1);

            delete [] (char *)sysex;
         }
         else if (pEvent->GetClock() > 0)
         {
           // OSS wants small packets with max 6 bytes
           JZSysExEvent* sx = pEvent->IsSysEx();
           const unsigned char* pData = pEvent->IsSysEx()->GetData();
           const int N = 6;
           int i, j;
           char buf[N];
           buf[0] = (char)0xf0;
           i = 1;
           for (j = 0; j < sx->GetDataLength(); ++j)
           {
             if (i == N)
             {
               SEQ_SYSEX(mididev, (unsigned char *)buf, N);
               i = 0;
             }
             buf[i++] = pData[j];
           }
           if (i > 0)
           {
             SEQ_SYSEX(mididev, (unsigned char *)buf, i);
           }
         }
      }
      break;
    case StatTimeSignat:
    default:
      break;
  }

  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSeq2Player::OutBreak(int clock)
{
  if (play_clock < clock)
  {
    while (echo_clock + 48 < clock)
    {
      echo_clock += 48;
      cout << "<< echo " << echo_clock - start_clock << endl;
      SEQ_WAIT_TIME(echo_clock - start_clock);
      SEQ_ECHO_BACK(echo_clock - start_clock);
    }
    if (echo_clock < clock)
    {
      cout << "<< wait: " << clock - start_clock << endl;
      SEQ_WAIT_TIME(clock - start_clock);
    }
  }
  play_clock = clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSeq2Player::OutBreak()
{
  OutBreak(mOutClock);
  seqbuf_dump();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSeq2Player::StartPlay(int Clock, int LoopClock, int Continue)
{
  char buf[512];
  cout<<"JZSeq2Player::StartPlay"<<endl;
  if (through)
  {
    delete through;
  }
  through = 0;
  if (ioctl(seqfd, SNDCTL_SEQ_RESET, 0) < 0)
  {
    perror("ioctl reset");
  }

  if (card_id == SNDCARD_MPU401)
  {
    int timer_mode;
    switch (gpConfig->GetValue(C_ClockSource))
    {
      case CsInt:
        timer_mode = TMR_INTERNAL;
        break;
      case CsFsk:
        timer_mode = TMR_EXTERNAL | TMR_MODE_FSK;
        break;
      case CsMidi:
        timer_mode = TMR_EXTERNAL | TMR_MODE_MIDI;
        break;
      default:
        timer_mode = TMR_INTERNAL;
        break;
    }

    if (ioctl(seqfd, SNDCTL_TMR_SOURCE, &timer_mode) < 0)
    {
      perror("ioctl tmr_source");
    }
  }

  seqbuf_clear();

  // empty queue
  while (read(seqfd, buf, sizeof(buf)) > 0)
    ;

  start_clock  = Clock;
  echo_clock   = Clock;
  play_clock   = Clock;
  recd_clock   = Clock;

  JZProjectManager::Instance().NewPlayPosition(
    mpPlayLoop->Ext2IntClock(Clock));

  // send initial program changes, controller etc
  SEQ_START_TIMER();
  seqbuf_dump();
  JZPlayer::StartPlay(Clock, LoopClock, Continue);
  seqbuf_dump();
  ioctl(seqfd, SNDCTL_SEQ_SYNC);
  SEQ_STOP_TIMER();
  seqbuf_dump();

  // setup timebase and current speed
  int time_base = mpSong->GetTicksPerQuarter();
  int cur_speed = mpSong->GetTrack(0)->GetCurrentSpeed(Clock);
  if (ioctl(seqfd, SNDCTL_TMR_TIMEBASE, &time_base) < 0)
    perror("ioctl time_base");
  if (ioctl(seqfd, SNDCTL_TMR_TEMPO, &cur_speed) < 0)
    perror("ioctl speed");

  // start play
  SEQ_START_TIMER();
  StartAudio();
  JZPlayer::Notify();
  seqbuf_dump();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSeq2Player::StopPlay()
{
  seqbuf_clear();
  SEQ_STOP_TIMER();
  seqbuf_dump();
  ioctl(seqfd, SNDCTL_SEQ_RESET, 0);

  SEQ_START_TIMER();
  JZPlayer::StopPlay();
  AllNotesOff();
  SEQ_STOP_TIMER();
  seqbuf_dump();
  ioctl(seqfd, SNDCTL_SEQ_SYNC);
  ioctl(seqfd, SNDCTL_SEQ_RESET, 0);

  if (gpConfig->GetValue(C_SoftThru))
  {
    through = new JZOssThru();
  }
  JZProjectManager::Instance().NewPlayPosition(-1);
  mRecdBuffer.Keyoff2Length();
}

//-----------------------------------------------------------------------------
// try to send all events up to mOutClock to device
//-----------------------------------------------------------------------------
void JZSeq2Player::FlushToDevice()
{
  JZEventIterator Iterator(&mPlayBuffer);
  JZEvent* pEvent = Iterator.Range(0, mOutClock);
  if (pEvent)
  {
    do
    {
      OutEvent(pEvent);
      pEvent->Kill();
      pEvent = Iterator.Next();
    } while (pEvent);

    mPlayBuffer.Cleanup(0);
  }
  OutBreak(mOutClock);
  seqbuf_dump();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSeq2Player::GetRealTimeClock()
{
  unsigned char buf[256];
  int  size;
  while ((size = read(seqfd, buf, sizeof(buf))) > 0)
  {
    int i = 0;
    while (i < size)
    {
      JZEvent* pEvent = 0;
      switch (buf[i])
      {

        case EV_TIMING:
          switch (buf[i+1])
          {
            case TMR_WAIT_ABS:
            case TMR_ECHO:
              recd_clock = *(unsigned int *)&buf[i+4] + start_clock;
              break;

            default:
              fprintf(stderr, "unknown EV_TIMING %02x\n", buf[i+1]);
              break;
          }
          i += 8;
          break;

        case EV_CHN_COMMON:
          {
            unsigned char chn = buf[i+3];
            unsigned char ctl = buf[i+4];
            //unsigned char par = buf[i+5];
            short val = *(short *)&buf[i+6];
            //printf("got: chn %d, ctl %d, val %d\n", chn, ctl, val);
            switch (buf[i+2])
            {
              case MIDI_CTL_CHANGE:
                pEvent = new JZControlEvent(0, chn, ctl, val);
                break;
              case MIDI_PGM_CHANGE:
                pEvent = new JZProgramEvent(0, chn, ctl);
                break;
              case MIDI_CHN_PRESSURE:
                 pEvent = new JZChnPressureEvent(0, chn, ctl);
                 break;
              case MIDI_PITCH_BEND:
                pEvent = new JZPitchEvent(0, chn, val - 8192);
                break;
            }

            // midi thru
            if (gpConfig->GetValue(C_SoftThru))
            {
              buf[i+1] = mididev;
              ioctl(seqfd, SNDCTL_SEQ_OUTOFBAND, &buf[i]);
            }
          }
          i += 8;
          break;

        case EV_CHN_VOICE:
          {
            unsigned char chn = buf[i+3];
            unsigned char key = buf[i+4];
            unsigned char vel = buf[i+5];
            switch (buf[i+2])
            {
              case MIDI_NOTEOFF:  // SN++ added veloc
                pEvent = new JZKeyOffEvent(0, chn, key, vel);
                break;
              case MIDI_NOTEON:
                if (vel == 0)
                {
                  pEvent = new JZKeyOffEvent(0, chn, key);
                }
                else
                {
                  pEvent = new JZKeyOnEvent(0, chn, key, vel);
                }
                break;
              case MIDI_KEY_PRESSURE:
                 pEvent = new JZKeyPressureEvent(0, chn, key, vel);
                 break;
            }

            // midi thru
            if (gpConfig->GetValue(C_SoftThru))
            {
              buf[i+1] = mididev;
              ioctl(seqfd, SNDCTL_SEQ_OUTOFBAND, &buf[i]);
            }
          }
          i += 8;
          break;

        default:
          fprintf(stderr, "unknown sequencer status %02x ", buf[i]);
          i += 4;
          break;
      }

      if (pEvent)
      {
        pEvent->SetClock(mpPlayLoop->Ext2IntClock(recd_clock));
        mRecdBuffer.Put(pEvent);
        pEvent = 0;
      }

    }
  }

  JZProjectManager::Instance().NewPlayPosition(
    mpPlayLoop->Ext2IntClock(recd_clock/48 * 48));
  return recd_clock;
}

#endif // DEV_SEQUENCER2
