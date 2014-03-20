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

#include "Player.h"
#include "AlsaThru.h"

#include <alsa/asoundlib.h>

#include <string>

class JZAlsaDeviceList : public JZDeviceList
{
  public:
    unsigned add(const char* pName, const snd_seq_addr_t& a);
    snd_seq_addr_t& operator[](unsigned i);
    void AsciiWrite(const std::string& Message);
  private:
    std::vector<snd_seq_addr_t> addr;
};



class JZAlsaPlayer : public JZPlayer
{
  friend class JZAlsaThru;
  public:
    JZAlsaPlayer(JZSong *song);
    virtual ~JZAlsaPlayer();

    void Notify();
    bool IsInstalled();
    int  OutEvent(JZEvent *e, int now);
    int  OutEvent(JZEvent *e)
    {
      return OutEvent(e, 0);
    }
    void OutNow(JZEvent *e)
    {
      OutEvent(e, 1);
    }
    void OutNow(JZParam *r);
    void OutBreak();
    void OutBreak(int BreakOver);
    void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);
    void StopPlay();
    int GetRealTimeClock();
    virtual void SetSoftThru(int on, int idev, int odev);
    virtual int SupportsMultipleDevices()
    {
      return 1;
    }
    virtual JZDeviceList & GetOutputDevices()
    {
      return oaddr;
    }
    virtual JZDeviceList & GetInputDevices()
    {
      return iaddr;
    }
    virtual int GetThruInputDevice()
    {
      return ithru;
    }
    virtual int GetThruOutputDevice()
    {
      return othru;
    }
    int FindMidiDevice();

  protected:
    snd_seq_t *handle;
    JZAlsaDeviceList iaddr;        // addresses of input devices
    JZAlsaDeviceList oaddr;        // addresses of output devices
    int client;            // me
    snd_seq_addr_t self;   // my address
    int queue;             // queue
    int mInputDeviceIndex, mOutputDeviceIndex;
    int sync_in, sync_in_dev, sync_in_mtcType;
    int sync_out, sync_out_dev, sync_out_mtcType;

    bool mInstalled;

    static int create_port(snd_seq_t *handle, const char *name);
    static void set_client_info(snd_seq_t *handle, const char *name);
    void subscribe_inp(int dev);
    void unsubscribe_inp(int dev);
    void subscribe_out(int dev);
    void unsubscribe_out(int dev);
    void thru_connect();
    void thru_disconnect();
    void scan_clients(
      JZAlsaDeviceList& Devicelist,
      unsigned DeviceCapabilities);
    int select_list(JZAlsaDeviceList &list, const char *title, int def_device);
    int  start_timer(int clock);
    int write(snd_seq_event_t *ev)
    {
      // 0 == ok
      return write(ev, 0);
    }
    int write(snd_seq_event_t *ev, int now); // 0 == ok
    void set_event_header(snd_seq_event_t *ev, int clock, int type);
    void set_event_header(snd_seq_event_t *ev, int clock, int len, void *ptr);
    void init_queue_tempo(int time_base, int bpm);
    void start_queue_timer(int clock);
    void stop_queue_timer();
    void recd_event(snd_seq_event_t *ev);
    void flush_output();
    int  set_blocking_mode(int enable);
    void clear_input_queue();
    void purge_queues();
    void set_pool_sizes();
    int compose_echo(int clock, unsigned int arg = 0);
    virtual void StartAudio();
    virtual void ResetPlay(int clock);
    int sync_master();
    void sync_master_remove();
    int sync_slave();
    void sync_slave_remove();

    int play_clock;   // current clock
    int recd_clock;  // clock received so far from recorded events or echo events
    int echo_clock;  // echo events have been sent up to this clock

    JZAlsaThru *thru;
    int ithru, othru;  // index in iaddr, oaddr of source/target device
};
