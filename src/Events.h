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

#include <fstream>
#include <string>

#include <wx/pen.h>

class JZEvent;

//*****************************************************************************
// Output device, can be
//   - Midi-Standard-File
//   - Ascii-File
//   - Midi-Port
//*****************************************************************************
class JZReadBase
{
  public:

    JZReadBase();

    virtual ~JZReadBase();

    virtual int Open(const std::string& FileName);

    virtual void Close();

    int GetTicksPerQuarter() const;

    virtual JZEvent* Read() = 0;

    virtual int NextTrack() = 0;

  protected:

    // Ths value is known after a call to Open.
    int mTicksPerQuarter;

    int mTrackCount;

    std::ifstream mIfs;
};

//*****************************************************************************
// Description:
//   These are the read base class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZReadBase::GetTicksPerQuarter() const
{
  return mTicksPerQuarter;
}

//*****************************************************************************
//*****************************************************************************
class JZWriteBase
{
  public:

    JZWriteBase();

    virtual ~JZWriteBase();

    virtual int Open(
      const std::string& FileName,
      int TrackCount,
      int TicksPerQuarter);

    virtual void Close();

    virtual int Write(JZEvent* pEvent);

    virtual int Write(JZEvent* pEvent, unsigned char Character);

    virtual int Write(
      JZEvent* pEvent,
      unsigned char Character1,
      unsigned char Character2);

    virtual int Write(
      JZEvent* pEvent,
      unsigned char Character1,
      unsigned char Character2,
      unsigned char Character3);

    virtual int Write(
      JZEvent* pEvent,
      unsigned char Character1,
      unsigned char Character2,
      unsigned char Character3,
      unsigned char Character4);

    virtual int Write(
      JZEvent* pEvent,
      unsigned char* pData,
      int Length) = 0;

    virtual void NextTrack();

  protected:

    std::ofstream mOfs;
};

