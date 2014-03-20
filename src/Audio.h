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

#include "DynamicArray.h"
#include "Project.h"

#include <wx/app.h>
#include <wx/string.h>

class JZSamplesDialog;
class JZTrack;
class JZAudioBufferQueue;
class JZAudioRecordBuffer;
class JZEventArray;
class JZSample;
class JZSampleVoice;
class JZSampleFrame;
struct JZAudioBuffer;

// These should be variables and queried from the driver!
//
// There is still a bug somewhere:
//   FRAGBITS 13
//   BUFCOUNT 64
//   MIDI-speed 114 (trackwin)
// Does not work, sounds like it skips a buffer after 18 bars.
//
// 1MB of buffer data seems to be reasonable.


#ifdef __WXMSW__

// Microsoft Windows has big buffers.

#define FRAGBITS   14
#define FRAGBYTES  (1 << FRAGBITS)  // # bytes
#define FRAGSHORTS (FRAGBYTES/2)    // # shorts
#define BUFSHORTS  FRAGSHORTS
#define BUFBYTES   FRAGBYTES
#define BUFCOUNT   64               // # buffers

#else

// Linux only has 64K buffers and wastes one fragment, so keep the fragments
// small.

#define FRAGBITS   13
#define FRAGBYTES  (1 << FRAGBITS)  // # bytes
#define FRAGSHORTS (FRAGBYTES/2)    // # shorts
#define BUFSHORTS  FRAGSHORTS
#define BUFBYTES   FRAGBYTES
#define BUFCOUNT   128               // # buffers

#define WAVEHDR char

#endif

//*****************************************************************************
// Description:
//   This is the audio buffer structure declaration.
//*****************************************************************************
struct JZAudioBuffer
{
  // This is a Microsoft Windows for mswin wavehdr
  WAVEHDR* hdr;
  short* data;

  JZAudioBuffer(int dummy)
    : hdr(0),
      data(0)
  {
    data = new short [BUFSHORTS];
    // in case recording stops inside a buffer
    memset(data, 0, BUFBYTES);
  }

  ~JZAudioBuffer()
  {
    delete hdr;
    delete [] data;
  }

  void Clear()
  {
    memset(data, 0, BUFBYTES);
  }

  short* Data()
  {
    return data;
  }
};

//*****************************************************************************
//*****************************************************************************
class JZAudioBufferQueue
{
  public:

    JZAudioBufferQueue()
    {
      Clear();
    }

    ~JZAudioBufferQueue()
    {
    }

    void Clear()
    {
      written = read = 0;
      for (int i = 0; i < BUFCOUNT; i++)
      {
        array[i] = 0;
      }
    }

    int Count() const
    {
      return written - read;
    }

    int Empty() const
    {
      return written == read;
    }

    void Put(JZAudioBuffer *buf)
    {
      array[written++ % BUFCOUNT] = buf;
    }

    JZAudioBuffer* Get()
    {
      if (written == read)
      {
        return 0;
      }
      return(array[read++ % BUFCOUNT]);
    }

    void UnGet(JZAudioBuffer* buf)
    {
      array[ --read % BUFCOUNT ] = buf;
    }

  private:

    JZAudioBuffer* array[BUFCOUNT];

    int read, written;
};

//*****************************************************************************
//*****************************************************************************
class JZAudioRecordBuffer
{
  friend class JZSampleSet;
  friend class JZWindowsAudioPlayer;

  public:

    JZAudioRecordBuffer()
    {
      num_buffers = 0;
    }

    ~JZAudioRecordBuffer()
    {
      Clear();
    }

    void Clear();
    JZAudioBuffer * RequestBuffer();
    void UndoRequest()
    {
      --num_buffers;
    }
    void ResetBufferSize(int size)
    {
      bufbytes = size;
    }

  private:

    TTDynamicArray<JZAudioBuffer*> mBuffers;
    int num_buffers;
    int bufbytes;
};

//*****************************************************************************
// Description:
//   This is the sample set class declaration.  This class holds a collection
// of audio samples that are played when a particular MIDI signal is received.
//*****************************************************************************
class JZSampleSet
{
  private:

    friend class JZWindowsAudioPlayer;
    friend class JZAudioPlayer;
    friend class JZAlsaAudioPlayer;

  public:

    enum TESampleSize
    {
      eSampleCount = 128
    };

    JZSampleSet(long TicksPerMinute);

    virtual ~JZSampleSet();

    int Load(const wxString& FileName);

    // load jazz.spl
    void LoadDefaultSettings();

    int Save(const wxString& FileName);

    void ReloadSamples();

    void Edit(int key);

    int GetSamplingRate() const
    {
      return mSamplingRate;
    }

    void SetSamplingRate(int SamplingRate)
    {
      dirty |= (mSamplingRate != SamplingRate);
      mSamplingRate = SamplingRate;
    }

    int GetChannelCount() const
    {
      return mChannelCount;
    }

    void SetChannelCount(int ChannelCount)
    {
      dirty |= (mChannelCount != ChannelCount);
      mChannelCount = ChannelCount;
    }

