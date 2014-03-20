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

#include "Sample.h"

#include "Audio.h"
#include "Random.h"
#include "SampleCommand.h"
#include "Mapper.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include <sys/stat.h>

using namespace std;

JZSample::JZSample(JZSampleSet &s)
  : set(s),
    mLabel(),
    mFileName()
{
  mpData   = 0;
  length   = 0;
  external_flag = 1;  // auto reload when file changes on disk
  external_time = 0;
  volume   = 127;
  pan      = 0;
  pitch    = 0;
  dirty    = 0;
}


JZSample::~JZSample()
{
  delete [] mpData;
}

void JZSample::SetLabel(const std::string& Label)
{
  mLabel = Label;
}

void JZSample::SetFileName(const string& FileName)
{
  if (mFileName != FileName)
  {
    dirty = 1;
    mLabel = wxFileNameFromPath(FileName.c_str());
  }
  mFileName = FileName;
}

void JZSample::Clear()
{
  FreeData();
  mLabel.clear();
  mFileName.clear();
  volume = 127;
  pan    = 0;
  pitch  = 0;
  dirty  = 0;
}

void JZSample::FreeData()
{
  delete [] mpData;
  mpData = 0;
  length = 0;
  dirty  = 1;
}

void JZSample::MakeData(int new_length, int zero)
{
  delete [] mpData;
  length = new_length;
  mpData = new short[length];
  if (zero)
    memset(mpData, 0, length * sizeof(short));
}


void JZSample::Set(JZFloatSample &fs)
{
  MakeData(fs.GetLength());
  for (int i = 0; i < length; i++)
  {
    mpData[i] = (short)fs[i];
  }
}


void JZSample::Set(JZFloatSample &fs, int offs)
{
  int len = fs.GetLength();
  AssureLength(offs + len);
  for (int i = 0; i < len; i++)
  {
    mpData[offs + i] = (short)fs[i];
  }
}

/**
 * like Set(JZFloatSample, ofs) but makes a smooth fade in / fade out
 */

void JZSample::SetSmooth(JZFloatSample &fs, int offs, int fade)
{
  int len = fs.GetLength();
  AssureLength(offs + len);

  if (fade < 0)
    fade = fs.sampling_rate / 100;  // 10 millisec

  if (fade <= 0 || fade >= fs.length/2)
    return;

  int i = 0;
  int ofs1 = fade;
  int ofs2 = fs.length - fade;
  int ofs3 = fs.length;
  JZMapper fi(0, fade, 0, 1);
  JZMapper fo(0, fade, 1, 0);

  while (i < ofs1)
  {
    mpData[offs + i] = (short)(fi.XToY(i) * fs[i] + fo.XToY(i) * mpData[offs + i]);
    i++;
  }
  while (i < ofs2)
  {
    mpData[offs + i] = (short)fs[i];
    i++;
  }
  while (i < ofs3)
  {
    mpData[offs + i] = (short)(fo.XToY(i-ofs2) * fs[i] + fi.XToY(i-ofs2) * mpData[offs + i]);
    i++;
  }
}

#if 0
int JZSample::LoadWav()
{
  struct stat buf;
  if (stat(mFileName.c_str(), &buf) == -1)
  {
    perror(mFileName.c_str());
    return 1;
  }
  external_time = buf.st_mtime;

  // read and check header info
  ifstream is(mFileName.c_str(), ios::binary | ios::in);
  WaveHeader wh;
  memset(&wh, 0, sizeof(wh));
  is.read((char *)&wh, sizeof(wh));
  if (wh.main_chunk  != RIFF
    || wh.chunk_type != WAVE
    || wh.sub_chunk  != FMT
    || wh.data_chunk != DATA)
  {
    //fprintf(stderr, "%s format not recognized\n", mFileName.c_str());
    return 2;
  }
  if (wh.format != PCM_CODE)
  {
    //fprintf(stderr, "%s must be PCM_CODE\n", mFileName.c_str());
    return 3;
  }

  int channels = (wh.modus == WAVE_STEREO) ? 2 : 1;
  return Convert(is, wh.data_length, channels, wh.bit_p_spl, wh.sample_fq);
}

