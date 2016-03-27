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

#include "WindowsPlayer.h"

#include "WindowsMidiInterface.h"
#include "JazzPlusPlusApplication.h"
#include "ProjectManager.h"
#include "TrackFrame.h"
#include "TrackWindow.h"
#include "Dialogs.h"
#include "MidiDeviceDialog.h"
#include "Globals.h"

#include <wx/msgdlg.h>

using namespace std;

// for msvc uncomment these
#ifdef _MSC_VER
#define enable()
#define disable()
#endif

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZWindowsPlayer::JZWindowsPlayer(JZSong* pSong)
  : JZPlayer(pSong),
    mpState(0)
{
  mPollMillisec = 25;
  timer_installed = FALSE;
  mpState = NewWinPlayerState();

  mpState->hinp   = 0;
  mpState->hout   = 0;
  mpState->recd_buffer.Clear();
  mpState->play_buffer.Clear();
  mpState->thru_buffer.Clear();
  mpState->playing = FALSE;
  mpState->soft_thru = gpConfig->GetValue(C_SoftThru);
  mpState->doing_mtc_rec = FALSE;
  mpState->audio_player = 0;

  int ilong = -1, olong = -1;
  if (
    !gpConfig->Get(C_WinInputDevice, ilong) ||
    !gpConfig->Get(C_WinOutputDevice, olong))
  {
    SettingsDlg(ilong, olong);
  }
  // only output device MUST be there
  else if (olong < 0)
  {
    SettingsDlg(ilong, olong);
  }

  // select input device
  if (ilong >= 0)
  {
    UINT DeviceId = (UINT)ilong;
    UINT rc;
    switch (gpConfig->GetValue(C_ClockSource))
    {
      case CsMidi:
        rc = midiInOpen(
          &mpState->hinp,
          DeviceId,
          (DWORD_PTR)midiMidiInputHandler,
          (DWORD_PTR)mpState,
          CALLBACK_FUNCTION);
        break;
      case CsMtc:
        rc = midiInOpen(
          &mpState->hinp,
          DeviceId,
          (DWORD_PTR)midiMtcInputHandler,
          (DWORD_PTR)mpState,
          CALLBACK_FUNCTION);
        break;
      case CsInt:
      case CsFsk:
      default:
        rc = midiInOpen(
          &mpState->hinp,
          DeviceId,
          (DWORD_PTR)midiIntInputHandler,
          (DWORD_PTR)mpState,
          CALLBACK_FUNCTION);
        break;
    }
    if (rc)
    {
      wchar_t ErrorMessage[MAXERRORLENGTH];
      midiInGetErrorText(rc, ErrorMessage, sizeof(ErrorMessage));
      ::wxMessageBox(ErrorMessage, "Open MIDI Input", wxOK);
    }
  }

  // select output device
  if (olong >= 0)
  {
    UINT DeviceId = (UINT)olong;
    if (DeviceId == MAX_MIDI_DEVS)
    {
      DeviceId = MIDI_MAPPER;
    }

    UINT rc = midiOutOpen(
      &mpState->hout,
      DeviceId,
      (DWORD_PTR)MidiOutProc,
      (DWORD_PTR)mpState,
      CALLBACK_FUNCTION);
    if (rc)
    {
      wchar_t ErrorMessage[MAXERRORLENGTH];
      midiOutGetErrorText(rc, ErrorMessage, sizeof(ErrorMessage));
      ::wxMessageBox(ErrorMessage, "Open MIDI Output", wxOK);
    }
  }

  // install timer
  {
    TIMECAPS caps;
    if (timeGetDevCaps(&caps, sizeof(caps)) == 0)
    {
      mpState->min_timer_period = caps.wPeriodMin;
      mpState->max_timer_period = caps.wPeriodMax;
      if (timeBeginPeriod(mpState->min_timer_period) == 0)
      {
        timer_installed = TRUE;
      }
    }
    if (!timer_installed)
    {
      ::wxMessageBox("Could not install timer", "MIDI timer", wxOK);
    }
  }

  maxSysLen = 2000;
  hSysHdr = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD)sizeof(MIDIHDR));
  pSysHdr = (MIDIHDR *)GlobalLock(hSysHdr);
  hSysBuf = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD)maxSysLen);
  pSysBuf = (unsigned char *)GlobalLock(hSysBuf);

  if (mpState->hinp)
  {
    midiInStart(mpState->hinp);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZWindowsPlayer::IsInstalled()
{
  return timer_installed && mpState->hout;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZWindowsPlayer::~JZWindowsPlayer()
{
  if (mpState->hinp)
  {
    midiInReset(mpState->hinp);
    midiInClose(mpState->hinp);
  }
  if (mpState->hout)
  {
    midiOutReset(mpState->hout);
    midiOutClose(mpState->hout);
  }
  if (timer_installed)
  {
    timeEndPeriod(mpState->min_timer_period);
  }

  GlobalUnlock(hSysHdr);
  GlobalFree(hSysHdr);
  GlobalUnlock(hSysBuf);
  GlobalFree(hSysBuf);

  DeleteWinPlayerState(mpState);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::SetSoftThru(int on, int InputDevice, int OutputDevice)
{
  mpState->soft_thru = on;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZEvent *JZWindowsPlayer::Dword2Event(DWORD dw)
{
  union
  {
    DWORD w;
    unsigned char c[4];
  } u;
  u.w = dw;

  JZEvent* pEvent = 0;

  switch (u.c[0] & 0xf0)
  {
    case 0x80:
      pEvent = new JZKeyOffEvent(0, u.c[0] & 0x0f, u.c[1]);
      break;

    case 0x90:
      if (u.c[2])
      {
        pEvent = new JZKeyOnEvent(0, u.c[0] & 0x0f, u.c[1], u.c[2], 0);
      }
      else
      {
        pEvent = new JZKeyOffEvent(0, u.c[0] & 0x0f, u.c[1]);
      }
      break;

    case 0xA0:
      pEvent = new JZKeyPressureEvent(0, u.c[0] & 0x0f, u.c[1], u.c[2]);
      break;

    case 0xB0:
      if (u.c[1] != 0x7b)
      {
        pEvent = new JZControlEvent(0, u.c[0] & 0x0f, u.c[1], u.c[2]);
      }
      break;

    case 0xC0:
      pEvent = new JZProgramEvent(0, u.c[0] & 0x0f, u.c[1]);
      break;

    case 0xD0:
      pEvent = new JZChnPressureEvent(0, u.c[0] & 0x0f, u.c[1]);
      break;

    case 0xE0:
      pEvent = new JZPitchEvent(0, u.c[0] & 0x0f, u.c[1], u.c[2]);
      break;
  }
  return pEvent;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD JZWindowsPlayer::Event2Dword(JZEvent* pEvent)
{
  union
  {
    DWORD w;
    unsigned char c[4];
  } u;
  u.w = 0;

  int Stat = pEvent->GetStat();
  switch (Stat)
  {
    case StatKeyOn:
      {
        JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
        u.c[0] = 0x90 | pKeyOn->GetChannel();
        u.c[1] = pKeyOn->GetKey();
        u.c[2] = pKeyOn->GetVelocity();
      }
      break;

    case StatKeyOff:
      {
        JZKeyOffEvent* pKeyOff = pEvent->IsKeyOff();
        u.c[0] = 0x80 | pKeyOff->GetChannel();
        u.c[1] = pKeyOff->GetKey();
        u.c[2] = 0;
      }
      break;

    case StatProgram:
      {
        JZProgramEvent* pProgram = pEvent->IsProgram();
        u.c[0] = 0xC0 | pProgram->GetChannel();
        u.c[1] = pProgram->GetProgram();
      }
      break;

    case StatChnPressure:
      {
        JZChnPressureEvent* k = pEvent->IsChnPressure();
        u.c[0] = 0xC0 | k->GetChannel();
        u.c[1] = k->Value;
      }
      break;

    case StatControl:
      {
        JZControlEvent* pControl = pEvent->IsControl();
        u.c[0] = 0xB0 | pControl->GetChannel();
        u.c[1] = pControl->GetControl();
        u.c[2] = pControl->GetControlValue();
      }
      break;

    case StatKeyPressure:
      {
        JZKeyPressureEvent* pKeyPressure = pEvent->IsKeyPressure();
        u.c[0] = 0xA0 | pKeyPressure->GetChannel();
        u.c[1] = pKeyPressure->GetKey();
        u.c[2] = pKeyPressure->GetPressureValue();
      }
      break;

    case StatPitch:
      {
        JZPitchEvent *k = pEvent->IsPitch();
        int     v = k->Value + 8192;
        u.c[0] = 0xE0 | k->GetChannel();
        u.c[1] = (unsigned char)(v & 0x7F);
        u.c[2] = (unsigned char)(v >> 7);
      }
      break;

    case StatMidiClock:
    case StatStartPlay:
    case StatContPlay:
    case StatStopPlay:
      {
        u.c[0] = Stat;
      }
      break;

    case StatSysEx:
      break;

    case StatSetTempo:
      {
        JZSetTempoEvent *t = pEvent->IsSetTempo();
        if (t && t->GetClock() > 0)
        {
          SetTempo(t->GetBPM(), t->GetClock());
          OutOfBandEvents.Put(pEvent->Copy());
        }
      }
      break;

    default:
      break;
  }
  return u.w;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsPlayer::Clock2Time(int clock)
{
  if (clock < mpState->start_clock)
  {
    return mpState->start_time;
  }
  return (int)((double)(clock - mpState->start_clock) * 60000.0 /
    (double)mpState->ticks_per_minute + mpState->start_time);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsPlayer::Time2Clock(int time)
{
  if (time < mpState->start_time)
  {
    return mpState->start_clock;
  }
  return (int)(
    (double)(time - mpState->start_time) *
    (double)mpState->ticks_per_minute / 60000.0 +
    mpState->start_clock);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::SetTempo(int bpm, int clock)
{
  int t1 = Clock2Time(clock);
  mpState->ticks_per_minute = (int)bpm * (int)mpSong->GetTicksPerQuarter();
  int t2 = Clock2Time(clock);
  mpState->start_time += (t1 - t2);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsPlayer::RealTimeClock2Time(int clock)
{
  if (clock < mpState->start_clock)
  {
    return real_start_time;
  }
  return (int)((double)(clock - mpState->start_clock) * 60000.0 /
    (double)real_ticks_per_minute + real_start_time);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsPlayer::Time2RealTimeClock(int time)
{
  if (time < real_start_time)
  {
    return mpState->start_clock;
  }
  return (int)(
    (double)(time - real_start_time) *
    (double)real_ticks_per_minute / 60000.0 +
    mpState->start_clock);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::SetRealTimeTempo(int bpm, int clock)
{
  int t1 = RealTimeClock2Time(clock);
  real_ticks_per_minute = (int)bpm * (int)mpSong->GetTicksPerQuarter();
  int t2 = RealTimeClock2Time(clock);
  real_start_time += (t1 - t2);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZWindowsPlayer::OutSysex(JZEvent* pEvent, DWORD time)
{
  JZSysExEvent* pSysExEvent = pEvent->IsSysEx();
  if (pSysExEvent == 0)
  {
    return true;
  }

  if (mpState->play_buffer.nfree() < 2)
  {
    return true;
  }

  mpState->mSysexFound = true;
  JZWinSysexBuffer* pWinSysexBuffer =
    mpState->mpOutputSysexBuffers->AllocBuffer();
  pWinSysexBuffer->PrepareOut(
    mpState->hout,
    pSysExEvent->GetData(),
    pSysExEvent->GetLength() - 1);
  mpState->play_buffer.put(SYSEX_EVENT, time);
  mpState->play_buffer.put((DWORD)pWinSysexBuffer, time);

  return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsPlayer::OutEvent(JZEvent* pEvent)
{
  DWORD d = Event2Dword(pEvent);
  if (d)
  {
    mpState->play_buffer.put(d, Clock2Time(pEvent->GetClock()));
  }
  else if (pEvent->IsSysEx() && (pEvent->GetClock() > 0))
  {
    OutSysex(pEvent, Clock2Time(pEvent->GetClock()));
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsMidiPlayer::OutEvent(JZEvent* pEvent)
{
  DWORD d = Event2Dword(pEvent);
  if (d)
  {
    mpState->play_buffer.put(d, pEvent->GetClock());
  }
  else if (pEvent->IsSysEx() && (pEvent->GetClock() > 0))
  {
    OutSysex(pEvent, pEvent->GetClock());
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::OutNow(JZEvent* pEvent)
{
  DWORD d = Event2Dword(pEvent);
  if (d)
  {
    midiOutShortMsg(mpState->hout, d);
  }
  else if (pEvent->GetStat() == StatSetTempo)
  {
    if (mpState->playing)
    {
      SetTempo(pEvent->IsSetTempo()->GetBPM(), mOutClock);
    }
  }
  else if (pEvent->GetStat() == StatSysEx)
  {
    JZSysExEvent* s = pEvent->IsSysEx();
    if (s->GetDataLength() + 1 < maxSysLen)
    {
      pSysBuf[0] = 0xf0;
      memcpy(pSysBuf + 1, s->GetData(), s->GetDataLength());

      pSysHdr->lpData = (LPSTR)pSysBuf;
      pSysHdr->dwBufferLength = s->GetDataLength() + 1;
      pSysHdr->dwUser = 0;

      if (midiOutPrepareHeader(mpState->hout, pSysHdr, sizeof(MIDIHDR)) == 0)
      {
        midiOutLongMsg(mpState->hout, pSysHdr, sizeof(MIDIHDR));
      }
      // here we should wait, until the data are physically sent.
      // but there is no API call for this?!
      midiOutUnprepareHeader(mpState->hout, pSysHdr, sizeof(MIDIHDR));
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::OutNow(JZParam *r)
{
  OutNow(&r->mMsb);
  OutNow(&r->mLsb);
  OutNow(&r->mDataMsb);
  OutNow(&r->mResetMsb);
  OutNow(&r->mResetLsb);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::FillMidiClocks(int to)
{
  while (midiClockOut <= to)
  {
    JZMidiClockEvent* pEvent = new JZMidiClockEvent(midiClockOut);
    mPlayBuffer.Put(pEvent);
    midiClockOut = midiClockOut + mpState->ticks_per_signal;
  }
  mPlayBuffer.Sort();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::OutBreak(int clock)
{
  if (gpConfig->GetValue(C_RealTimeOut))
  {
    FillMidiClocks(mOutClock);
    FlushToDevice(mOutClock);
  }
  else
  {
    mpState->play_buffer.put(0, Clock2Time(clock));
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsMidiPlayer::OutBreak(int clock)
{
  if (gpConfig->GetValue(C_RealTimeOut))
  {
    FillMidiClocks(mOutClock);
    FlushToDevice(mOutClock);
  }
  else
  {
    mpState->play_buffer.put(0, clock);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::OutBreak()
{
  OutBreak(mOutClock);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static DWORD GetMtcTime(tWinPlayerState* pState)
{
  DWORD frames = pState->mtc_frames;
  switch (pState->mtc_start.type)
  {
    case 0:
      return (
        ((frames / 24) * 1000) +
        (((frames % 24) * pState->time_per_frame) / 1000));
    case 1:
      return (
        ((frames / 25) * 1000) +
        (((frames % 25) * pState->time_per_frame) / 1000));
    case 2:
    case 3:
      return (
        ((frames / 30) * 1000) +
        (((frames % 30) * pState->time_per_frame) / 1000));
    default:
      return 0;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::StartPlay(int Clock, int LoopClock, int Continue)
{
  mpState->play_buffer.Clear();
  mpState->recd_buffer.Clear();
  mpState->mSysexFound = false;

  mpState->ticks_per_minute  = mpSong->GetTicksPerQuarter() * mpSong->Speed();
  real_ticks_per_minute    = mpState->ticks_per_minute;
  mpState->ticks_per_signal  = mpSong->GetTicksPerQuarter() / 24;
  mpState->time_per_tick = 60000000L / mpState->ticks_per_minute;
  mpState->time_correction   = 0;

  if (gpConfig->GetValue(C_ClockSource) == CsMtc)
  {
    if (mpState->doing_mtc_rec)
    {
      Clock = 0;
    }
    if (!Continue)
    {
      JZMtcTime* pMtcOffset = mpSong->GetTrack(0)->GetMtcOffset();
      mpState->start_time = pMtcOffset->ToMillisec();
      real_start_time = mpState->start_time;
      mpState->mtc_start.type = pMtcOffset->GetType();
      delete pMtcOffset;
      mpState->start_clock = 0;
    }
  }
  else
  {
    mpState->start_time  = (int)timeGetTime() + 500;
    real_start_time = mpState->start_time;
    mpState->start_clock = Clock;
  }

  mpState->play_time   = mpState->start_time;
  midiClockOut = Clock;

  if (GetAudioEnabled())
  {
    mpState->play_buffer.put(START_AUDIO, mpState->start_time);
  }

  OutOfBandEvents.Clear();

  JZProjectManager::Instance().NewPlayPosition(
    mpPlayLoop->Ext2IntClock(Clock));

  mpState->playing = TRUE;  // allow for SetTempo in OutNow()
  JZPlayer::StartPlay(Clock, LoopClock, Continue);

  if (gpConfig->GetValue(C_RealTimeOut))
  {
    JZMetaEvent* pEvent;
    if (!Continue)
    {
      pEvent = new JZStartPlayEvent(0);
    }
    else
    {
      pEvent = new JZContPlayEvent(0);
    }
    OutNow(pEvent);
    FillMidiClocks(mPlayBuffer.GetLastClock()); // also does a sort
  }


  switch (gpConfig->GetValue(C_ClockSource))
  {
    case CsMidi:
      mpState->virtual_clock = Clock - mpState->ticks_per_signal;
      mpState->signal_time = mpState->start_time - 5000L;
      // mpState->playing = TRUE;
      timeSetEvent(
        mpState->min_timer_period,
        mpState->min_timer_period,
        (LPTIMECALLBACK) midiMidiTimerHandler,
        (DWORD_PTR) mpState,
        TIME_ONESHOT);
      break;
    case CsMtc:
      if (!Continue)
      {
        // In microseconds: 1 sec / frames_per_sec
        switch (mpState->mtc_start.type)
        {
          case 0:
            mpState->time_per_frame = 1000000 / 24;
            break;
          case 1:
            mpState->time_per_frame = 1000000 / 25;
            break;
          case 2:
          case 3:
            mpState->time_per_frame = 1000000 / 30;
            break;
        }
        mpState->signal_time = 0;
        mpState->mtc_valid = FALSE;
        mpState->last_qfm = 0;
        mpState->qfm_bits = 0;
        lastValidMtcClock = Clock;
      }
      // mpState->playing = TRUE;
      timeSetEvent(
        mpState->min_timer_period,
        mpState->min_timer_period,
        (LPTIMECALLBACK) midiMtcTimerHandler,
        (DWORD_PTR) mpState,
        TIME_ONESHOT);
      break;
    case CsInt:
    case CsFsk:
    default:
      // mpState->playing = TRUE;
      timeSetEvent(
        mpState->min_timer_period,
        mpState->min_timer_period,
        (LPTIMECALLBACK) midiIntTimerHandler,
        (DWORD_PTR) mpState,
        TIME_ONESHOT);
      break;
  }

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::StopPlay()
{
  wxBeginBusyCursor();
  mpState->playing = FALSE;
  JZPlayer::StopPlay();
  if (gpConfig->GetValue(C_RealTimeOut))
  {
    JZStopPlayEvent* pEvent = new JZStopPlayEvent(0);
    OutNow(pEvent);
    delete pEvent;
  }
  AllNotesOff();
  mRecdBuffer.Keyoff2Length();

  if (mpState->hout)
  {
    if (mpState->mSysexFound)
    {
      midiOutReset(mpState->hout);
    }
    size_t n = mpState->mpOutputSysexBuffers->Size();
    for (size_t i = 0; i < n; ++i)
    {
      JZWinSysexBuffer* pWinSysexBuffer = mpState->mpOutputSysexBuffers->At(i);
      if (pWinSysexBuffer->IsPrepared())
      {
        pWinSysexBuffer->UnprepareOut(mpState->hout);
      }
    }
    mpState->mpOutputSysexBuffers->ReleaseAllBuffers();
  }

//  // sysex recording not finished yet.
//  if (mpState->hinp)
//  {
//    midiInReset(mpState->hinp);
//    size_t n = mpState->pInputSysexBuffers->Size();
//    for (size_t i = 0; i < n; ++i)
//    {
//      JZWinSysexBuffer* pWinSysexBuffer = mpState->pInputSysexBuffers->At(i);
//      if (pWinSysexBuffer->IsPrepared())
//      {
//        pWinSysexBuffer->UnprepareIn(mpState->hinp);
//      }
//      mpState->pInputSysexBuffers->ReleaseAllBuffers();
//    }
//  }

  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//   Attempt to send all events up to mOutClock to device
//-----------------------------------------------------------------------------
void JZWindowsPlayer::FlushToDevice()
{
  if (gpConfig->GetValue(C_RealTimeOut))
  {
    FillMidiClocks(mOutClock);
  }
  FlushToDevice(mOutClock);
  OutBreak(mOutClock);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::FlushToDevice(int clock)
{
  JZEventIterator Iterator(&mPlayBuffer);
  JZEvent* pEvent = Iterator.Range(0, clock);
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
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsIntPlayer::GetRealTimeClock()
{
  while (!mpState->recd_buffer.empty())
  {
    JZMidiEvent* pMidiEvent = mpState->recd_buffer.get();

    // Event?
    JZEvent* pEvent = Dword2Event(pMidiEvent->data);
    if (pEvent)
    {
      pEvent->SetClock(mpPlayLoop->Ext2IntClock(Time2RealTimeClock(pMidiEvent->ref)));
      mRecdBuffer.Put(pEvent);
    }
  }

  int clock = Time2RealTimeClock(
    (int)timeGetTime() + mpState->time_correction);

  JZProjectManager::Instance().NewPlayPosition(
    mpPlayLoop->Ext2IntClock(clock / 48 * 48));

  if (!OutOfBandEvents.IsEmpty())
  {
    JZEventIterator Iterator(&OutOfBandEvents);
    JZEvent* pEvent = Iterator.Range(0, clock);
    while (pEvent)
    {
      switch (pEvent->GetStat())
      {
        case StatSetTempo:
          SetRealTimeTempo(((JZSetTempoEvent *)pEvent)->GetBPM(), clock);
          break;
        default:
          break;
      }
      pEvent->Kill();
      pEvent = Iterator.Next();
    }
    OutOfBandEvents.Cleanup(0);
  }

  return clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsMidiPlayer::GetRealTimeClock()
{
  int clock;

  while (!mpState->recd_buffer.empty())
  {
    JZMidiEvent* pMidiEvent = mpState->recd_buffer.get();
    if (pMidiEvent->data == 0xfc)
    {
      // Stop play received
      AllNotesOff();
      return -1;
    }
    else if ((pMidiEvent->data & 0x000000ff) == 0xf2)
    {
      // Song pointer received
      union
      {
        DWORD w;
        unsigned char c[4];
      } u;
      gpMidiPlayer->StopPlay();
      u.w = pMidiEvent->data;
      clock =
        ((int)u.c[1] + (128L * (int)u.c[2])) *
        (mpSong->GetTicksPerQuarter() / 4);
      gpMidiPlayer->StartPlay(clock, 0, 1);
      return -1;
    }

    // Event?
    JZEvent* pEvent = Dword2Event(pMidiEvent->data);
    if (pEvent)
    {
      pEvent->SetClock(mpPlayLoop->Ext2IntClock(pMidiEvent->ref));
      mRecdBuffer.Put(pEvent);
    }
  }

  int delta_clock = (((int)timeGetTime() - mpState->signal_time) * 1000L) / mpState->time_per_tick;

  if (delta_clock > (2 * mpState->ticks_per_signal))
  {
    clock = mpState->virtual_clock;
  }
  else
  {
    clock = mpState->virtual_clock + delta_clock;
  }

  JZProjectManager::Instance().NewPlayPosition(
    mpPlayLoop->Ext2IntClock(clock / 48 * 48));

  return clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWindowsMtcPlayer::GetRealTimeClock()
{
  int clock;

  while (!mpState->recd_buffer.empty())
  {
    JZMidiEvent* pMidiEvent = mpState->recd_buffer.get();
    if (pMidiEvent->data == 0xf1)
    {
      // MTC starting (from midi input handler)
      gpMidiPlayer->StopPlay();
      clock = mpPlayLoop->Ext2IntClock(Time2Clock(pMidiEvent->ref));
      lastValidMtcClock = clock;
      gpMidiPlayer->StartPlay(clock, 0, 1);
      return -1;
    }

    // Event?
    JZEvent* pEvent = Dword2Event(pMidiEvent->data);
    if (pEvent)
    {
      pEvent->SetClock(mpPlayLoop->Ext2IntClock(Time2Clock(pMidiEvent->ref)));
      mRecdBuffer.Put(pEvent);
    }
  }

  if (mpState->mtc_valid)
  {
    if (((int)timeGetTime() - mpState->signal_time) > 500)
    {
      // Assume tape stopped.
      disable();
      mpState->mtc_valid = 0;
      mpState->qfm_bits = 0;
      enable();
      AllNotesOff();
      return(-1);
    }
    if (mpState->doing_mtc_rec)
    {
      clock = 0;
    }
    else
    {
      clock = Time2Clock(GetMtcTime(mpState));
    }
    lastValidMtcClock = clock;
  }
  else
  {
    clock = lastValidMtcClock;
  }

  JZProjectManager::Instance().NewPlayPosition(
    mpPlayLoop->Ext2IntClock(clock / 48 * 48));

  if (!OutOfBandEvents.IsEmpty())
  {
    JZEventIterator Iterator(&OutOfBandEvents);
    JZEvent* pEvent = Iterator.Range(0, clock);
    while (pEvent)
    {
      switch (pEvent->GetStat())
      {
        case StatSetTempo:
          SetRealTimeTempo(((JZSetTempoEvent *)pEvent)->GetBPM(), clock);
          break;
        default:
          break;
      }
      pEvent->Kill();
      pEvent = Iterator.Next();
    }
    OutOfBandEvents.Cleanup(0);
  }

  return clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsMtcPlayer::InitMtcRec()
{
  mpState->doing_mtc_rec = TRUE;
  StartPlay(0, 0, 0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMtcTime* JZWindowsMtcPlayer::FreezeMtcRec()
{
  StopPlay();
  mpState->doing_mtc_rec = FALSE;
  return
    new JZMtcTime((int)GetMtcTime(mpState), (tMtcType)mpState->mtc_start.type);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWindowsPlayer::SettingsDlg(int& InputDevice, int& OutputDevice)
{
  vector<pair<wxString, int> > MidiDevices;

  //=========================
  // Select the input device.
  //=========================

  // Get a list of the available input devices.
  UINT i;
  UINT InputMidiDeviceCount = midiInGetNumDevs();
  for (i = 0; i < InputMidiDeviceCount; ++i)
  {
    MIDIINCAPS caps;
    midiInGetDevCaps(i, &caps, sizeof(caps));
    MidiDevices.push_back(make_pair(caps.szPname, i));
  }

  if (InputMidiDeviceCount > 0)
  {
    JZMidiDeviceDialog MidiInputDeviceDialog(
      MidiDevices,
      InputDevice,
      ::wxGetApp().GetMainFrame(),
      "Input MIDI device");
    MidiInputDeviceDialog.ShowModal();
  }

  MidiDevices.clear();

  // select output device
  UINT OutputMidiDeviceCount = midiOutGetNumDevs();
  for (i = 0; i < OutputMidiDeviceCount; ++i)
  {
    MIDIOUTCAPS caps;
    midiOutGetDevCaps(i, &caps, sizeof(caps));
    MidiDevices.push_back(make_pair(caps.szPname, i));
  }
  MidiDevices.push_back(make_pair("Midi Mapper", MAX_MIDI_DEVS));

  JZMidiDeviceDialog MidiOutputDeviceDialog(
    MidiDevices,
    OutputDevice,
    gpTrackWindow,
    "Output MIDI device");
  MidiOutputDeviceDialog.ShowModal();

  if (InputDevice >= 0)
  {
    gpConfig->Put(C_WinInputDevice, InputDevice);
  }
  else
  {
    gpConfig->Get(C_WinInputDevice, InputDevice);
  }

  if (OutputDevice >= 0)
  {
    gpConfig->Put(C_WinOutputDevice, OutputDevice);
  }
  else
  {
    gpConfig->Get(C_WinOutputDevice, OutputDevice);
  }
}
