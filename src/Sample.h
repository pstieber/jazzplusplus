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

class JZSampleSet;

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

#include <sys/stat.h>

#include "SignalInterface.h"

// msvc 5.0 has a buggy optimizer!! it cannot compute the following:
// pow(2.0, 1.0 / 12.0)
#define FSEMI    1.059463094
// log ( pow(2.0, 1.0 / 12.0) )
#define LOGFSEMI 0.057762264

// --------------------------------------------------------
// --------------- stolen from vplay ----------------------
// --------------------------------------------------------

// Definitions for Microsoft WAVE format.

#define RIFF            0x46464952
#define WAVE            0x45564157
#define FMT             0x20746D66
#define DATA            0x61746164
#define PCM_CODE        1
#define WAVE_MONO       1
#define WAVE_STEREO     2

//   It's in chunks like .voc and AMIGA iff, but my source says they
// are in only in this combination, so I combined them in one header;
// it works on all WAVE-file I have tested.

// 'old' format for writing .wav files
typedef struct _waveheader
{
  // 'RIFF'
  unsigned main_chunk;

  // File length
  unsigned length;

  // 'WAVE'
  unsigned chunk_type;

  unsigned sub_chunk;         // 'fmt '
  unsigned sc_len;            // length of sub_chunk, = 16
  unsigned short format;      // should be 1 for PCM-code
  unsigned short modus;       // 1 Mono, 2 Stereo
  unsigned sample_fq;
  unsigned byte_p_sec;
  unsigned short byte_p_spl;  // samplesize; 1 or 2 bytes
  unsigned short bit_p_spl;   // 8, 12 or 16 bit

  unsigned data_chunk;
  unsigned data_length;       // # sample bytes
} WaveHeader;






//*****************************************************************************
// There are two different kinds of samples, short and float.
// The class hierarchy is a little buggy, they should have been
// derived from a common base. Most of the functionality is in
// the short version. Some functionality is duplicated in
// both classes.
//*****************************************************************************

//*****************************************************************************
//   This is the floating-point representation of a sample.  This simplifies
// algorithms because there is no need to take care of overruns.
//
// used for the 'big' CMIX interface too.
//*****************************************************************************
class JZFloatSample // : public tCMIX
{
  friend class JZSample;
  public:
    JZFloatSample(JZSample &spl);
    JZFloatSample(JZSample &spl, int fr, int to);
    JZFloatSample(int ch, int sr);
    virtual ~JZFloatSample();
    float Peak(int fr = -1, int to = -1);
    void Rescale(float maxval = 32766.0, int fr = -1, int to = -1);
    void RescaleToShort(int fr = -1, int to = -1);
    float &operator[](int i)
    {
      return mpData[i];
    }
    void Initialize(int size = 0);
    void PasteMix(JZFloatSample &src, int offs = 0);
    void PasteMix(JZSample &src, int offs = 0);
    void RemoveTrailingSilence(float peak = 50);

    // CMIX Interface functions

    virtual int SetNote(float offs, float dur);
    virtual void EndNote();
    virtual int AddOut(float *p);
    virtual int GetIn(float *p);
    int GetSample(float i, float *p);
    int Seconds2Samples(float time);
    float Samples2Seconds(int samples);
    void InsertSilence(int pos, int length);
    void Convert2Mono();
    void ClipToCurrent();

    // CMIX wavetables (gen routines)
    void Normalize();  // make values in 0..1
    void HanningWindow(int size);

    // Effects

    void Echo(int num_echos, int delay, float ampl);
    void RndEcho(int num_echos, int delay, float ampl);
    void RndEchoStereo(int num_echos, int delay, float ampl);

    // see args of JZSplFilter::Setup() for this.
    void Filter(
      int fr,
      int to,
      JZSplFilter::Type type,
      int order,
      double freq,
      double bw);

    // signal template classes interface

    void AssureLength(int new_length);

    int GetChannelCount() const
    {
      return channels;
    }

    int GetSamplingRate() const
    {
      return sampling_rate;
    }

    float* GetData()
    {
      return mpData;
    }

    int GetLength() const
    {
      return length;
    }

  protected:
    float* mpData;
    int length;
    int channels;
    int sampling_rate;

    // CMIX-IO position
    int current;
};


