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

#include "SampleCommand.h"

#include "Audio.h"
#include "Mapper.h"
#include "Sample.h"
#include "SignalInterface.h"
//#include "util.h"

#include <cmath>
#include <iostream>

using namespace std;

#define db(a) cout << #a << " = " << a << endl


void JZPaintableCommand1::Initialize()
{
  for (int i = 0; i < arr.Size(); i++)
    arr[i] = 0;
}

/**
 * adjust volume graphically. Compute in float because
 * new volume may exceed 32767
 */

void JZSplVolume::Execute(long fr, long to)
{
  long n = to - fr;
  if (n <= 0)
    return; // no data

  JZFloatSample fs(spl);
  float i_fact  = (float)arr.Size() / (float)n;
  for (long i = 0; i < n; i++)
  {
    float x = (float)i * i_fact;
    fs[fr+i] *= (100.0 + arr[x]) / 100.0;
  }
  fs.RescaleToShort();
  spl.Set(fs);
}


/**
 * adjust pan graphically, that is decrease volume of
 * one channel.
 */

void JZSplPan::Execute(long fr, long to)
{
  long n = to - fr;
  if (n <= 0)
    return; // no data

  float i_fact  = (float)arr.Size() / (float)n;
  short* pData = spl.mpData;
  for (long i = 0; i < n-1; i += 2)
  {
    float x = (float)i * i_fact;
    float p = arr[x];
    if (p < 0)
      pData[fr+i]   = (short)((float)pData[fr+i]   * (100 + p) / 100);
    else
      pData[fr+i+1] = (short)((float)pData[fr+i+1] * (100 - p) / 100);
  }
}


/**
 * adjust pitch graphically
 */

void JZSplPitch::Execute(long fr, long to)
{
  long n = to - fr;
  if (n <= 0)
    return; // no data

  long channels = spl.GetChannelCount();
  float N       = spl.length / channels - 2;
  short* pData   = spl.mpData;

  JZFloatSample out(channels, spl.GetSamplingRate());
  out.SetNote(0, 0);

  float p[10];
  JZMapper arr_ind(0, N, 0, arr.Size());
  JZExponentialMapper arr_val(100, range);

  float x = 0;
  while (x < N)
  {
    float ofs = floor(x);
    float rem = x - ofs;

    long i = (long)ofs * channels;
    for (long c = 0; c < channels; c++)
    {
      JZMapper Mapper(0, 1, pData[i + c], pData[i + channels + c]);
      p[c] = Mapper.XToY(rem);
    }
    out.AddOut(p);
    x += arr_val.XToY(arr[arr_ind.XToY(x)]);
  }
  out.EndNote();
  out.ClipToCurrent();
  spl.Set(out);
}


// ----------------------------------------------------------------
//                       small CMIX interface
// ----------------------------------------------------------------

JZCMixCmd::JZCMixCmd(float sr)
  : SR(sr)
{
  int i;
  resetval = 1000;
  for (i = 0; i < SIZE; i++)
    array[i] = 1;
  lineset = 0;
}

void JZCMixCmd::tableset(float dur, int size, float *tab)
{
  *tab = (long)(dur * SR  -.9999);
  *(tab+1) = size - 1;
}


float JZCMixCmd::tablei(long nsample, float *array, float *tab)
{
        register int loc1,loc2;
        float frac = ((float)(nsample)/(*tab)) * *(tab+1);
        if (frac < 0)
        {
          return(array[0]);
        }
        if (frac >= *(tab+1))
        {
          return(array[(int)*(tab+1)]);
        }
        loc1 = (int)frac;
        loc2 = loc1+1;
        frac = frac - (float)loc1;
        return(*(array+loc1) + frac * (*(array+loc2) - *(array+loc1)));
}

/* p0,2,4,5,6,8,10.. are times, p1,3,5,7,9,11.. are amps, total number of
 * arguments is n_args, result is stuffed into array array of length length
 */

void JZCMixCmd::setline(const float *p, short n_args,int length,float *array)
{
  double increm;
  int i,j,k,points;

  increm = (double)(p[n_args - 2] - p[0])/(double)length;
  for (j = 0, i = 0; j < (n_args - 2); j += 2)
    {
      points = (int)((double)(p[j+2] - p[j]) / increm +.5);
      if (p[j+2] != p[j])
      {
        if (points <= 0)
        {
          points = 1;
        }
        for (k=0; k < points; k++)
        {
          array[i++] = ((float)k/(float)points) * (p[j+3] - p[j+1]) + p[j+1];
          if (i == length)
          {
            return;
          }
        }
    }
  }
  i--;
  while (++i < length)
  {
    array[i] = array[i-1];
  }
}


