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

#include "BitSet.h"
#include "DrumEnums.h"
#include "Events.h"
#include "NamedValue.h"

#include <string>

class JZTrackWindow;
class wxDialog;

// Track-States

#define tsPlay 0
#define tsMute 1
#define tsSolo 2

// Mixer-defs
enum
{
  MxVol = 0,
  MxPan,
  MxRev,
  MxCho,
  MxParams
};

// Param (Nrpn / Rpn) things
enum
{
  NrpnVibRate = 0,
  NrpnVibDepth,
  NrpnVibDelay,
  NrpnVibParams
};

enum
{
  NrpnCutoff = 0,
  NrpnResonance,
  NrpnSoundParams
};

enum
{
  NrpnEnvAttack = 0,
  NrpnEnvDecay,
  NrpnEnvRelease,
  NrpnEnvParams
};

//*****************************************************************************
//*****************************************************************************
class JZParam
{
  public:

    JZParam(
      int Clock,
      int Channel,
      unsigned char id1,
      unsigned char msb,
      unsigned char id2,
      unsigned char lsb,
      unsigned char msbval)
      : mMsb(Clock, Channel, id1, msb),
        mLsb(Clock, Channel, id2, lsb),
        mDataMsb(Clock, Channel, 0x06, msbval),
        mResetMsb(Clock, Channel, id1, 0x7f),
        mResetLsb(Clock, Channel, id2, 0x7f)
    {
    }

    virtual ~JZParam()
    {
    }

    virtual int Write(JZWriteBase& Io);

    virtual void SetChannel(unsigned char Channel);

    virtual int GetVal()
    {
      return mDataMsb.GetValue();
    }

    JZControlEvent mMsb;
    JZControlEvent mLsb;
    JZControlEvent mDataMsb;
    JZControlEvent mResetMsb;
    JZControlEvent mResetLsb;
};

//*****************************************************************************
//*****************************************************************************
class JZNrpn : public JZParam
{
  public:

    JZNrpn(
      int Clock,
      int Channel,
      unsigned char msb,
      unsigned char lsb,
      unsigned char msbval)
      : JZParam(Clock, Channel, 0x63, msb, 0x62, lsb, msbval)
    {
    }
};

//*****************************************************************************
//*****************************************************************************
class JZRpn : public JZParam
{
  public:

    JZRpn(
      int Clock,
      int Channel,
      unsigned char msb,
      unsigned char lsb,
      unsigned char msbval)
      : JZParam(Clock, Channel, 0x65, msb, 0x64, lsb, msbval)
    {
    }
};

unsigned char* SysExDT1(
  unsigned char aa,
  unsigned char bb,
  unsigned char cc,
  int length,
  unsigned char* dd);

enum ModulationSysexParameter
{
  mspModPitchControl = 0,
  mspModTvfCut,
  mspModAmpl,
  mspModLfo1Rate,
  mspModLfo1Pitch,
  mspModLfo1Tvf,
  mspModLfo1Tva,
  mspModLfo2Rate,
  mspModLfo2Pitch,
  mspModLfo2Tvf,
  mspModLfo2Tva,
  mspModulationSysexParameters
};

enum BenderSysexParameter
{
  bspBendPitchControl = 0,
  bspBendTvfCut,
  bspBendAmpl,
  bspBendLfo1Rate,
  bspBendLfo1Pitch,
  bspBendLfo1Tvf,
  bspBendLfo1Tva,
  bspBendLfo2Rate,
  bspBendLfo2Pitch,
  bspBendLfo2Tvf,
  bspBendLfo2Tva,
  bspBenderSysexParameters
};

enum CAfSysexParameter
{
  cspCAfPitchControl = 0,
  cspCAfTvfCut,
  cspCAfAmpl,
  cspCAfLfo1Rate,
  cspCAfLfo1Pitch,
  cspCAfLfo1Tvf,
  cspCAfLfo1Tva,
  cspCAfLfo2Rate,
  cspCAfLfo2Pitch,
  cspCAfLfo2Tvf,
  cspCAfLfo2Tva,
  cspCAfSysexParameters
};