#else

typedef struct
{
  char main_type[4]; // 'RIFF'
  int length;        // filelen
  char sub_type[4];  // 'WAVE'
} RIFFHeader;

typedef struct
{
  char type[4];         // 'fmt '
  unsigned int length; // length of sub_chunk, = 16
} ChunkHeader;

typedef struct
{
  unsigned short format;     // should be 1 for PCM-code
  unsigned short modus;      // 1 Mono, 2 Stereo
  unsigned int sample_fq;        // frequence of sample
  unsigned int byte_p_sec;
  unsigned short byte_p_spl; // samplesize; 1 or 2 bytes
  unsigned short bit_p_spl;  // 8, 12 or 16 bit
} FmtChunk;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSample::LoadWav()
{
  struct stat buf;
  if (stat(mFileName.c_str(), &buf) == -1)
  {
    perror(mFileName.c_str());
    return 1;
  }
  external_time = buf.st_mtime;

  ChunkHeader ch;

  // read and check header info
  ifstream Ifs(mFileName.c_str(), ios::binary | ios::in);
  RIFFHeader rh;
  Ifs.read((char *)&rh, sizeof(rh));
  if (strncmp(rh.main_type, "RIFF", 4) || strncmp(rh.sub_type, "WAVE", 4))
    return 2;

  int channels = 0;
  int data_length = 0;
  unsigned short bit_p_spl = 0;
  unsigned short sample_fq = 0;

  while (rh.length > 0)
  {
    Ifs.read((char *)&ch, sizeof(ch));
    rh.length -= sizeof(ch);
    ios::pos_type Position = Ifs.tellg();

    // "fmt " chunk
    if (strncmp(ch.type, "fmt ", 4) == 0)
    {
      FmtChunk fc;
      Ifs.read((char *)&fc, sizeof(fc));
      if (fc.format != PCM_CODE)
      {
        return 2;
      }
      channels = (fc.modus == WAVE_STEREO) ? 2 : 1;
      bit_p_spl = fc.bit_p_spl;
      sample_fq = fc.sample_fq;
    }

    // "data" chunk
    if (strncmp(ch.type, "data", 4) == 0)
    {
      data_length = ch.length;
      break;
    }

    // skip to beginning of next chunk
    rh.length -= ch.length;
    Position += ch.length;
    Ifs.seekg(Position);
  }

  if (!data_length || !bit_p_spl)
  {
    return 2;
  }

  return Convert(Ifs, data_length, channels, bit_p_spl, sample_fq);
}

#endif


int JZSample::Convert(
  istream& is,
  int bytes,
  int channels,
  int bits,
  int speed)
{

  // load the file
  length = bytes / 2;
  delete [] mpData;
  mpData = new short [length];
  is.read((char *)mpData, length * 2);

  // convert 8 -> 16 bit
  if (bits == 8)
  {
    int i;
    char *tmp = (char *)mpData;
    length = bytes;
    mpData = new short [length];
    for (i = 0; i < length; ++i)
    {
      mpData[i] = ((short) ((signed char)tmp[i] ^ (signed char)0x80)) << 8;
    }
    delete [] tmp;
  }

  // convert mono -> stereo
  if (channels == 1 && set.GetChannelCount() == 2)
  {
    short *old = mpData;
    length = length * 2;
    mpData = new short [length];
    int i = 0;
    int j = 0;
    while (i < length)
    {
      mpData[i++] = old[j];
      mpData[i++] = old[j++];
    }
    delete [] old;
    channels = 2;
  }
  // convert stereo -> mono
  else if (channels == 2 && set.GetChannelCount() == 1)
  {
    short *old = mpData;
    length = length / 2;
    mpData = new short [length];
    int i = 0;
    int j = 0;
    while (i < length)
    {
      int val = ((int)old[j] + old[j+1]) / 2;
      mpData[i++] = (short)val;
      j += 2;
    }
    delete [] old;
    channels = 2;
  }

  // convert sampling speed
  if (pitch != 0)
    speed = (int)(speed * pow(FSEMI, pitch));
  if (speed != set.GetSamplingRate())
  {
    float f = (float)speed / (float)set.GetSamplingRate();
    Transpose(f);
  }

  // apply volume and pan
  if (volume != 127 || pan != 0)
  {
    int ch1 = volume;
    int ch2 = volume;
    int  ppan = (set.GetChannelCount() == 2) ? pan : 0;
    if (ppan > 0)
    {
      ch1 = (int)volume * (63L - ppan) / 64L;
    }
    else if (ppan < 0)
    {
      ch2 = (int)volume * (63L + ppan) / 64L;
    }
    for (int i = 0; i < length-1; i += 2)
    {
      mpData[i]   = (short)((int)mpData[i]   * ch1 >> 7);
      mpData[i+1] = (short)((int)mpData[i+1] * ch2 >> 7);
    }
  }

  return 0;
}


