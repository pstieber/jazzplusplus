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

#include <vector>

struct tWinPlayerState;

extern "C"
{

void CALLBACK midiIntInputHandler(HMIDIIN, WORD, DWORD, DWORD, DWORD);
void CALLBACK midiMidiInputHandler(HMIDIIN, WORD, DWORD, DWORD, DWORD);
void CALLBACK midiMtcInputHandler(HMIDIIN, WORD, DWORD, DWORD, DWORD);
void CALLBACK midiIntTimerHandler(UINT, UINT, DWORD, DWORD, DWORD);
void CALLBACK midiMidiTimerHandler(UINT, UINT, DWORD, DWORD, DWORD);
void CALLBACK midiMtcTimerHandler(UINT, UINT, DWORD, DWORD, DWORD);
tWinPlayerState FAR * FAR PASCAL NewWinPlayerState();
void FAR PASCAL DeleteWinPlayerState(tWinPlayerState FAR * state);

void CALLBACK MidiOutProc(
  HMIDIOUT hmo,
  UINT wMsg,
  DWORD dwInstance,
  DWORD dwParam1,
  DWORD dwParam2
);

} // extern "C"

//*****************************************************************************
//*****************************************************************************
class JZWindowsAudioPlayer;

// pseudo data word of struct JZMidiEvent, values must be less
// than 128 to be distinguished from a midi status
#define START_AUDIO 1
#define SYSEX_EVENT 2

class JZWinSysexBufferArray;

//*****************************************************************************
//*****************************************************************************
class JZWinSysexBuffer
{
  public:

    JZWinSysexBuffer(JZWinSysexBufferArray* pWinSysexBufferArray)
      : mMidiHeader(),
        mpData(0),
        mSize(0),
        mMaxSize(0),
        mPrepared(false),
        mpNextFree(0),
        mpParent(pWinSysexBufferArray)
    {
      memset(&mMidiHeader, 0, sizeof(mMidiHeader));
    }

    MIDIHDR* MidiHdr()
    {
      return &mMidiHeader;
    }

    bool IsPrepared() const
    {
      return mPrepared;
    }

    JZWinSysexBuffer* GetNextFree() const
    {
      return mpNextFree;
    }

    void SetNextFree(JZWinSysexBuffer* pNextFree)
    {
      mpNextFree = pNextFree;
    }

    // sysex and length NOT including 0xf0 and 0xf7
    void PrepareOut(HMIDIOUT hmo, const unsigned char* pSysex, int Length)
    {
      if (mPrepared)
      {
        UnprepareOut(hmo);
      }

      if (Length + 2 >= mMaxSize)
      {
        mMaxSize = Length + 20;
        delete [] mpData;
        mpData = new unsigned char[mMaxSize];
      }

      mpData[0] = (char)0xf0;
      memcpy(mpData + 1, pSysex, Length);
      mpData[Length + 1] = (char)0xf7;
      mSize = Length + 2;

      memset(&mMidiHeader, 0, sizeof(mMidiHeader));
      mMidiHeader.dwUser = (DWORD)this;
      mMidiHeader.lpData = (char *)mpData;
      mMidiHeader.dwBufferLength = mSize;
      OutputDebugString(L"prepare\n");
      midiOutPrepareHeader(hmo, &mMidiHeader, sizeof(mMidiHeader));
      mPrepared = true;
    }

    void UnprepareOut(HMIDIOUT hmo)
    {
      if (mPrepared)
      {
        OutputDebugString(L"unprepare\n");
        midiOutUnprepareHeader(hmo, &mMidiHeader, sizeof(mMidiHeader));
        mSize = 0;
        mPrepared = false;
      }
    }

    void Release();

//    void PrepareInp(HMIDIIN hmi)
//    {
//      memset(&mMidiHeader, 0, sizeof(mMidiHeader));
//      mMidiHeader.dwUser = (DWORD)this;
//      mMidiHeader.lpData = (char *)mpData;
//      mMidiHeader.dwBufferLength = mMaxSize;
//      midiInPrepareHeader(hmi, &mMidiHeader, sizeof(mMidiHeader));
//      mPrepared = true;
//    }
//    void UnprepareInp(HMIDIIN hmi)
//    {
//      midiInUnprepareHeader(hmi, &mMidiHeader, sizeof(mMidiHeader));
//      mPrepared = false;
//    }

  private:

    MIDIHDR mMidiHeader;
    unsigned char* mpData;
    int mSize;
    int mMaxSize;
    bool mPrepared;
    JZWinSysexBuffer* mpNextFree;
    JZWinSysexBufferArray* mpParent;
};

//*****************************************************************************
//*****************************************************************************
class JZWinSysexBufferArray
{
  public:

    JZWinSysexBufferArray()
      : mPointerArray(),
        mpNextFree(0)
    {
    }

    ~JZWinSysexBufferArray()
    {
      for (size_t i = 0; i < mPointerArray.size(); ++i)
      {
        delete mPointerArray[i];
      }
    }