enum PAfSysexParameter
{
  pspPAfPitchControl = 0,
  pspPAfTvfCut,
  pspPAfAmpl,
  pspPAfLfo1Rate,
  pspPAfLfo1Pitch,
  pspPAfLfo1Tvf,
  pspPAfLfo1Tva,
  pspPAfLfo2Rate,
  pspPAfLfo2Pitch,
  pspPAfLfo2Tvf,
  pspPAfLfo2Tva,
  pspPAfSysexParameters
};

enum CC1SysexParameter
{
  cspCC1PitchControl = 0,
  cspCC1TvfCut,
  cspCC1Ampl,
  cspCC1Lfo1Rate,
  cspCC1Lfo1Pitch,
  cspCC1Lfo1Tvf,
  cspCC1Lfo1Tva,
  cspCC1Lfo2Rate,
  cspCC1Lfo2Pitch,
  cspCC1Lfo2Tvf,
  cspCC1Lfo2Tva,
  cspCC1SysexParameters
};

enum CC2SysexParameter
{
  cspCC2PitchControl = 0,
  cspCC2TvfCut,
  cspCC2Ampl,
  cspCC2Lfo1Rate,
  cspCC2Lfo1Pitch,
  cspCC2Lfo1Tvf,
  cspCC2Lfo1Tva,
  cspCC2Lfo2Rate,
  cspCC2Lfo2Pitch,
  cspCC2Lfo2Tvf,
  cspCC2Lfo2Tva,
  cspCC2SysexParameters
};

enum ReverbSysexParameter
{
  rspRevCharacter = 0,
  rspRevPreLpf,
  rspRevLevel,
  rspRevTime,
  rspRevDelayFeedback,
  rspRevSendChorus,
  rspReverbSysexParameters
};

enum ChorusSysexParameter
{
  cspChoPreLpf = 0,
  cspChoLevel,
  cspChoFeedback,
  cspChoDelay,
  cspChoRate,
  cspChoDepth,
  cspChoSendReverb,
  cspChorusSysexParameters
};

enum ModeSysexParameter
{
  mspRxChannel = 0x02,
  mspRxCAf = 0x04,
  mspRxPAf = 0x07,
  mspUseForRhythm = 0x15
};

class JZDrumInstrumentParameterList;

//*****************************************************************************
//*****************************************************************************
class JZDrumInstrumentParameter
{
    friend class JZDrumInstrumentParameterList;

  public:

    JZDrumInstrumentParameter(JZNrpn* par);
    JZNrpn* Get(int index);
    void Put(JZNrpn* par);
    JZDrumInstrumentParameter* Next();
    int Pitch();

  private:

    int mPitch;
    JZNrpn* mParam[numDrumParameters];
    JZDrumInstrumentParameter* mpNext;
};

//*****************************************************************************
//*****************************************************************************
class JZDrumInstrumentParameterList
{
  public:

    JZDrumInstrumentParameterList()
      : mpList(0)
    {
    }
    JZDrumInstrumentParameter* GetElem(int pit);
    JZNrpn* GetParam(int pit, int index);
    void PutParam(JZNrpn* par);
    void DelParam(int pit, int index);
    JZDrumInstrumentParameter *FirstElem();
    JZDrumInstrumentParameter *NextElem(JZDrumInstrumentParameter *cur);
    void DelElem(int pit);
    void Clear();
    bool IsEmpty() const
    {
      return mpList == 0;
    }

  private:

    JZDrumInstrumentParameter* mpList;
};

enum tMtcType
{
  Mtc24 = 0,
  Mtc25,
  Mtc30Df,
  Mtc30Ndf
};

//*****************************************************************************
// Description:
//   MTC stands for MIDI time code or MIDI time division.
//*****************************************************************************
class JZMtcTime
{
  public:

    JZMtcTime(JZMtcOffsetEvent* pMtcOffsetEvent);
    JZMtcTime(int millisek, tMtcType t);
    JZMtcTime(char* str, tMtcType t);
    tMtcType GetType() const
    {
      return mType;
    }
    JZMtcTime(unsigned h, unsigned m, unsigned s, unsigned f, unsigned t);
    void ToString(std::string& String);
    JZMtcOffsetEvent* ToOffset();
    int ToMillisec();

  private:

    tMtcType mType;
    int mHours;
    int mMinutes;
    int mSeconds;
    int fm;
};


//*****************************************************************************
//*****************************************************************************
class JZSimpleEventArray
{
  public:

    // Resize **mppEvents
    void Resize();

    virtual void Clear();

    void Put(JZEvent* pEvent);

    void GrabData(JZSimpleEventArray &src);

    void Copy(JZSimpleEventArray& src, int frClock, int toClock);

    JZSimpleEventArray();

    virtual ~JZSimpleEventArray();

    void Sort();

    void RemoveEOT();

  public:

    // Actual number of events in **mppEvents.
    int mEventCount;

    // Memory allocated in **mppEvents
    int mMaxEvents;

    JZEvent** mppEvents;
};


//*****************************************************************************
//*****************************************************************************
class JZUndoBuffer : public JZSimpleEventArray
{
  friend class JZTrack;

  public:

    virtual void Clear();

    void Put(JZEvent* pEvent, int killed)
    {
      mBits.set(mEventCount, killed);
      JZSimpleEventArray::Put(pEvent);
    }

  private:

    // Set for killed events.
    JZBitset mBits;
};


//*****************************************************************************
//*****************************************************************************
class JZEventArray : public JZSimpleEventArray
{
    friend class JZEventIterator;
    friend class JZTrackDlg;
    friend class JZTrack;

  public:

    JZTrackNameEvent* mpName;
    JZCopyrightEvent* mpCopyright;
    JZProgramEvent* mpPatch;
    JZSetTempoEvent* mpSpeed;
    JZControlEvent* mpVolume;
    JZControlEvent* mpPan;
    JZControlEvent* mpReverb;
    JZControlEvent* mpChorus;
    JZControlEvent* mpBank;
    JZControlEvent* mpBank2;

  public:

    JZSysExEvent* mpReset;

    JZSysExEvent* mpModulationSettings[mspModulationSysexParameters];
    JZSysExEvent* mpBenderSettings[bspBenderSysexParameters];
    JZSysExEvent* mpCAfSettings[cspCAfSysexParameters];
    JZSysExEvent* mpPAfSettings[pspPAfSysexParameters];
    JZSysExEvent* mpCC1Settings[cspCC1SysexParameters];
    JZSysExEvent* mpCC2Settings[cspCC2SysexParameters];

    JZSysExEvent* mpCC1ControllerNr;
    JZSysExEvent* mpCC2ControllerNr;

    JZSysExEvent* mpReverbType;
    JZSysExEvent* mpChorusType;
    JZSysExEvent* mpEqualizerType;

    JZSysExEvent* mpReverbSettings[rspReverbSysexParameters];
    JZSysExEvent* mpChorusSettings[cspChorusSysexParameters];

    JZSysExEvent* mpPartialReserve;
    JZSysExEvent* mpMasterVol;
    JZSysExEvent* mpMasterPan;

    JZSysExEvent* mpRxChannel;
    JZSysExEvent* mpUseForRhythm ;

    JZMtcOffsetEvent* mpMtcOffset;

    JZNrpn* mpVibRate;
    JZNrpn* mpVibDepth;
    JZNrpn* mpVibDelay;
    JZNrpn* mpCutoff;
    JZNrpn* mpResonance;
    JZNrpn* mpEnvAttack;
    JZNrpn* mpEnvDecay;
    JZNrpn* mpEnvRelease;
    JZRpn* mpBendPitchSens;