//*****************************************************************************
// Description:
//   These are the write base class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZWriteBase::Write(JZEvent* pEvent)
{
  return Write(pEvent, 0, 0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZWriteBase::Write(JZEvent* pEvent, unsigned char Character)
{
  return Write(pEvent, &Character, 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZWriteBase::Write(
  JZEvent* pEvent,
  unsigned char Character1,
  unsigned char Character2)
{
  unsigned char Array[2];

  Array[0] = Character1;
  Array[1] = Character2;

  return Write(pEvent, Array, 2);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZWriteBase::Write(
  JZEvent* pEvent,
  unsigned char Character1,
  unsigned char Character2,
  unsigned char Character3)
{
  unsigned char Array[3];

  Array[0] = Character1;
  Array[1] = Character2;
  Array[2] = Character3;

  return Write(pEvent, Array, 3);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZWriteBase::Write(
  JZEvent* pEvent,
  unsigned char Character1,
  unsigned char Character2,
  unsigned char Character3,
  unsigned char Character4)
{
  unsigned char Array[4];

  Array[0] = Character1;
  Array[1] = Character2;
  Array[2] = Character3;
  Array[3] = Character4;

  return Write(pEvent, Array, 4);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZWriteBase::NextTrack()
{
}

//*****************************************************************************
//*****************************************************************************
class JZGetMidiBytes : public JZWriteBase
{
  public:

    int Open(const std::string& FileName, int nTracks, int TicksPerQuarter)
    {
      return 1;
    }

    void Close()
    {
    }

    void NextTrack()
    {
    }

    // Get JZEvent's bytes.
    int Write(JZEvent* pEvent, unsigned char* pString, int Length);

    unsigned char mBuffer[10];
    int mByteCount;
};

// ********************************************************************
// Midi-Events
// ********************************************************************

// Normal events (with the channel)
#define StatKeyOff         0x80
#define StatKeyOn          0x90
#define StatKeyPressure    0xA0
#define StatControl        0xB0
#define StatProgram        0xC0
#define StatChnPressure    0xD0  // Channel pressure
#define StatPitch          0xE0

// Meta events (no channel).
#define StatSysEx          0xF0
#define StatSongPtr        0xF2
#define StatSongSelect     0xF3
#define StatTuneRequest    0xF6
#define StatMidiClock      0xf8
#define StatStartPlay      0xfa
#define StatContPlay       0xfb
#define StatStopPlay       0xfc
#define StatText           0x01
#define StatCopyright      0x02
#define StatTrackName      0x03
#define StatMarker         0x06
#define StatEndOfTrack     0x2F
#define StatSetTempo       0x51
#define StatMtcOffset      0x54
#define StatTimeSignat     0x58
#define StatKeySignat      0x59

#define StatUnknown        0x00

// Proprietary event status
#define StatJazzMeta       0x7F
#define StatPlayTrack      0x7E


#define LAST_CLOCK        (0x7fffffff)
#define KILLED_CLOCK      (0x80000000)




// Event classes
class JZEvent;
class JZChannelEvent;
class JZMetaEvent;
class JZKeyOnEvent;
class JZKeyOffEvent;
class JZPitchEvent;
class JZControlEvent;
class JZProgramEvent;
class JZSysExEvent;
class JZSongPtrEvent;
class JZMidiClockEvent;
class JZStartPlayEvent;
class JZContPlayEvent;
class JZStopPlayEvent;
class JZTextEvent;
class JZCopyrightEvent;
class JZTrackNameEvent;
class JZMarkerEvent;
class JZSetTempoEvent;
class JZMtcOffsetEvent;
class JZTimeSignatEvent;
class JZKeySignatEvent;
class JZKeyPressureEvent;
class JZJazzMetaEvent;
class JZChnPressureEvent;
class JZPlayTrackEvent;
class JZEndOfTrackEvent;

//*****************************************************************************
// Description:
//   This is the MIDI event base class declaration.
//*****************************************************************************
class JZEvent
{
  public:

    unsigned char GetStat() const
    {
      return mStatusByte;
    }

    int GetClock() const
    {
      return mClock & ~KILLED_CLOCK;
    }

    void SetClock(int Clock)
    {
      mClock = Clock;
    }

    // The device is dynamically set when events are copied to the playback
    // queue (from the track device).
    enum
    {
      BROADCAST_DEVICE = 0
    };

    JZEvent(int Clock, unsigned char StatusByte)
      : mStatusByte(StatusByte),
        mClock(Clock),
        mDevice(BROADCAST_DEVICE)
    {
    }

    virtual ~JZEvent()
    {
    }

    void Kill()
    {
      mClock |= KILLED_CLOCK;
    }

    void UnKill()
    {
      mClock &= ~KILLED_CLOCK;
    }

    int IsKilled()
    {
      return (mClock & KILLED_CLOCK) != 0;
    }

    virtual JZMetaEvent* IsMetaEvent()
    {
      return 0;
    }
    virtual JZChannelEvent* IsChannelEvent()
    {
      return 0;
    }
    virtual JZKeyOnEvent* IsKeyOn()
    {
      return 0;
    }
    virtual JZKeyOffEvent* IsKeyOff()
    {
      return 0;
    }
    virtual JZPitchEvent* IsPitch()
    {
      return 0;
    }
    virtual JZControlEvent* IsControl()
    {
      return 0;
    }
    virtual JZProgramEvent* IsProgram()
    {
      return 0;
    }
    virtual JZSysExEvent* IsSysEx()
    {
      return 0;
    }
    virtual JZSongPtrEvent* IsSongPtr()
    {
      return 0;
    }
    virtual JZMidiClockEvent* IsMidiClock()
    {
      return 0;
    }
    virtual JZStartPlayEvent* IsStartPlay()
    {
      return 0;
    }
    virtual JZContPlayEvent* IsContPlay()
    {
      return 0;
    }
    virtual JZStopPlayEvent* IsStopPlay()
    {
      return 0;
    }
    virtual JZTextEvent* IsText()
    {
      return 0;
    }
    virtual JZCopyrightEvent* IsCopyright()
    {
      return 0;
    }
    virtual JZTrackNameEvent* IsTrackName()
    {
      return 0;
    }
    virtual JZMarkerEvent* IsMarker()
    {
      return 0;
    }
    virtual JZSetTempoEvent* IsSetTempo()
    {
      return 0;
    }
    virtual JZMtcOffsetEvent* IsMtcOffset()
    {
      return 0;
    }
    virtual JZTimeSignatEvent* IsTimeSignat()
    {
      return 0;
    }
    virtual JZKeySignatEvent* IsKeySignat()
    {
      return 0;
    }
    virtual JZKeyPressureEvent* IsKeyPressure()
    {
      return 0;
    }
    virtual JZJazzMetaEvent* IsJazzMeta()
    {
      return 0;
    }
    virtual JZPlayTrackEvent* IsPlayTrack()
    {
      return 0;
    }
    virtual JZEndOfTrackEvent* IsEndOfTrack()
    {
      return 0;
    }
    virtual JZChnPressureEvent* IsChnPressure()
    {
      return 0;
    }

    virtual int Write(JZWriteBase& io)
    {
      return io.Write(this);
    }

    int Compare(JZEvent& Event)
    {
      if ((unsigned)Event.mClock > (unsigned)mClock)
      {
        return -1;
      }
      if ((unsigned)Event.mClock < (unsigned)mClock)
      {
        return 1;
      }
      return 0;
    }

    virtual void BarInfo(
      int& TicksPerBar,
      int& CountsPerBar,
      int TicksPerQuarter) const
    {
    }

    virtual JZEvent* Copy() const
    {
      return new JZEvent(*this);
    }

    // Filter
    virtual int GetValue() const
    {
      return 0;
    }

    virtual unsigned short GetEventLength() const
    {
      return 16;
    }

    // Painting
    virtual int GetLength() const
    {
      return 16;
    }

    // Value normalized to 0 to 127.
    virtual int GetPitch() const
    {
      return 64;
    }

    virtual void SetPitch(int p)
    {
    }

    virtual const wxPen* GetPen() const
    {
      return wxBLACK_PEN;
    }

    virtual const wxBrush* GetBrush() const
    {
      return wxBLACK_BRUSH;
    }

    int GetDevice() const
    {
      return mDevice;
    }

    void SetDevice(int Device)
    {
      mDevice = Device;
    }

  protected:

    unsigned char mStatusByte;

    int mClock;

  private:

    int mDevice;
};

//*****************************************************************************
//*****************************************************************************
class JZChannelEvent : public JZEvent
{
  public:

    JZChannelEvent(int Clock, unsigned char StatusByte, int Channel)
      : JZEvent(Clock, StatusByte)
    {
      mChannel = Channel;
    }

    virtual JZChannelEvent* IsChannelEvent()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZChannelEvent(*this);
    }

    unsigned char GetChannel() const
    {
      return mChannel;
    }

    void SetChannel(unsigned char Channel)
    {
      mChannel = Channel;
    }

  private:

    unsigned char mChannel;
};

//*****************************************************************************
//*****************************************************************************
class JZKeyOnEvent : public JZChannelEvent
{
  public:

    JZKeyOnEvent(
      int Clock,
      int Channel,
      unsigned char Key,
      unsigned char Velocity,
      unsigned short Length = 0)
      : JZChannelEvent(Clock, StatKeyOn, Channel),
        mKey(Key),
        mVelocity(Velocity),
        mLength(Length),
        mOffVelocity(0)
    {
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, mKey, mVelocity);
    }

    virtual JZKeyOnEvent* IsKeyOn()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZKeyOnEvent(*this);
    }

    virtual int GetValue() const
    {
      return mKey;
    }

    virtual int GetPitch() const
    {
      return mKey;
    }

    virtual void SetPitch(int p)
    {
      mKey = p;
    }

    unsigned char GetKey() const
    {
      return mKey;
    }

    void SetKey(unsigned char Key)
    {
      mKey = Key;
    }

    unsigned char GetVelocity() const
    {
      return mVelocity;
    }

    void SetVelocity(unsigned char Velocity)
    {
      mVelocity = Velocity;
    }

    virtual unsigned short GetEventLength() const
    {
      return mLength;
    }

    virtual int GetLength() const
    {
      return mLength;
    }

    void SetLength(unsigned short Length)
    {
      mLength = Length;
    }

    unsigned short GetOffVelocity() const
    {
      return mOffVelocity;
    }

    void SetOffVelocity(int OffVelocity)
    {
      mOffVelocity = OffVelocity;
    }

    virtual const wxPen* GetPen() const
    {
      return wxBLACK_PEN;
    }

    virtual const wxBrush* GetBrush() const
    {
      return wxBLACK_BRUSH;
    }

  private:

    unsigned char mKey;

    unsigned char mVelocity;

    // Length is 0 if a corresponding JZKeyOffEvent exists.
    unsigned short mLength;

    unsigned short mOffVelocity;
};

//*****************************************************************************
//*****************************************************************************
class JZKeyOffEvent : public JZChannelEvent
{
  public:
    JZKeyOffEvent(
      int Clock,
      int Channel,
      unsigned char Key,
      unsigned char OffVelocity = 0)
      : JZChannelEvent(Clock, StatKeyOff, Channel),
        mKey(Key),
        mOffVelocity(OffVelocity)
    {
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, mKey, mOffVelocity);
    }

    virtual JZKeyOffEvent* IsKeyOff()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZKeyOffEvent(*this);
    }

    unsigned char GetKey() const
    {
      return mKey;
    }

    void SetKey(unsigned char Key)
    {
      mKey = Key;
    }

    unsigned char GetOffVelocity() const
    {
      return mOffVelocity;
    }

  private:

    unsigned char mKey;

    unsigned char mOffVelocity;
};

//*****************************************************************************
//*****************************************************************************
class JZPitchEvent : public JZChannelEvent
{
  public:
    short Value;

    JZPitchEvent(
      int Clock,
      unsigned short Channel,
      unsigned char lo,
      unsigned char hi)
      : JZChannelEvent(Clock, StatPitch, Channel)
    {
      Value  = ((hi << 7) | lo) - 8192;
    }

    JZPitchEvent(int Clock, unsigned short Channel, short val)
      : JZChannelEvent(Clock, StatPitch, Channel)
    {
      Value  = val;
    }

    virtual int Write(JZWriteBase &io)
    {
      int v = Value + 8192;
      return
        io.Write(this, (unsigned char)(v & 0x7F), (unsigned char)(v >> 7));
    }

    virtual JZPitchEvent* IsPitch()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZPitchEvent(*this);
    }

    virtual int GetValue() const
    {
      return Value;
    }

    virtual int GetPitch() const
    {
      return (Value + 8192) >> 7;
    }

    virtual void SetPitch(int p)
    {
      Value = (p << 7) - 8192;
    }

    virtual const wxPen* GetPen() const
    {
      return wxRED_PEN;
    }

    virtual const wxBrush* GetBrush() const
    {
      return wxRED_BRUSH;
    }
};

//*****************************************************************************
//*****************************************************************************
class JZControlEvent : public JZChannelEvent
{
  public:

    JZControlEvent(
      int Clock,
      int Channel,
      unsigned char Control,
      unsigned char Value)
      : JZChannelEvent(Clock, StatControl, Channel),
        mControl(Control),
        mValue(Value)
    {
    }

    virtual int Write(JZWriteBase& io)
    {
      return io.Write(this, mControl, mValue);
    }

    virtual JZControlEvent* IsControl()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZControlEvent(*this);
    }

    virtual int GetValue() const
    {
      return mControl;
    }

    virtual int GetPitch() const
    {
      return mControl;
    }

    virtual void SetPitch(int Pitch)
    {
      mControl = Pitch;
    }

    virtual const wxPen* GetPen() const
    {
      return wxCYAN_PEN;
    }

    virtual const wxBrush* GetBrush() const
    {
      return wxCYAN_BRUSH;
    }

    unsigned char GetControl() const
    {
      return mControl;
    }

    void SetControl(unsigned char Control)
    {
      mControl = Control;
    }

    unsigned char GetControlValue() const
    {
      return mValue;
    }

    void SetControlValue(unsigned char Value)
    {
      mValue = Value;
    }

  private:

    unsigned char mControl;
    unsigned char mValue;
};

