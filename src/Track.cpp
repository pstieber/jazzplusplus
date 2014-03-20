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

#include "Track.h"

#include "Configuration.h"
#include "Dialogs/TrackDialog.h"
#include "DrumUtilities.h"
#include "Globals.h"
#include "JazzPlusPlusApplication.h"
#include "Player.h"
#include "Song.h"
#include "Synth.h"
#include "SysexChannel.h"
#include "TrackWindow.h"

#include <cassert>
#include <cstdlib>
#include <sstream>

using namespace std;

int JZParam::Write(JZWriteBase& Io)
{
  return mMsb.Write(Io) + mLsb.Write(Io) + mDataMsb.Write(Io);
}

void JZParam::SetChannel(unsigned char Channel)
{
  mMsb.SetChannel(Channel);
  mLsb.SetChannel(Channel);
  mDataMsb.SetChannel(Channel);

#ifdef OBSOLETE
  mResetMb.SetChannel(Channel); //???? JAVE commented out this while porting
#endif // OBSOLETE

  mResetLsb.SetChannel(Channel);
}

/*
unsigned char sys_Sysex[7] = { 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x00 };

unsigned char *SysExDT1(
  unsigned char aa,
  unsigned char bb,
  unsigned char cc,
  int datalen,
  unsigned char *data)
{
  int length = 9 + datalen;
  unsigned char* mess = new unsigned char[length];
  mess[0] = 0x41;
  mess[1] = 0x10;
  mess[2] = 0x42;
  mess[3] = 0x12;
  mess[4] = aa;
  mess[5] = bb;
  mess[6] = cc;
  int i;
  for (i = 0; i < datalen; i++)
  {
    mess[i+7] = data[i];
  }
  unsigned char sum = 0x00;
  for (i = 4; i < (length-2); i++)
  {
    sum += mess[i];
  }
  mess[length - 2] = (0x80 - (sum & 0x7f)) & 0x7f;
  mess[length - 1] = 0xf7;
  return mess;
}
*/

static double gFramesPerSecond[] =
{
  24.0,
  25.0,
  30.0,
  30.0
};

JZMtcTime::JZMtcTime(JZMtcOffsetEvent* pMtcOffset)
{
  const unsigned char* pData = pMtcOffset->GetData();
  mType = (tMtcType) ((pData[0] & 0x60) >> 5);
  if (mType < Mtc24)
  {
    mType = Mtc24;
  }
  if (mType > Mtc30Ndf)
  {
    mType = Mtc30Ndf;
  }
  mHours = pData[0] & 0x1f;
  mMinutes = pData[1];
  mSeconds = pData[2];
  fm = pData[3];
}

JZMtcTime::JZMtcTime(int millisec, tMtcType Type)
{
  mType = Type;
  if (mType < Mtc24)
  {
    mType = Mtc24;
  }
  if (mType > Mtc30Ndf)
  {
    mType = Mtc30Ndf;
  }
  mSeconds = millisec / 1000;
  int msec = millisec % 1000;
  mMinutes = mSeconds / 60;
  mSeconds = mSeconds % 60;
  mHours = mMinutes / 60;
  mMinutes = mMinutes % 60;
  double frametime = 1000.0 / gFramesPerSecond[mType];
  fm = (int) ((double) msec / frametime);
}

JZMtcTime::JZMtcTime(char* str, tMtcType Type)
  : mHours(0),
    mMinutes(0),
    mSeconds(0),
    fm(0)
{
  mType = Type;
  if (mType < Mtc24)
  {
    mType = Mtc24;
  }
  if (mType > Mtc30Ndf)
  {
    mType = Mtc30Ndf;
  }
  sscanf(str, "%d:%d:%d.%d", &mHours, &mMinutes, &mSeconds, &fm);
  if (fm >= gFramesPerSecond[mType])
  {
    fm = (int) gFramesPerSecond[mType] - 1;
  }
}

JZMtcTime::JZMtcTime(unsigned Hours, unsigned Minutes, unsigned Seconds, unsigned f, unsigned Type)
{
  mHours = Hours;
  mMinutes = Minutes;
  mSeconds = Seconds;
  fm = f;
  mType = (tMtcType) Type;
  if (mType < Mtc24)
  {
    mType = Mtc24;
  }
  if (mType > Mtc30Ndf)
  {
    mType = Mtc30Ndf;
  }
}

void JZMtcTime::ToString(string& String)
{
  ostringstream Oss;
  Oss << mHours << ':' << mMinutes << ':' << mSeconds << '.' << fm;
  String = Oss.str();
}

JZMtcOffsetEvent* JZMtcTime::ToOffset()
{
  unsigned char* mess = new unsigned char[5];
  mess[0] = (unsigned char) mHours | ((unsigned char) mType << 5);
  mess[1] = (unsigned char) mMinutes;
  mess[2] = (unsigned char) mSeconds;
  mess[3] = (unsigned char) fm;
  mess[4] = 0x00;
  JZMtcOffsetEvent* s = new JZMtcOffsetEvent(0, mess, 5);
  delete mess;
  return s;
}

int JZMtcTime::ToMillisec()
{
  int msec = (((((mHours * 60) + mMinutes) * 60) + mSeconds) * 1000) +
              ((fm * 1000) / (int) gFramesPerSecond[mType]);
  return msec;
}

JZDrumInstrumentParameter::JZDrumInstrumentParameter(JZNrpn* par)
  : mPitch(par->mLsb.GetControlValue()),
    mpNext(0)
{
  for (int i = drumPitchIndex; i < numDrumParameters; i++)
  {
    mParam[i] = 0;
  }
  mParam[drumParam2Index(par->mMsb.GetControlValue())] = par;
}

JZNrpn* JZDrumInstrumentParameter::Get(int index)
{
  assert((index >= drumPitchIndex) && (index < numDrumParameters));
  return(mParam[index]);
}

void JZDrumInstrumentParameter::Put(JZNrpn* par)
{
  mParam[par->mLsb.GetControlValue()] = par;
}

JZDrumInstrumentParameter* JZDrumInstrumentParameter::Next()
{
  return mpNext;
}

int JZDrumInstrumentParameter::Pitch()
{
  return mPitch;
}

JZDrumInstrumentParameter*
JZDrumInstrumentParameterList::GetElem(int pit)
{
  JZDrumInstrumentParameter* ptr = mpList;
  while (ptr)
  {
    if (ptr->mPitch == pit)
    {
      break;
    }
    ptr = ptr->mpNext;
  }
  return ptr;
}

JZNrpn* JZDrumInstrumentParameterList::GetParam(int pit, int index)
{
  JZDrumInstrumentParameter* ptr = GetElem(pit);
  if (ptr)
  {
    return ptr->Get(index);
  }
  return 0;
}

void JZDrumInstrumentParameterList::PutParam(JZNrpn* par)
{
  JZDrumInstrumentParameter* ptr = GetElem(par->mLsb.GetControlValue());
  if (!ptr)
  {
    ptr = new JZDrumInstrumentParameter(par);
    ptr->mpNext = mpList;
    mpList = ptr;
  }
  else
  {
    ptr->mParam[drumParam2Index(par->mMsb.GetControlValue())] = par;
  }
}

void JZDrumInstrumentParameterList::DelParam(int pit, int index)
{
  if (mpList)
  {
    JZDrumInstrumentParameter* elem = GetElem(pit);
    if (elem)
    {
      if (elem->Get(index))
      {
        delete elem->mParam[index];
      }
      elem->mParam[index] = 0;
    }
  }
}

void JZDrumInstrumentParameterList::DelElem(int pit)
{
  for (int i = drumPitchIndex; i < numDrumParameters; i++)
  {
    DelParam(pit, i);
  }

  JZDrumInstrumentParameter* ptr = mpList;
  JZDrumInstrumentParameter* prev = 0;
  while (ptr)
  {
    if (ptr->mPitch == pit)
    {
      if (prev)
      {
        prev->mpNext = ptr->mpNext;
      }
      else
      {
        mpList = ptr->mpNext;
      }
      delete ptr;
      break;
    }
    prev = ptr;
    ptr = ptr->mpNext;
  }
}

JZDrumInstrumentParameter* JZDrumInstrumentParameterList::FirstElem()
{
  return mpList;
}