    JZDrumInstrumentParameterList mDrumParams;

    int mChannel;  // 1..16, set from first ChannelEvent, 0 = multichannel/nochannel
    int mDevice;   // 0 for JZSeq2Player/JZMpuPlayer
    int mForceChannel;

    virtual void Clear();
    void Cleanup(bool dont_delete_killed_events = false);

    void Keyoff2Length();
    void Length2Keyoff();

    JZEventArray();
    virtual ~JZEventArray();

    void Read(JZReadBase& Io);
    void Write(JZWriteBase& Io);

    int GetLastClock() const;
    bool IsEmpty() const;
    int GetFirstClock();

    int mState;    // tsXXX

  public:

    bool GetAudioMode() const
    {
      return mAudioMode;
    }

    void SetAudioMode(bool AudioMode)
    {
      mAudioMode = AudioMode;
    }

  protected:

    bool mAudioMode;
};


#define MaxUndo 20

//*****************************************************************************
// Description:
//   This is the track class declaration.
//*****************************************************************************
class JZTrack : public JZEventArray
{
  public:

    static bool mChanged;

    JZTrack();

    virtual ~JZTrack();

    bool IsDrumTrack();

  public:

    void Edit(JZTrackWindow* pParent);

    bool IsEditing() const;

    void Put(JZEvent* pEvent)
    {
      mChanged = true;
      JZEventArray::Put(pEvent);
      mUndoBuffers[mUndoIndex].Put(pEvent, 0);
    }

    void Kill(JZEvent* pEvent)
    {
      mChanged = true;
      pEvent->Kill();
      mUndoBuffers[mUndoIndex].Put(pEvent, 1);
    }

    void Merge(JZEventArray *other);

    void MergeRange(
      const JZEventArray& Other,
      int FromClock,
      int ToClock,
      int Replace = 0);

    void Undo();
    void Redo();
    void NewUndoBuffer();
    void Clear();
    void Cleanup();

    const char* GetName();
    void SetName(const char* Name);

    const char* GetCopyright();
    void SetCopyright(char *Copyright);

    const char* GetStateChar();
    void SetState(int NewState);
    void ToggleState(int Direction);   // +1 = next, -1 = prev

    int GetChannel()
    {
      return mChannel;
    }
    void SetChannel(int NewChannel);

    int GetDevice() const
    {
      return mDevice;
    }
    void SetDevice(int Device)
    {
      mDevice = Device;
    }

    int  GetPatch();
    void SetPatch(int PatchNr);

    int  GetVolume();
    void SetVolume(int Volume);
    bool DecreaseVolume();
    bool IncreaseVolume();

    int  GetPan();
    void SetPan(int Pan);

    int  GetReverb();
    void SetReverb(int Reverb);

    int  GetChorus();
    void SetChorus(int Chorus);

    int  GetVibRate();
    void SetVibRate(int VibRate);

    int  GetVibDepth();
    void SetVibDepth(int VibDepth);

    int  GetVibDelay();
    void SetVibDelay(int VibDelay);

    int  GetCutoff();
    void SetCutoff(int Cutoff);

    int  GetResonance();
    void SetResonance(int Resonance);

    int  GetEnvAttack();
    void SetEnvAttack(int EnvAttack);

    int  GetEnvDecay();
    void SetEnvDecay(int EnvDecay);

    int  GetEnvRelease();
    void SetEnvRelease(int EnvRelease);

    int  GetDrumParam(int pitch, int index);
    void SetDrumParam(int pitch, int index, int Value);

    int  GetBendPitchSens();
    void SetBendPitchSens(int BendPitchSens);

    int  GetModulationSysex(int msp);
    void SetModulationSysex(int msp, int value);

    int  GetBenderSysex(int bsp);
    void SetBenderSysex(int bsp, int value);

    int  GetCAfSysex(int csp);
    void SetCAfSysex(int csp, int value);

    int  GetPAfSysex(int psp);
    void SetPAfSysex(int psp, int value);