//*****************************************************************************
//*****************************************************************************
class JZProgramEvent : public JZChannelEvent
{
  public:

    JZProgramEvent(int Clock, int Channel, unsigned char Program)
      : JZChannelEvent(Clock, StatProgram, Channel)
    {
      mProgram = Program;
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, mProgram);
    }

    virtual JZProgramEvent* IsProgram()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZProgramEvent(*this);
    }

    virtual int GetValue() const
    {
      return mProgram;
    }

    virtual int GetPitch() const
    {
      return mProgram;
    }

    virtual void SetPitch(int Pitch)
    {
      mProgram = Pitch;
    }

    virtual const wxPen* GetPen() const
    {
      return wxGREEN_PEN;
    }

    virtual const wxBrush* GetBrush() const
    {
      return wxGREEN_BRUSH;
    }

    unsigned char GetProgram() const
    {
      return mProgram;
    }

    void SetProgram(unsigned char Program)
    {
      mProgram = Program;
    }

  private:

    unsigned char mProgram;
};

//*****************************************************************************
//*****************************************************************************
class JZMetaEvent : public JZEvent
{
  public:

    JZMetaEvent(
      int Clock,
      unsigned char StatusByte,
      unsigned char* pData,
      unsigned short Length)
      : JZEvent(Clock, StatusByte),
        mpData(0),
        mLength(Length)
    {
      mpData = new unsigned char [Length + 1];
      if (pData)
      {
        memcpy(mpData, pData, Length);
      }
      mpData[Length] = 0;
    }