int JZSample::LoadRaw()
{
  // determine file size
  struct stat buf;
  if (stat(mFileName.c_str(), &buf) == -1)
  {
    perror(mFileName.c_str());
    return 1;
  }
  length = buf.st_size/2;
  external_time = buf.st_mtime;

  ifstream is(mFileName.c_str(), ios::binary | ios::in);
  mpData = new short [length];
  is.read((char *)mpData, length * 2);
  return 0;
}


int JZSample::Load(int force)
{
  // sample modified on disk?
  if (!mFileName.empty() && !force && !dirty && external_flag)
  {
    struct stat buf;
    if (stat(mFileName.c_str(), &buf) == -1)
    {
      perror(mFileName.c_str());
      return 1;
    }
    if (external_time != buf.st_mtime)
      dirty = 1;
  }

  if (force || dirty)
  {
    FreeData();
    if (!mFileName.empty())
    {
      int rc = LoadWav();
      dirty = 0;
      return rc;
    }
  }

  return 0;
}

int JZSample::Align(int offs) const
{
  if (offs < 0)
    offs = 0;
  else if (offs > length)
    offs = length;
  return offs & - set.GetChannelCount();
}


void JZSample::Copy(JZSample &dst, int fr_smpl, int to_smpl)
{
  fr_smpl = (fr_smpl < 0) ? 0 : fr_smpl;
  to_smpl = (to_smpl < 0) ? length : to_smpl;
  int count = to_smpl - fr_smpl;
  dst.MakeData(count);
  memcpy(dst.mpData, mpData + fr_smpl, count * sizeof(short));
}


void JZSample::Delete(int fr_smpl, int to_smpl)
{
  fr_smpl = (fr_smpl < 0) ? 0 : fr_smpl;
  to_smpl = (to_smpl < 0) ? length : to_smpl;
  int new_length = length - (to_smpl - fr_smpl);
  short *new_data = new short [new_length];

  int fr_offs = fr_smpl * sizeof(short);
  memcpy(new_data, mpData, fr_offs);
  memcpy(
    new_data + fr_smpl,
    mpData + to_smpl,
    (length - to_smpl) * sizeof(short));

  delete [] mpData;
  mpData = new_data;
  length = new_length;
}

void JZSample::Cut(JZSample &dst, int fr_smpl, int to_smpl)
{
  Copy(dst, fr_smpl, to_smpl);
  Delete(fr_smpl, to_smpl);
}

void JZSample::InsertSilence(int pos, int len)
{
  int new_length = length + len;
  short *new_data = new short [new_length];

  int bytes1 = pos * sizeof(short);
  int bytes2 = len * sizeof(short);
  int bytes3 = (length - pos) * sizeof(short);
  memcpy(new_data, mpData, bytes1);
  memset(new_data + pos, 0, bytes2);
  memcpy(new_data + pos + len, mpData + pos, bytes3);

  delete [] mpData;
  mpData = new_data;
  length = new_length;
}


void JZSample::ReplaceSilence(int offs, int len)
{
  AssureLength(offs + len);
  while (len-- > 0)
    mpData[offs++] = 0;
}

