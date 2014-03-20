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

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <alsa/asoundlib.h>
#include <unistd.h>

//
// define this if use kernel mode connection:
// it may leave connection between thru ports if jazz quits
// abnormally.  in such a case, disconnect manually via aconnect
// utility.

#define USE_DIRECT_CONNECTION


// does not work together with wxwin $%!
// #include <pthread.h>

class JZAlsaThru
{
  public:

    JZAlsaThru();
    virtual ~JZAlsaThru();

    void SetSource(int client, int port)
    {
      source.client = client;
      source.port   = port;
    }
    void SetDestin(int client, int port)
    {
      destin.client = client;
      destin.port   = port;
    }
    void GetSource(int &client, int &port)
    {
      client = source.client;
      port   = source.port;
    }
    void GetDestin(int &client, int &port)
    {
      client = destin.client;
      port   = destin.port;
    }
    void Start();
    void Stop();
    int  IsRunning() const
    {
      return running;
    }

  private:

    snd_seq_t *handle;

    snd_seq_addr_t self;
    snd_seq_addr_t source;
    snd_seq_addr_t destin;

    void connect(snd_seq_addr_t &src, snd_seq_addr_t &dest);
    void disconnect(snd_seq_addr_t &src, snd_seq_addr_t &dest);
    void initialize();
    void loop();
    void midithru();
    static void * startworker(void *p);
    static void stopworker(int signum);

    // pthread_t worker;
    pid_t worker;
    int running;
};