    virtual ~JZMetaEvent()
    {
      delete [] mpData;
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, mpData, mLength);
    }

    virtual JZMetaEvent* IsMetaEvent()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZMetaEvent(mClock, mStatusByte, mpData, mLength);
    }

    const unsigned char* GetData() const
    {
      return mpData;
    }

    unsigned short GetDataLength() const
    {
      return mLength;
    }

    virtual void FixCheckSum();

  protected:

    unsigned char* mpData;
    unsigned short mLength;
};

//*****************************************************************************
// Description:
//   This event contains proprietary information for Jazz++.  This event
// should should not go into a track itself but is read/written from/to
// the file.
//*****************************************************************************
class JZJazzMetaEvent : public JZMetaEvent
{
  public:

    enum
    {
      DATALEN = 20
    };

    JZJazzMetaEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatJazzMeta, pData, Length)
    {
    }

    JZJazzMetaEvent()
      : JZMetaEvent(0, StatJazzMeta, 0, DATALEN)
    {
      memset(mpData, 0, DATALEN);
      memcpy(mpData, "JAZ2", 4);
      mpData[4] = 1; // version or so
    }

    bool GetAudioMode() const
    {
      return mpData[5] != 0;
    }

    void SetAudioMode(bool AudioMode)
    {
      mpData[5] = AudioMode;
    }

    char GetTrackState() const
    {
      return mpData[6];
    }

    void SetTrackState(char c)
    {
      mpData[6] = c;
    }

    // mpData[7] is unused

    unsigned char GetTrackDevice() const
    {
      return mpData[8];
    }

    void SetTrackDevice(unsigned char x)
    {
      mpData[8] = x;
    }

    unsigned char GetIntroLength() const
    {
      return mpData[9];
    }

    void SetIntroLength(unsigned char x)
    {
      mpData[9] = x;
    }

    virtual JZJazzMetaEvent* IsJazzMeta()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZJazzMetaEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZSysExEvent : public JZMetaEvent
{
  public:

    JZSysExEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatSysEx, pData, Length)
    {
    }

    virtual JZSysExEvent* IsSysEx()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZSysExEvent(mClock, mpData, mLength);
    }

    // todo
    virtual int GetPitch() const;
};