    int GetBitsPerSample() const
    {
      return mBitsPerSample;
    }

    double GetClocksPerBuffer() const
    {
      return mClocksPerBuffer;
    }

    bool GetSoftSync() const
    {
      return mSoftwareSynchonization;
    }

    void SetSoftSync(bool SoftwareSynchonization)
    {
      mSoftwareSynchonization = SoftwareSynchonization;
    }

    int ResetBuffers(JZEventArray *, long start_clock, long TicksPerMinute);

    int ResetBufferSize(unsigned int bytes);

    int FillBuffers(long last_clock);

    JZAudioBuffer* GetBuffer(int i) const
    {
      // 0 < i < BUFCOUNT
      return mpBuffers[i];
    }

    void AdjustAudioLength(JZTrack *t, long TicksPerMinute);

    long Ticks2Samples(long ticks) const
    {
      long spl = (long)(
        60.0 * ticks * mSamplingRate * mChannelCount /
        (double)mTicksPerMinute);

      // Align to the first channel.
      return spl & -mChannelCount;
    }

    double Samples2Ticks(long samples) const
    {
      return (double)
        samples * mTicksPerMinute / 60.0 / mSamplingRate / mChannelCount;
    }

    // time in millisec
    long Ticks2Time(long ticks) const
    {
      return (long)(60000.0 * ticks / mTicksPerMinute);
    }

    long Time2Ticks(long time) const
    {
      return (long)((double)time * mTicksPerMinute / 60000.0);
    }

    long Samples2Time(long samples) const
    {
      return (long)(1000.0 * samples / mSamplingRate / mChannelCount);
    }

    long Time2Samples(long time) const
    {
      return (long)(0.001 * time * mSamplingRate * mChannelCount);
    }

    virtual const std::string& GetSampleLabel(int Index);

    void StartPlay(int clock);

    void StopPlay();

    // returns number of buffers prepared. Output starts at offs.
    int PrepareListen(int key, long fr_smpl = -1, long to_smpl = -1);

    int PrepareListen(JZSample *spl, long fr_smpl = -1, long to_smpl = -1);

    int ContinueListen(); // return number of buffers

    void SaveRecordingDlg(long frc, long toc, JZAudioRecordBuffer &buf);

    void SaveWave(
      const wxString& FileName,
      long frc,
      long toc,
      JZAudioRecordBuffer &buf);

    void AddNote(const std::string& FileName, long frc, long toc);

    void RefreshDialogs();

    JZSample& operator[](int i)
    {
      return *mSamples[i];
    }

    void EditAudioGlobalSettings(wxWindow* pParent);

    void EditAudioSamples(wxWindow* pParent);

    void LoadSampleSet(wxWindow* pParent);

    void SaveSampleSetAs(wxWindow* pParent);

    void SaveSampleSet(wxWindow* pParent);

    void ClearSampleSet(wxWindow* pParent);

    const JZAudioBufferQueue& GetFullBuffers() const;

  protected:

    long SampleSize(long num_samples)
    {
      return mChannelCount * (mBitsPerSample == 8 ? 1L : 2L) * num_samples;
    }

    long BufferClock(int i) const
    {
      return (long)(start_clock + i * mClocksPerBuffer);
    }

    void SamplesDlg();

  protected:

    // Sampling rate in samples per second or Hz.
    int mSamplingRate;

    // mono  = 1, stereo = 2
    int mChannelCount;

    // This must be 16!
    int mBitsPerSample;

    // Indicates if software MIDI/audio synchronization is on.
    bool mSoftwareSynchonization;

    JZSample* mSamples[eSampleCount];
    JZSampleFrame* mSampleFrames[eSampleCount];

    // MIDI sampling rate for audio/midi sync.
    long mTicksPerMinute;

    double mClocksPerBuffer;
    long   start_clock;       // when did play start

    int event_index;

    unsigned int bufbytes;           // buffer size in byte
    unsigned int bufshorts;          // buffer size in short
    JZAudioBuffer* mpBuffers[BUFCOUNT]; // all the audio buffers
    JZAudioBufferQueue mFreeBuffers;  // to be filled with data
    JZAudioBufferQueue mFullBuffers;  // to be played by driver
    JZAudioBufferQueue mDriverBuffers;  // actually played by driver

    // return the start clock for i-th free buffer
    long buffers_written;            // for computing buffers clock

    JZSamplesDialog* mpSamplesDialog;

    JZEventArray* events;

    enum
    {
      MAXPOLY = 100
    };

    JZSampleVoice* voices[MAXPOLY];
    int num_voices;
    int adjust_audio_length;

    wxString mDefaultFileName;
    wxString mRecordFileName;
    bool has_changed;
    int  is_playing;

    int dirty;  // needs reloading

    // to communicate between PrepareListen and ContinueListen
    JZSample* listen_sample;
};

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const JZAudioBufferQueue& JZSampleSet::GetFullBuffers() const
{
  return mFullBuffers;
}
