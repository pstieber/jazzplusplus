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

class JZRndArray;
class JZSample;

//*****************************************************************************
// interface header for signal library
//*****************************************************************************

void sig_chorus(
  JZSample& spl,
  float pitch_freq,   // pitch modification speed
  float pitch_range,  // 1 = one octave
  float pan_freq,     // in Hz
  float pan_spread,   // 0..1
  float volume        // -1..1
);

void sig_reverb(
  JZSample& spl,
  float rvbtime_val, // echo absorbtion
  float bright_val,  // lowpass filter freq
  float volume_val,  // effect volume
  float room_val);   // echo delay

void sig_wavsynth(
  JZSample& spl,         // destin
  double durat,         // length in seconds
  int midi_key,         // base freq
  double fshift,        // frequeny modulation factor
  int N,                // number of wavetables to be mixed
  JZRndArray* arr[][4], // (fft, vol, pitch, pan) * N
  int noisegen          // first array is noise filter
);


// virtual version of JZOpLowpass, JZOpHighpass, etc classes

class JZOpFilter;

class JZSplFilter
{
  public:

    JZSplFilter();

    ~JZSplFilter();

    enum Type
    {
      LOWPASS,
      HIGHPASS,
      BANDPASS,
      BANDSTOP
    };

    // bw = bandwidth relative to f0 for bandpass/bandstop
    void Init(Type t, float sr, double f0, double bw = 0.0);
    void ReInit(double f0, double bw = 0.0);
    float Loop(float sample);

  protected:

    JZOpFilter* filter;
    float sr;
};