//*****************************************************************************
//*****************************************************************************
class JZSongPtrEvent : public JZMetaEvent
{
  public:

    JZSongPtrEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatSongPtr, pData, Length)
    {
    }

    virtual JZSongPtrEvent* IsSongPtr()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZSongPtrEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZMidiClockEvent : public JZMetaEvent
{
  public:
    JZMidiClockEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatMidiClock, pData, Length)
    {
    }

    JZMidiClockEvent(int Clock)
      : JZMetaEvent(Clock, StatMidiClock, 0, 0)
    {
    }

    virtual JZMidiClockEvent *IsMidiClock()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZMidiClockEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZStartPlayEvent : public JZMetaEvent
{
  public:

    JZStartPlayEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatStartPlay, pData, Length)
    {
    }

    JZStartPlayEvent(int Clock)
      : JZMetaEvent(Clock, StatStartPlay, 0, 0)
    {
    }

    virtual JZStartPlayEvent* IsStartPlay()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZStartPlayEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZContPlayEvent : public JZMetaEvent
{
  public:

    JZContPlayEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatContPlay, pData, Length)
    {
    }

    JZContPlayEvent(int Clock)
      : JZMetaEvent(Clock, StatContPlay, 0, 0)
    {
    }

    virtual JZContPlayEvent* IsContPlay()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZContPlayEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZStopPlayEvent : public JZMetaEvent
{
  public:

    JZStopPlayEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatStopPlay, pData, Length)
    {
    }

    JZStopPlayEvent(int Clock)
      : JZMetaEvent(Clock, StatStopPlay, 0, 0)
    {
    }

    virtual JZStopPlayEvent* IsStopPlay()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZStopPlayEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZTextEvent : public JZMetaEvent
{
  public:

    JZTextEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatText, pData, Length)
    {
    }

    JZTextEvent(int Clock, unsigned char* pData)
      : JZMetaEvent(Clock, StatText, pData, strlen((const char*)pData))
    {
    }

    virtual JZTextEvent* IsText()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZTextEvent(mClock, mpData, mLength);
    }

    virtual unsigned char* GetText()
    {
      return mpData;
    }

    void SetText(const char* pText)
    {
      if (pText && strlen(pText) > 0)
      {
        delete [] mpData;
        mpData = new unsigned char[strlen(pText)];
        memcpy(mpData, pText, strlen(pText));
      }
    }
};