    size_t Size() const
    {
      return mPointerArray.size();
    }

    JZWinSysexBuffer* At(int i) const
    {
      return mPointerArray.at(i);
    }

    JZWinSysexBuffer* AllocBuffer()
    {
      if (mpNextFree == 0)
      {
        JZWinSysexBuffer* pWinSysexBuffer = new JZWinSysexBuffer(this);
        mPointerArray.push_back(pWinSysexBuffer);
        return pWinSysexBuffer;
      }
      JZWinSysexBuffer* pWinSysexBuffer = mpNextFree;
      mpNextFree = pWinSysexBuffer->GetNextFree();
      return pWinSysexBuffer;
    }

    void ReleaseBuffer(JZWinSysexBuffer* pWinSysexBuffer)
    {
      pWinSysexBuffer->SetNextFree(mpNextFree);
      mpNextFree = pWinSysexBuffer;
    }

    void ReleaseAllBuffers()
    {
      mpNextFree = 0;
      for (auto& pWinSysexBuffer : mPointerArray)
      {
        pWinSysexBuffer->SetNextFree(mpNextFree);
        mpNextFree = pWinSysexBuffer;
      }
    }

  private:

    std::vector<JZWinSysexBuffer*> mPointerArray;
    JZWinSysexBuffer* mpNextFree;
};

//*****************************************************************************
//*****************************************************************************
inline
void JZWinSysexBuffer::Release()
{
  mpParent->ReleaseBuffer(this);
}

//*****************************************************************************
//*****************************************************************************
struct JZMidiEvent
{
  DWORD ref;  // Means time or clock depending on sync mode.
  DWORD data; // MIDI event or pseudo data.
};


//*****************************************************************************
//*****************************************************************************
class JZMidiQueue
{
  public:

    JZMidiEvent* get()
    {
      if (mWriteIndex == mReadIndex)
      {
        return 0;
      }
      struct JZMidiEvent* pMidiEvent = &buffer[mReadIndex];
      mReadIndex = (mReadIndex + 1) % eMidiBufferSize;
      return pMidiEvent;
    }

    JZMidiEvent* peek()
    {
      if (mWriteIndex == mReadIndex)
      {
        return 0;
      }
      return &buffer[mReadIndex];
    }

    void put(DWORD data, DWORD ref)
    {
      if (nfree() < 1)
      {
        return;
      }
      JZMidiEvent* pMidiEvent = &buffer[mWriteIndex];
      pMidiEvent->data = data;
      pMidiEvent->ref = ref;
      mWriteIndex = (mWriteIndex + 1) % eMidiBufferSize;
    }

    int empty() const
    {
      return mReadIndex == mWriteIndex;
    }

    int nfree() const
    {
      return
        (mReadIndex - mWriteIndex - 1 + eMidiBufferSize) % eMidiBufferSize;
    }

    void Clear()
    {
      mReadIndex = mWriteIndex = 0;
    }

    JZMidiQueue()
    {
      Clear();
    }

  private:

    // Number of events in record/play queue.
    enum JZSizes
    {
      eMidiBufferSize = 4096
    };

  private:

    int mReadIndex, mWriteIndex;
    JZMidiEvent buffer[eMidiBufferSize];
};

#define WIN_SYNC_INTERNAL 0
#define WIN_SYNC_SONGPTR  1
#define WIN_SYNC_MTC      2

#define WIN_MTC_TYPE_24    0
#define WIN_MTC_TYPE_25    1
#define WIN_MTC_TYPE_30DF  2
#define WIN_MTC_TYPE_30NDF 3

//*****************************************************************************
//*****************************************************************************
struct tWinPlayerMtcTime
{
  DWORD hour;
  DWORD min;
  DWORD sec;
  DWORD fm;
  DWORD type;
};

//*****************************************************************************
//*****************************************************************************
struct tWinPlayerState
{
  HANDLE hmem;
  HMIDIIN hinp;
  HMIDIOUT hout;

  int start_time;
  int play_time;
  int start_clock;
  int ticks_per_minute;

  int play_clock;
  int virtual_clock;
  int ticks_per_signal;
  int signal_time;
  int time_per_tick;

  tWinPlayerMtcTime mtc_start;
  DWORD mtc_frames;
  BOOL mtc_valid;
  DWORD last_qfm;
  DWORD qfm_bits;
  DWORD time_per_frame;

  UINT min_timer_period;
  UINT max_timer_period;
  BOOL playing;
  BOOL soft_thru;
  BOOL doing_mtc_rec;

  JZMidiQueue recd_buffer;
  JZMidiQueue play_buffer;
  JZMidiQueue thru_buffer;

  JZWindowsAudioPlayer* audio_player;
  int time_correction;

  JZWinSysexBufferArray* mpInputSysexBuffers;
  JZWinSysexBufferArray* mpOutputSysexBuffers;
  bool mSysexFound;
};