//*****************************************************************************
//   This class contains the data for one sample.  For fastest playback access
// samples are stored as signed shorts.
//   mpData[0]   == 1st value for left channel
//   mpData[1]   == 1st value for right channel
//   mpData[n]   == 1st value for n-th channel
//   mpData[n+1] == 2nd value for left channel
//   ...
// All length values mean the number of shorts and should be multiples of
// set.GetChannelCount().  Offsets should start on channel boundaries, that is
// offs % set.GetChannelCount() == 0.
//*****************************************************************************
class JZSample
{
  friend class JZFloatSample;
  friend class JZSplPan;
  friend class JZSplPitch;
  public:
    friend class JZSampleSet;
    friend class JZSampleVoice;
    JZSample(JZSampleSet &s);
    virtual ~JZSample();

    int Load(int force = 0);
    int LoadWav();
    int LoadRaw();

    int SaveWave();
    int Save();

    // Properties
    void SetLabel(const std::string& Label);
    const std::string& GetLabel() const
    {
      return mLabel;
    }

    void SetVolume(int vol)
    {
      dirty |= (vol != volume);
      volume = vol;
    }

    int GetVolume() const
    {
      return volume;
    }

    void SetPan(int p)
    {
      dirty |= (p != pan);
      pan = p;
    }

    int GetPan() const
    {
      return pan;
    }

    void SetPitch(int p)
    {
      dirty |= (p != pitch);
      pitch = p;
    }

    int GetPitch() const
    {
      return pitch;
    }

    void SetFileName(const std::string& FileName);
    const std::string& GetFileName() const
    {
      return mFileName;
    }

    int GetLength() const
    {
      return length;
    }

    bool IsEmpty() const
    {
      return length == 0;
    }

    void SetExternal(int ext)
    {
      external_flag = ext;
      //external_time = 0;
    }

    int GetExternal() const
    {
      return external_flag;
    }

    void Clear();

    void GotoRAM()
    {
      // Try to swap this sample into memory.
      volatile short dummy;
      for (int i = 0; i < length; i++)
        dummy = mpData[i];
    }

    short *GetData()
    {
      return mpData;
    }


    // access global adustments from JZSampleSet

    JZSampleSet& SampleSet()
    {
      return set;
    }

    JZSampleSet *operator->()
    {
      return &set;
    }

    int GetChannelCount() const;

    int GetSamplingRate() const;

    // align offset to channel boundary
    int Align(int offs) const;

    // copy part of the data into another JZSample o. If o
    // contains other data these will be erased.

    void Copy(JZSample &dst, int fr_smpl = -1, int to_smpl = -1);

    // like Copy but deletes the source selection afterwards.

    void Cut(JZSample &dst, int fr_smpl = -1, int to_smpl = -1);

    // delete part of this sample.
    void Delete(int fr_smpl = -1, int to_smpl = -1);

    // paste some data into this sample, data are inserted
    void PasteIns(JZSample &src, int offs);

    // paste some data into this sample, data are mixed with
    // the current contents.
    void PasteMix(JZSample &src, int offs);
    void PasteOvr(JZSample &src, int fr, int to);
    void ReplaceSilence(int offs, int len);
    void Rescale(short maxval = 32766);
    void Reverse(int fr, int to);
    int Peak();

    // flip phase of left/right channel
    void Flip(int ch);


    void Transpose(float freq_factor);
    void TransposeSemis(float semis);

    // inserts some zero values at pos

    void InsertSilence(int pos, int length);

    // initialize length and data from the float sample

    void Set(JZFloatSample &fs);

    // replace part of the data with the data in fs,
    // original data are overwritten.

    void Set(JZFloatSample &fs, int offs);

    // like Set() but try to make a smooth transition
    void SetSmooth(JZFloatSample &fs, int offs, int fade_len = -1);

    int Seconds2Samples(float time);
    float Samples2Seconds(int samples);

    void AssureLength(int len);

  protected:

    void FreeData();
    void MakeData(int length, int zero = 1);

    int Convert(
      std::istream& is,
      int byte_count,
      int channels,
      int bits,
      int speed);

    int length;  // number of shorts
    short* mpData; // signed shorts
    JZSampleSet& set;

    std::string mLabel;
    std::string mFileName;
    int volume;
    int pan;
    int pitch;  // delta pitch

#ifdef HAVE_IOS_OPENMODE
    std::_Ios_Openmode openread;
#else
    int openread;
#endif

    int dirty;

    int external_flag;  // reload on disk change?
    int external_time;  // last modified on disk
};