//*****************************************************************************
//*****************************************************************************
class JZCopyrightEvent : public JZMetaEvent
{
  public:

    JZCopyrightEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatCopyright, pData, Length)
    {
    }

    virtual JZCopyrightEvent* IsCopyright()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZCopyrightEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZTrackNameEvent : public JZMetaEvent
{
  public:

    JZTrackNameEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatTrackName, pData, Length)
    {
    }

    virtual JZTrackNameEvent* IsTrackName()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZTrackNameEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZMarkerEvent : public JZMetaEvent
{
  public:

    JZMarkerEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatMarker, pData, Length)
    {
    }

    virtual JZMarkerEvent* IsMarker()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZMarkerEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
// the meaning of this event is to be able to reference a track and have that
// play the instant the event is executed.  You can also transpose the
// referenced track and loop it for the duration of the playtrack event.  This
// makes it possible to compose in a structured fashion.
//
// Execution of the events takes place in song.cpp.
//*****************************************************************************
class JZPlayTrackEvent : public JZMetaEvent
{
  public:

    // The index of the track to play.
    int track;

    // How many steps to transpose the track.
    int transpose;

    // The length of the event, confusing with the Length field of JZMetaEvent,
    // that seems to be for serialization.
    int eventlength;

    JZPlayTrackEvent(int Clock, unsigned char *chardat, unsigned short Length)
      : JZMetaEvent(Clock, StatPlayTrack, chardat, Length)
    {
      int* pData = (int *)chardat;

      // Fill in the fields from the data.
      track = 0;
      transpose = 0;
      eventlength = 0;
      if (pData != 0)
      {
        track = pData[0];
        transpose = pData[1];
        eventlength = pData[2];
      }
    }

    JZPlayTrackEvent(int Clock, int track, int transpose, int eventlength)
      : JZMetaEvent(Clock, StatPlayTrack, 0, 0)
    {
      this->track=track;
      this->transpose=transpose;
      this->eventlength=eventlength;
    }

    virtual int GetLength() const
    {
      return eventlength;
    }

    virtual int Write(JZWriteBase &io)
    {
      mpData = new unsigned char [mLength + 1];
      int* pData = (int *)mpData;
      pData[0] = track;
      pData[1] = transpose;
      pData[2] = eventlength;
      mLength = sizeof(int) * 3;

      mpData[mLength] = 0;
      return io.Write(this, mpData, mLength);
    }

    virtual JZPlayTrackEvent* IsPlayTrack()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZPlayTrackEvent(mClock, track, transpose, eventlength);
    }

    // This event has no real "pitch" but the rest of jazz use the pitch.  In
    // the piano winow editor, pitch is the y coordinate of the event.
    virtual int GetPitch() const
    {
      return track;
    }
};

//*****************************************************************************
//*****************************************************************************
class JZSetTempoEvent : public JZEvent
{
  public:
    int uSec;

    JZSetTempoEvent(
      int Clock,
      unsigned char Character1,
      unsigned char Character2,
      unsigned char Character3)
      : JZEvent(Clock, StatSetTempo)
    {
      uSec =
        ((unsigned)Character1 << 16L) +
        ((unsigned)Character2 << 8L) +
        Character3;
    }

    JZSetTempoEvent(int Clock, int bpm)
      : JZEvent(Clock, StatSetTempo)
    {
      SetBPM(bpm);
    }

    virtual int GetBPM() const
    {
      return int(60000000L / uSec);
    }

    virtual void SetBPM(int bpm)
    {
      uSec = 60000000L / bpm;
    }

    virtual int GetPitch() const
    {
      return GetBPM() / 2;
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, (char)(uSec >> 16), (char)(uSec >> 8), (char)uSec);
    }

    virtual JZSetTempoEvent* IsSetTempo()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZSetTempoEvent(*this);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZMtcOffsetEvent : public JZMetaEvent
{
  public:
    JZMtcOffsetEvent(int Clock, unsigned char* pData, unsigned short Length)
      : JZMetaEvent(Clock, StatMtcOffset, pData, Length)
    {
    }

    virtual JZMtcOffsetEvent* IsMtcOffset()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZMtcOffsetEvent(mClock, mpData, mLength);
    }
};