float JZCMixCmd::cpspch(float pch)
{
  int oct = (int)pch;
  return (float) ((pow(2.,oct+8.333333*(pch-oct))*1.021975));
}

// ------------------------ wahwah ------------------------

JZWahWah::JZWahWah(JZSample &s)
: JZPaintableCommand1(s, 200, 0, 100)
{
  for (int i = 0; i < arr.Size(); i++)
    arr[i]  = i * 100 / arr.Size();
  filter_type = JZSplFilter::BANDPASS;
  order       = 2;
  lo_freq     = 400;
  hi_freq     = 2000;
  band_width  = 0.2;
}



void JZWahWah::Initialize()
{
}


void JZWahWah::Execute(long fr, long to)
{
  JZFloatSample *out = new JZFloatSample(spl);
  for (int c = 0; c < out->GetChannelCount(); c++)
    Wah(c, *out);
  out->Rescale();
  spl.Set(*out);
  delete out;
}



void JZWahWah::Wah(int channel, JZFloatSample &out)
{
  long N = spl.GetLength();
  float SR = spl.GetSamplingRate();
  long channels = spl.GetChannelCount();
  JZMapper XMap(0, N, 0, arr.Size());
  JZMapper fmap(0, 100, lo_freq, hi_freq);

  JZSplFilter flt;
  {
    float f = fmap.XToY(arr[0]);
    flt.Init(filter_type, SR, f, band_width);
  }
  int j = 0;
  short* pData = spl.GetData();
  for (long i = channel; i < N; i += channels)
  {
    if (j-- == 0)
    {
      float x = (float)XMap.XToY(i);
      float f = fmap.XToY(arr[x]);
      flt.ReInit(f, band_width);
      j = 100;
    }
    out[i] = flt.Loop(pData[i]);
  }
}


// ------------------------ rotater ------------------------

JZShifterCmd::JZShifterCmd(long sampling_rate)
  : JZCMixCmd(sampling_rate)
{
}

/**
 * change the pitch of a sample. If keep_length == true, winsize should
 * be in 0..100
 */

void JZShifterCmd::ShiftPitch(JZSample &spl, float semis,  bool keep_length, float winsize)
{
  if (semis == 0)
    return;

  if (keep_length)
  {
    JZMapper wmap(0, 100, 0.05, 0.3);
    JZFloatSample inp(spl);
    JZFloatSample out(inp.GetChannelCount(), inp.GetSamplingRate());
    float p[8];
    p[0] = 0;
    p[1] = 0;
    p[2] = spl.Samples2Seconds(spl.GetLength());
    p[3] = 1.0;
    p[4] = semis / 100.0;
    p[5] = (float)wmap.XToY(winsize);
    p[6] = 0;
    p[7] = 0;
    rotate(p, 8, inp, out);
    if (inp.GetChannelCount() == 2)
    {
      p[6] = 1;
      p[7] = 1;
      rotate(p, 8, inp, out);
    }
    out.ClipToCurrent();
    spl.Set(out);
  }
  else
    spl.TransposeSemis(semis);
}

void JZShifterCmd::StretchLength(JZSample &spl, long newlen, bool keep_pitch, float winsize)
{
  long oldlen = spl.GetLength();
  if (oldlen == newlen)
    return;

  double df = (double)oldlen / (double)newlen;
  // msvc cannot do this!
  // float  semis = (float)(log(df) / log( pow(2.0, 1.0/12.0) ));
  float semis = (float)( log(df) / 0.057762264 );
  spl.TransposeSemis(semis);

  if (keep_pitch)
  {
    JZMapper wmap(0, 100, 0.05, 0.3);
    JZFloatSample inp(spl);
    JZFloatSample out(inp.GetChannelCount(), inp.GetSamplingRate());
    float p[8];
    p[0] = 0;
    p[1] = 0;
    p[2] = spl.Samples2Seconds(spl.GetLength());
    p[3] = 1.0;
    p[4] = -semis / 100.0;
    p[5] = (float)wmap.XToY(winsize);
    p[6] = 0;
    p[7] = 0;
    rotate(p, 8, inp, out);
    if (inp.GetChannelCount() == 2)
    {
      p[6] = 1;
      p[7] = 1;
      rotate(p, 8, inp, out);
    }
    out.ClipToCurrent();
    spl.Set(out);
  }
}


