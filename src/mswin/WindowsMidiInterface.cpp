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

#include <windows.h>
#include <mmsystem.h>
#include <memory.h>

#include "WindowsMidiInterface.h"

#include "WindowsAudioInterface.h"

extern "C"
{

//*****************************************************************************
// LibMain - Generic for a DLL.  Just initializes a little local memory.
//*****************************************************************************
int FAR PASCAL LibMain(
  HANDLE hInstance,
  WORD wDataSeg,
  WORD wHeapSize,
  LPSTR lpCmdLine)

{
  // Nothing to do - SDK Libentry does the LocalInit
  return TRUE;
}

//*****************************************************************************
// WEP - Generic for a DLL.  Doesn't do a whole lot.
//*****************************************************************************
void FAR PASCAL _WEP(WORD wParam)
{
}

//*****************************************************************************
// Allocate memory for the player variables that are accessed in interrupt.
//*****************************************************************************
tWinPlayerState FAR * FAR PASCAL NewWinPlayerState()
{
  tWinPlayerState FAR * pState = 0;
  // Allocate Fixed Memory for interrupt handler
  HANDLE hMem = GlobalAlloc(
    GMEM_SHARE | GMEM_FIXED | GMEM_ZEROINIT,
    (DWORD)sizeof(tWinPlayerState));
  pState = (tWinPlayerState FAR *)GlobalLock(hMem);
#ifdef WIN32
  VirtualLock(pState, sizeof(tWinPlayerState));
#else
  GlobalPageLock((HGLOBAL)HIWORD(pState));
#endif
  memset(pState, 0, sizeof(tWinPlayerState));
  pState->hmem = hMem;

  pState->mpInputSysexBuffers = new JZWinSysexBufferArray();
  pState->mpOutputSysexBuffers = new JZWinSysexBufferArray();

  return pState;
}

//*****************************************************************************
// free interrupt data
//*****************************************************************************
void FAR PASCAL DeleteWinPlayerState(tWinPlayerState FAR * pState)
{
  delete pState->mpInputSysexBuffers;
  pState->mpInputSysexBuffers = 0;
  delete pState->mpOutputSysexBuffers;
  pState->mpOutputSysexBuffers = 0;


  HANDLE hMem = pState->hmem;
#ifdef WIN32
  VirtualUnlock(pState, sizeof(tWinPlayerState));
#else
  GlobalPageUnlock((HGLOBAL)HIWORD(pState));
#endif
  GlobalUnlock(hMem);
  GlobalFree(hMem);
}

//*****************************************************************************
//*****************************************************************************
#define Mtc2Frames(pState) \
{ \
  switch (pState->mtc_start.type) \
  { \
    case 0: \
      pState->mtc_frames = (((((pState->mtc_start.hour * 60) + pState->mtc_start.min) * 60) + pState->mtc_start.sec) * 24) + pState->mtc_start.fm; \
      break; \
    case 1: \
      pState->mtc_frames = (((((pState->mtc_start.hour * 60) + pState->mtc_start.min) * 60) + pState->mtc_start.sec) * 25) + pState->mtc_start.fm; \
      break; \
    case 2: \
    case 3: \
      pState->mtc_frames = (((((pState->mtc_start.hour * 60) + pState->mtc_start.min) * 60) + pState->mtc_start.sec) * 30) + pState->mtc_start.fm; \
      break; \
  } \
}

//*****************************************************************************
//*****************************************************************************
#define GetMtcTime(pState, msec) \
{ \
  switch (pState->mtc_start.type) \
  { \
    case 0: \
      msec = ((pState->mtc_frames / 24) * 1000) + \
      (((pState->mtc_frames % 24) * pState->time_per_frame) / 1000); \
      break; \
    case 1: \
      msec = ((pState->mtc_frames / 25) * 1000) + (((pState->mtc_frames % 25) * pState->time_per_frame) / 1000); \
      break; \
    case 2: \
    case 3: \
      msec = ((pState->mtc_frames / 30) * 1000) + (((pState->mtc_frames % 30) * pState->time_per_frame) / 1000); \
      break; \
    default: \
      msec = 0; \
  } \
}

//*****************************************************************************
//*****************************************************************************
static inline void outsysex(tWinPlayerState* pState)
{
  // Take away the SYSEX_EVENT meta event.
  (void) pState->play_buffer.get();

  // The next entry is the actual data.
  JZMidiEvent* pMidiEvent = pState->play_buffer.peek();
  JZWinSysexBuffer* pWinSysexBuffer = (JZWinSysexBuffer*)pMidiEvent->data;
  MIDIHDR *hdr = pWinSysexBuffer->MidiHdr();
  midiOutLongMsg(pState->hout, hdr, sizeof(MIDIHDR));
  // Don't care about return codes because the SYSEX_EVENT was already taken
  // from the queue.
}

//*****************************************************************************
// handle incoming midi data (internal clock)
//*****************************************************************************
void CALLBACK midiIntInputHandler(
  HMIDIIN hMidiIn,
  WORD wMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2)
{
  tWinPlayerState* pState = (tWinPlayerState*)dwInstance;
  int now;

  now = (int)timeGetTime();

  switch (wMsg)
  {
    case MIM_DATA:
      // Ignore active sensing and real time messages except midi stop.
      if ((dwParam1 & 0x000000ff) < 0xf8)
      {
        pState->recd_buffer.put(dwParam1, now);

        // Midi thru
        if (pState->soft_thru)
        {
          if (
            !pState->thru_buffer.empty() ||
            midiOutShortMsg(pState->hout, dwParam1) == MIDIERR_NOTREADY)
          {
            // device busy, output during normal play
            pState->thru_buffer.put(dwParam1, 0);
          }
        }
      }
      break;
    case MIM_OPEN:
    case MIM_ERROR:
    default:
      break;
  }
}

//*****************************************************************************
// play output (internal clock)
//*****************************************************************************
void CALLBACK midiIntTimerHandler(
  UINT wTimerId,
  UINT wMsg,
  DWORD dwUser,
  DWORD dw1,
  DWORD dw2)
{
  tWinPlayerState* pState = (tWinPlayerState *)dwUser;
  if (!pState->playing)
  {
    return;
  }

  // output what was left from midi thru
  while (!pState->thru_buffer.empty())
  {
    JZMidiEvent* pMidiEvent = pState->thru_buffer.peek();
    if (midiOutShortMsg(pState->hout, pMidiEvent->data) == MIDIERR_NOTREADY)
    {
      timeSetEvent(
        pState->min_timer_period,
        pState->min_timer_period * 5,
        (LPTIMECALLBACK) midiIntTimerHandler,
        (DWORD) pState,
        TIME_ONESHOT);
      return;
    }
    (void)pState->thru_buffer.get();
  }

  pState->play_time = (int)timeGetTime() + pState->time_correction;

  JZMidiEvent* pMidiEvent = pState->play_buffer.peek();
  while (pMidiEvent)
  {
    if (pMidiEvent->ref > pState->play_time)
    {
      break;
    }

    if (pMidiEvent->data)
    {
      if (pMidiEvent->data == START_AUDIO)
      {
        pState->audio_player->StartAudio();
      }
      else
      {
        if (pMidiEvent->data == SYSEX_EVENT)
        {
          outsysex(pState);
        }
        else
        {
          if (midiOutShortMsg(pState->hout, pMidiEvent->data) == MIDIERR_NOTREADY)
          {
            // try again later
            timeSetEvent(
              pState->min_timer_period,
              pState->min_timer_period * 5,
              (LPTIMECALLBACK) midiIntTimerHandler,
              (DWORD) pState,
              TIME_ONESHOT);
            return;
          }
        }
      }
    }
    (void) pState->play_buffer.get();
    pMidiEvent = pState->play_buffer.peek();
  }

  // compute delta time for next interrupt
  int delay = 100; // default in millisec
  if (pMidiEvent)
  {
    delay = (int)pMidiEvent->ref - (int)pState->play_time;
  }
  if (delay < (int)pState->min_timer_period)
  {
    delay = (int)pState->min_timer_period;
  }
  else if (delay > (int)pState->max_timer_period)
  {
    delay = (int)pState->max_timer_period;
  }
  timeSetEvent(
    (UINT) delay,
    pState->min_timer_period,
    (LPTIMECALLBACK) midiIntTimerHandler,
    (DWORD) pState,
    TIME_ONESHOT);
}

//*****************************************************************************
// handle incoming midi data (midi clock source) (songpointer)
//*****************************************************************************
void CALLBACK midiMidiInputHandler(
  HMIDIIN hMidiIn,
  WORD wMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2)
{
  tWinPlayerState* pState = (tWinPlayerState*)dwInstance;
  int now;

  now = (int)timeGetTime();

  switch (wMsg)
  {
    case MIM_DATA:
      if (dwParam1 == 0xf8)
      {
        pState->signal_time = now;
        pState->virtual_clock += pState->ticks_per_signal;
        return;
      }

      // ignore active sensing and real time messages except midi stop
      if (
        (dwParam1 != 0xf8) &&
        (dwParam1 != 0xfa) &&
        (dwParam1 != 0xfb) &&
        (dwParam1 != 0xFE))
      {
        pState->recd_buffer.put(
          dwParam1,
          pState->virtual_clock +
            (((now - pState->signal_time) * 1000L) / pState->time_per_tick));

        // Midi thru, do not put stop-play thru
        if (pState->soft_thru && (dwParam1 != 0xfc))
        {
          if (
            !pState->thru_buffer.empty() ||
            midiOutShortMsg(pState->hout, dwParam1) == MIDIERR_NOTREADY)
          {
            // device busy, output during normal play
            pState->thru_buffer.put(dwParam1, 0);
          }
        }
      }
      break;
    case MIM_OPEN:
    case MIM_ERROR:
    default:
      break;
  }
}

//*****************************************************************************
// play output (midi clock source) (songpointer)
//*****************************************************************************
void CALLBACK midiMidiTimerHandler(
  UINT wTimerId,
  UINT wMsg,
  DWORD dwUser,
  DWORD dw1,
  DWORD dw2)
{
  tWinPlayerState* pState = (tWinPlayerState*)dwUser;
  if (!pState->playing)
  {
    return;
  }

  // output what was left from midi thru
  while (!pState->thru_buffer.empty())
  {
    JZMidiEvent* pMidiEvent = pState->thru_buffer.peek();
    if (midiOutShortMsg(pState->hout, pMidiEvent->data) == MIDIERR_NOTREADY)
    {
      timeSetEvent(
        pState->min_timer_period,
        pState->min_timer_period * 5,
        (LPTIMECALLBACK) midiMidiTimerHandler,
        (DWORD) pState,
        TIME_ONESHOT);
      return;
    }
    (void)pState->thru_buffer.get();
  }

  pState->play_time = (int)timeGetTime();

  // How many ticks since last signal?
  int delta_clock =
    ((pState->play_time - pState->signal_time) * 1000L) /
    pState->time_per_tick;

  if (delta_clock > (2 * pState->ticks_per_signal)) // Too many?
  {
    pState->play_clock = pState->virtual_clock; // Yes, means tape stopped.
  }
  else
  {
    pState->play_clock = pState->virtual_clock + delta_clock;
  }

  JZMidiEvent* pMidiEvent = pState->play_buffer.peek();
  while (pMidiEvent)
  {
    if ((int)pMidiEvent->ref > pState->play_clock)
    {
      break;
    }

    if (pMidiEvent->data)
    {
      if (pMidiEvent->data == SYSEX_EVENT)
      {
        outsysex(pState);
      }
      else
      {
        if (midiOutShortMsg(pState->hout, pMidiEvent->data) == MIDIERR_NOTREADY)
        {
          // try again later
          timeSetEvent(
            pState->min_timer_period,
            pState->min_timer_period * 5,
            (LPTIMECALLBACK) midiMidiTimerHandler,
            (DWORD) pState,
            TIME_ONESHOT);
          return;
        }
      }
    }
    (void) pState->play_buffer.get();
    pMidiEvent = pState->play_buffer.peek();
  }

  // compute delta time for next interrupt
  int delay = 100; // default in millisec

  if (pMidiEvent)
  {
    delay = (((int)pMidiEvent->ref - pState->play_clock) * pState->time_per_tick) / 1000L;
  }
  if (delay < (int)pState->min_timer_period)
  {
    delay = (int)pState->min_timer_period;
  }
  else if (delay > (int)pState->max_timer_period)
  {
    delay = (int)pState->max_timer_period;
  }

  timeSetEvent(
    (UINT) delay,
    pState->min_timer_period,
    (LPTIMECALLBACK) midiMidiTimerHandler,
    (DWORD) pState,
    TIME_ONESHOT);
}

//*****************************************************************************
// handle incoming midi data (MTC clock source)
//*****************************************************************************
void CALLBACK midiMtcInputHandler(
  HMIDIIN hMidiIn,
  WORD wMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2)
{
  tWinPlayerState* pState = (tWinPlayerState*)dwInstance;
  int now;

  now = (int)timeGetTime();

  switch (wMsg)
  {
    case MIM_DATA:
      if ((dwParam1 & 0x000000ff) == 0xf1)
      {
        pState->last_qfm = (dwParam1 & 0x00007000) >> 12;
        if (pState->mtc_valid)
        {
          if ((pState->last_qfm % 4) == 0)
          {
            pState->signal_time = now;
            pState->mtc_frames++;
          }
        }
        else
        {
          union
          {
            DWORD w;
            unsigned char c[4];
          } u;
          u.w = dwParam1;
          pState->qfm_bits |= (0x0001 << pState->last_qfm);
          switch (pState->last_qfm)
          {
            case 0:
              pState->signal_time = now;
              pState->mtc_start.fm = u.c[1] & 0x0f;
              break;
            case 1:
              pState->mtc_start.fm |= ((u.c[1] & 0x0f) << 4);
              break;
            case 2:
              pState->mtc_start.sec = u.c[1] & 0x0f;
              break;
            case 3:
              pState->mtc_start.sec |= ((u.c[1] & 0x0f) << 4);
              break;
            case 4:
              pState->mtc_start.min = u.c[1] & 0x0f;
              break;
            case 5:
              pState->mtc_start.min |= ((u.c[1] & 0x0f) << 4);
              break;
            case 6:
              pState->mtc_start.hour = u.c[1] & 0x0f;
              break;
            case 7:
              pState->mtc_start.hour |= ((u.c[1] & 0x01) << 4);
              pState->mtc_start.type = ((u.c[1] & 0x06) >> 1);
              if (pState->qfm_bits == 0xff)
              {
                int mtc_time;
                pState->signal_time = now;
                Mtc2Frames(pState);
                GetMtcTime(pState, mtc_time);
                pState->recd_buffer.put(0xf1, mtc_time);
                pState->mtc_valid = TRUE;
              }
              pState->qfm_bits = 0;
              break;
          } // switch last_qfm
        } // mtc_valid
        return;
      } // 0xf1

      // ignore active sensing and real time messages except midi stop
      if ((dwParam1 & 0x000000ff) < 0xf8)
      {
        if (pState->mtc_valid)
        {
          int mtc_time;
          GetMtcTime(pState, mtc_time);
          pState->recd_buffer.put(
            dwParam1,
            mtc_time + (now - pState->signal_time));
        }

        // Midi thru
        if (pState->soft_thru)
        {
          if (!pState->thru_buffer.empty() || midiOutShortMsg(pState->hout, dwParam1) == MIDIERR_NOTREADY)
          {
            // device busy, output during normal play
            pState->thru_buffer.put(dwParam1, 0);
          }
        }
      }
      break;
    case MIM_OPEN:
    case MIM_ERROR:
    default:
      break;
  }
}

//*****************************************************************************
// play output (MTC clock source)
//*****************************************************************************
void CALLBACK midiMtcTimerHandler(
  UINT wTimerId,
  UINT wMsg,
  DWORD dwUser,
  DWORD dw1,
  DWORD dw2)
{
  tWinPlayerState* pState = (tWinPlayerState*)dwUser;
  if (!pState->playing)
  {
    return;
  }
  if (pState->doing_mtc_rec)
  {
    return;
  }

  // output what was left from midi thru
  while (!pState->thru_buffer.empty())
  {
    JZMidiEvent* pMidiEvent = pState->thru_buffer.peek();
    if (midiOutShortMsg(pState->hout, pMidiEvent->data) == MIDIERR_NOTREADY)
    {
      timeSetEvent(
        pState->min_timer_period,
        pState->min_timer_period * 5,
        (LPTIMECALLBACK) midiMtcTimerHandler,
        (DWORD) pState,
        TIME_ONESHOT);
      return;
    }
    (void)pState->thru_buffer.get();
  }

  int now = (int)timeGetTime();
  if (pState->mtc_valid)
  {
    GetMtcTime(pState, pState->play_time);
    pState->play_time += now - pState->signal_time;
  }
  else
  {
    // Tape not running
    return;
  }

  JZMidiEvent* pMidiEvent = pState->play_buffer.peek();
  while (pMidiEvent)
  {
    if (pMidiEvent->ref > pState->play_time)
    {
      break;
    }

    if (pMidiEvent->data)
    {
      if (pMidiEvent->data == SYSEX_EVENT)
      {
        outsysex(pState);
      }
      else
      {
        if (midiOutShortMsg(pState->hout, pMidiEvent->data) == MIDIERR_NOTREADY)
        {
          // try again later
          timeSetEvent(
            pState->min_timer_period,
            pState->min_timer_period * 5,
            (LPTIMECALLBACK) midiMtcTimerHandler,
            (DWORD) pState,
            TIME_ONESHOT);
          return;
        }
      }
    }
    (void) pState->play_buffer.get();
    pMidiEvent = pState->play_buffer.peek();
  }

  // compute delta time for next interrupt
  int delay = 100; // default in millisec
  if (pMidiEvent)
  {
    delay = (int)pMidiEvent->ref - (int)pState->play_time;
  }
  if (delay < (int)pState->min_timer_period)
  {
    delay = (int)pState->min_timer_period;
  }
  else if (delay > (int)pState->max_timer_period)
  {
    delay = (int)pState->max_timer_period;
  }
  timeSetEvent(
    (UINT) delay,
    pState->min_timer_period,
    (LPTIMECALLBACK) midiMtcTimerHandler,
    (DWORD) pState,
    TIME_ONESHOT);
}

//*****************************************************************************
//*****************************************************************************
void CALLBACK MidiOutProc(
  HMIDIOUT hmo,
  UINT wMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2
)
{
  if (wMsg == MOM_DONE)
  {
    MIDIHDR *hdr = (MIDIHDR *)dwParam1;
    JZWinSysexBuffer* pWinSysexBuffer = (JZWinSysexBuffer *)hdr->dwUser;
    if (pWinSysexBuffer != 0)
    {
      // ignore OutNow() buffers
      pWinSysexBuffer->Release();
      OutputDebugString(L"release\n");
    }
  }
}

} // extern "C"