//*****************************************************************************
//*****************************************************************************
class JZTimeSignatEvent : public JZEvent
{
  public:

    unsigned char Numerator, Denomiator, Clocks, Quarter;

    JZTimeSignatEvent(
      int Clock,
      unsigned char Character1,
      unsigned char Character2,
      unsigned char Character3 = 24,
      unsigned char Character4 = 8)
      : JZEvent(Clock, StatTimeSignat)
    {
      Numerator   = Character1;
      Denomiator  = Character2;
      Clocks      = Character3;
      Quarter     = Character4;
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, Numerator, Denomiator, Clocks, Quarter);
    }

    virtual JZTimeSignatEvent* IsTimeSignat()
    {
      return this;
    }

    virtual void BarInfo(
      int &TicksPerBar,
      int &CountsPerBar,
      int TicksPerQuarter) const
    {
      TicksPerBar = 4 * TicksPerQuarter * Numerator / (1 << Denomiator);
      CountsPerBar = Numerator;
    }

    virtual JZEvent* Copy() const
    {
      return new JZTimeSignatEvent(*this);
    }
};

//*****************************************************************************
//   This is the end-of-track event.
//*****************************************************************************
class JZEndOfTrackEvent : public JZEvent
{
  public:

    JZEndOfTrackEvent(int Clock)
      : JZEvent(Clock, StatEndOfTrack)
    {
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this);
    }

    virtual JZEndOfTrackEvent* IsEndOfTrack()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZEndOfTrackEvent(this->GetClock());
    }
};

//*****************************************************************************
//*****************************************************************************
class JZKeySignatEvent : public JZEvent
{
  public:
    int Sharps;
    int Minor;

    JZKeySignatEvent(int Clock, int Character1, int Character2)
      : JZEvent(Clock, StatKeySignat)
    {
      Sharps = Character1;
      Minor  = Character2;
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, Sharps, Minor);
    }

    virtual JZKeySignatEvent* IsKeySignat()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZKeySignatEvent(*this);
    }
};

//*****************************************************************************
// Aftertouch
//*****************************************************************************
class JZKeyPressureEvent: public JZChannelEvent
{
  public:

    JZKeyPressureEvent(
      int Clock,
      unsigned short Channel,
      unsigned char Key,
      unsigned char Value)
      : JZChannelEvent(Clock, StatKeyPressure, Channel),
        mKey(Key),
        mValue(Value)
    {
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, mKey, mValue);
    }

    virtual JZKeyPressureEvent* IsKeyPressure()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZKeyPressureEvent(*this);
    }

    virtual int GetValue() const
    {
      return mValue;
    }

    virtual int GetPitch() const
    {
      return mKey;
    }

    virtual void SetPitch(int Pitch)
    {
      mKey = Pitch;
    }

    unsigned char GetKey() const
    {
      return mKey;
    }

    void SetKey(unsigned char Key)
    {
      mKey = Key;
    }

    short GetPressureValue() const
    {
      return mValue;
    }

    void SetPressureValue(short Value)
    {
      mValue = Value;
    }

  private:

    short mKey;
    short mValue;
};

//*****************************************************************************
// Channel Pressure
//*****************************************************************************
class JZChnPressureEvent : public JZChannelEvent
{
  public:
    unsigned char Value;

    JZChnPressureEvent(int Clock, int Channel, unsigned char val)
      : JZChannelEvent(Clock, StatChnPressure, Channel)
    {
      Value = val;
    }

    virtual int Write(JZWriteBase &io)
    {
      return io.Write(this, Value);
    }

    virtual JZChnPressureEvent* IsChnPressure()
    {
      return this;
    }

    virtual JZEvent* Copy() const
    {
      return new JZChnPressureEvent(*this);
    }

    virtual int GetValue() const
    {
      return Value;
    }

    virtual int GetPitch() const
    {
      return 0;
    }

    virtual void  SetPitch(int v)
    {
    }

    virtual const wxPen* GetPen() const
    {
      return wxGREEN_PEN;
    }

    virtual const wxBrush* GetBrush() const
    {
      return wxGREEN_BRUSH;
    }
};