/*  rotate -- a pitch-shifting instrument based upon the idea
*        of old rotating tape-head pitch shifters
*
*  p0 = input skip
*  p1 = output skip
*  p2 = duration
*  p3 = amplitude multiplier (switched off - av)
*  p4 = pitch shift up or down (oct.pc)
*  p5 = window size
*  p6 = input channel number
*  p7 = stereo spread (0-1) [optional]
*  assumes function table 1 is the amplitude envelope  (switched off - av)
*  assumes function table 2 is the window envelope     (inline - av)
*        <usually a hanning window -- use "makegen(2, 25, 1000, 1)">
*
*/


double JZShifterCmd::rotate(float p[], int n_args, JZFloatSample &sinp, JZFloatSample &sout)
{
  float samplenum1,samplenum2,x,interval;
  float val1,val2,in[2],out[2];
  int nsamps,i,j,k,off,reinit,chans,inchan;
  int octpart;
  float pcpart;
  float *wintable;
  int wlen;

  sinp.SetNote(p[0], p[2]);
  nsamps = sout.SetNote(p[1], p[2]);

  JZFloatSample hanning(1, sout.GetSamplingRate());
  hanning.HanningWindow(1000);
  wlen = 1000;
  wintable = hanning.GetData();

  octpart = (int)p[4] * 12;
  pcpart = (p[4] * 100.0) - (float)(octpart*100);
  interval =  pow(2.0, ((float)octpart + pcpart)/12.0) - 1.0;

  reinit = (int)(p[5] * SR);
  off = reinit/2;
  k = off;
  chans = sout.GetChannelCount();
  inchan = (int)p[6];
  j = 0;
  for (i = 0; i < nsamps; i++)
  {
    j = (j+1) % reinit;
    k = (k+1) % reinit;

    samplenum1 = (float)i + (float)j * interval;
    if (!sinp.GetSample(samplenum1, in))
    {
      break;
    }
    x = wintable[(int)(((float)j/reinit) * wlen)];
    val1 = in[inchan] * x;

    samplenum2 = (float)(i) + (float)(k-off) * interval;
    if (!sinp.GetSample(samplenum2, in))
    {
      break;
    }
    x = wintable[(int)(((float)k/reinit) * wlen)];
    val2 = in[inchan] * x;

    out[0] = (val1 + val2);
    if (chans > 1)
    {
      out[1] = (1.0 - p[7]) * out[0];
      out[0] *= p[7];
    }

    sout.AddOut(out);
  }

  sout.EndNote();
  return 0.0;
}


// -------------------------------------------------------------------------
//                        2-nd nogo equalizer
// -------------------------------------------------------------------------

JZSplEqualizer::JZSplEqualizer(JZRndArray &arr, long sr)
  : array(arr),
    sampling_rate(sr)
{
  nfilters = array.Size();
  filters = new JZSplFilter[nfilters];
}

void JZSplEqualizer::Prepare()
{
  int i;

  JZSplFilter::Type type;
  double sr = (double)sampling_rate;

  // first one is lowpass/highpass
  type = array[0] > 0 ? JZSplFilter::LOWPASS : JZSplFilter::HIGHPASS;
  filters[0].Init(type, sr, Index2Hertz(0), 0);
  // some band pass filters
  for (i = 1; i < nfilters-1; i++)
  {
    double f  = Index2Hertz(i);
    double f0 = Index2Hertz(i-1);
    double f1 = Index2Hertz(i+1);
    //double bw = (1 - f0/f1) / (1 + f0/f1);
    double bw = (f1 - f0) / f;
    type = array[i] > 0 ? JZSplFilter::BANDPASS : JZSplFilter::BANDSTOP;
    filters[i].Init(type, sr, f, bw);
  }
  // last one is high/low pass
  type = array[nfilters-1] > 0 ? JZSplFilter::HIGHPASS : JZSplFilter::LOWPASS;
  filters[nfilters-1].Init(type, sr, Index2Hertz(nfilters-1), 0);
}


JZSplEqualizer::~JZSplEqualizer()
{
  delete [] filters;
}


float JZSplEqualizer::Index2Hertz(float index)
{
  JZMapper XMap(0, nfilters, -1, 1);
  JZExponentialMapper ExponentialMapper(1, 10);
  return 2000.0 * ExponentialMapper.XToY(XMap.XToY(index));
}

float JZSplEqualizer::operator()(float x)
{
  // add all the outputs
  double y = x;
  JZMapper ymap(array.GetMin(), array.GetMax(), -1, 1);
  for (int i = 0; i < nfilters; i++)
  {
    double amp = fabs(ymap.XToY(array[i]));
    y = (1 - amp) * y + amp * filters[i].Loop(y);
  }
  return (float)y / (float)nfilters;
}
