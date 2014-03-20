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
// 2000.03.18    Takashi Iwai <iwai@ww.uni-erlangen.de>
// - Modified for ALSA 0.5.x
// - Thru-connection is done inside ALSA sequencer core if
//   USE_DIRECT_CONNECTION is defined in alsathru.h.
//   It reduces latency.
//   However, if jazz program is terminated abnormally by ctrl-c,
//   or signal, the established connection will remain.
//   You'll need to disconnect it via aconnect program (included
//   in alsa-utils) manually.
//   If USE_DIRECT_CONNECTION is not defined (as default), soft-
//   thru is done.  The event goes thru via either forked child
//   or a thread.
//*****************************************************************************

#include "AlsaThru.h"

#include "AlsaPlayer.h"

#include <cstdlib>
#include <errno.h>
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

/*
** midi thru for alsa. it creates a new process (because threads dont work
** with wxwin) that copies from input to output.
*/

JZAlsaThru::JZAlsaThru()
{
  running = 0;
}


JZAlsaThru::~JZAlsaThru()
{
  // Calling Stop() caused the creation of an unkillable process on Mandriva
  // 2008.0.
//  if (running)
//    Stop();
}


void JZAlsaThru::connect(snd_seq_addr_t &src, snd_seq_addr_t &dest)
{
  snd_seq_port_subscribe_t *subs;
  snd_seq_port_subscribe_alloca(&subs);
  snd_seq_port_subscribe_set_sender(subs, &src);
  snd_seq_port_subscribe_set_dest(subs, &dest);
  if (snd_seq_subscribe_port(handle, subs) < 0)
    perror("subscribe");
}

void JZAlsaThru::disconnect(snd_seq_addr_t &src, snd_seq_addr_t &dest)
{
  snd_seq_port_subscribe_t* subs;
  snd_seq_port_subscribe_alloca(&subs);
  snd_seq_port_subscribe_set_sender(subs, &src);
  snd_seq_port_subscribe_set_dest(subs, &dest);
  if (snd_seq_unsubscribe_port(handle, subs) < 0)
  {
    perror("unsubscribe");
  }
}

void JZAlsaThru::initialize()
{
  if (snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0 ) < 0)
  {
    perror("open");
    exit(1);
  }
  JZAlsaPlayer::set_client_info(handle, "Jazz++ Midi Thru");

  if (snd_seq_nonblock(handle, 0) < 0)
  {
    perror("blocking mode");
    exit(1);
  }

  self.client  = snd_seq_client_id(handle);
  self.port    = JZAlsaPlayer::create_port(handle, "Input/Output");
#ifndef USE_DIRECT_CONNECTION
  connect(source, self);
  connect(self, destin);
#endif
}


#ifdef USE_DIRECT_CONNECTION
void JZAlsaThru::Start()
{
  if (! running)
  {
    initialize();
    connect(source, destin);
    running = 1;
    snd_seq_close(handle);
  }
}

// disconnect midi-thru
void JZAlsaThru::Stop()
{
  if (running)
  {
    initialize();
    disconnect(source, destin);
    running = 0;
    snd_seq_close(handle);
  }
}

#else // USE_DIRECT_CONNECTION

void JZAlsaThru::loop()
{
  snd_seq_event_t *ev;
  while (snd_seq_event_input(handle, &ev) >= 0 && ev != 0)
  {
    if (ev->source.client == source.client && ev->source.port == source.port)
    {
      ev->flags &= ~SND_SEQ_TIME_STAMP_MASK;
      ev->flags |= SND_SEQ_TIME_STAMP_TICK;
      ev->time.tick = 0;
      ev->source = self;
      snd_seq_ev_set_subs(ev);
      snd_seq_ev_set_direct(ev);
      snd_seq_event_output_direct(handle, ev);
    }
    snd_seq_free_event(ev);
  }
}

#if 0

// thread version

void JZAlsaThru::stopworker(int sig)
{
  running = 0;
  snd_seq_close(handle);
  pthread_exit((void *)0);
}


void * JZAlsaThru::startworker(void *p)
{
  running = 1;
  signal(SIGHUP, stopworker);
  JZAlsaThru *thru = (JZAlsaThru *)p;
  thru->initialize();
  thru->loop();
  return 0;
}

void JZAlsaThru::Start()
{
  if (!running)
    pthread_create(&worker, (void *)0, startworker, (void *)this);
}

void JZAlsaThru::Stop()
{
  if (running)
    pthread_kill(worker, SIGHUP);
}



#else

// fork version

static snd_seq_t *static_handle;  // ugly!!

void JZAlsaThru::stopworker(int sig)
{
  snd_seq_close(static_handle);
  exit(0);
}

void JZAlsaThru::Start()
{
  if (!running)
  {
    worker = fork();
    if (worker < 0)
    {
      perror("fork");
      exit(1);
    }
    if (worker == 0)
    {
      signal(SIGHUP, stopworker);
      initialize();
      loop();
      static_handle = handle;
      // never come here
      perror("loop");
      exit(1);
    }
    running = 1;
  }
}

void JZAlsaThru::Stop()
{
  if (running)
  {
    int status = 0;
    kill(worker, SIGHUP);
    wait(&status);
    running = 0;
  }
}

#endif

#endif // !USE_DIRECT_CONNECTION