void JZSample::PasteIns(JZSample &src, int offs)
{
  InsertSilence(offs, src.length);
  memcpy(mpData + offs, src.mpData, src.length * sizeof(short));
}


void JZSample::PasteMix(JZSample &src, int offs)
{
  AssureLength(offs + src.length);
  JZFloatSample fs(*this);
  for (int i = 0; i < src.length; i++)
  {
    fs[offs + i] += src.mpData[i];
  }
  fs.RescaleToShort();
  Set(fs);
}

void JZSample::Reverse(int fr, int to)
{
  // maybe swaps channels too
  if (to >= length)
    to = length - 1;
  while (to > fr)
  {
    short tmp = mpData[fr];
    mpData[fr] = mpData[to];
    mpData[to] = tmp;
    to--;
    fr++;
  }
}

// swap phase on left/right channel
void JZSample::Flip(int ch)
{
  int i = ch;
  int step = set.GetChannelCount();
  while (i < length)
  {
    mpData[i] = -mpData[i];
    i += step;
  }
}


void JZSample::PasteOvr(JZSample &src, int fr, int to)
{
  Delete(fr, to);
  PasteIns(src, fr);
}


void JZSample::AssureLength(int new_len)
{
  if (new_len > length)
    InsertSilence(length, new_len - length);
}


int JZSample::GetSamplingRate() const
{
  return set.GetSamplingRate();
}


int JZSample::GetChannelCount() const
{
  return set.GetChannelCount();
}


int JZSample::Peak()
{
  int peak = 0;
  for (int i = 0; i < length; i++)
  {
    int d = abs(mpData[i]);
    if (d > peak)
      peak = d;
  }
  return peak;
}

void JZSample::Rescale(short maxval)
{
  float peak = (float)Peak();
  if (peak > 0.0)
  {
    float f = maxval / peak;
    for (int i = 0; i < length; i++)
      mpData[i] = (short)(f * mpData[i]);
  }
}


void JZSample::TransposeSemis(float semis)
{
  /* PAT - The following error prompted the addition of the casts to double.
     sample.cpp: In member function `void JZSample::TransposeSemis(float)':
                 choosing `double pow(double, double)' over `float
                 std::pow(float, float)'
                 because worst conversion for the former is better than worst
                 conversion for the latter
  */
  float f = (double)pow((double)FSEMI, (double)semis);
  Transpose(f);
}


void JZSample::Transpose(float f)
{
  int channels   = set.GetChannelCount();
  int new_length = ((int)((double)length / (double)f) & (-channels));
  short *new_data = new short [new_length];

  const int N = new_length / channels;
  for (int i = 0; i < N; i++)
  {
    float x = f * i;
    float ofs = floor(x);
    float rem = x - ofs;
    int j = (int)ofs * channels;
    int k = i * channels;
    for (int c = 0; c < channels; c++)
    {
      JZMapper Map(0, 1, mpData[j + c], mpData[j + channels + c]);
      new_data[k + c] = (short)Map.XToY(rem);
    }
  }
  delete mpData;
  mpData = new_data;
  length = new_length;
}


int JZSample::Seconds2Samples(float time)
{
  JZMapper Map(
    0.0,
    1.0,
    0.0,
    (double)set.GetSamplingRate() * set.GetChannelCount());

  return static_cast<int>(Map.XToY(time));
}

float JZSample::Samples2Seconds(int samples)
{
  JZMapper Map(
    0.0,
    (double)set.GetSamplingRate() * set.GetChannelCount(),
    0.0,
    1.0);

  return (float)Map.XToY(samples);
}

/**
 * parameters:
 *   fr - to : sample range
 *   type    : filter type
 *   order   : filter order
 *   freq    : corner freq in Hz
 *   bw      : bandwith as fraction of freq in 0..1
 */

void JZFloatSample::Filter(int fr, int to, JZSplFilter::Type type, int order, double freq, double bw)
{
  int i;
  if (fr < 0)
    fr = 0;
  if (to < 0)
    to = length;

  //double a0 = freq / (double)sampling_rate;
  JZSplFilter *filters = new JZSplFilter[channels];
  for (i = 0; i < channels; i++)
    filters[i].Init(type, (float)sampling_rate, freq, bw);
  for (i = fr; i < to; i += channels)
  {
    for (int c = 0; c < channels; c++)
      mpData[i + c] = filters[c].Loop(mpData[i + c]);
  }
  delete [] filters;
}

