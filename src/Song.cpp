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

#include "Song.h"

//#include "Audio.h"
#include "Configuration.h"
//#include "Command.h"
#include "Globals.h"
#include "Metronome.h"
#include "StringUtilities.h"
#include "Synth.h"

#include <wx/cursor.h>

#include <iomanip>
#include <sstream>

using namespace std;

//*****************************************************************************
// Description:
//   This is the bar information class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZBarInfo::JZBarInfo(const JZSong& Song)
  : mBarIndex(0),
    mClock(0),
    mCountsPerBar(4),
    mTicksPerQuarter(Song.GetTicksPerQuarter()),
    mTicksPerBar(mTicksPerQuarter * 4),
    mIterator(&Song.mTracks[0]),
    mpEvent(nullptr)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZBarInfo::SetBar(int BarIndex)
{
  if (BarIndex < 0)  // avoid infinite loop
  {
    BarIndex = 0;
  }

  mBarIndex = BarIndex;
  mClock = 0;
  mTicksPerBar = mTicksPerQuarter * 4;
  mpEvent = mIterator.First();
  while (1)
  {
    // Events bis Taktanfang nach MeterChange durchsuchen
    // Meter-Event vor oder genau auf Taktanfang stehen
    while (mpEvent && mpEvent->GetClock() <= mClock)
    {
      mpEvent->BarInfo(mTicksPerBar, mCountsPerBar, mTicksPerQuarter);
      mpEvent = mIterator.Next();
    }

    if (!BarIndex)
    {
      return;
    }

    // Clock + Bar auf Anfang naechster Takt
    --BarIndex;
    mClock += mTicksPerBar;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZBarInfo::SetClock(int Clock)
{
  mBarIndex = 0;
  mClock = 0;
  mTicksPerBar = mTicksPerQuarter * 4;
  mpEvent = mIterator.First();
  while (1)
  {
    while (mpEvent && mpEvent->GetClock() <= mClock)
    {
      mpEvent->BarInfo(mTicksPerBar, mCountsPerBar, mTicksPerQuarter);
      mpEvent = mIterator.Next();
    }

    if (mClock + mTicksPerBar > Clock)
    {
      return;
    }

    // Clock + Bar at the beginning of the next cycle.
    mClock += mTicksPerBar;
    ++mBarIndex;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZBarInfo::Next()
{
  ++mBarIndex;
  mClock += mTicksPerBar;

  while (mpEvent && mpEvent->GetClock() <= mClock)
  {
    mpEvent->BarInfo(mTicksPerBar, mCountsPerBar, mTicksPerQuarter);
    mpEvent = mIterator.Next();
  }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSong::JZSong()
  : // Start with 100 measures of 4:4 time.
    mMaxQuarters(100 * 4),

    mTicksPerQuarter(120),
    mIntroLength(0),
    mTrackCount(eMaxTrackCount),
    mTracks()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSong::~JZSong()
{
  Clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::Clear()
{
  for (int i = 0; i < eMaxTrackCount; ++i)
  {
    mTracks[i].Clear();
  }
  mTrackCount = eMaxTrackCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSong::Speed()
{
  return mTracks[0].GetDefaultSpeed();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::Read(JZReadBase& Io, const string& FileName)
{
  int i;
  wxBeginBusyCursor();
  for (i = 0; i < eMaxTrackCount; ++i)
  {
    mTracks[i].Clear();
  }
  int n = Io.Open(FileName);
  for (i = 0; i < n && i < eMaxTrackCount; ++i)
  {
    mTracks[i].Read(Io);
  }
  Io.Close();
  mTicksPerQuarter = Io.GetTicksPerQuarter();

  if (mTicksPerQuarter < 48)
  {
    SetTicksPerQuarter(48);
  }
  else if (mTicksPerQuarter > 192)
  {
    SetTicksPerQuarter(192);
  }

  // Adjust the song length to equal the MIDI file length plus 16 bars.
  int NewLength = GetLastClock() / mTicksPerQuarter + 16 * 4;
  if (NewLength > mMaxQuarters)
  {
    mMaxQuarters = NewLength;
  }

  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::Write(JZWriteBase& Io, const string& FileName)
{
  // Make sure track 0 has a synth reset
  if (!mTracks[0].mpReset)
  {
    JZEvent* pEvent = gpSynth->CreateResetEvent();
    mTracks[0].mpReset = dynamic_cast<JZSysExEvent*>(pEvent);
  }

  int n = NumUsedTracks();
  if (!Io.Open(FileName, n, mTicksPerQuarter))
  {
    return;
  }

  wxBeginBusyCursor();
  for (int i = 0; i < n; ++i)
  {
    mTracks[i].Write(Io);
  }
  Io.Close();
  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrack* JZSong::GetTrack(int TrackIndex)
{
  if (TrackIndex >= 0 && TrackIndex < mTrackCount)
  {
    return &mTracks[TrackIndex];
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSong::GetLastClock() const
{
  int LastClock = 0;
  for (int i = 0; i < mTrackCount; ++i)
  {
    int Clock = mTracks[i].GetLastClock();
    if (Clock > LastClock)
    {
      LastClock = Clock;
    }
  }
  return LastClock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::ClockToString(int Clock, string& ClockString) const
{
  JZBarInfo BarInfo(*this);
  BarInfo.SetClock(Clock);
  Clock -= BarInfo.GetClock();
  int TicksPerCount = BarInfo.GetTicksPerBar() / BarInfo.GetCountsPerBar();
  int Count = Clock / TicksPerCount;

  ostringstream Oss;
  Oss
    << setw(3) << BarInfo.GetBarIndex() + 1 - mIntroLength
    << ':' << Count + 1
    << ':' << setw(3) << setfill('0') << Clock % TicksPerCount;
  ClockString = Oss.str();
#ifdef TEST_CLOCK_TO_STRING
  char Buffer[500];
  sprintf(
    Buffer,
    "%3d:%d:%03d",
    BarInfo.GetBarIndex() + 1 - mIntroLength,
    Count + 1,
    Clock % TicksPerCount);
  if (ClockString != string(buf))
  {
    cout << ClockString << '\n' << buf << endl;
  }
#endif // TEST_CLOCK_TO_STRING
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSong::StringToClock(const string& ClockString) const
{
  int Bar = 1;
  int Clock = 0;
  int Count = 1;
//OLD  sscanf(buf, "%d:%d:%d", &Bar, &Count, &Clock);

  const string Delimiters(":");
  vector<string> Tokens;
  if (TNStringUtilities::Tokenize(Delimiters, ClockString, Tokens) < 3)
  {
    return 0;
  }

  istringstream Iss;

  Iss.clear();
  Iss.str(Tokens[0]);
  Iss >> Bar;

  // Adjust for the start count index.  Users start with 1; the code starts
  // with 0.
  --Bar;

  // The string is from user input so we must adjust by the intro length.
  Bar += mIntroLength;

  Iss.clear();
  Iss.str(Tokens[1]);
  Iss >> Count;

  // Adjust for the start count index.  Users start with 1; the code starts
  // with 0.
  --Count;

  Iss.clear();
  Iss.str(Tokens[2]);
  Iss >> Clock;

  JZBarInfo BarInfo(*this);
  BarInfo.SetBar(Bar);
  int TicksPerCount = BarInfo.GetTicksPerBar() / BarInfo.GetCountsPerBar();
  return BarInfo.GetClock() + Count * TicksPerCount + Clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::MergeTracks(
  int FrClock,
  int ToClock,
  JZEventArray* pDestin,
  const JZMetronomeInfo& MetronomeInfo,
  int delta,
  bool AudioMode)
{
  // Make metronome
  if (MetronomeInfo.IsOn())
  {
    MakeMetronome(FrClock, ToClock, pDestin, MetronomeInfo, delta);
  }

  // Find solo-tracks.
  int i;
  bool DoSoloTracksExist = false;
  for (i = 0; i < mTrackCount; i++)
  {
    if (mTracks[i].mState == tsSolo)
    {
      DoSoloTracksExist = true;
      break;
    }
  }

  for (i = 0; i < mTrackCount; ++i)
  {
    JZTrack* pTrack = &mTracks[i];
    if (
      pTrack->mState == tsSolo ||
      (!DoSoloTracksExist && pTrack->mState == tsPlay))
    {
      if (pTrack->GetAudioMode() != AudioMode)
      {
        continue;
      }

      JZEventIterator Iterator(&mTracks[i]);
      JZEvent* pEvent = Iterator.Range(FrClock, ToClock);
      while (pEvent)
      {
        JZEvent* pEventCopy = pEvent->Copy();
        pEventCopy->SetClock(pEventCopy->GetClock() + delta);
        pEventCopy->SetDevice(pTrack->GetDevice());
        pDestin->Put(pEventCopy);

        if (pEventCopy->IsPlayTrack())
        {
          MergePlayTrackEvent(pEventCopy->IsPlayTrack(), pDestin, 0);
        }

        pEvent = Iterator.Next();
      }
    }
  }
}

//-----------------------------------------------------------------------------
// Should call recursively, so playtrack events will resolve playtrack events.
//-----------------------------------------------------------------------------
void JZSong::MergePlayTrackEvent(
  JZPlayTrackEvent* c, //the playtrack event
  JZEventArray* pDestin,
  int recursionDepth)
{
  // Recursion might be simple, but we have the infinite loop problem, if a
  // playtrack point to itself, either directly or indirectly.
  // We probaly need to keep a list of seen tracks, simpler is to limit the
  // recursion level to 100 or something.
  // The actual recursion is done later in the loop.
  recursionDepth++;
  if (recursionDepth > 100) //yes yes, you should use symbolics...
  {
    return;
  }

  fprintf(stderr, "playtrack %d\n", c->track);
  JZTrack* pTrack = &mTracks[c->track]; // the track we want to play

  // Get an iterator of all events the playtrack is pointing to.
  JZEventIterator IteratorPL(pTrack);
  JZEvent* f;

  // FIXME this is just to test the idea.  It would be good to modify
  // GetLastClock instead.
  // Find an EOT event, otherwise default to the last clock (should be +
  // length of the last event as well).
  int loopLength = 0;

  // Get an iterator of all events the playtrack is pointing to.
  JZEventIterator IteratorEOT(pTrack);
  f = IteratorEOT.Range(0, pTrack->GetLastClock());

  // looplength will be used to loop the track, for the duration of
  // the playtrack event.
  loopLength = pTrack->GetLastClock();
  while (f)
  {
    if (f->IsEndOfTrack())
    {
      loopLength = pTrack->GetLastClock();
    }
    f = IteratorEOT.Next();
  }

  int loopOffset = 0;

  //   The loop below is supposed to repeat the referenced track for the
  // duration of the playtrack event.  Also, we should look out for if we
  // start playing in the middle of an playtrack event, curently it wont
  // play anything.
  //   To fix this we could search all unmuted tracks from the beginning to
  // the start of the loop-point, and see if there are any playtrack events
  // which start + length end up after the startloop.  Then we could move the
  // start of those to the beginning of the loop, and shorten the length.
  // There would still be a problem with playtracks not even bar length.
  while (loopOffset < c->eventlength)
  {
    // No more events than the length of the playtrack!  and ensure the last
    // iteration is no longer than what is left.
    f = IteratorPL.Range(0, (c->eventlength-loopOffset));
    while (f)
    {
      JZEvent* d = f->Copy();
      d->SetClock(d->GetClock() + c->GetClock() + loopOffset);
      if (d->IsKeyOn())
      {
        d->IsKeyOn()->SetKey(d->IsKeyOn()->GetKey() + c->transpose);
      }
      if (d->IsPlayTrack())
      {
        MergePlayTrackEvent(d->IsPlayTrack(), pDestin, recursionDepth);
      }
      d->SetDevice(pTrack->GetDevice());
      pDestin->Put(d);
      f = IteratorPL.Next();
    }
    loopOffset+=loopLength;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::MakeMetronome(
  int FrClock,
  int ToClock,
  JZEventArray* pDestin,
  const JZMetronomeInfo& MetronomeInfo,
  int delta)
{
  JZBarInfo BarInfo(*this);
  BarInfo.SetClock(FrClock);
  int clk = BarInfo.GetClock();
  int count = 1;

  while (clk < FrClock)
  {
    clk += BarInfo.GetTicksPerBar() / BarInfo.GetCountsPerBar();
    count++;
  }

  while (clk < ToClock)
  {
    if (count > BarInfo.GetCountsPerBar())
    {
      BarInfo.Next();
      clk = BarInfo.GetClock();
      count = 1;
    }

    // Insert normal click always
    pDestin->Put(MetronomeInfo.CreateNormalEvent(clk + delta));

    //  On a bar?
    if (count == 1 && MetronomeInfo.IsAccented())
    {
      // Insert accented click also
      pDestin->Put(MetronomeInfo.CreateAccentedEvent(clk + delta));
    }

    clk += BarInfo.GetTicksPerBar() / BarInfo.GetCountsPerBar();
    count++;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::NewUndoBuffer()
{
  for (int i = 0; i < mTrackCount; ++i)
  {
    mTracks[i].NewUndoBuffer();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::Undo()
{
  wxBeginBusyCursor();
  for (int i = 0; i < mTrackCount; ++i)
  {
    mTracks[i].Undo();
  }
  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::Redo()
{
  wxBeginBusyCursor();
  for (int i = 0; i < mTrackCount; ++i)
  {
    mTracks[i].Redo();
  }
  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSong::SetTicksPerQuarter(int TicksPerQuarter)
{
  double f = (double)TicksPerQuarter / (double)mTicksPerQuarter;
  for (int TrackIndex = 0; TrackIndex < mTrackCount; ++TrackIndex)
  {
    JZTrack* pTrack = &mTracks[TrackIndex];
    for (int EventIndex = 0; EventIndex < pTrack->mEventCount; ++EventIndex)
    {
      JZEvent* pEvent = pTrack->mppEvents[EventIndex];
      pEvent->SetClock((int)(f * pEvent->GetClock() + 0.5));
      JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
      if (pKeyOn)
      {
        pKeyOn->SetLength((int)(f * pKeyOn->GetEventLength() + 0.5));
      }
    }
  }
  mTicksPerQuarter = TicksPerQuarter;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSong::SetMeterChange(int BarNr, int Numerator, int Denomiator)
{
  NewUndoBuffer();

  // Clock Taktanfang und -ende

  JZBarInfo BarInfo(*this);
  BarInfo.SetBar(BarNr - 1);
  int FrClock = BarInfo.GetClock();
  BarInfo.Next();
  int ToClock = BarInfo.GetClock();

  // evtl vorhandene TimeSignatures loeschen

  JZTrack* pTrack = &mTracks[0];
  JZEventIterator Iterator(pTrack);
  JZEvent* pEvent = Iterator.Range(FrClock, ToClock);
  while (pEvent)
  {
    if (pEvent->IsTimeSignat())
    {
      pTrack->Kill(pEvent);
    }
    pEvent = Iterator.Next();
  }

  // neues TimeSignature Event ablegen

  int Shift = 2;
  switch (Denomiator)
  {
    case 1:  Shift = 0; break;
    case 2:  Shift = 1; break;
    case 4:  Shift = 2; break;
    case 8:  Shift = 3; break;
    case 16: Shift = 4; break;
    case 32: Shift = 5; break;
  }

  pEvent = new JZTimeSignatEvent(FrClock, Numerator, Shift);
  pTrack->Put(pEvent);
  pTrack->Cleanup();
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSong::NumUsedTracks()
{
  int UsedTrackCount;
  for (UsedTrackCount = mTrackCount; UsedTrackCount > 1; --UsedTrackCount)
  {
    if (!mTracks[UsedTrackCount - 1].IsEmpty())
    {
      break;
    }
  }
  return UsedTrackCount;
}

//-----------------------------------------------------------------------------
// SN++
//-----------------------------------------------------------------------------
void JZSong::moveTrack(int from, int to)
{
  if (from == to)
  {
    return;
  }

  JZTrack* pTrack = &mTracks[from];
  if (from > to)
  {
    for (int i = from; i >= to; --i)
    {
       mTracks[i] = mTracks[i - 1];
    }
  }
  else
  {
    for (int i = from; i <= to; ++i)
    {
       mTracks[i] = mTracks[i + 1];
    }
  }
  mTracks[to] = *pTrack;
}