JZDrumInstrumentParameter* JZDrumInstrumentParameterList::NextElem(
  JZDrumInstrumentParameter* cur)
{
  if (cur)
  {
    JZDrumInstrumentParameter* ptr = GetElem(cur->mPitch);
    if (ptr)
    {
      return ptr->mpNext;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

void JZDrumInstrumentParameterList::Clear()
{
  JZDrumInstrumentParameter* ptr = mpList;
  while (ptr)
  {
    mpList = ptr->mpNext;
    delete ptr;
    ptr = mpList;
  }
}


JZSimpleEventArray::JZSimpleEventArray()
  : mEventCount(0),
    mMaxEvents(0),
    mppEvents(0)
{
}

JZSimpleEventArray::~JZSimpleEventArray()
{
  Clear();
  delete [] mppEvents;
  mppEvents = 0;
}

void JZSimpleEventArray::Clear()
{
  int i;
  for (i = 0; i < mEventCount; i++)
  {
    delete mppEvents[i];
  }
  mEventCount = 0;
}


void JZUndoBuffer::Clear()
{
  int i;
  for (i = 0; i < mEventCount; i++)
  {
    if (mBits(i))
    {
      delete mppEvents[i];
    }
  }
  mEventCount = 0;
}


void JZSimpleEventArray::Resize()
{
  int i;
  mMaxEvents += 50;
  JZEvent** ppEvents = new JZEvent* [mMaxEvents];

  // Copy the previuosly existing event pointers.
  for (i = 0; i < mEventCount; ++i)
  {
    ppEvents[i] = mppEvents[i];
  }

  // Initialize the new event pointers to 0.
  for (; i < mMaxEvents; ++i)
  {
    ppEvents[i] = 0;
  }

  // Delete the old event pointers
  delete [] mppEvents;

  // Set the data member to the new storage location.
  mppEvents = ppEvents;
}

//   Remove any end of track (EOT) events from the track.  There can only be
// one EOT event in a track, so remove it before inserting a new one.
void JZSimpleEventArray::RemoveEOT()
{
  int j = 0;
  int newnEvents = mEventCount;
  for (int i = 0; i < mEventCount; ++i)
  {
    if (mppEvents[i] != 0 && mppEvents[i]->IsEndOfTrack())
    {
      delete mppEvents[i];
      ++j;
      --newnEvents;
    }

    JZEvent* item;
    if (j <= mMaxEvents)
    {
      item = mppEvents[j++];
    }
    else
    {
      item = 0;
    }
    mppEvents[i] = item;
  }
  mEventCount = newnEvents;
}

void JZSimpleEventArray::Put(JZEvent* pEvent)
{
  if (pEvent->IsEndOfTrack())
  {
    // Remove the old EOT if we are adding a new one.
    RemoveEOT();
  }
  if (mEventCount >= mMaxEvents)
  {
    Resize();
  }
  mppEvents[mEventCount++] = pEvent;
}

// Description:
//   Move the data from passed event array this instance.
void JZSimpleEventArray::GrabData(JZSimpleEventArray& src)
{
  Clear();

  delete [] mppEvents;

  mppEvents = src.mppEvents;
  mEventCount = src.mEventCount;
  mMaxEvents = src.mMaxEvents;

  src.mppEvents = 0;
  src.mEventCount = 0;
  src.mMaxEvents = 0;
}


void JZSimpleEventArray::Copy(JZSimpleEventArray& src, int frclk, int toclk)
{
  JZEventIterator iter(&src);
  JZEvent* pEvent = iter.Range(frclk, toclk);
  while (pEvent)
  {
    Put(pEvent->Copy());
    pEvent = iter.Next();
  }
}


JZEventArray::JZEventArray()
  : JZSimpleEventArray(),
    mpName(0),
    mpCopyright(0),
    mpPatch(0),
    mpSpeed(0),
    mpVolume(0),
    mpPan(0),
    mpReverb(0),
    mpChorus(0),
    mpBank(0),
    mpBank2(0),
    mpReset(0),
    mAudioMode(false)
{
  mEventCount = 0;

  mMaxEvents = 0;
  mppEvents = 0;
  mChannel = 0;
  mDevice = 0;
  mForceChannel = 0;
  mState = tsPlay;

  Clear();
}


void JZEventArray::Clear()
{
  int i;

  JZSimpleEventArray::Clear();

//  delete mpName;
  mpName = 0;

//  delete mpCopyright;
  mpCopyright = 0;

  delete mpPatch;
  mpPatch = 0;

//  delete mpVolume;
  mpVolume = 0;

//  delete mpPan;
  mpPan = 0;

//  delete mpReverb;
  mpReverb = 0;

//  delete mpChorus;
  mpChorus = 0;

  delete mpBank;
  mpBank = 0;

  delete mpBank2;
  mpBank2 = 0;

  delete mpReset;
  mpReset = 0;

//  delete mpSpeed;
  mpSpeed = 0;

  mChannel = 1;
  mDevice = 0;

  for (i = 0; i < mspModulationSysexParameters; i++)
  {
    mpModulationSettings[i] = 0;
  }

  for (i = 0; i < bspBenderSysexParameters; i++)
  {
    mpBenderSettings[i] = 0;
  }

  for (i = 0; i < cspCAfSysexParameters; i++)
  {
    mpCAfSettings[i] = 0;
  }

  for (i = 0; i < pspPAfSysexParameters; i++)
  {
    mpPAfSettings[i] = 0;
  }

  for (i = 0; i < cspCC1SysexParameters; i++)
  {
    mpCC1Settings[i] = 0;
  }

  for (i = 0; i < cspCC2SysexParameters; i++)
  {
    mpCC2Settings[i] = 0;
  }

  mpCC1ControllerNr = 0;
  mpCC2ControllerNr = 0;

  mpReverbType = 0;
  mpChorusType = 0;
  mpEqualizerType = 0;

  for (i = 0; i < rspReverbSysexParameters; i++)
  {
    mpReverbSettings[i] = 0;
  }

  for (i = 0; i < cspChorusSysexParameters; i++)
  {
    mpChorusSettings[i] = 0;
  }

  mpPartialReserve = 0;
  mpMasterVol = 0;
  mpMasterPan = 0;
  mpRxChannel = 0;
  mpUseForRhythm  = 0;
  mpMtcOffset = 0;

  mpVibRate = 0;
  mpVibDepth = 0;
  mpVibDelay = 0;
  mpCutoff = 0;
  mpResonance = 0;
  mpEnvAttack = 0;
  mpEnvDecay = 0;
  mpEnvRelease = 0;
  mpBendPitchSens = 0;

  mDrumParams.Clear();

  if (mppEvents)
  {
    delete [] mppEvents;
  }
  mppEvents = 0;
  mMaxEvents = 0;

  mState = tsPlay;
  mAudioMode = false;
}


JZEventArray::~JZEventArray()
{
  Clear();
}





static int compare(const void* p1, const void* p2)
{
  JZEvent* e1 = *(JZEvent **)p1;
  JZEvent* e2 = *(JZEvent **)p2;
  return e1->Compare(*e2);
}


void JZSimpleEventArray::Sort()
{
  qsort(mppEvents, mEventCount, sizeof(JZEvent*), compare);
}



void JZEventArray::Cleanup(bool dont_delete_killed_events)
{
  JZEvent* pEvent;
  JZControlEvent* pControl;
  JZSysExEvent* s;
  int i;

  Sort();  // moves all killed events to the end of array

  // clear track defaults
//  delete mpName;
  mpName = 0;

  mpCopyright = 0;

  mpSpeed = 0;

  mpVolume = 0;

  mpPan = 0;

  mpReverb = 0;

  mpChorus = 0;

  for (i = 0; i < mspModulationSysexParameters; i++)
  {
    mpModulationSettings[i] = 0;
  }

  for (i = bspBendPitchControl; i < bspBenderSysexParameters; i++)
  {
    mpBenderSettings[i] = 0;
  }

  for (i = 0; i < cspCAfSysexParameters; i++)
  {
    mpCAfSettings[i] = 0;
  }

  for (i = 0; i < pspPAfSysexParameters; i++)
  {
    mpPAfSettings[i] = 0;
  }

  for (i = 0; i < cspCC1SysexParameters; i++)
  {
    mpCC1Settings[i] = 0;
  }

  for (i = 0; i < cspCC2SysexParameters; i++)
  {
    mpCC2Settings[i] = 0;
  }

  mpCC1ControllerNr = 0;
  mpCC2ControllerNr = 0;

  mpReverbType = 0;
  mpChorusType = 0;
  mpEqualizerType = 0;

  for (i = 0; i < rspReverbSysexParameters; i++)
  {
    mpReverbSettings[i] = 0;
  }

  for (i = 0; i < cspChorusSysexParameters; i++)
  {
    mpChorusSettings[i] = 0;
  }

  mpPartialReserve = 0;
  mpMasterVol = 0;
  mpMasterPan = 0;
  mpRxChannel = 0;
  mpUseForRhythm  = 0;
  mpMtcOffset = 0;

  for (i = 0; i < mEventCount; i++)
  {
    if ((pEvent = mppEvents[i])->IsKilled())
    {
      if (!dont_delete_killed_events)
      {
        for (int j = i; j < mEventCount; j++)
        {
          delete mppEvents[j];
        }
      }
      mEventCount = i;
      break;
    }

    // accept only events having clock == 0 as track defaults
    if (pEvent->GetClock() != 0)
    {
      continue;
    }

    if (!mpName)
    {
      mpName = pEvent->IsTrackName();
    }

    if (!mpCopyright)
    {
      mpCopyright = pEvent->IsCopyright();
    }
    if (!mpSpeed)
    {
      mpSpeed = pEvent->IsSetTempo();
    }
    if (!mpMtcOffset)
    {
      mpMtcOffset = pEvent->IsMtcOffset();
    }
    if ((pControl = pEvent->IsControl()) != 0)
    {
      switch (pControl->GetControl())
      {
        case 0x07:
          if (!mpVolume)
          {
            mpVolume = pControl;
          }
          break;
        case 0x0a:
          if (!mpPan)
          {
            mpPan = pControl;
          }
          break;
        case 0x5b:
          if (!mpReverb)
          {
            mpReverb = pControl;
          }
          break;
        case 0x5d:
          if (!mpChorus)
          {
            mpChorus = pControl;
          }
          break;
      }
    }
    if ((s = pEvent->IsSysEx()) != 0)
    {
      int SysExId = gpSynth->GetSysexId(s);

      if (!gpSynth->IsGS())
      {
        switch (SysExId)
        {
          case SX_GM_MasterVol:
            // GS has its own; SC-55 doesn't recognize GM Mastervol
            mpMasterVol = s;
            break;
          default:
            break;
        }
      }

      if (gpSynth->IsGS())
      {
        switch (SysExId)
        {
          case SX_GS_MasterVol:
            mpMasterVol = s;
            break;
          case SX_GS_MasterPan:
            mpMasterPan = s;
            break;
          case SX_GS_BendPitch:
          case SX_GS_BendTvf:
          case SX_GS_BendAmpl:
          case SX_GS_BendLfo1Rate:
          case SX_GS_BendLfo1Pitch:
          case SX_GS_BendLfo1Tvf:
          case SX_GS_BendLfo1Tva:
          case SX_GS_BendLfo2Rate:
          case SX_GS_BendLfo2Pitch:
          case SX_GS_BendLfo2Tvf:
          case SX_GS_BendLfo2Tva:
            mpBenderSettings[SysExId - SX_GS_BendPitch] = s;
            break;

          case SX_GS_ModPitch:
          case SX_GS_ModTvf:
          case SX_GS_ModAmpl:
          case SX_GS_ModLfo1Rate:
          case SX_GS_ModLfo1Pitch:
          case SX_GS_ModLfo1Tvf:
          case SX_GS_ModLfo1Tva:
          case SX_GS_ModLfo2Rate:
          case SX_GS_ModLfo2Pitch:
          case SX_GS_ModLfo2Tvf:
          case SX_GS_ModLfo2Tva:
            mpModulationSettings[SysExId - SX_GS_ModPitch] = s;
            break;

          case SX_GS_CafPitch:
          case SX_GS_CafTvf:
          case SX_GS_CafAmpl:
          case SX_GS_CafLfo1Rate:
          case SX_GS_CafLfo1Pitch:
          case SX_GS_CafLfo1Tvf:
          case SX_GS_CafLfo1Tva:
          case SX_GS_CafLfo2Rate:
          case SX_GS_CafLfo2Pitch:
          case SX_GS_CafLfo2Tvf:
          case SX_GS_CafLfo2Tva:
            mpCAfSettings[SysExId - SX_GS_CafPitch] = s;
            break;

          case SX_GS_PafPitch:
          case SX_GS_PafTvf:
          case SX_GS_PafAmpl:
          case SX_GS_PafLfo1Rate:
          case SX_GS_PafLfo1Pitch:
          case SX_GS_PafLfo1Tvf:
          case SX_GS_PafLfo1Tva:
          case SX_GS_PafLfo2Rate:
          case SX_GS_PafLfo2Pitch:
          case SX_GS_PafLfo2Tvf:
          case SX_GS_PafLfo2Tva:
            mpPAfSettings[SysExId - SX_GS_PafPitch] = s;
            break;

          case SX_GS_CC1Pitch:
          case SX_GS_CC1Tvf:
          case SX_GS_CC1Ampl:
          case SX_GS_CC1Lfo1Rate:
          case SX_GS_CC1Lfo1Pitch:
          case SX_GS_CC1Lfo1Tvf:
          case SX_GS_CC1Lfo1Tva:
          case SX_GS_CC1Lfo2Rate:
          case SX_GS_CC1Lfo2Pitch:
          case SX_GS_CC1Lfo2Tvf:
          case SX_GS_CC1Lfo2Tva:
            mpCC1Settings[SysExId - SX_GS_CC1Pitch] = s;
            break;

          case SX_GS_CC2Pitch:
          case SX_GS_CC2Tvf:
          case SX_GS_CC2Ampl:
          case SX_GS_CC2Lfo1Rate:
          case SX_GS_CC2Lfo1Pitch:
          case SX_GS_CC2Lfo1Tvf:
          case SX_GS_CC2Lfo1Tva:
          case SX_GS_CC2Lfo2Rate:
          case SX_GS_CC2Lfo2Pitch:
          case SX_GS_CC2Lfo2Tvf:
          case SX_GS_CC2Lfo2Tva:
            mpCC2Settings[SysExId - SX_GS_CC2Pitch] = s;
            break;

          case SX_GS_ReverbMacro:
            mpReverbType = s;
            break;

          case SX_GS_RevCharacter:
          case SX_GS_RevPreLpf:
          case SX_GS_RevLevel:
          case SX_GS_RevTime:
          case SX_GS_RevDelayFeedback:
          case SX_GS_RevSendChorus:
            mpReverbSettings[SysExId - SX_GS_RevCharacter] = s;
            break;

          case SX_GS_ChorusMacro:
            mpChorusType = s;
            break;

          case SX_GS_ChoPreLpf:
          case SX_GS_ChoLevel:
          case SX_GS_ChoFeedback:
          case SX_GS_ChoDelay:
          case SX_GS_ChoRate:
          case SX_GS_ChoDepth:
          case SX_GS_ChoSendReverb:
            mpChorusSettings[SysExId - SX_GS_ChoPreLpf] = s;
            break;

          case SX_GS_CC1CtrlNo:
            mpCC1ControllerNr = s;
            break;

          case SX_GS_CC2CtrlNo:
            mpCC2ControllerNr = s;
            break;

          case SX_GS_PartialReserve:
            mpPartialReserve = s;
            break;

          case SX_GS_RxChannel:
            mpRxChannel = s;
            break;

          case SX_GS_UseForRhythm:
            mpUseForRhythm  = s;
            break;

          default:
            break;
        }
      }
      else if (gpSynth->IsXG())
      {
        switch (SysExId)
        {
          case SX_XG_BendPitch:
          case SX_XG_BendTvf:
          case SX_XG_BendAmpl:
            mpBenderSettings[SysExId - SX_XG_BendPitch] = s;
            break;

          case SX_XG_BendLfoPitch:
          case SX_XG_BendLfoTvf:
          case SX_XG_BendLfoTva:
            mpBenderSettings[SysExId + 1 - SX_XG_BendPitch] = s;
            break;

          case SX_XG_ModPitch:
          case SX_XG_ModTvf:
          case SX_XG_ModAmpl:
            mpModulationSettings[SysExId - SX_XG_ModPitch] = s;
            break;

          case SX_XG_ModLfoPitch:
          case SX_XG_ModLfoTvf:
          case SX_XG_ModLfoTva:
            mpModulationSettings[SysExId + 1 - SX_XG_ModPitch] = s;
            break;

          case SX_XG_CafPitch:
          case SX_XG_CafTvf:
          case SX_XG_CafAmpl:
            mpCAfSettings[SysExId - SX_XG_CafPitch] = s;
            break;

          case SX_XG_CafLfoPitch:
          case SX_XG_CafLfoTvf:
          case SX_XG_CafLfoTva:
            mpCAfSettings[SysExId + 1 - SX_XG_CafPitch] = s;
            break;

          case SX_XG_PafPitch:
          case SX_XG_PafTvf:
          case SX_XG_PafAmpl:
            mpPAfSettings[SysExId - SX_XG_PafPitch] = s;
            break;

          case SX_XG_PafLfoPitch:
          case SX_XG_PafLfoTvf:
          case SX_XG_PafLfoTva:
            mpPAfSettings[SysExId + 1 - SX_XG_PafPitch] = s;
            break;

          case SX_XG_CC1Pitch:
          case SX_XG_CC1Tvf:
          case SX_XG_CC1Ampl:
            mpCC1Settings[SysExId - SX_XG_CC1Pitch] = s;
            break;

          case SX_XG_CC1LfoPitch:
          case SX_XG_CC1LfoTvf:
          case SX_XG_CC1LfoTva:
            mpCC1Settings[SysExId + 1 - SX_XG_CC1Pitch] = s;
            break;

          case SX_XG_CC2Pitch:
          case SX_XG_CC2Tvf:
          case SX_XG_CC2Ampl:
            mpCC2Settings[SysExId - SX_XG_CC2Pitch] = s;
            break;

          case SX_XG_CC2LfoPitch:
          case SX_XG_CC2LfoTvf:
          case SX_XG_CC2LfoTva:
            mpCC2Settings[SysExId + 1 - SX_XG_CC2Pitch] = s;
            break;

          case SX_XG_ReverbMacro:
            mpReverbType = s;
            break;

          case SX_XG_ChorusMacro:
            mpChorusType = s;
            break;

          case SX_XG_EqualizerMacro:
            mpEqualizerType = s;
            break;

          case SX_XG_CC1CtrlNo:
            mpCC1ControllerNr = s;
            break;

          case SX_XG_CC2CtrlNo:
            mpCC2ControllerNr = s;
            break;

          case SX_XG_RxChannel:
            mpRxChannel = s;
            break;

          case SX_XG_UseForRhythm:
            mpUseForRhythm  = s;
            break;

          default:
            break;
        }
      }
    }
  }
}


void JZEventArray::Length2Keyoff()
{
  int n = mEventCount;
  for (int i = 0; i < n; i++)
  {
    JZKeyOnEvent* pKeyOn;
    if ((pKeyOn = mppEvents[i]->IsKeyOn()) != 0 && pKeyOn->GetEventLength() != 0)
    {
//      JZEvent* pKeyOff = new JZKeyOffEvent(
//        pKeyOn->GetClock() + pKeyOn->GetEventLength(),
//        pKeyOn->Channel,
//        pKeyOn->Key);

      // SN++ added off veloc
      JZEvent* pKeyOff = new JZKeyOffEvent(
        pKeyOn->GetClock() + pKeyOn->GetEventLength(),
        pKeyOn->GetChannel(),
        pKeyOn->GetKey(),
        pKeyOn->GetOffVelocity());

      pKeyOn->SetLength(0);
      pKeyOff->SetDevice(pKeyOn->GetDevice());
      Put(pKeyOff);
    }
  }
  Sort();
}


#if 0

void JZEventArray::Keyoff2Length()
{
  int i;
  for (i = 1; i < mEventCount; i++)
  {
    JZKeyOffEvent* pKeyOff;
    if ((pKeyOff = mppEvents[i]->IsKeyOff()) != 0)
    {
      JZEvent** ppEvent = &mppEvents[i - 1];
      while (ppEvent >= mppEvents)
      {
        JZKeyOnEvent* pKeyOn = (*ppEvent)->IsKeyOn();
        if (
          pKeyOn &&
          pKeyOn->Key == pKeyOff->Key &&
          pKeyOn->Channel == pKeyOff->Channel &&
          pKeyOn->Length == 0)
        {
          pKeyOn->Length = pKeyOff->GetClock() - pKeyOn->GetClock();
          if (pKeyOn->Length <= 0L)
          {
            pKeyOn->Length = 1;
          }
          pKeyOff->Kill();
          break;
        }
        --ppEvent;
      }
    }
  }

  // kill all KeyOn's with non matching KeyOff's
  for (i = 0; i < mEventCount; i++)
  {
    JZKeyOnEvent* k = mppEvents[i]->IsKeyOn();
    if (k && k->Length <= 0)
    {
      k->Kill();
    }
  }
  Cleanup(0);
}

#else

void JZEventArray::Keyoff2Length()
{
  // Searches forward from a KeyOn to find the matching KeyOff.
  // This is compatible with Cubase.
  int i;
  for (i = 0; i < mEventCount; i++)
  {
    JZKeyOnEvent* pKeyOn;
    if ((pKeyOn = mppEvents[i]->IsKeyOn()) != 0 && pKeyOn->GetEventLength() == 0)
    {
      int j;
      for (j = i + 1; j < mEventCount; j++)
      {
        JZKeyOffEvent* pKeyOff = mppEvents[j]->IsKeyOff();
        if (
          pKeyOff &&
          !pKeyOff->IsKilled() &&
          pKeyOn->GetKey() == pKeyOff->GetKey() &&
          pKeyOn->GetChannel() == pKeyOff->GetChannel())
        {
          pKeyOn->SetLength(pKeyOff->GetClock() - pKeyOn->GetClock());
          if (pKeyOn->GetEventLength() <= 0)
          {
            pKeyOn->SetLength(1);
          }
          pKeyOff->Kill();
          break;
        }
      }
    }
  }

  // kill all KeyOn's with non matching KeyOff's
  // and kill all remaining KeyOff's
  for (i = 0; i < mEventCount; i++)
  {
    JZKeyOnEvent* pKeyOn = mppEvents[i]->IsKeyOn();
    if (pKeyOn && pKeyOn->GetEventLength() <= 0)
    {
      pKeyOn->Kill();
    }
    JZKeyOffEvent* pKeyOff = mppEvents[i]->IsKeyOff();
    if (pKeyOff)
    {
      pKeyOff->Kill();
    }
  }
  Cleanup(0);
}

#endif


void JZEventArray::Write(JZWriteBase& Io)
{
  JZEvent* pEvent;
  int WrittenBefore;

  Length2Keyoff();
  Io.NextTrack();

  // Write copyright notice first (according to spec):
  if (mpCopyright)
  {
    mpCopyright->Write(Io);
  }

  // Write MTC offset before any transmittable events (spec)
  if (mpMtcOffset)
  {
    mpMtcOffset->Write(Io);
  }

  // Synth reset
  if (mpReset)
  {
    mpReset->Write(Io);
  }

  // Rpn / Nrpn:
  // All these must be written in order (three JZControlEvent's in a row)
  if (mpVibRate)
  {
    mpVibRate->Write(Io);
  }
  if (mpVibDepth)
  {
    mpVibDepth->Write(Io);
  }
  if (mpVibDelay)
  {
    mpVibDelay->Write(Io);
  }
  if (mpCutoff)
  {
    mpCutoff->Write(Io);
  }
  if (mpResonance)
  {
    mpResonance->Write(Io);
  }
  if (mpEnvAttack)
  {
    mpEnvAttack->Write(Io);
  }
  if (mpEnvDecay)
  {
    mpEnvDecay->Write(Io);
  }
  if (mpEnvRelease)
  {
    mpEnvRelease->Write(Io);
  }
  if (mpBendPitchSens)
  {
    mpBendPitchSens->Write(Io);
  }

  JZDrumInstrumentParameter* dpar = mDrumParams.FirstElem();
  while (dpar)
  {
    int index;
    for (index = drumPitchIndex; index < numDrumParameters; index++)
    {
      if (dpar->Get(index))
      {
        dpar->Get(index)->Write(Io);
      }
    }
    dpar = mDrumParams.NextElem(dpar);
  }

  // mpBank: Must be sure bank is written before program:
  if (mpBank)
  {
    mpBank->Write(Io);
  }

  if (mpBank2)
  {
    mpBank2->Write(Io);
  }

  if (mpPatch)
  {
    mpPatch->Write(Io);
  }

  JZJazzMetaEvent JazzMeta;
  JazzMeta.SetAudioMode(mAudioMode);
  JazzMeta.SetTrackState(mState);
  JazzMeta.SetTrackDevice(mDevice);
  JazzMeta.SetIntroLength(gpSong->GetIntroLength());
  JazzMeta.Write(Io);

  for (int i = 0; i < mEventCount; i++)
  {
    pEvent = mppEvents[i];
    WrittenBefore = 0;
    if (pEvent->IsControl())
    {
      switch (pEvent->IsControl()->GetControl())
      {
        // Don't write these again if present as events
        // and clock == 0 (should not happen)
        case 0x65: // Rpn Msb
        case 0x64: // Rpn Lsb
        case 0x63: // Nrpn Msb
        case 0x62: // Nrpn Lsb
        case 0x06: // Rpn/Nrpn Data
        case 0x00: // mpBank
        case 0x20: // Bank2
          if (pEvent->GetClock() == 0)
          {
            WrittenBefore = 1;
          }
          break;
        default:
          WrittenBefore = 0;
      }
    }
    else if (pEvent->IsProgram())
    {
      // Don't write these again if present as events
      // and clock == 0 (should not happen)
      if (pEvent->GetClock() == 0)
      {
        WrittenBefore = 1;
      }
    }
    else if (pEvent->IsCopyright() || pEvent->IsMtcOffset())
    {
      // Will probably happen
      WrittenBefore = 1;
    }
    if (!WrittenBefore)
    {
      pEvent->Write(Io);
    }
  }
  Keyoff2Length();
}

void JZEventArray::Read(JZReadBase& Io)
{
  JZEvent* pEvent;
  mChannel = 0;
  unsigned char Msb, Lsb, Data;
  bool SpecialEvent;

  Msb = Lsb = Data = 0xff;
  int cha;

  bool NeedToDelete;

  Io.NextTrack();
  while ((pEvent = Io.Read()) != 0)
  {
    NeedToDelete = false;
    SpecialEvent = false;
    if (pEvent->IsJazzMeta())
    {
      JZJazzMetaEvent* pJazzMetaEvent = pEvent->IsJazzMeta();
      mAudioMode = pJazzMetaEvent->GetAudioMode();
      mState     = (int)pJazzMetaEvent->GetTrackState();
      mDevice    = (int)pJazzMetaEvent->GetTrackDevice();
      gpSong->SetIntroLength((int)pJazzMetaEvent->GetIntroLength());
      delete pJazzMetaEvent;
      continue;
    }
    if (pEvent->IsControl())
    {
      switch (pEvent->IsControl()->GetControl())
      {
        // Grab Rpn/Nrpn/Bank from file and save them, don't put
        // them into event-array
        case 0x63:
        case 0x65:
          Msb = pEvent->IsControl()->GetControlValue(); // Rpn/Nrpn Msb
          SpecialEvent = true;
          break;
        case 0x62:
        case 0x64:
          Lsb = pEvent->IsControl()->GetControlValue(); // Rpn/Nrpn Lsb
          SpecialEvent = true;
          break;
        case 0x06:
          Data = pEvent->IsControl()->GetControlValue(); // Rpn/Nrpn Data
          SpecialEvent = true;
          cha = pEvent->IsControl()->GetChannel();
          switch (Msb)
          {
            case 0x01: // Nrpn
              switch (Lsb)
              {
                case 0x08:
                  if (!mpVibRate)
                  {
                    mpVibRate = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                case 0x09:
                  if (!mpVibDepth)
                  {
                    mpVibDepth = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                case 0x0a:
                  if (!mpVibDelay)
                  {
                    mpVibDelay = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                case 0x20:
                  if (!mpCutoff)
                  {
                    mpCutoff = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                case 0x21:
                  if (!mpResonance)
                  {
                    mpResonance = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                case 0x63:
                  if (!mpEnvAttack)
                  {
                    mpEnvAttack = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                case 0x64:
                  if (!mpEnvDecay)
                  {
                    mpEnvDecay = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                case 0x66:
                  if (!mpEnvRelease)
                  {
                    mpEnvRelease = new JZNrpn(0, cha, Msb, Lsb, Data);
                  }
                  break;
                default:
                  break;
              }
              break;
            case drumPitch:
            case drumTva:
            case drumPan:
            case drumReverb:
            case drumChorus:
              mDrumParams.PutParam(new JZNrpn(0, cha, Msb, Lsb, Data));
              break;
            case 0x00: // Rpn
              if (Lsb == 0x00)
              {
                // Pitch Bend Sensivity
                if (!mpBendPitchSens)
                {
                  mpBendPitchSens = new JZRpn(0, cha, Msb, Lsb, Data);
                }
              }
              break;
            default:
              break;
          }
          Msb = Lsb = Data = 0xff;
          break;
        case 0x00:
          if (!mpBank)
          {
            SpecialEvent = true;
            mpBank = pEvent->IsControl();
            mpBank->SetClock(0);
          }
          break;
        case 0x20:
          if (!mpBank2)
          {
            SpecialEvent = true;
            mpBank2 = pEvent->IsControl();
            mpBank2->SetClock(0);
          }
          break;
        default:
          SpecialEvent = false; // Other control
          break;
      }
    }
    else if (pEvent->IsProgram())
    {
      if (!mpPatch)
      {
        mpPatch = pEvent->IsProgram();
        mpPatch->SetClock(0);
        SpecialEvent = true;
      }
    }
    else if (pEvent->IsCopyright())
    {
      if (!mpCopyright)
      {
        mpCopyright = pEvent->IsCopyright();

        // Just make sure clock is zero, then put into event array
        mpCopyright->SetClock(0);
      }
    }
    else if (pEvent->IsSysEx())
    {
      NeedToDelete = true;

      // Get hold of the Reset sysex...
      int SysExId = gpSynth->GetSysexId(pEvent->IsSysEx());

      if (SysExId == SX_GM_ON || SysExId == SX_GS_ON || SysExId == SX_XG_ON)
      {
        // Take them all away
        SpecialEvent = true;

        // Save it in the track defaults if it fits with synth
        // type settings
        if (
          (gpSynth->IsGM() && (SysExId == SX_GM_ON)) ||
          (gpSynth->IsGS() && (SysExId == SX_GS_ON)) ||
          (gpSynth->IsXG() && (SysExId == SX_XG_ON)))
        {
          if (!mpReset)
          {
            mpReset = pEvent->IsSysEx();
            NeedToDelete = false;
          }
        }
      }
    }

    if (!SpecialEvent)
    {
      Put(pEvent);
      NeedToDelete = false;
      if (!mChannel && pEvent->IsChannelEvent())
      {
        mChannel = pEvent->IsChannelEvent()->GetChannel() + 1;
      }
    }
    if (pEvent->IsEndOfTrack())
    {
      // JAVE I want explicit end of track events
      // Break out of loop here because endoftrack is end, and we want it read
      // FIXME we shoulnt break, we should keep on reading, and handle eot in
      // track play instead.
      break;
    }

    if (NeedToDelete)
    {
      delete pEvent;
    }

  } // while read

  if (!mChannel)
  {
    mChannel = 1;
  }

  Keyoff2Length();
}


int JZEventArray::GetLastClock() const
{
  if (!mEventCount)
  {
    return 0;
  }
  return mppEvents[mEventCount - 1]->GetClock();
}

bool JZEventArray::IsEmpty() const
{
  return mEventCount == 0;
}

int JZEventArray::GetFirstClock()
{
  if (mEventCount)
  {
    return mppEvents[0]->GetClock();
  }
  return LAST_CLOCK;
}

// ***********************************************************************
// Dialog
// ***********************************************************************
#ifdef OBSOLETE

class JZTrackDlg : public wxForm
{
  JZTrackWindow* TrackWin;
  JZTrack* trk;
  std::string& mTrackName;
  JZNamedChoice PatchChoice;
  JZNamedChoice DeviceChoice;
  int PatchNr;
  int mDevice;
  int BankNr;
  int ClearTrack;
  int AudioMode;

 public:
  JZTrackDlg::JZTrackDlg(JZTrackWindow* w, JZTrack* t);
  void EditForm(wxPanel* panel);
  virtual void OnOk();
  virtual void OnCancel();
  virtual void OnHelp();
};


JZTrackDlg::JZTrackDlg(JZTrackWindow* w, JZTrack* t)
  : wxForm(USED_WXFORM_BUTTONS),
    PatchChoice(
      "Patch",
      t->IsDrumTrack() ? &gpConfig->GetDrumSet(0) : &gpConfig->GetVoiceName(0),
      &PatchNr),
    DeviceChoice("Device", gpMidiPlayer->GetOutputDevices().AsNamedValue(), &mDevice)
{
  TrackWin = w;
  trk = t;
}

void JZTrackDlg::OnCancel()
{
  trk->mpDialog = 0;
  TrackWin->Redraw();
  wxForm::OnCancel();
}

void JZTrackDlg::OnHelp()
{
  JZHelp::Instance().ShowTopic("Trackname, midi channel etc");
}

void JZTrackDlg::OnOk()
{
  trk->mpDialog->GetPosition(&Config(C_TrackDlgXpos), &Config(C_TrackDlgYpos));
  trk->mpDialog = 0;
  trk->SetAudioMode(AudioMode);

  if (ClearTrack)
  {
    trk->Clear();
    mTrackName.clear();
    TrackWin->Redraw();
    wxForm::OnOk();
    return;
  }
  trk->SetName(mTrackName);
  mTrackName.clear();
  PatchChoice.GetValue();
  DeviceChoice.GetValue();
  BankNr = (PatchNr & 0x0000ff00) >> 8;
  PatchNr = PatchNr & 0x000000ff;
  trk->SetBank(BankNr);
  trk->SetPatch(PatchNr);
  trk->SetDevice(mDevice);
  if (trk->mForceChannel)
  {
    JZChannelEvent* c;
    JZSysExEvent* s;
    JZEventIterator Iterator(trk);
    trk->Sort();
    JZEvent* pEvent = Iterator.Range(0, (unsigned) trk->GetLastClock() + 1);
    while (pEvent)
    {
      if ((c = pEvent->IsChannelEvent()) != 0)
      {
        c = (JZChannelEvent*)pEvent->Copy();
        c->SetChannel(trk->mChannel - 1);
        trk->Kill(pEvent);
        trk->Put(c);
      }
      else if ((s = pEvent->IsSysEx()) != 0)
      {
        // Check for sysex that contains channel number
        const unsigned char* pChannel = gpSynth->GetSysexChaPtr(s);
        if (pChannel)
        {
          if (gpSynth->IsXG())
          {
            *pChannel = trk->mChannel - 1;
          }
          else
          {
            *pChannel &= 0xf0;
            *pChannel |= sysex_channel(trk->mChannel);
          }

          s = (JZSysExEvent*) pEvent->Copy();
          trk->Kill(pEvent);
          trk->Put(s);
        }
      }
      pEvent = Iterator.Next();
    } // while pEvent

    if (trk->mpVibRate)
    {
      trk->mpVibRate->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpVibDepth)
    {
      trk->mpVibDepth->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpVibDelay)
    {
      trk->mpVibDelay->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpCutoff)
    {
      trk->mpCutoff->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpResonance)
    {
      trk->mpResonance->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpEnvAttack)
    {
      trk->mpEnvAttack->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpEnvDecay)
    {
      trk->mpEnvDecay->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpEnvRelease)
    {
      trk->mpEnvRelease->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpBendPitchSens)
    {
      trk->mpBendPitchSens->SetChannel(trk->mChannel - 1);
    }
    if (trk->mpBank)
    {
      trk->mpBank->mChannel = trk->mChannel - 1;
    }
    if (trk->mpPatch)
    {
      trk->mpPatch->mChannel = trk->mChannel - 1;
    }
    if (!trk->mDrumParams.IsEmpty())
    {
      JZDrumInstrumentParameter* dpar = trk->mDrumParams.FirstElem();
      while (dpar)
      {
        for (int index = drumPitchIndex; index < numDrumParameters; ++index)
        {
          if (dpar->Get(index))
          {
            dpar->Get(index)->SetChannel(trk->mChannel - 1);
          }
        }
        dpar = trk->mDrumParams.NextElem(dpar);
      }
    }
    trk->Cleanup();
  }
  TrackWin->Canvas->Refresh();
  wxForm::OnOk();
}

void JZTrackDlg::EditForm(wxPanel* panel)
{
  PatchNr   = trk->GetPatch() + (trk->GetBank() << 8);
  mDevice   = trk->GetDevice();
  TrackName = copystring(trk->GetName());
  Add(wxMakeFormString(
    "Trackname:",
    &TrackName,
    wxFORM_DEFAULT,
    NULL,
    NULL,
    wxVERTICAL,
    300));

  Add(wxMakeFormNewLine());
  Add(PatchChoice.mkFormItem(300, 200));
  Add(wxMakeFormNewLine());
  {
    char buf[500];
    sprintf(
      buf,
      "Set Channel to %d to make a drum track",
      Config(C_DrumChannel));
    Add(wxMakeFormMessage(buf));
    Add(wxMakeFormNewLine());
  }
  Add(wxMakeFormShort(
    "Channel",
    &trk->mChannel,
    wxFORM_DEFAULT,
    new wxList(wxMakeConstraintRange(1.0, 16.0), 0)));
  AudioMode = trk->GetAudioMode();
  Add(wxMakeFormNewLine());
  Add(wxMakeFormBool("Audio Track", &AudioMode));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormBool("Force channel number onto all events on track", &trk->mForceChannel));
  ClearTrack = 0;
  Add(wxMakeFormNewLine());
  Add(wxMakeFormBool("Clear track (NB! erase all events, name etc...)", &ClearTrack));

  if (gpMidiPlayer->SupportsMultipleDevices())
  {
    Add(wxMakeFormNewLine());
    Add(DeviceChoice.mkFormItem(300, 50));
  }
  AssociatePanel(panel);
}

#endif // OBSOLETE

void JZTrack::Edit(JZTrackWindow* pParent)
{
  JZTrackDialog TrackDialog(*this, pParent);
  TrackDialog.ShowModal();
#ifdef OBSOLETE
  mpDialog = new wxDialogBox(
    pParent,
    "Track Settings",
    modal,
    Config(C_TrackDlgXpos),
    Config(C_TrackDlgYpos));
#endif // OBSOLETE
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZTrack::IsEditing() const
{
  if (mpDialog)
  {
    return (mpDialog->GetHandle() != 0);
  }
  return false;
}


//*****************************************************************************
// Description:
//   This is the track class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZTrack::mChanged = false;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrack::JZTrack()
  : JZEventArray(),
    mUndoIndex(0),
    mRedoCount(0),
    mUndoCount(0),
    mpDialog(0)
{
  mForceChannel = 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrack::~JZTrack()
{
  Clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZTrack::IsDrumTrack()
{
  return mChannel == gpConfig->GetValue(C_DrumChannel);
}

void JZTrack::Merge(JZEventArray* t)
{
  for (int i = 0; i < t->mEventCount; i++)
  {
    Put(t->mppEvents[i]);
  }
  t->mEventCount = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrack::MergeRange(
  const JZEventArray& Other,
  int FromClock,
  int ToClock,
  int Replace)
{
  // Erase destin
  if (Replace)
  {
    JZEventIterator Erase(this);
    JZEvent* pEvent = Erase.Range(FromClock, ToClock);
    while (pEvent)
    {
      Kill(pEvent);
      pEvent = Erase.Next();
    }
  }

  // Merge Recorded mppEvents
  JZEventIterator Copy(&Other);
  JZEvent* pEvent = Copy.Range(FromClock, ToClock);
  while (pEvent)
  {
    JZEvent* c = pEvent->Copy();
    if (mForceChannel)
    {
      JZChannelEvent* pChannelEvent = c->IsChannelEvent();
      if (pChannelEvent)
      {
        pChannelEvent->SetChannel(mChannel - 1);
      }
    }
    Put(c);
    pEvent = Copy.Next();
  }
  Cleanup();
}


void JZTrack::Cleanup()
{
  // on audio tracks, adjust length of keyon events to
  // actual sample length
  gpMidiPlayer->AdjustAudioLength(this);
  JZEventArray::Cleanup(TRUE);
}




void JZTrack::Undo()
{
  if (mUndoCount > 0)
  {
    JZUndoBuffer* undo = &mUndoBuffers[mUndoIndex];
    for (int i = undo->mEventCount - 1; i >= 0; i--)
    {
      JZEvent* pEvent = undo->mppEvents[i];
      if (undo->mBits(i))
      {
        undo->mBits.set(i, 0);
        pEvent->UnKill();
        JZEventArray::Put(pEvent);
      }
      else
      {
        undo->mBits.set(i, 1);
        pEvent->Kill();
      }
    }
    JZEventArray::Cleanup(TRUE);

    mUndoIndex = (mUndoIndex - 1 + MaxUndo) % MaxUndo;
    --mUndoCount;
    ++mRedoCount;
  }
}

void JZTrack::Redo()
{
  if (mRedoCount > 0)
  {
    mUndoIndex = (mUndoIndex + 1) % MaxUndo;

    JZUndoBuffer* undo = &mUndoBuffers[mUndoIndex];
    for (int i = 0; i < undo->mEventCount; i++)
    {
      JZEvent* pEvent = undo->mppEvents[i];
      if (undo->mBits(i))
      {
        undo->mBits.set(i, 0);
        pEvent->UnKill();
        JZEventArray::Put(pEvent);
      }
      else
      {
        undo->mBits.set(i, 1);
        pEvent->Kill();
      }
    }
    JZEventArray::Cleanup(TRUE);

    --mRedoCount;
    ++mUndoCount;
  }
}


void JZTrack::NewUndoBuffer()
{
  mRedoCount = 0;
  ++mUndoCount;
  if (mUndoCount > MaxUndo)
  {
    mUndoCount = MaxUndo;
  }

  mUndoIndex = (mUndoIndex + 1) % MaxUndo;
  mUndoBuffers[mUndoIndex].Clear();
};


void JZTrack::Clear()
{
  for (int i = 0; i < MaxUndo; i++)
  {
    mUndoBuffers[i].Clear();
  }
  mState = tsPlay;
  JZEventArray::Clear();
}

// ----------------------- Copyright ------------------------------------

const char* JZTrack::GetCopyright()
{
  if (mpCopyright)
  {
    return (const char*)mpCopyright->GetData();
  }
  return "";
}



void JZTrack::SetCopyright(char* str)
{
  if (mpCopyright)
  {
    Kill(mpCopyright);
  }
  if (str && strlen(str))
  {
    int len = 127;
    if ((int)strlen(str) < len)
    {
      len = strlen(str);
    }
    Put(new JZCopyrightEvent(0, (unsigned char*)str, len));
  }
  Cleanup();
}

// ----------------------- Name ------------------------------------

const char* JZTrack::GetName()
{
  if (mpName)
  {
    return (const char*)mpName->GetData();
  }
  return "";
}

void JZTrack::SetName(const char* pTrackName)
{
  if (mpName)
  {
    Kill(mpName);
  }
  if (strlen(pTrackName))
  {
    Put(new JZTrackNameEvent(
      0,
      (unsigned char*)pTrackName,
      strlen(pTrackName)));
  }
  Cleanup();
}

// ------------------------ Volume ------------------------------

int JZTrack::GetVolume()
{
  if (mpVolume)
  {
    return mpVolume->GetControlValue() + 1;
  }
  return 0;
}

void JZTrack::SetVolume(int Value)
{
  if (mpVolume)
  {
    Kill(mpVolume);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = new JZControlEvent(0, mChannel - 1, 0x07, Value - 1);
    Put(pEvent);
    gpMidiPlayer->OutNow(this, pEvent);
  }
  Cleanup();
}

bool JZTrack::DecreaseVolume()
{
  if (mpVolume && mpVolume->GetControlValue() > 0)
  {
    Kill(mpVolume);

    mpVolume->SetControlValue(mpVolume->GetControlValue() - 1);

    JZEvent* pEvent = new JZControlEvent(
      0,
      mChannel - 1,
      0x07,
      mpVolume->GetControlValue());
    Put(pEvent);
    gpMidiPlayer->OutNow(this, pEvent);

    Cleanup();

    return true;
  }
  return false;
}

bool JZTrack::IncreaseVolume()
{
  if (mpVolume && mpVolume->GetControlValue() < 127)
  {
    Kill(mpVolume);

    mpVolume->SetControlValue(mpVolume->GetControlValue() + 1);

    JZEvent* pEvent = new JZControlEvent(
      0,
      mChannel - 1,
      0x07,
      mpVolume->GetControlValue());

    Put(pEvent);
    gpMidiPlayer->OutNow(this, pEvent);

    Cleanup();

    return true;
  }
  return false;
}

// ------------------------ Pan ------------------------------

int JZTrack::GetPan()
{
  if (mpPan)
  {
    return mpPan->GetControlValue() + 1;
  }
  return 0;
}

void JZTrack::SetPan(int Value)
{
  if (mpPan)
  {
    Kill(mpPan);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = new JZControlEvent(0, mChannel - 1, 0x0a, Value - 1);
    Put(pEvent);
    gpMidiPlayer->OutNow(this, pEvent);
  }
  Cleanup();
}

// ------------------------ Reverb ------------------------------

int JZTrack::GetReverb()
{
  if (mpReverb)
  {
    return mpReverb->GetControlValue() + 1;
  }
  return 0;
}

void JZTrack::SetReverb(int Value)
{
  if (mpReverb)
  {
    Kill(mpReverb);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = new JZControlEvent(0, mChannel - 1, 0x5B, Value - 1);
    Put(pEvent);
    gpMidiPlayer->OutNow(this, pEvent);
  }
  Cleanup();
}

// ------------------------ Chorus ------------------------------

int JZTrack::GetChorus()
{
  if (mpChorus)
  {
    return mpChorus->GetControlValue() + 1;
  }
  return 0;
}

void JZTrack::SetChorus(int Value)
{
  if (mpChorus)
  {
    Kill(mpChorus);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = new JZControlEvent(0, mChannel - 1, 0x5D, Value - 1);
    Put(pEvent);
    gpMidiPlayer->OutNow(this, pEvent);
  }
  Cleanup();
}

// ------------------------  Bank ------------------------------

int JZTrack::GetBank()
{
  if (!gpConfig->GetValue(C_UseTwoCommandBankSelect))
  {
    DEBUG(fprintf(stderr, "Get single bank select command\n");)
    if (mpBank)
    {
      DEBUG(fprintf(stderr,"Bank %d selected.\n\n",mpBank->Value);)
      return mpBank->GetControlValue();
    }
    else
    {
      return 0;
    }
  }
  DEBUG(fprintf(stderr, "Get double bank select command.\n");)
  if (mpBank && mpBank2)
  {
    for (int i=0; gpConfig->BankEntry(i).Command[0]>=0; i++)
    {
      if (
        gpConfig->BankEntry(i).Command[0] == mpBank->GetControlValue() &&
        gpConfig->BankEntry(i).Command[1] == mpBank2->GetControlValue())
      {
        DEBUG(fprintf(stderr,"Bank %d selected.\n\n",i);)
        return i;
      }
    }
  }
  return 0;
}

void JZTrack::SetBank(int Value)
{
  if (mpBank)
  {
    delete mpBank;
    mpBank = 0;
  }

  if (mpBank2)
  {
    delete mpBank2;
    mpBank2 = 0;
  }

  if (Value >= 0)
  {
    if (!gpConfig->GetValue(C_UseTwoCommandBankSelect))
    {
      DEBUG(fprintf (stderr, "Single command bank select (Bank %d).\n",
            Value);)
      mpBank = new JZControlEvent(
        0,
        mChannel - 1,
        gpConfig->GetValue(C_BankControlNumber),
        Value);
      gpMidiPlayer->OutNow(this, mpBank);
      return;
    }
    while (gpConfig->BankEntry(Value).Command[0]<0 && Value>0)
    {
      Value--;
    }
    assert(gpConfig->BankEntry(Value).Command[0] >= 0);
    DEBUG(fprintf(stderr, "Double command bank select (Bank %d).\n",Value);)
    mpBank  = new JZControlEvent(
      0,
      mChannel - 1,
      gpConfig->GetValue(C_BankControlNumber),
      gpConfig->BankEntry(Value).Command[0]);
    gpMidiPlayer->OutNow(this, mpBank);
    DEBUG(
      fprintf(
        stderr,
        "First bank select command: %d %d\n",
        mpBank->Control,
        mpBank->Value);)
    mpBank2 = new JZControlEvent(
      0,
      mChannel - 1,
      gpConfig->GetValue(C_BankControlNumber2),
      gpConfig->BankEntry(Value).Command[1]);

    gpMidiPlayer->OutNow(this, mpBank2);

    DEBUG(fprintf(
      stderr,
      "Second bank select command: %d %d\n\n",
      mpBank2->Control,
      mpBank2->Value);
    )
    mChanged = true;
  }
}

// ------------------------  Patch ------------------------------

int JZTrack::GetPatch()
{
  if (mpPatch)
  {
    return mpPatch->GetProgram() + 1;
  }
  return 0;
}

void JZTrack::SetPatch(int PatchNr)
{
  if (mpPatch)
  {
    delete mpPatch;
    mpPatch = 0;
  }
  if (PatchNr > 0)
  {
    mpPatch = new JZProgramEvent(0, mChannel - 1, PatchNr - 1);
    gpMidiPlayer->OutNow(this, mpPatch);
    mChanged = true;
  }
}

// ------------------------  VibRate ------------------------------

int JZTrack::GetVibRate()
{
  if (mpVibRate)
  {
    return mpVibRate->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetVibRate(int Value)
{
  if (mpVibRate)
  {
    delete mpVibRate;
    mpVibRate = 0;
  }

  if (Value > 0)
  {
    mpVibRate = new JZNrpn(0, mChannel - 1, 0x01, 0x08, Value - 1);
    gpMidiPlayer->OutNow(this, mpVibRate);
    mChanged = true;
  }
}

// ------------------------  VibDepth ------------------------------

int JZTrack::GetVibDepth()
{
  if (mpVibDepth)
  {
    return mpVibDepth->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetVibDepth(int Value)
{
  if (mpVibDepth)
  {
    delete mpVibDepth;
    mpVibDepth = 0;
  }
  if (Value > 0)
  {
    mpVibDepth = new JZNrpn(0, mChannel - 1, 0x01, 0x09, Value - 1);
    gpMidiPlayer->OutNow(this, mpVibDepth);
    mChanged = true;
  }
}

// ------------------------  VibDelay ------------------------------

int JZTrack::GetVibDelay()
{
  if (mpVibDelay)
  {
    return mpVibDelay->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetVibDelay(int Value)
{
  if (mpVibDelay)
  {
    delete mpVibDelay;
    mpVibDelay = 0;
  }

  if (Value > 0)
  {
    mpVibDelay = new JZNrpn(0, mChannel - 1, 0x01, 0x0a, Value - 1);
    gpMidiPlayer->OutNow(this, mpVibDelay);
    mChanged = true;
  }
}

// ------------------------  Cutoff ------------------------------

int JZTrack::GetCutoff()
{
  if (mpCutoff)
  {
    return mpCutoff->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetCutoff(int Value)
{
  if (mpCutoff)
  {
    delete mpCutoff;
    mpCutoff = 0;
  }

  if (Value > 0)
  {
    mpCutoff = new JZNrpn(0, mChannel - 1, 0x01, 0x20, Value - 1);
    gpMidiPlayer->OutNow(this, mpCutoff);
    mChanged = true;
  }
}

// ------------------------  Resonance ------------------------------

int JZTrack::GetResonance()
{
  if (mpResonance)
  {
    return mpResonance->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetResonance(int Value)
{
  if (mpResonance)
  {
    delete mpResonance;
    mpResonance = 0;
  }

  if (Value > 0)
  {
    mpResonance = new JZNrpn(0, mChannel - 1, 0x01, 0x21, Value - 1);
    gpMidiPlayer->OutNow(this, mpResonance);
    mChanged = true;
  }
}

// ------------------------  EnvAttack ------------------------------

int JZTrack::GetEnvAttack()
{
  if (mpEnvAttack)
  {
    return mpEnvAttack->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetEnvAttack(int Value)
{
  if (mpEnvAttack)
  {
    delete mpEnvAttack;
    mpEnvAttack = 0;
  }

  if (Value > 0)
  {
    mpEnvAttack = new JZNrpn(0, mChannel - 1, 0x01, 0x63, Value - 1);
    gpMidiPlayer->OutNow(this, mpEnvAttack);
    mChanged = true;
  }
}

// ------------------------  EnvDecay ------------------------------

int JZTrack::GetEnvDecay()
{
  if (mpEnvDecay)
  {
    return mpEnvDecay->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetEnvDecay(int Value)
{
  if (mpEnvDecay)
  {
    delete mpEnvDecay;
    mpEnvDecay = 0;
  }

  if (Value > 0)
  {
    mpEnvDecay = new JZNrpn(0, mChannel - 1, 0x01, 0x64, Value - 1);
    gpMidiPlayer->OutNow(this, mpEnvDecay);
    mChanged = true;
  }
}

// ------------------------  EnvRelease ------------------------------

int JZTrack::GetEnvRelease()
{
  if (mpEnvRelease)
  {
    return mpEnvRelease->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetEnvRelease(int Value)
{
  if (mpEnvRelease)
  {
    delete mpEnvRelease;
    mpEnvRelease = 0;
  }

  if (Value > 0)
  {
    mpEnvRelease = new JZNrpn(0, mChannel - 1, 0x01, 0x66, Value - 1);
    gpMidiPlayer->OutNow(this, mpEnvRelease);
    mChanged = true;
  }
}

// ------------------------  DrumParam ------------------------------

int JZTrack::GetDrumParam(int pitch, int index)
{
  if (!mDrumParams.IsEmpty())
  {
    JZNrpn* par = mDrumParams.GetParam(pitch, index);
    if (par)
    {
      return(par->GetVal() + 1);
    }
  }
  return 0;
}

void JZTrack::SetDrumParam(int pitch, int index, int Value)
{
  mDrumParams.DelParam(pitch, index);
  if (Value > 0)
  {
    mDrumParams.PutParam(
      new JZNrpn(0, mChannel - 1, drumIndex2Param(index), pitch, Value - 1));
    gpMidiPlayer->OutNow(this, mDrumParams.GetParam(pitch, index));
    mChanged = true;
  }
}

// ------------------------  BendPitchSens ------------------------------

int JZTrack::GetBendPitchSens()
{
  if (mpBendPitchSens)
  {
    return mpBendPitchSens->GetVal() + 1;
  }
  return 0;
}

void JZTrack::SetBendPitchSens(int Value)
{
  if (mpBendPitchSens)
  {
    delete mpBendPitchSens;
    mpBendPitchSens = 0;
  }

  if (Value > 0)
  {
    mpBendPitchSens = new JZRpn(0, mChannel - 1, 0x00, 0x00, Value - 1);
    gpMidiPlayer->OutNow(this, mpBendPitchSens);
    mChanged = true;
  }
}

// ------------------------  Modulation Sysex ------------------------------

int JZTrack::GetModulationSysex(int msp)
{
  const unsigned char* pValue =
    gpSynth->GetSysexValPtr(mpModulationSettings[msp]);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetModulationSysex(int msp, int Value)
{
  if (mpModulationSettings[msp])
  {
    Kill(mpModulationSettings[msp]);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->ModSX(msp, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  Bender Sysex ------------------------------

int JZTrack::GetBenderSysex(int bsp)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpBenderSettings[bsp]);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetBenderSysex(int bsp, int Value)
{
  if (mpBenderSettings[bsp])
  {
    Kill(mpBenderSettings[bsp]);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->BendSX(bsp, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  CAf Sysex ------------------------------

int JZTrack::GetCAfSysex(int csp)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpCAfSettings[csp]);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetCAfSysex(int csp, int Value)
{
  if (mpCAfSettings[csp])
  {
    Kill(mpCAfSettings[csp]);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->CafSX(csp, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  PAf Sysex ------------------------------

int JZTrack::GetPAfSysex(int psp)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpPAfSettings[psp]);

  if (pValue)
  {
    return* pValue + 1;
  }

  return 0;
}

void JZTrack::SetPAfSysex(int psp, int Value)
{
  if (mpPAfSettings[psp])
  {
    Kill(mpPAfSettings[psp]);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->PafSX(psp, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  CC1 Sysex ------------------------------

int JZTrack::GetCC1Sysex(int csp)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpCC1Settings[csp]);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetCC1Sysex(int csp, int Value)
{
  if (mpCC1Settings[csp])
  {
    Kill(mpCC1Settings[csp]);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->CC1SX(csp, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  CC2 Sysex ------------------------------

int JZTrack::GetCC2Sysex(int csp)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpCC2Settings[csp]);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetCC2Sysex(int csp, int Value)
{
  if (mpCC2Settings[csp])
  {
    Kill(mpCC2Settings[csp]);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->CC2SX(csp, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  CC1ControllerNr Sysex ------------------------------

int JZTrack::GetCC1ControllerNr()
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpCC1ControllerNr);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetCC1ControllerNr(int Value)
{
  if (mpCC1ControllerNr)
  {
    Kill(mpCC1ControllerNr);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->ControllerNumberSX(1, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  CC2ControllerNr Sysex ------------------------------

int JZTrack::GetCC2ControllerNr()
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpCC2ControllerNr);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetCC2ControllerNr(int Value)
{
  if (mpCC2ControllerNr)
  {
    Kill(mpCC2ControllerNr);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->ControllerNumberSX(2, 0, mChannel, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  Reverb Type ------------------------------

int JZTrack::GetReverbType(int lsb)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpReverbType);

  if (pValue)
  {
    if (lsb)
    {
      ++pValue;
    }
    return *pValue + 1;
  }

 return 0;
}

void JZTrack::SetReverbType(int Value, int lsb)
{
  if (mpReverbType)
  {
    Kill(mpReverbType);
  }

  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->ReverbMacroSX(0, Value - 1, lsb - 1);
    if (pEvent)
    {
      Put(pEvent);
      if (gpConfig->GetValue(C_UseReverbMacro))
      {
        gpMidiPlayer->OutNow(this, pEvent);
      }
    }
  }
  Cleanup();
}

// ------------------------  Chorus Type ------------------------------

int JZTrack::GetChorusType(int lsb)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpChorusType);

  if (pValue)
  {
    if (lsb)
    {
      ++pValue;
    }

    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetChorusType(int Value, int lsb)
{
  if (mpChorusType)
  {
    Kill(mpChorusType);
  }

  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->ChorusMacroSX(0, Value - 1, lsb - 1);
    if (pEvent)
    {
      Put(pEvent);
      if (gpConfig->GetValue(C_UseChorusMacro))
      {
        gpMidiPlayer->OutNow(this, pEvent);
      }
    }
  }
  Cleanup();
}

// -----------------------  Equalizer Type ------------------------------

int JZTrack::GetEqualizerType()
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpEqualizerType);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetEqualizerType(int Value)
{
  if (mpEqualizerType)
  {
    Kill(mpEqualizerType);
  }

  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->EqualizerMacroSX(0, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  Reverb Parameters Sysex ------------------------------

int JZTrack::GetRevSysex(int rsp)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpReverbSettings[rsp]);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetRevSysex(int rsp, int Value)
{
  if (mpReverbSettings[rsp])
  {
    Kill(mpReverbSettings[rsp]);
  }

  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->ReverbParamSX(rsp, 0, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      if (!gpConfig->GetValue(C_UseReverbMacro))
      {
        gpMidiPlayer->OutNow(this, pEvent);
      }
    }
  }
  Cleanup();
}

// ------------------------  Chorus Parameters Sysex ------------------------------

int JZTrack::GetChoSysex(int csp)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpChorusSettings[csp]);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetChoSysex(int csp, int Value)
{
  if (mpChorusSettings[csp])
  {
    Kill(mpChorusSettings[csp]);
  }

  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->ChorusParamSX(csp, 0, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      if (!gpConfig->GetValue(C_UseChorusMacro))
      {
        gpMidiPlayer->OutNow(this, pEvent);
      }
    }
  }
  Cleanup();
}


// ------------------------  Partial Reserve ------------------------------

int JZTrack::GetPartRsrv(int chan)
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpPartialReserve);

  if (pValue)
  {
    return *(pValue + sysex_channel(chan)) + 1;
  }

  return 0;
}

void JZTrack::SetPartRsrv(unsigned char* rsrv)
{
  if (mpPartialReserve)
  {
    Kill(mpPartialReserve);
  }

  if (rsrv)
  {
    JZEvent* pEvent = gpSynth->PartialReserveSX(0, mChannel, rsrv);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  Master Volume ------------------------------

int JZTrack::GetMasterVol()
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpMasterVol);

  if (pValue)
  {
    if (gpSynth->GetSysexId(mpMasterVol) == SX_GM_MasterVol)
    {
      // first data byte is lsb; get msb instead!
      ++pValue;
    }

    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetMasterVol(int Value)
{
  if (mpMasterVol)
  {
    Kill(mpMasterVol);
  }
  if (Value > 0)
  {
    JZEvent* pEvent = gpSynth->MasterVolSX(0, Value - 1);
    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}


// ------------------------  Master Pan ------------------------------

int JZTrack::GetMasterPan()
{
  const unsigned char* pValue = gpSynth->GetSysexValPtr(mpMasterPan);

  if (pValue)
  {
    return *pValue + 1;
  }

  return 0;
}

void JZTrack::SetMasterPan(int Value)
{
  if (mpMasterPan)
  {
    Kill(mpMasterPan);
  }
  if (Value > 0)
  {
     JZEvent* pEvent = gpSynth->MasterPanSX(0, Value - 1);
    if (pEvent)
    {
       Put(pEvent);
       gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  Mode Sysex ------------------------------

int JZTrack::GetModeSysex(int param)
{
   const unsigned char* pValue = 0;

   switch (param)
   {
     case mspRxChannel:
       pValue = gpSynth->GetSysexValPtr(mpRxChannel);
       break;

     case mspUseForRhythm:
       pValue = gpSynth->GetSysexValPtr(mpUseForRhythm );
       break;
   }

   if (pValue)
   {
     return *pValue + 1;
   }

   return 0;
}

void JZTrack::SetModeSysex(int param, int Value)
{
  switch (param)
  {
    case mspRxChannel:
      if (mpRxChannel)
      {
        Kill(mpRxChannel);
      }
      break;

    case mspUseForRhythm:
      if (mpUseForRhythm )
      {
        Kill(mpUseForRhythm );
      }
      break;
  }

  JZEvent* pEvent = 0;

  if (Value > 0)
  {
    switch (param)
    {
      case mspRxChannel:
        pEvent = gpSynth->RxChannelSX(0, mChannel, Value - 1);
        break;
      case mspUseForRhythm:
        pEvent = gpSynth->UseForRhythmSX(0, mChannel, Value - 1);
        break;
    }

    if (pEvent)
    {
      Put(pEvent);
      gpMidiPlayer->OutNow(this, pEvent);
    }
  }
  Cleanup();
}

// ------------------------  Mtc offset (Time Code Offset) ------------------------------

JZMtcTime* JZTrack::GetMtcOffset()
{
  if (mpMtcOffset)
  {
    return new JZMtcTime(mpMtcOffset);
  }
  return(new JZMtcTime(0, Mtc30Ndf));
}

void JZTrack::SetMtcOffset(JZMtcTime* mtc)
{
  if (mpMtcOffset)
  {
    Kill(mpMtcOffset);
  }
  if (mtc)
  {
    JZEvent* pEvent = mtc->ToOffset();
    Put(pEvent);
  }
  Cleanup();
}

// ------------------------ Speed ------------------------------

int JZTrack::GetDefaultSpeed()
{
  if (mpSpeed)
  {
    return mpSpeed->GetBPM();
  }
  return 120;
}


void JZTrack::SetDefaultSpeed(int bpm)
{
  JZEvent* pEvent = new JZSetTempoEvent(0, bpm);
  if (mpSpeed)
  {
    Kill(mpSpeed);
  }
  Put(pEvent);
  gpMidiPlayer->OutNow(this, pEvent);
  Cleanup();
}

JZSetTempoEvent* JZTrack::GetCurrentTempo(int clk)
{
  JZEventIterator Iterator(this);
  Sort();
  JZEvent* pEvent = Iterator.Range(0, clk + 1);
  JZSetTempoEvent* t = mpSpeed;
  while (pEvent)
  {
    if (pEvent->IsSetTempo())
    {
      t = pEvent->IsSetTempo();
    }
    pEvent = Iterator.Next();
  } // while pEvent

  return t;
}

int JZTrack::GetCurrentSpeed(int clk)
{
  JZSetTempoEvent* t = GetCurrentTempo(clk);
  if (t)
  {
    return t->GetBPM();
  }
  return 120;
}



// ------------------------- State ----------------------------------


const char* JZTrack::GetStateChar()
{
  switch (mState)
  {
    case tsPlay:
      return "P";
    case tsMute:
      return "M";
    case tsSolo:
      return "S";
  }
  return "?";
}

void JZTrack::SetState(int NewState)
{
  mState = NewState % 3;
}

void JZTrack::ToggleState(int Direction)
{
  mState = (mState + Direction + 3) % 3;
}

// ------------------------- Channel ---------------------------

void JZTrack::SetChannel(int Channel)
{
  mChannel = Channel;
}