int JZSample::Save()
{
  int err = SaveWave();  // the only format supported yet
  if (!err)
  {
    dirty  = 0;
    volume = 127;
    pan    = 0;
  }
  return err;
}

int JZSample::SaveWave()
{
  WaveHeader wh;
  wh.main_chunk = RIFF;
  wh.chunk_type = WAVE;
  wh.sub_chunk = FMT;
  wh.data_chunk = DATA;
  wh.format = PCM_CODE;
  wh.modus = set.GetChannelCount();
  wh.sc_len = 16;
  wh.sample_fq = set.GetSamplingRate();
  wh.bit_p_spl = set.GetBitsPerSample();
  wh.byte_p_spl = set.GetChannelCount() * (set.GetBitsPerSample() > 8 ? 2 : 1);
  wh.byte_p_sec = wh.byte_p_spl * wh.sample_fq;
  wh.data_length = length * sizeof(short);
  wh.length = wh.data_length + sizeof(WaveHeader);

#ifdef __WXMSW__
  unlink(mFileName.c_str()); // buggy, sigh!
#endif
  ofstream os(mFileName.c_str(), ios::out | ios::binary | ios::trunc);

  os.write((char *)&wh, sizeof(wh));
  os.write((char *)mpData, length * sizeof(short));

  return os.bad();
}

//*************************************************************
//                  JZFloatSample
//*************************************************************

JZFloatSample::JZFloatSample(JZSample &spl)
{
  current = 0;
  length = spl.length;
  mpData = new float [length];
  for (int i = 0; i < length; i++)
    mpData[i] = (float)spl.mpData[i];
  channels = spl->GetChannelCount();
  sampling_rate = spl->GetSamplingRate();
}

JZFloatSample::JZFloatSample(JZSample &spl, int fr, int to)
{
  current = 0;
  length = to - fr;
  mpData   = new float [length];
  for (int i = 0; i < length; i++)
    mpData[i] = (float)spl.mpData[i + fr];
  channels = spl->GetChannelCount();
  sampling_rate = spl->GetSamplingRate();
}

JZFloatSample::JZFloatSample(int ch, int sr)
{
  current = 0;
  channels = ch;
  sampling_rate = sr;
  length = 1000;
  mpData = new float [length];
  memset(mpData, 0, length * sizeof(float));
}


JZFloatSample::~JZFloatSample()
{
  delete [] mpData;
}


float JZFloatSample::Peak(int fr, int to)
{
  if (fr < 0)
    fr = 0;
  if (to < 0)
    to = length;
  float peak = 0;
  for (int i = fr; i < to; i++)
  {
    float d = fabs(mpData[i]);
    if (d > peak)
      peak = d;
  }
  return peak;
}


void JZFloatSample::Rescale(float maxval, int fr, int to)
{
  if (fr < 0)
    fr = 0;
  if (to < 0)
    to = length;
  float peak = Peak(fr, to);
  if (peak > 0.0)
  {
    float f = maxval / peak;
    for (int i = fr; i < to; i++)
      mpData[i] *= f;
  }
}


void JZFloatSample::RescaleToShort(int fr, int to)
{
  if (fr < 0)
    fr = 0;
  if (to < 0)
    to = length;
  float peak = Peak(fr, to);
  if (peak > 32767)
  {
    float f = 32767.0 / peak;
    for (int i = fr; i < to; i++)
      mpData[i] *= f;
  }
}


void JZFloatSample::Initialize(int size)
{
  delete [] mpData;
  length = 0;
  if (size > 0)
  {
    length = size;
    mpData = new float [length];
    memset(mpData, 0, length * sizeof(float));
  }
}


void JZFloatSample::PasteMix(JZFloatSample &src, int offs)
{
  AssureLength(offs + src.length);
  for (int i = 0; i < src.length; i++)
    mpData[offs + i] += src.mpData[i];
}