    int  GetCC1Sysex(int csp);
    void SetCC1Sysex(int csp, int value);

    int  GetCC2Sysex(int csp);
    void SetCC2Sysex(int csp, int value);

    int  GetCC1ControllerNr();
    void SetCC1ControllerNr(int ctrlno);

    int  GetCC2ControllerNr();
    void SetCC2ControllerNr(int ctrlno);

    int  GetReverbType(int lsb = 0);
    void SetReverbType(int ReverbType, int lsb = 0);

    int  GetChorusType(int lsb = 0);
    void SetChorusType(int ChorusType, int lsb = 0);

    int  GetEqualizerType();
    void SetEqualizerType(int EqualizerType);

    int  GetRevSysex(int rsp);
    void SetRevSysex(int rsp, int value);

    int  GetChoSysex(int csp);
    void SetChoSysex(int csp, int value);

    int  GetBank();
    void SetBank(int Bank);

    int  GetDefaultSpeed();  // beats per minute
    void SetDefaultSpeed(int bpm);

    int  GetCurrentSpeed(int Clock);  // beats per minute

    JZSetTempoEvent* GetCurrentTempo(int Clock);

    int  GetMasterVol();
    void SetMasterVol(int MasterVol);

    int  GetMasterPan();
    void SetMasterPan(int MasterPan);

    int GetPartRsrv(int chan);
    void SetPartRsrv(unsigned char* rsrv);

    int  GetModeSysex(int param);
    void SetModeSysex(int param, int value);

    JZMtcTime* GetMtcOffset();
    void SetMtcOffset(JZMtcTime* mtc);

  private:

    // Index in the actual undo buffer.
    int mUndoIndex;

    // Current number of possible redo's.
    int mRedoCount;

    // Current number of possible undo's.
    int mUndoCount;

    JZUndoBuffer mUndoBuffers[MaxUndo];

    wxDialog* mpDialog;
};


//*****************************************************************************
// Description:
//   This is the event iterator class declaration.
//*****************************************************************************
class JZEventIterator
{
  public:

    JZEventIterator(const JZSimpleEventArray* pTrack)
    {
      mpTrack = pTrack;
      mStart  = 0;
      mStop   = mpTrack->mEventCount;
      mActual = mStart;
    }

    JZEvent* GreaterEqual(int Clock)
    {
      int Lo = mStart;
      int Hi = mStop;
      int TestClock = 0;
      while (Lo < Hi)
      {
        mActual  = (Hi + Lo) / 2;
        TestClock = mpTrack->mppEvents[mActual]->GetClock();
        if (TestClock < Clock)
        {
          Lo = mActual + 1;
        }
        else
        {
          Hi = mActual;
        }
      }
      if (mActual < mStop - 1 && TestClock < Clock)
      {
        TestClock = mpTrack->mppEvents[++mActual]->GetClock();
      }
      if (mActual < mStop && TestClock >= Clock)
      {
        return mpTrack->mppEvents[mActual];
      }
      return 0;
    }

    JZEvent* First(int Clock = 0)
    {
      mActual = mStart;
      return GreaterEqual(Clock);
    }

    JZEvent* Range(int FromClock, unsigned ToClock)
    {
      mStart = mActual = 0;
      mStop  = mpTrack->mEventCount;

      if (!GreaterEqual(FromClock))
      {
        return 0;
      }
      mStart = mActual;
      if (GreaterEqual(ToClock))
      {
        mStop = mActual;
      }
      mActual = mStart;
      return (mActual < mStop ? mpTrack->mppEvents[mActual] : 0);
    }

    JZEvent* Next()
    {
      if (mActual < mStop)
      {
        ++mActual;
      }
      return (mActual < mStop ? mpTrack->mppEvents[mActual] : 0);
    }

    int EventsLeft()
    {
      return mStop - mActual;
    }

  private:

    const JZSimpleEventArray* mpTrack;
    int mStart, mStop, mActual;
};
