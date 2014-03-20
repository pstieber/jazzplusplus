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
//
// Changes
//
// 2000.03.18        Takashi Iwai <iwai@ww.uni-erlangen.de>
// - Modified for ALSA 0.5.x.
//   You'll need the latest CVS version of ALSA-lib, which is expected
//   to be released as ver.0.5.7.
// - The input/output devices are selected via a pop-up window
//   like OSS driver mode.
//*****************************************************************************

#include "AlsaPlayer.h"

#include "Configuration.h"
#include "Dialogs.h"
#include "Globals.h"
#include "ProjectManager.h"
#include "TrackFrame.h"
#include "TrackWindow.h"

#include <wx/choicdlg.h>

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sys/ioctl.h>

using namespace std;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZAlsaPlayer::JZAlsaPlayer(JZSong* pSong)
  : JZPlayer(pSong)
{
  ithru = othru = 0;

  mInstalled = true;
  mPollMillisec = 25;
  recd_clock = 0;
  echo_clock = 0;

  if (snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0)
  {
    perror("open sequencer");
    mInstalled = false;
    return;
  }

  // set myself into non blocking mode
  if (set_blocking_mode(0) < 0)
  {
    mInstalled = false;
    return;
  }
  client = snd_seq_client_id(handle);
  struct pollfd pfds;
  snd_seq_poll_descriptors(handle, &pfds, 1, POLLIN|POLLOUT);

  //JAVE seqfd doesnt seem to be used for anything, not here nor in the base
  // class JZPlayer(but heavily in JZSeq2Player)
//  seqfd = pfds.fd;

  // create my input/output port
  memset(&self, 0, sizeof(self));
  self.client  = client;
  self.port    = create_port(handle, "Input/Output");

  cout
    << "INFO: Created client:port = " << static_cast<int>(self.client)
    << ':' << static_cast<int>(self.port)
    << endl;

  // Allocate a queue.
  queue = snd_seq_alloc_named_queue(handle, "Jazz++");

  // Register the name of this application.
  set_client_info(handle, "The JAZZ++ Midi Sequencer");

  // scan input addressess
  scan_clients(iaddr, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ);

  cout << "INFO: Input device count: " << iaddr.GetCount() << endl;
  if (iaddr.GetCount())
  {
    iaddr.AsciiWrite("Input Devices");
  }

  // scan output addresses
  scan_clients(oaddr, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE);

  cout << "INFO: Output device count: " << oaddr.GetCount() << endl;
  if (oaddr.GetCount())
  {
    oaddr.AsciiWrite("Output Devices");
  }

  mInputDeviceIndex = gpConfig->GetValue(C_AlsaInputDevice);
  if (mInputDeviceIndex < 0)
  {
    cout << "INFO: input device is -1, so selecting one." << endl;
    mInputDeviceIndex = select_list(
      iaddr,
      "Input Device",
      mInputDeviceIndex);
    cout << "INFO: Input device is: " << mInputDeviceIndex << endl;
    gpConfig->Put(C_AlsaInputDevice, mInputDeviceIndex);
  }
  else if (static_cast<unsigned>(mInputDeviceIndex) > iaddr.GetCount())
  {
    cout << "INFO: Input device is out of range, so selecting one." << endl;
    mInputDeviceIndex = select_list(
      iaddr,
      "Output Device",
      mInputDeviceIndex);
  }

  mOutputDeviceIndex = gpConfig->GetValue(C_AlsaOutputDevice);
  if (mOutputDeviceIndex < 0)
  {
    cout << "INFO: Output device is -1, so selecting one." << endl;
    mOutputDeviceIndex = select_list(
      oaddr,
      "Output Device",
      mOutputDeviceIndex);
  }
  else if (static_cast<unsigned>(mOutputDeviceIndex) > oaddr.GetCount())
  {
    cout << "INFO: Output device is out of range, so selecting one." << endl;
    mOutputDeviceIndex = select_list(
      oaddr,
      "Output Device",
      mOutputDeviceIndex);
  }

  if (mInputDeviceIndex >= 0)
  {
    if (static_cast<unsigned>(mInputDeviceIndex) < iaddr.GetCount())
    {
      subscribe_inp(mInputDeviceIndex);
    }
    else
    {
      cout
        << "WARNING: The input device index (" << mInputDeviceIndex
        << ") is out of range!" << '\n'
        << "Setting the value to -1"
        << endl;
      mInputDeviceIndex = -1;
    }
  }
  if (mOutputDeviceIndex >= 0)
  {
    if (static_cast<unsigned>(mOutputDeviceIndex) < oaddr.GetCount())
    {
      subscribe_out(mOutputDeviceIndex);
    }
    else
    {
      cout << "WARNING: The output device index is out of range!" << endl;
      cout
        << "WARNING: The output device index (" << mOutputDeviceIndex
        << ") is out of range!" << '\n'
        << "Setting the value to -1"
        << endl;
      mOutputDeviceIndex = -1;
    }
  }

  set_pool_sizes();

  snd_seq_set_output_buffer_size(handle, 65536);

  if (mInstalled)
  {
    thru = new JZAlsaThru();
    SetSoftThru(
      gpConfig->GetValue(C_SoftThru),
      gpConfig->GetValue(C_ThruInput),
      gpConfig->GetValue(C_ThruOutput));
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::clear_input_queue()
{
  snd_seq_drop_input(handle);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::set_pool_sizes()
{
  if (snd_seq_set_client_pool_output(handle, 2000) < 0)
  {
    perror("set pool output");
  }
  if (snd_seq_set_client_pool_input(handle, 2000) < 0)
  {
    perror("set pool input");
  }
  if (snd_seq_set_client_pool_output_room(handle, 1000) < 0)
  {
    perror("set pool output room");
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::SetSoftThru(int on, int idev, int odev)
{
  if (idev != ithru || odev != othru)
  {
    ithru = idev;
    othru = odev;

    thru->Stop();
  }

  if (on && !thru->IsRunning())
  {
    bool StartThru = false;
    if (static_cast<unsigned>(ithru) < iaddr.GetCount())
    {
      thru->SetSource(iaddr[ithru].client, iaddr[ithru].port);
      StartThru = true;
    }
    else
    {
      cout
        << "WARNING: The input MIDI thru device index (" << ithru
        << ") is out of range!" << '\n'
        << "Setting the value to -1"
        << endl;
      ithru = -1;
    }

    if (static_cast<unsigned>(othru) < oaddr.GetCount())
    {
      thru->SetDestin(oaddr[othru].client, oaddr[othru].port);
      StartThru = true;
    }
    else
    {
      cout
        << "WARNING: The output MIDI thru device index (" << othru
        << ") is out of range!" << '\n'
        << "Setting the value to -1"
        << endl;
      othru = -1;
    }

    if (StartThru)
    {
      thru->Start();
    }
  }
  else if (!on && thru->IsRunning())
  {
    thru->Stop();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Connect output addrs/queue with my client/oport.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::subscribe_out(int outp)
{
  if (
    snd_seq_connect_to(
      handle,
      self.port,
      oaddr[outp].client,
      oaddr[outp].port) < 0)
  {
    perror("subscribe output");
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Connect input addrs/queue with my client/iport
//-----------------------------------------------------------------------------
void JZAlsaPlayer::subscribe_inp(int inp)
{
  snd_seq_port_subscribe_t* subs;
  snd_seq_port_subscribe_alloca(&subs);
  snd_seq_port_subscribe_set_time_update(subs, 1);
  snd_seq_port_subscribe_set_queue(subs, queue);
  snd_seq_port_subscribe_set_sender(subs, &iaddr[inp]);
  snd_seq_port_subscribe_set_dest(subs, &self);
  if (snd_seq_subscribe_port(handle, subs) < 0)
  {
    perror("subscribe input");
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Disconnect output addrs/queue with my client/oport.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::unsubscribe_out(int outp)
{
  if (
    snd_seq_disconnect_to(
      handle,
      self.port,
      oaddr[outp].client,
      oaddr[outp].port) < 0)
  {
    perror("unsubscribe output");
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Connect input addrs/queue with my client/iport
//-----------------------------------------------------------------------------
void JZAlsaPlayer::unsubscribe_inp(int inp)
{
  snd_seq_port_subscribe_t *subs;
  snd_seq_port_subscribe_alloca(&subs);
  snd_seq_port_subscribe_set_time_update(subs, 1);
  snd_seq_port_subscribe_set_queue(subs, queue);
  snd_seq_port_subscribe_set_sender(subs, &iaddr[inp]);
  snd_seq_port_subscribe_set_dest(subs, &self);
  if (snd_seq_unsubscribe_port(handle, subs) < 0)
  {
    perror("unsubscribe input");
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Set the name of this client.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::set_client_info(snd_seq_t *handle, const char *name)
{
  if (snd_seq_set_client_name(handle, (char *)name) < 0)
  {
    perror("ioctl");
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Create a new port.
//-----------------------------------------------------------------------------
int JZAlsaPlayer::create_port(snd_seq_t *handle, const char *name)
{
  return snd_seq_create_simple_port(
    handle,
    (char *)name,
    SND_SEQ_PORT_CAP_READ |
      SND_SEQ_PORT_CAP_SUBS_READ |
      SND_SEQ_PORT_CAP_WRITE |
      SND_SEQ_PORT_CAP_SUBS_WRITE,
      SND_SEQ_PORT_TYPE_MIDI_GENERIC);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZAlsaPlayer::IsInstalled()
{
  return mInstalled;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZAlsaPlayer::~JZAlsaPlayer()
{
  if (thru)
  {
    delete thru;
  }
  // The following call caused an unkillable process on Mandriva 2008.0.
//  snd_seq_close(handle);
}

//-----------------------------------------------------------------------------
// 0 = event successfully sent to driver
// 1 = try again later
//-----------------------------------------------------------------------------
int JZAlsaPlayer::OutEvent(JZEvent* pEvent, int now)
{
  int rc = 0;
  snd_seq_event_t ev;
  memset(&ev, 0, sizeof(ev));
  switch (pEvent->GetStat())
  {
    case StatKeyOn:
      {
        JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_NOTEON);
        ev.data.note.channel = pKeyOn->GetChannel();
        ev.data.note.note = pKeyOn->GetKey();
        ev.data.note.velocity = pKeyOn->GetVelocity();
        rc = write(&ev, now);
      }
      break;

    case StatKeyOff:
      {
        JZKeyOffEvent* pKeyOff = pEvent->IsKeyOff();
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_NOTEOFF);
        ev.data.note.channel = pKeyOff->GetChannel();
        ev.data.note.note = pKeyOff->GetKey();
        ev.data.note.velocity = pKeyOff->GetOffVelocity();
        rc = write(&ev, now);
      }
      break;

    case StatProgram:
      {
        JZProgramEvent* pProgram = pEvent->IsProgram();
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_PGMCHANGE);
        ev.data.control.channel = pProgram->GetChannel();
        ev.data.control.value = pProgram->GetProgram();
        rc = write(&ev, now);
      }
      break;

    case StatKeyPressure:
      {
        JZKeyPressureEvent* pKeyPressure = pEvent->IsKeyPressure();
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_KEYPRESS);
        ev.data.note.channel = pKeyPressure->GetChannel();
        ev.data.note.note = pKeyPressure->GetKey();
        ev.data.note.velocity = pKeyPressure->GetPressureValue();
        rc = write(&ev, now);
      }
      break;

    case StatChnPressure:
      {
        JZChnPressureEvent *k = pEvent->IsChnPressure();
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_CHANPRESS);
        ev.data.control.channel = k->GetChannel();
        ev.data.control.value = k->Value;
        rc = write(&ev, now);
      }
      break;

    case StatControl:
      {
        JZControlEvent* k = pEvent->IsControl();
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_CONTROLLER);
        ev.data.control.channel = k->GetChannel();
        ev.data.control.param = k->GetControl();
        ev.data.control.value = k->GetControlValue();
        rc = write(&ev, now);
      }
      break;

    case StatPitch:
      {
        JZPitchEvent *k = pEvent->IsPitch();
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_PITCHBEND);
        ev.data.control.channel = k->GetChannel();
        ev.data.control.value = k->Value;
        rc = write(&ev, now);
      }
      break;

    case StatSetTempo:
      {
        int bpm = pEvent->IsSetTempo()->GetBPM();
        int us  = (int)( 60.0E6 / (double)bpm );
        set_event_header(&ev, pEvent->GetClock(), SND_SEQ_EVENT_TEMPO);
        snd_seq_ev_set_queue_tempo(&ev, queue, us);
        rc = write(&ev, now);
      }
      break;

    case StatSysEx:
      {
        JZSysExEvent* pSysEx = pEvent->IsSysEx();
        // prepend 0xf0
        char* pBuffer = new char[pSysEx->GetDataLength() + 1];
        pBuffer[0] = 0xF0;
        memcpy(pBuffer + 1, pSysEx->GetData(), pSysEx->GetDataLength());
        set_event_header(
          &ev,
          pEvent->GetClock(),
          pSysEx->GetDataLength() + 1,
          pBuffer);
        rc = write(&ev, now);
        delete [] pBuffer;
      }
      break;

    default:
      break;
  }
  return rc < 0 ? 1 : 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::OutBreak()
{
  OutBreak(mOutClock);
}

//-----------------------------------------------------------------------------
// Description:
//   "echos" are used to synchronize.  They are supposed to be read later by
// the Notify call chain.
//-----------------------------------------------------------------------------
int JZAlsaPlayer::compose_echo(int clock, unsigned int arg)
{
  snd_seq_event_t ev;
  memset(&ev, 0, sizeof(ev));
  ev.source = self;
  ev.dest = self;
  snd_seq_ev_schedule_tick(&ev, queue, 0, clock);
  snd_seq_ev_set_fixed(&ev);
  ev.type = SND_SEQ_EVENT_ECHO;
  ev.data.raw32.d[0] = arg;
  return write(&ev);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::OutBreak(int clock)
{
  while (echo_clock + 48 < clock)
  {
    echo_clock += 48;
    if (compose_echo(echo_clock, 0) < 0)
    {
      break;
    }
  }

  flush_output();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::StartPlay(int clock, int loopClock, int cont)
{
  recd_clock = clock;
  echo_clock = clock;
  play_clock = clock;
  flush_output();
  start_timer(clock);
  JZPlayer::StartPlay(clock, loopClock, cont);
  Notify();
//  flush_output();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::ResetPlay(int clock)
{
  // Purge queues.
  snd_seq_drop_output_buffer(handle);
  snd_seq_drop_output(handle);

  AllNotesOff();
}

#define FIRST_DELTACLOCK 720
#define DELTACLOCK 960
#define ADVANCE_PLAY 480

//-----------------------------------------------------------------------------
// Description:
//   Notify is periodically called by the timer.  It will output events to the
// output buffer, and update play_clock and recd_clock
//-----------------------------------------------------------------------------
void JZAlsaPlayer::Notify()
{
  // called by timer
  int Now = GetRealTimeClock();

#ifdef DEBUG_ALSA
  cout << "JZAlsaPlayer::Notify " << Now << ' ' << play_clock << endl;
#endif // DEBUG_ALSA

  if (Now < 0)
  {
    return;
  }

  if (Now < play_clock)
  {
    // rewind..
    // clear and rebuild
#ifdef DEBUG_ALSA
    cout << "JZAlsaPlayer::Notify rewind" << endl;
#endif // DEBUG_ALSA
    ResetPlay(Now);
    mPlayBuffer.Clear();
    mOutClock = Now + FIRST_DELTACLOCK;
    mpPlayLoop->PrepareOutput(&mPlayBuffer, mpSong, Now, mOutClock, 0);
    if (mpAudioBuffer)
    {
      mpAudioBuffer->Clear();
      mpPlayLoop->PrepareOutput(mpAudioBuffer, mpSong, Now, mOutClock, 1);
    }
    mPlayBuffer.Length2Keyoff();
  }
  else
  {
    // time to put more events
    if (Now >= (mOutClock - ADVANCE_PLAY))
    {
      mpPlayLoop->PrepareOutput(
        &mPlayBuffer,
        mpSong,
        mOutClock,
        Now + DELTACLOCK,
        0);
      if (mpAudioBuffer)
      {
        mpPlayLoop->PrepareOutput(
          mpAudioBuffer,
          mpSong,
          mOutClock,
          Now + DELTACLOCK,
          1);
      }
      mOutClock = Now + DELTACLOCK;
      mPlayBuffer.Length2Keyoff();
    }
  }

  play_clock = Now;
  if (
    mPlayBuffer.mEventCount &&
    mPlayBuffer.mppEvents[0]->GetClock() < mOutClock)
  {
    FlushToDevice();
  }
  else
  {
    OutBreak();        // does nothing unless mOutClock has changed
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::set_event_header(snd_seq_event_t *ev, int clock, int type)
{
  memset(ev, 0, sizeof(*ev));
  snd_seq_ev_set_source(ev, self.port);
  snd_seq_ev_set_subs(ev);
  snd_seq_ev_schedule_tick(ev, queue, 0, clock);
  snd_seq_ev_set_fixed(ev);
  ev->type = type;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::set_event_header(
  snd_seq_event_t *ev,
  int clock,
  int len,
  void *ptr)
{
  memset(ev, 0, sizeof(*ev));
  snd_seq_ev_set_source(ev, self.port);
  snd_seq_ev_set_subs(ev);
  snd_seq_ev_schedule_tick(ev, queue, 0, clock);
  snd_seq_ev_set_variable(ev, len, ptr);
}

//-----------------------------------------------------------------------------
// Description:
//   Initialize the alsa timer.
//-----------------------------------------------------------------------------
int JZAlsaPlayer::start_timer(int clock)
{
  int time_base = mpSong->GetTicksPerQuarter();
  int cur_speed = mpSong->GetTrack(0)->GetCurrentSpeed(clock);
  init_queue_tempo(time_base, cur_speed);
  start_queue_timer(clock);
  return 0;
}

//-----------------------------------------------------------------------------
// Description:
//   Set initial tempo.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::init_queue_tempo(int time_base, int bpm)
{
  snd_seq_queue_tempo_t *qtempo;
  snd_seq_queue_tempo_alloca(&qtempo);
  snd_seq_queue_tempo_set_ppq(qtempo, time_base);
  snd_seq_queue_tempo_set_tempo(qtempo, 60*1000000/bpm);
  if (snd_seq_set_queue_tempo(handle, queue, qtempo) < 0)
  {
    perror("set_queue_tempo");
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Immediately start the alsa queue timer.  Do this by sending an "start"
// event to the queue.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::start_queue_timer(int clock)
{
  stop_queue_timer(); // to be sure

  snd_seq_event_t ev;
  memset(&ev, 0, sizeof(ev));
  snd_seq_ev_set_source(&ev, self.port);
  snd_seq_ev_set_direct(&ev);
  snd_seq_ev_set_queue_pos_tick(&ev, queue, clock);
  int rv = 0;
  rv = write(&ev, 1);
  if (rv < 0)
  {
    cout << "JZAlsaPlayer::start_queue_timer write failed" << endl;
  }
  snd_seq_ev_set_queue_continue(&ev, queue);
  rv = write(&ev, 1);
  if (rv < 0)
  {
    cout << "JZAlsaPlayer::start_queue_timer write failed" << endl;
  }

  cout
    << "JZAlsaPlayer::start_queue_timer added trial-and-error start_queue"
    << endl;

  snd_seq_start_queue(handle, queue, NULL);
}

//-----------------------------------------------------------------------------
// Description:
//   Immediately stop the timer, by sending a stop event to the alsa queue.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::stop_queue_timer()
{
  snd_seq_event_t ev;
  memset(&ev, 0, sizeof(ev));
  snd_seq_ev_set_source(&ev, self.port);
  snd_seq_ev_set_direct(&ev);
  snd_seq_ev_set_queue_stop(&ev, queue);
  write(&ev, 1);
}

//-----------------------------------------------------------------------------
// Description:
//   Write an event to the queue.
//
// Returns:
//   int:
//     returns a negative value on failure.
//-----------------------------------------------------------------------------
int JZAlsaPlayer::write(snd_seq_event_t *ev, int now)
{
  if (now)
  {
    snd_seq_ev_set_direct(ev);
    return snd_seq_event_output_direct(handle, ev);
  }
  int rc = snd_seq_event_output(handle, ev);
  if (rc < 0 && rc != -EAGAIN)
  {
    snd_seq_extract_output(handle, NULL); // remove the error event
  }
  return rc;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::flush_output()
{
  snd_seq_drain_output(handle);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAlsaPlayer::set_blocking_mode(int enable)
{
  int rc;

  if ((rc = snd_seq_nonblock(handle, !enable)) < 0)
  {
    perror("blocking mode");
  }

  return rc;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::StopPlay()
{
  JZPlayer::StopPlay();
  ResetPlay(0);
  flush_output();
  stop_queue_timer();
  clear_input_queue();
  JZProjectManager::Instance()->NewPlayPosition(-1);
  mRecdBuffer.Keyoff2Length();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaPlayer::StartAudio()
{
}

//-----------------------------------------------------------------------------
// Description:
//   Called from GetRealTimeClock.  Parses events in the queue.  Sets
// recd_clock, from event time stamps.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::recd_event(snd_seq_event_t* ev)
{
  JZEvent* pEvent = 0;

#ifdef DEBUG_ALSA
  cout
    << "JZAlsaPlayer::recd_event got " << (int)ev->type
    << " (echo is " << SND_SEQ_EVENT_ECHO << ')'
    << endl;
#endif // DEBUG_ALSA

  switch (ev->type)
  {
    case SND_SEQ_EVENT_NOTEON:
      if (ev->data.note.velocity > 0)
      {
        pEvent = new JZKeyOnEvent(
          0,
          ev->data.note.channel,
          ev->data.note.note,
          ev->data.note.velocity);
      }
      else
      {
        pEvent = new JZKeyOffEvent(
          0,
          ev->data.note.channel,
          ev->data.note.note,
          0);
      }
      break;

    case SND_SEQ_EVENT_NOTEOFF:
      pEvent = new JZKeyOffEvent(
        0,
        ev->data.note.channel,
        ev->data.note.note,
        ev->data.note.velocity);
      break;

    case SND_SEQ_EVENT_PGMCHANGE:
      pEvent = new JZProgramEvent(
        0,
        ev->data.control.channel,
        ev->data.control.value);
      break;

    case SND_SEQ_EVENT_KEYPRESS:
      pEvent = new JZKeyPressureEvent(
        0,
        ev->data.note.channel,
        ev->data.note.note,
        ev->data.note.velocity);
      break;

    case SND_SEQ_EVENT_CHANPRESS:
      pEvent = new JZChnPressureEvent(
        0,
        ev->data.control.channel,
        ev->data.control.value);
      break;

    case SND_SEQ_EVENT_CONTROLLER:
      pEvent = new JZControlEvent(
        0,
        ev->data.control.channel,
        ev->data.control.param,
        ev->data.control.value);
      break;

    case SND_SEQ_EVENT_PITCHBEND:
      pEvent = new JZPitchEvent(
        0,
        ev->data.control.channel,
        ev->data.control.value);
      break;

    case SND_SEQ_EVENT_SYSEX:
      pEvent = new JZSysExEvent(
        0,
        ((unsigned char *)ev->data.ext.ptr) + 1,
        ev->data.ext.len - 1);
      break;

    case SND_SEQ_EVENT_ECHO:
      if (ev->data.raw32.d[0])
      {
        StartAudio();
      }
      else
      {
        recd_clock = ev->time.tick;
#ifdef DEBUG_ALSA
        cout << "recd_clock now: " << recd_clock << endl;
#endif // DEBUG_ALSA
      }
      break;

  }

  if (pEvent)
  {
    // Not all events are to be recorded.  Only those filtered out and put
    // into the event.
    pEvent->SetClock(mpPlayLoop->Ext2IntClock(ev->time.tick));
    mRecdBuffer.Put(pEvent);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Called periodically from Notify.  Calculates the real time clock by
// looking at time stamps on events in the queue, and also updates the
// display, so the name is not well chosen.
//-----------------------------------------------------------------------------
int JZAlsaPlayer::GetRealTimeClock()
{
  // input recorded events (including my echo events)
  snd_seq_event_t *ie;
  int old_recd_clock = recd_clock;
  while (snd_seq_event_input(handle, &ie) >= 0 && ie != 0)
  {
    recd_event(ie);
    snd_seq_free_event(ie);
  }
  if (recd_clock != old_recd_clock)
  {
    JZProjectManager::Instance()->NewPlayPosition(
      mpPlayLoop->Ext2IntClock(recd_clock / 48 * 48));
  }
  return recd_clock;
}

//-----------------------------------------------------------------------------
// Description:
//   This function goes through each client, and each port on each client.
//-----------------------------------------------------------------------------
void JZAlsaPlayer::scan_clients(
  JZAlsaDeviceList& DeviceList,
  unsigned DeviceCapabilities)
{
  snd_seq_client_info_t *cinfo;
  snd_seq_port_info_t *pinfo;

  snd_seq_client_info_alloca(&cinfo);
  snd_seq_port_info_alloca(&pinfo);

  DeviceList.Clear();

  snd_seq_client_info_set_client(cinfo, 0);
  while (snd_seq_query_next_client(handle, cinfo) >= 0)
  {
    int c = snd_seq_client_info_get_client(cinfo);
    if (c == self.client)
    {
      continue;
    }
    snd_seq_port_info_set_client(pinfo, c);
    snd_seq_port_info_set_port(pinfo, -1);
    while (snd_seq_query_next_port(handle, pinfo) >= 0)
    {
      if (
        (snd_seq_port_info_get_capability(pinfo) & DeviceCapabilities) ==
          DeviceCapabilities)
      {
        snd_seq_addr_t a = *snd_seq_port_info_get_addr(pinfo);
        char buf[500];
        strcpy(buf, snd_seq_client_info_get_name(cinfo));
        strcat(buf, " ");
        strcat(buf, snd_seq_port_info_get_name(pinfo));
        DeviceList.add(buf, a);
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAlsaPlayer::FindMidiDevice()
{
  if (mInputDeviceIndex != -1)
  {
    unsubscribe_inp(mInputDeviceIndex);
  }
  mInputDeviceIndex = select_list(
    iaddr,
    "Input MIDI device",
    mInputDeviceIndex);
  gpConfig->Put(C_AlsaInputDevice, mInputDeviceIndex);
  if (mInputDeviceIndex != -1)
  {
    subscribe_inp(mInputDeviceIndex);
  }

  if (mOutputDeviceIndex != -1)
  {
    unsubscribe_out(mOutputDeviceIndex);
  }
  mOutputDeviceIndex = select_list(
    oaddr,
    "Output MIDI device",
    mOutputDeviceIndex);
  gpConfig->Put(C_AlsaOutputDevice, mOutputDeviceIndex);
  if (mOutputDeviceIndex != -1)
  {
    subscribe_out(mOutputDeviceIndex);
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAlsaPlayer::select_list(
  JZAlsaDeviceList& list,
  const char* title,
  int def_device)
{
  if (list.GetCount() > 0)
  {
    int ndevs = list.GetCount();
    wxString devs[ndevs];

    for (int i = 0; i < ndevs; i++)
    {
      devs[i] = list.GetName(i);
    }

    wxSingleChoiceDialog dialog(gpTrackWindow, title, title, ndevs, devs);

    if (def_device != -1)
    {
      dialog.SetSelection(def_device);
    }

    int res = dialog.ShowModal();
    int k = dialog.GetSelection();

    if (res == wxCANCEL)
    {
      k = -1;
    }

    return k;
  }
  else
  {
    cerr << "INFO: No device found!" << endl;
    return -1;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAlsaDeviceList::AsciiWrite(const string& Message)
{
  cout << Message << endl;
  int i = 0;
  for (
    vector<snd_seq_addr_t>::const_iterator iSound = addr.begin();
    iSound != addr.end();
    ++iSound)
  {
    const snd_seq_addr_t& a = *iSound;
    cout
      << GetName(i++) << " = " << (int)a.client << ":" << (int)a.port
      << endl;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned JZAlsaDeviceList::add(const char* pName, const snd_seq_addr_t& a)
{
  mDeviceNames.push_back(pName);
  addr.push_back(a);
  return addr.size();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
snd_seq_addr_t& JZAlsaDeviceList::operator[](unsigned i)
{
  if (i >= addr.size())
  {
    return addr[0];
  }
  return addr[i];
}