void JZFloatSample::RemoveTrailingSilence(float peak)
{
  int len1 = length - channels;  // last value
  while (len1 > 0 && fabs(mpData[len1]) < peak)
    len1 -= channels;
  length = len1 + channels;
}


void JZFloatSample::PasteMix(JZSample &src, int offs)
{
  AssureLength(offs + src.length);
  for (int i = 0; i < src.length; i++)
    mpData[offs + i] += src.mpData[i];
}


// ----------------------------------------------------------
//                CMIX Wavetable functions
// ----------------------------------------------------------

void JZFloatSample::Normalize()
{
  int j;
  float wmax, xmax = 0;
  for (j = 0; j < length; j++)
  {
    if ((wmax = (float)fabs(mpData[j])) > xmax)
      xmax = wmax;
  }
  for (j = 0; j < length; j++)
  {
    mpData[j] /= xmax;
  }
}

// gen25 1
void JZFloatSample::HanningWindow(int size)
{
  channels = 1;
  Initialize(size);
  for (int i = 0; i < length; i++)
    mpData[i] = -cos(2.0*M_PI * (float)i/(float)(length)) * 0.5 + 0.5;
  Normalize();
}

#if 0

// gen25 2
void JZFloatSample::HammingWindow(int size)
{
  channels = 1;
  Initialize(size);
  for (int i = 0; i < length; i++)
    mpData[i] = 0.54 - 0.46*cos(2.0*M_PI * (float)i/(float)(length));
  Normalize();
}

// gen5(gen)
JZFloatSample::ExpSegments(int size, int nargs, float pval[])
{
  channels = 1;
  Initialize(size);
  float c,amp2,amp1;
  int j,k,l,i = 0;

  amp2 = pvals[0];
  for (k = 1; k < nargs; k += 2)
  {
    amp1 = amp2;
    amp2 = pvals[k+1];
    j = i + 1;
    mpData[i] = amp1;
    c = (float) pow((amp2/amp1),(1./ pvals[k]));
    i = (j - 1) + pvals[k];
    for (l = j; l < i; l++)
    {
      if (l < size)
        mpData[l] = mpData[l-1] * c;
    }
  }
  Normalize();
}

// gen6
JZFloatSample::LineSegments(int size, int nargs, float pval[])
{
  channels = 1;
  Initialize(size);
  CMixCmd cmix(sampling_rate);
  cmix.setline(pvals, nargs, size, mpData);
  Normalize();
}
#endif


// **********************************************************
//                         CMIX Interface
// **********************************************************

int JZFloatSample::Seconds2Samples(float time)
{
  JZMapper Map(0.0, 1.0, 0.0, (double)sampling_rate * channels);
  return static_cast<int>(Map.XToY(time));
}

float JZFloatSample::Samples2Seconds(int samples)
{
  JZMapper Map(0.0, (double)sampling_rate * channels, 0.0, 1.0);
  return (float)Map.XToY(samples);
}

void JZFloatSample::AssureLength(int new_len)
{
  if (new_len > length)
    InsertSilence(length, new_len - length);
}


void JZFloatSample::InsertSilence(int pos, int len)
{
  int new_length = length + len;
  float *new_data = new float [new_length];

  int bytes1 = pos * sizeof(float);
  int bytes2 = len * sizeof(float);
  int bytes3 = (length - pos) * sizeof(float);
  memcpy(new_data, mpData, bytes1);
  memset(new_data + pos, 0, bytes2);
  memcpy(new_data + pos + len, mpData + pos, bytes3);

  delete [] mpData;
  mpData = new_data;
  length = new_length;
}


int JZFloatSample::SetNote(float foffs, float durat)
{
  int offs;
  int size;
  if (foffs < 0)
    offs = -(int)foffs;
  else
    offs = Seconds2Samples(foffs);

  if (durat < 0)
    size = -(int)durat;
  else
    size = Seconds2Samples(durat);

  AssureLength(offs + size);
  current = offs;
  return size / channels;
}


