//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2015 Peter J. Stieber
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

#include "Command.h"

#include "Filter.h"
#include "Globals.h"
#include "Song.h"
#include "Track.h"
#include "Random.h"

#include <cstdlib>
#include <limits>

using namespace std;

//*****************************************************************************
// JZCommand
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommand::JZCommand(JZFilter* pFilter)
  : mpFilter(pFilter),
    mpSong(pFilter->GetSong()),
    mReverse(false)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommand::~JZCommand()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommand::Execute(int NewUndo)
{
  wxBeginBusyCursor();
  if (NewUndo)
  {
    mpSong->NewUndoBuffer();
  }
  JZTrackIterator Tracks(mpFilter, mReverse);
  JZTrack* pTrack = Tracks.First();
  while (pTrack)
  {
    ExecuteTrack(pTrack);
    pTrack = Tracks.Next();
  }
  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommand::ExecuteTrack(JZTrack* pTrack)
{
  JZEventIterator Iterator(pTrack);
  JZEvent* pEvent = Iterator.Range(
    mpFilter->GetFromClock(),
    mpFilter->GetToClock());
  while (pEvent)
  {
    if (mpFilter->IsSelected(pEvent))
    {
      ExecuteEvent(pTrack, pEvent);
    }
    pEvent = Iterator.Next();
  }
  pTrack->Cleanup();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommand::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZCommand::Interpolate(int Clock, int vmin, int vmax)
{
  int ClockMin = mpFilter->GetFromClock();
  int ClockMax = mpFilter->GetToClock();
  return (Clock - ClockMin) * (vmax - vmin) / (ClockMax - ClockMin) + vmin;
}

//*****************************************************************************
// tSelectedEvents
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class JZSelectedKeys : public JZCommand
{
  public:

    JZSelectedKeys(JZFilter* pFilter);

    void ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent);

  public:

    long Keys[128];
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSelectedKeys::JZSelectedKeys(JZFilter* pFilter)
  : JZCommand(pFilter)
{
  int i;
  for (i = 0; i <  128; ++i)
  {
    Keys[i] = 0;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSelectedKeys::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
  if (pKeyOn)
  {
    Keys[pKeyOn->GetKey()] += pKeyOn->GetEventLength();
  }
}

//*****************************************************************************
// JZScale
//*****************************************************************************
//-----------------------------------------------------------------------------
// Description:
//   This is a C-major scale.
//                              c     d     e  f     g    a      b
//-----------------------------------------------------------------------------
static const int CMajor[12] = { 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1 };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZScale::Init(int ScaleIndex, JZFilter* pFilter)
{
  for (int i = 0; i < 12; ++i)
  {
    ScaleKeys[i] = 0;
  }

  if (ScaleIndex == gScaleChromatic)
  {
    for (int i = 0; i < 12; ++i)
    {
      ScaleKeys[i] = 1;
    }
  }
  else if (ScaleIndex == gScaleSelected)
  {
    bool Found = false;
    JZSelectedKeys cmd(pFilter);
    cmd.Execute(0);
    for (int i = 0; i < 128; ++i)
    {
      if (cmd.Keys[i])
      {
        ScaleKeys[ i % 12 ] = 1;
        Found = true;
      }
    }
    if (!Found)
    {
      ScaleKeys[0] = 1; // avoid loop in Member()
    }
  }
  else
  {
    for (int i = 0; i < 12; ++i)
    {
      ScaleKeys[ (i + ScaleIndex) % 12 ] = CMajor[i];
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZScale::Analyze(JZFilter* pFilter)
{
  long Keys[12];
  for (int i = 0; i < 12; ++i)
  {
    Keys[i] = 0;
  }

  JZSelectedKeys cmd(pFilter);
  cmd.Execute(0);
  for (int i = 0; i < 128; ++i)
  {
    Keys[i % 12] += cmd.Keys[i];
  }

  long Min = std::numeric_limits<long>::max();
  int ScaleIndex = 0;
  for (int TestScaleIndex = 0; TestScaleIndex < 12; ++TestScaleIndex)
  {
    JZScale Scale;
    Scale.Init(TestScaleIndex);
    long Error = 0;
    for (int i = 0; i < 12; ++i)
    {
      if (Scale.ScaleKeys[i] == 0)
      {
        Error += Keys[i];
      }
    }
    if (Error < Min)
    {
      ScaleIndex = TestScaleIndex;
      Min = Error;
    }
  }

  return ScaleIndex;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZScale::Next(int Key)
{
  do
  {
    ++Key;
  } while (!Member(Key));
  return Key;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZScale::Prev(int Key)
{
  do
  {
    --Key;
  } while (!Member(Key));
  return Key;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZScale::Transpose(int Key, int Steps)
{
  int Offset = 0;

  while (!Member(Key))
  {
    ++Key;
    ++Offset;
  }

  while (Steps > 0)
  {
    Key = Next(Key);
    --Steps;
  }
  while (Steps < 0)
  {
    Key = Prev(Key);
    ++Steps;
  }
  return Key - Offset;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZScale::FitInto(int Key)
{
  int Offset = 0;

  while (!Member(Key))
  {
    ++Offset;
    if (Offset & 1)
    {
      Key += Offset;
    }
    else
    {
      Key -= Offset;
    }
  }
  return Key;
}

//*****************************************************************************
// JZCommandShift
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandShift::JZCommandShift(JZFilter* pFilter, long DeltaClock)
  : JZCommand(pFilter),
    mDeltaClock(DeltaClock)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandShift::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZEvent* pEventCopy = pEvent->Copy();
  pTrack->Kill(pEvent);
  pEventCopy->SetClock(pEventCopy->GetClock() + mDeltaClock);
  pTrack->Put(pEventCopy);
}

//*****************************************************************************
// JZCommandErase
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandErase::JZCommandErase(JZFilter* pFilter, int lvsp)
  : JZCommand(pFilter)
{
  LeaveSpace = lvsp;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandErase::Execute(int NewUndo)
{
  JZCommand::Execute(NewUndo);
  if (!LeaveSpace)
  {
    JZFilter Filter(*mpFilter);
    Filter.SetFromClock(mpFilter->GetToClock());
    Filter.SetToClock(mpSong->GetLastClock() + 1);
    long DeltaClock = mpFilter->GetFromClock() - mpFilter->GetToClock();
    JZCommandShift shift(&Filter, DeltaClock);
    shift.Execute(0);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandErase::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  pTrack->Kill(pEvent);
}

//*****************************************************************************
// JZCommandQuantize
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandQuantize::JZCommandQuantize(
  JZFilter* pFilter,
  int QntClocks,
  bool NoteStart,
  bool NoteLength,
  int Groove,
  int Delay)
  : JZCommand(pFilter),
    mQntClocks(QntClocks),
    mNoteStart(NoteStart),
    mNoteLength(NoteLength),
    mGroove(Groove),
    mDelay(Delay)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
long JZCommandQuantize::Quantize(int Clock, int islen)
{
  Clock += mQntClocks / 2;
  Clock -= Clock % mQntClocks;
  if (!islen && (Clock % (2 * mQntClocks) != 0))
  {
    Clock += mGroove;
  }
  Clock += mDelay;
  int MinClock = islen ? 2 : 0;
  return Clock > MinClock ? Clock : MinClock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandQuantize::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn;
  if ((pKeyOn = pEvent->IsKeyOn()) != 0)
  {
    pKeyOn = (JZKeyOnEvent *)pEvent->Copy();
    if (mNoteStart)
    {
      pKeyOn->SetClock(Quantize(pKeyOn->GetClock(), 0));
    }
    if (mNoteLength)
    {
      pKeyOn->SetLength(Quantize(pKeyOn->GetEventLength(), 2));
    }
    pTrack->Kill(pEvent);
    pTrack->Put(pKeyOn);
  }
}

//*****************************************************************************
// JZCommandTranspose
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandTranspose::JZCommandTranspose(
  JZFilter* pFilter,
  int Notes,
  int ScaleIndex,
  bool FitIntoScale)
  : JZCommand(pFilter),
    mNotes(Notes),
    mFitIntoScale(FitIntoScale),
    mScale()
{
  mScale.Init(ScaleIndex, mpFilter);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandTranspose::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn;
  if (pEvent->IsKeyOn())
  {
    pKeyOn = (JZKeyOnEvent *)pEvent->Copy();
    if (mFitIntoScale)
    {
      pKeyOn->SetKey(pKeyOn->GetKey() + mNotes);
      pKeyOn->SetKey(mScale.FitInto(pKeyOn->GetKey()));
    }
    else if (mNotes)
    {
      pKeyOn->SetKey(mScale.Transpose(pKeyOn->GetKey(), mNotes));
    }
    pTrack->Kill(pEvent);
    pTrack->Put(pKeyOn);
  }

  // After touch.
  if (pEvent->IsKeyPressure())
  {
    JZKeyPressureEvent* pKeyPressure = (JZKeyPressureEvent *)pEvent->Copy();
    if (mFitIntoScale)
    {
      pKeyPressure->SetKey(pKeyPressure->GetKey() + mNotes);
      pKeyPressure->SetKey(mScale.FitInto(pKeyPressure->GetKey()));
    }
    else if (mNotes)
    {
      pKeyPressure->SetKey(mScale.Transpose(pKeyPressure->GetKey(), mNotes));
    }
    pTrack->Kill(pEvent);
    pTrack->Put(pKeyPressure);
  }
}

//*****************************************************************************
// JZCommandSetChannel
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandSetChannel::JZCommandSetChannel(JZFilter* pFilter, int NewChannel)
  : JZCommand(pFilter),
    mNewChannel(NewChannel)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandSetChannel::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZChannelEvent* pChannelEvent;

  if ((pChannelEvent = pEvent->IsChannelEvent()) != 0)
  {
    pChannelEvent = (JZChannelEvent *)pEvent->Copy();
    pChannelEvent->SetChannel(mNewChannel);
    pTrack->Kill(pEvent);
    pTrack->Put(pChannelEvent);
  }
}

//*****************************************************************************
// JZtCommandVelocity
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZtCommandVelocity::JZtCommandVelocity(
  JZFilter* pFilter,
  int FromValue,
  int ToValue,
  JEValueAlterationMode Mode)
  : JZCommand(pFilter),
    mFromValue(FromValue),
    mToValue(ToValue),
    mMode(Mode)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZtCommandVelocity::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn;

  if (pEvent->IsKeyOn() != 0)
  {
    pKeyOn = (JZKeyOnEvent *)pEvent->Copy();
    int Value = 0;
    if (mToValue <= 0)
    {
      Value = mFromValue;
    }
    else
    {
      Value = Interpolate(pKeyOn->GetClock(), mFromValue, mToValue);
    }
    switch (mMode)
    {
      case eSetValues:
        break;
      case eAddValues:
        Value = pKeyOn->GetVelocity() + Value;
        break;
      case eSubtractValues:
        Value = pKeyOn->GetVelocity() - Value;
        break;
    }
    pKeyOn->SetVelocity(Value < 1 ? 1 : (Value > 127 ? 127 : Value));
    pTrack->Kill(pEvent);
    pTrack->Put(pKeyOn);
  }
}

//*****************************************************************************
// JZCommandLength
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandLength::JZCommandLength(
  JZFilter* pFilter,
  int FromValue,
  int ToValue,
  JEValueAlterationMode Mode)
  : JZCommand(pFilter),
    mFromValue(FromValue),
    mToValue(ToValue),
    mMode(Mode)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandLength::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn;

  if (pEvent->IsKeyOn() != 0)
  {
    pKeyOn = (JZKeyOnEvent *)pEvent->Copy();
    int Value = 0;
    if (mToValue <= 0)
    {
      Value = mFromValue;
    }
    else
    {
      Value = Interpolate(pKeyOn->GetClock(), mFromValue, mToValue);
    }
    switch (mMode)
    {
      case eSetValues:
        break;
      case eAddValues:
        Value = pKeyOn->GetEventLength() + Value;
        break;
      case eSubtractValues:
        Value = pKeyOn->GetEventLength() - Value;
        break;
    }

    pKeyOn->SetLength(Value < 1 ? 1 : Value);
    pTrack->Kill(pEvent);
    pTrack->Put(pKeyOn);
  }
}

//*****************************************************************************
// JZCommandSequenceLength
//   This command is supposed to stretch/contract a sequence of events in
// time by factor "scale" from starting point "startClock"
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandSequenceLength::JZCommandSequenceLength(JZFilter* pFilter, double scale)
  : JZCommand(pFilter)
{
  this->scale=scale;
  this->startClock=-1000;
}

//-----------------------------------------------------------------------------
// move an event according to startclock and scale
//-----------------------------------------------------------------------------
void JZCommandSequenceLength::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  // Make a copy of the current event.
  JZEvent* k;
  k = (JZKeyOnEvent *)pEvent->Copy();

  //little hack, if clock is -1000 it means set startclock from the first event.
  if (startClock==-1000)
  {
    startClock = k->GetClock();
  }

  //calculate the new start clock, move the new event and kill the old one
  k->SetClock((long int)(((k->GetClock() - startClock) * scale) + startClock));
  pTrack->Kill(pEvent);
  pTrack->Put(k);
}

//*****************************************************************************
// JZCommandConvertToModulation
//    JAVE this command is supposed convert a midi note sequence
// to a pitch bend/volume control sequence instead
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandConvertToModulation::JZCommandConvertToModulation(JZFilter* pFilter)
  : JZCommand(pFilter)
{
}

//-----------------------------------------------------------------------------
//   need to override executetrack, since we have begin/end behaviour in this
// filter
//-----------------------------------------------------------------------------
void JZCommandConvertToModulation::ExecuteTrack(JZTrack* pTrack)
{
  // JAVE:
  // Iterate over all events, make a long event from start until stop of the
  // sequence, convert all note-on messages to a pitch bend/volume controller
  // pair, velocity -> volume make a volume off controller at the end of the
  // current event.
  JZEventIterator Iterator(pTrack);
  JZEvent* pEvent =
    Iterator.Range(mpFilter->GetFromClock(), mpFilter->GetToClock());
  long startclock=-1;
  long endclock=-1;
  unsigned char channel=0;
  unsigned short lastlength=0;
  long previouspitch=0;
  long pitchdiff=0;

  //general midi recommends these values for half-note bends
  long pitchsteparray[] =
  {
    0x0000,
    0x07ff,
    0x1000,
    0x17ff,
    0x2000,
    0x27ff,
    0x3000,
    0x37ff,
    0x3fff
  };

  long startvelocity=0;
  long startkey=0;
  while (pEvent)
  {
    if (mpFilter->IsSelected(pEvent) && pEvent->IsKeyOn())
    {
      if (startclock == -1)
      {
        startclock = pEvent->IsKeyOn()->GetClock();
        startvelocity = pEvent->IsKeyOn()->GetVelocity();
        channel = pEvent->IsKeyOn()->GetChannel();
        startkey = pEvent->IsKeyOn()->GetKey();
        previouspitch = pEvent->GetPitch();
      }
      pitchdiff = pEvent->GetPitch()-previouspitch;

      JZPitchEvent* pitchmodulation=0;
      pitchmodulation = new JZPitchEvent(
        pEvent->GetClock(),
        channel,
        pitchsteparray[pitchdiff + 4]);

      pTrack->Put(pitchmodulation);

      pTrack->Kill(pEvent); //remove the old event

      pTrack->Put(new JZControlEvent(
        pEvent->GetClock(),
        channel,
        0x07,
        pEvent->IsKeyOn()->GetVelocity()));
      pTrack->Put(new JZControlEvent(
        pEvent->GetClock() + pEvent->IsKeyOn()->GetEventLength(),
        channel,
        0x07,
        0));

      lastlength = pEvent->IsKeyOn()->GetEventLength();
      endclock = pEvent->GetClock();
      previouspitch = pEvent->GetPitch();
    }
    //ExecuteEvent(pTrack, pEvent);
    pEvent = Iterator.Next();
  }
  //now insert the new long event
  JZKeyOnEvent* longevent = new JZKeyOnEvent(
    startclock,
    channel,
    startkey,
    startvelocity,
    endclock - startclock + lastlength);
  pTrack->Put(longevent);
  pTrack->Cleanup();
}

//*****************************************************************************
// JZMidiDelayDlg
//    JAVE this is a simple midi delay line
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandMidiDelay::JZCommandMidiDelay(
  JZFilter* pFilter,
  double scale,
  long clockDelay,
  int repeat)
  : JZCommand(pFilter)
{
  this->scale=scale;
  this->clockDelay=clockDelay;
  this->repeat=repeat;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandMidiDelay::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn;

  for (int i = 1; i < repeat; ++i)
  {
    if (pEvent->IsKeyOn())
    {
      // Only echo note events.
      pKeyOn = (JZKeyOnEvent *)pEvent->Copy();
      pKeyOn->SetClock(pKeyOn->GetClock()+ clockDelay * i);
      pKeyOn->SetVelocity(
        (unsigned char)(pow(scale, i) * pKeyOn->GetVelocity()));
      pTrack->Put(pKeyOn);
    }
  }
}

//*****************************************************************************
// JZCommandCleanup
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandCleanup::JZCommandCleanup(JZFilter* pFilter, long clks, int so)
  : JZCommand(pFilter)
{
  lengthLimit = clks;
  shortenOverlaps = so;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandCleanup::ExecuteTrack(JZTrack* pTrack)
{
  memset(prev_note, 0, sizeof(prev_note));
  JZCommand::ExecuteTrack(pTrack);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandCleanup::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn;
  if ((pKeyOn = pEvent->IsKeyOn()) != 0)
  {
    if (pKeyOn->GetEventLength() < lengthLimit)
    {
      // Remove short notes.
      pTrack->Kill(pEvent);
    }
    else if (shortenOverlaps)
    {
      // Shorten length of overlapping notes.
      JZKeyOnEvent* pPreviousKeyOn = prev_note[pKeyOn->GetChannel()][pKeyOn->GetKey()];
      if (
        pPreviousKeyOn &&
        pPreviousKeyOn->GetClock() + pPreviousKeyOn->GetEventLength() >=
          pKeyOn->GetClock())
      {
        pPreviousKeyOn->SetLength(
          pKeyOn->GetClock() - pPreviousKeyOn->GetClock() - 1);
        if (pPreviousKeyOn->GetEventLength() < lengthLimit)
        {
          pTrack->Kill(pPreviousKeyOn);
        }
      }
      prev_note[pKeyOn->GetChannel()][pKeyOn->GetKey()] = pKeyOn;
    }
  }
}

//*****************************************************************************
// JZCommandSearchReplace
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandSearchReplace::JZCommandSearchReplace(JZFilter* pFilter, short From, short To)
  : JZCommand(pFilter),
    mFrom(From),
    mTo(To)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandSearchReplace::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZControlEvent* pControl = pEvent->IsControl();
  if (pControl)
  {
    if (pControl->GetControl() == mFrom)
    {
      JZControlEvent* pControlCopy = (JZControlEvent *)pControl->Copy();
      pControlCopy->SetControl(mTo);
      pTrack->Kill(pControl);
      pTrack->Put(pControlCopy);
    }
  }
}

//*****************************************************************************
// JZCommandCopyToBuffer
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandCopyToBuffer::JZCommandCopyToBuffer(
  JZFilter* pFilter,
  JZEventArray* pBuffer)
  : JZCommand(pFilter)
{
  mpBuffer = pBuffer;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandCopyToBuffer::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  mpBuffer->Put(pEvent->Copy());
}

//*****************************************************************************
// JZCommandCopy
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandCopy::JZCommandCopy(JZFilter* pFilter, long dt, long dc)
  : JZCommand(pFilter)
{
  DestTrack = dt;
  DestClock = dc;

  EraseSource = 0;        // no
  EraseDestin = 1;        // yes
  InsertSpace = 0;        // no
  RepeatClock = -1;        // -1L

  mReverse = DestTrack > mpFilter->GetFromTrack();
  if (mReverse)
  {
    // ToTrack inclusive.
    DestTrack += mpFilter->GetToTrack() - mpFilter->GetFromTrack();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandCopy::ExecuteTrack(JZTrack *s)
{
  long StartClock, StopClock;
  JZTrack *d;

  StartClock = DestClock;

  if (RepeatClock < 0)
    StopClock = StartClock + mpFilter->GetToClock() - mpFilter->GetFromClock();
  else
    StopClock = RepeatClock;

  d = mpSong->GetTrack(DestTrack);

  if (mReverse)
  {
    if (DestTrack)
    {
      --DestTrack;
    }
  }
  else
  {
    ++DestTrack;
  }

  if (s && d)
  {

    JZEventArray tmp;
    {
      // Events after temporary copy.
      JZEventIterator Iterator(s);
      long  DeltaClock = StartClock - mpFilter->GetFromClock();
      JZEvent* pEvent =
        Iterator.Range(mpFilter->GetFromClock(), mpFilter->GetToClock());
      while (pEvent)
      {
        long NewClock = pEvent->GetClock() + DeltaClock;
        if (NewClock >= StopClock)
          break;

        if (mpFilter->IsSelected(pEvent))
        {
          JZEvent* cpy = pEvent->Copy();
          cpy->SetClock(NewClock);
          tmp.Put(cpy);
        }

        pEvent = Iterator.Next();
        if (!pEvent)
        {
          pEvent = Iterator.First();
          DeltaClock += mpFilter->GetToClock() - mpFilter->GetFromClock();
        }
      }
    }

    if (InsertSpace && d->GetLastClock() > StartClock)
    {
      JZEventIterator Iterator(d);
      JZEvent* pEvent = Iterator.Range(StartClock, d->GetLastClock() + 1);
      long DeltaClock = StopClock - StartClock;
      while (pEvent)
      {
        if (mpFilter->IsSelected(pEvent))
        {
          JZEvent* c = pEvent->Copy();
          c->SetClock(c->GetClock() + DeltaClock);
          d->Kill(pEvent);
          d->Put(c);
        }
        pEvent = Iterator.Next();
      }
      d->Cleanup();
    }

    if (EraseSource)
    {
      // Delete source.
      JZEventIterator Iterator(s);
      JZEvent* pEvent = Iterator.Range(mpFilter->GetFromClock(), mpFilter->GetToClock());
      while (pEvent)
      {
        if (mpFilter->IsSelected(pEvent))
        {
          s->Kill(pEvent);
        }
        pEvent = Iterator.Next();
      }
      s->Cleanup();
    }

    if (EraseDestin)
    {
      // Delete destination.
      JZEventIterator Iterator(d);
      JZEvent* pEvent = Iterator.Range(StartClock, StopClock);
      while (pEvent)
      {
        if (mpFilter->IsSelected(pEvent))
        {
          d->Kill(pEvent);
        }
        pEvent = Iterator.Next();
      }
      d->Cleanup();
    }

    // tmp track and target mix, aufraeumen

    d->Merge(&tmp);
    d->Cleanup();
  }
}

//*****************************************************************************
// JZCommandExchangeLeftRight
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandExchangeLeftRight::JZCommandExchangeLeftRight(JZFilter* pFilter)
  : JZCommand(pFilter)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandExchangeLeftRight::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  if (pEvent->IsKeyOn())
  {
    JZKeyOnEvent* pKeyOn = (JZKeyOnEvent *)pEvent->Copy();
    pKeyOn->SetClock(
      mpFilter->GetFromClock() + mpFilter->GetToClock() - pKeyOn->GetClock());
    pTrack->Kill(pEvent);
    pTrack->Put(pKeyOn);
  }
}

//*****************************************************************************
// JZCommandExchangeUpDown
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandExchangeUpDown::JZCommandExchangeUpDown(JZFilter* pFilter)
  : JZCommand(pFilter)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandExchangeUpDown::ExecuteTrack(JZTrack* pTrack)
{
  int i;
  int Keys[128];
  JZEvent* pEvent;

  // find all Key's selected

  for (i = 0; i < 128; i++)
  {
    Keys[i] = 0;
  }

  JZEventIterator Iterator(pTrack);
  pEvent =
    Iterator.Range(mpFilter->GetFromClock(), mpFilter->GetToClock());
  while (pEvent)
  {
    if (mpFilter->IsSelected(pEvent) && pEvent->IsKeyOn())
    {
      JZKeyOnEvent* pKeyOn = (JZKeyOnEvent *)pEvent;
      Keys[pKeyOn->GetKey()] = 1;
    }
    pEvent = Iterator.Next();
  }

  // reverse Key's

  pEvent =
    Iterator.Range(mpFilter->GetFromClock(), mpFilter->GetToClock());
  while (pEvent)
  {
    if (mpFilter->IsSelected(pEvent) && pEvent->IsKeyOn())
    {
      JZKeyOnEvent* pKeyOn = (JZKeyOnEvent *)pEvent->Copy();
      int n_th = 0;

      // the n'th key from bottom
      for (i = 0; i <= pKeyOn->GetKey(); i++)
      {
        n_th += Keys[i];
      }

      // .. becomes the n'th key from top
      for (i = 127; i > 0 && n_th; --i)
      {
        n_th -= Keys[i];
      }

      pKeyOn->SetKey(i + 1);

      pTrack->Kill(pEvent);
      pTrack->Put(pKeyOn);
    }
    pEvent = Iterator.Next();
  }
  pTrack->Cleanup();
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandMapper::JZCommandMapper(
  JZFilter* pFilter,
  prop Source,
  prop Destination,
  JZRndArray& RandomArray,
  int BarCount,
  bool Add)
  : JZCommand(pFilter),
    mBarCount(BarCount),
    mStartBar(0),
    mAdd(Add),
    mSource(Source),
    mDestination(Destination),
    mpBarInfo(nullptr),
    mRandomArray(RandomArray)
{
  mpBarInfo = new JZBarInfo(*mpSong);
  mpBarInfo->SetClock(mpFilter->GetFromClock());
  mStartBar = mpBarInfo->GetBarIndex();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCommandMapper::~JZCommandMapper()
{
  delete mpBarInfo;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCommandMapper::ExecuteEvent(JZTrack* pTrack, JZEvent* pEvent)
{
  JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
  if (pKeyOn)
  {
    int sval = 0;
    switch (mSource)
    {
      case veloc:
        sval = mRandomArray[(int)pKeyOn->GetVelocity()];
        break;

      case length:
        sval = mRandomArray[(int)pKeyOn->GetEventLength()];
        break;

      case key:
        sval = mRandomArray[(int)pKeyOn->GetKey()];
        break;

      case rhythm:
      {
        mpBarInfo->SetClock(pKeyOn->GetClock());
        long arr_tpb = mRandomArray.Size() / mBarCount;
        long arr_bar = (mpBarInfo->GetBarIndex() - mStartBar) % mBarCount;
        long i = arr_tpb * arr_bar + arr_tpb *
          (pKeyOn->GetClock() - mpBarInfo->GetClock()) / mpBarInfo->GetTicksPerBar();
//        cout
//          << "mpBarInfo->GetTicksPerBar() " << mpBarInfo->GetTicksPerBar()
//          << ", arr_tpb " << arr_tpb
//          << ", pKeyOn->GetClock() " << pKeyOn->GetClock()
//          << ", mpBarInfo->GetClock() " << mpBarInfo->GetClock()
//          << '\n'
//          << "mRandomArray.Size() " << mRandomArray.Size()
//          << ", mBarCount " << mBarCount
//          << ", i " << i
//          << endl;
        sval = mRandomArray[(int)i];
      }
      break;

      case random:
        sval = mRandomArray.Random();
        if (mAdd)
        {
          sval -= mRandomArray.Size()/2;
        }
        break;

      default:
        break;
    }

    switch (mDestination)
    {
      case veloc:
      {
        if (mAdd)
        {
          sval = pKeyOn->GetVelocity() + sval;
        }
        if (sval > 127)
        {
          sval = 127;
        }
        if (sval < 1)
        {
          sval = 1;
        }
        JZKeyOnEvent* pKeyOnCopy = (JZKeyOnEvent *)pKeyOn->Copy();
        pTrack->Kill(pKeyOn);
        pKeyOnCopy->SetVelocity(sval);
        pTrack->Put(pKeyOnCopy);
      }
      break;

      case key:
      {
        if (mAdd)
        {
          sval = pKeyOn->GetKey() + sval;
        }
        if (sval > 127)
        {
          sval = 127;
        }
        if (sval < 1)
        {
          sval = 1;
        }
        JZKeyOnEvent* pKeyOnCopy = (JZKeyOnEvent *)pKeyOn->Copy();
        pTrack->Kill(pKeyOn);
        pKeyOnCopy->SetKey(sval);
        pTrack->Put(pKeyOnCopy);
      }
      break;

      case length:
      {
        if (mAdd)
        {
          sval = pKeyOn->GetEventLength() + sval;
        }
        if (sval < 1)
        {
          sval = 1;
        }
        JZKeyOnEvent* pKeyOnCopy = (JZKeyOnEvent *)pKeyOn->Copy();
        pTrack->Kill(pKeyOn);
        pKeyOnCopy->SetLength(sval);
        pTrack->Put(pKeyOnCopy);
      }
      break;

      case clock:
      {
        JZKeyOnEvent* pKeyOnCopy = (JZKeyOnEvent *)pKeyOn->Copy();
        pKeyOnCopy->SetClock(pKeyOnCopy->GetClock() + sval);
        if (pKeyOnCopy->GetClock() < 0)
        {
          pKeyOnCopy->SetClock(0);
        }
        pTrack->Kill(pKeyOn);
        pTrack->Put(pKeyOnCopy);
      }
      break;

      case pan:
      case modul:
      case cc1:
      case cc2:
      case pitch:
      default:
        break;
    }
  }
}
