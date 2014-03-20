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

#include "Random.h"
#include "SignalInterface.h"
#include "Sample.h"

class JZSample;
class JZFloatSample;
class JZSplFilter;

/**
 * a paintable command with n array arguments
 */

class JZPaintableCommand
{
  public:
    JZPaintableCommand(JZSample &s) : spl(s)
    {
    }
    virtual int NumArrays() = 0;
    virtual JZRndArray & GetArray(int i) = 0;
    virtual const char * GetLabel(int i) = 0;
    virtual void Execute(long fr, long to) = 0;
    virtual void Initialize()
    {
    }
    virtual ~JZPaintableCommand()
    {
    }
  protected:
    JZSample &spl;
};

/**
 * a paintable command with 1 array
 */

class JZPaintableCommand1 : public JZPaintableCommand
{
  public:
    JZPaintableCommand1(JZSample &s, int num, int min, int max)
      : JZPaintableCommand(s),
        arr(num, min, max)
    {
    }
    virtual int NumArrays()
    {
      return 1;
    }
    virtual JZRndArray & GetArray(int i)
    {
      return arr;
    }
    virtual void Initialize();
  protected:
    JZRndArray arr;
};


class JZSplVolume : public JZPaintableCommand1
{
  public:
    const char * GetLabel(int i)
    {
      return "volume";
    }
    JZSplVolume(JZSample &s) : JZPaintableCommand1(s, 200, -100, 100)
    {
    }
    void Execute(long fr, long to);
};

class JZSplPan : public JZPaintableCommand1
{
  public:
    const char * GetLabel(int i)
    {
      return "pan";
    }
    JZSplPan(JZSample &s) : JZPaintableCommand1(s, 200, -100, 100)
    {
    }
    void Execute(long fr, long to);
};

class JZSplPitch : public JZPaintableCommand1
{
  public:
    const char * GetLabel(int i)
    {
      return "pitch";
    }
    JZSplPitch(JZSample &s) : JZPaintableCommand1(s, 200, -100, 100)
    {
      range = 1.2f;
    }
    void Execute(long fr, long to);
    // range = frequency factor, e.g. 2 will transpose on octave up or down
    void SetRange(float r)
    {
      range = r;
    }
  private:
    float range;
};



// ----------------------------------------------------------------
//                       CMIX interface
// ----------------------------------------------------------------
#define PI   3.14159265358979323846

#ifdef MSVC
#define      PI2    (2.0*PI)
#define      M_PI   PI
#define      M_PI_2 (PI/2.0)
#endif

class JZCMixCmd
{
  public:
    JZCMixCmd(float sr);   // sampling rate
    float cpspch(float pch);
    void tableset(float dur, int size, float *tab);
    float tablei(long nsample, float *array, float *tab);
    void setline(const float *p, short n_args,int length,float *array);
  protected:
    float SR;
    int resetval;

    enum
    {
      SIZE = 512
    };
    float array[SIZE];
    float tabs[2];        /* for lineset */
    int lineset;
};


class JZWahWah : public JZPaintableCommand1
{
  friend class JZWahSettingsForm;
  public:
    JZWahWah(JZSample &s);
    virtual const char * GetLabel(int i)
    {
      return "freq";
    }
    virtual void Execute(long fr, long to);
    virtual void Initialize();
  protected:
    void Wah(int channel, JZFloatSample &fs);
    JZSplFilter::Type filter_type;
    int order;
    double lo_freq;
    double hi_freq;
    double band_width;
};


class JZShifterCmd : public JZCMixCmd
{
  public:
    JZShifterCmd(long sampling_rate);

    void ShiftPitch(JZSample &spl, float semis,  bool keep_length, float winsize);
    void StretchLength(JZSample &spl, long newlen, bool keep_pitch, float winsize);

  protected:
    double rotate(float p[], int n_args, JZFloatSample &sinp, JZFloatSample &sout);
};


// 2-nd nogo equalizer
class JZSplEqualizer
{
  public:
    JZSplEqualizer(JZRndArray &amps, long sampling_rate);
    virtual ~JZSplEqualizer();
    float operator()(float sample);
    float Index2Hertz(float index);
    void  Prepare();
  private:
    JZRndArray  &array;
    JZSplFilter *filters;
    int       nfilters;
    long      sampling_rate;

};