void JZFloatSample::EndNote()
{
}

/// in case the cmix function does not fill up the output buffer
void JZFloatSample::ClipToCurrent()
{
  if (current < length)
    length = current;
}


int JZFloatSample::AddOut(float *p)
{
  if (current >= length)
    AssureLength(length * 2);
  for (int i = 0; i < channels; i++)
    mpData[current++] += p[i];
  return 1;
}


int JZFloatSample::GetIn(float *p)
{
  for (int i = 0; i < channels; i++)
    p[i] = mpData[current++];
  return current < length;
}

/**
 * return the x-th sample. x does NOT contain channels, so
 * 0 < x < length/channels. interpoates the values.
 * @return TRUE if x < length/channels, FALSE if x > end of samples.
 */
int JZFloatSample::GetSample(float x, float *p)
{
  float ofs = floor(x);
  float rem = x - ofs;

  int i = (int)ofs * channels;
  if (i >= length)
    return 0;
  for (int c = 0; c < channels; c++)
  {
    JZMapper Map(0, 1, mpData[i + c], mpData[i + channels + c]);
    p[c] = Map.XToY(rem);
  }
  return 1;
}


void JZFloatSample::Convert2Mono()
{
  // convert this sample to mono
  if (channels != 2)  // only stereo so far
    return;
  float *dst = mpData;
  for (int i = 0; i < length - 1; i += 2)
    *dst++ = (mpData[i] + mpData[i+1]) / 2.0;
  length = length / 2;
  channels = 1;
}


void JZFloatSample::Echo(int num_echos, int delay, float ampl)
{
  delay = (delay & -channels);
  AssureLength(length + num_echos * delay);
  const int N = length;
  for (int i = N-1; i >= 0; i--)
  {
    float a = ampl;
    int k = i - delay;
    for (int j = 0; k >= 0 && j < num_echos; j++)
    {
      mpData[i] += mpData[k] * a;
      a *= ampl;
      k -= delay;
    }
  }
}

void JZFloatSample::RndEcho(int num_echos, int delay, float ampl)
{
  int i;

  int *delays = new int [num_echos];
  for (i = 0; i < num_echos; i++)
  {
    // compute random delays in the range 0.5 * delay ... 1.5 * delay
    int d = (int) ((rnd.asDouble() + 0.5) * (double)delay);
    d &= -channels;
    delays[i] = d;
  }

  AssureLength(length + (int)num_echos * delay);
  const int N = length;
  for (i = N-1; i >= 0; i--)
  {
    float a = ampl;
    int k = i - delay;
    for (int j = 0; k >= 0 && j < num_echos; j++)
    {
      mpData[i] += mpData[k] * a;
      a *= ampl;
      k -= delays[j];
    }
  }
  delete [] delays;
}

void JZFloatSample::RndEchoStereo(int num_echos, int delay, float ampl)
{
  int i;
  assert(channels == 2);

  int *delays = new int [num_echos];
  float *ipans = new float [num_echos];
  float *opans = new float [num_echos];
  for (i = 0; i < num_echos; i++)
  {
    // compute random delays in the range 0.5 * delay ... 1.5 * delay
    int d = (int) ((rnd.asDouble() + 0.5) * (double)delay);
    d &= -channels;
    delays[i] = d;
    ipans[i] = (float)rnd.asDouble();
    opans[i] = (float)rnd.asDouble();
  }

  AssureLength(length + (int)num_echos * delay);
  const int N = length - channels;
  for (i = N; i >= 0; i -= channels)
  {
    float a = ampl;
    int k = i - delay;
    for (int j = 0; k >= 0 && j < num_echos; j++)
    {
      float ipan = ipans[j];
      float opan = opans[j];
      float val = a * ( ipan * mpData[k] + (1.0 - ipan) * mpData[k+1] );
      mpData[i]   += opan * val;
      mpData[i+1] += (1.0 - opan) * val;
      //mpData[i] += mpData[k] * a;
      a *= ampl;
      k -= delays[j];
    }
  }
  delete [] delays;
  delete [] ipans;
  delete [] opans;
}
