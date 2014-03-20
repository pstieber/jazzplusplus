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

#include "HarmonyBrowserAnalyzer.h"

#include "Command.h"
#include "EventWindow.h"
#include "Filter.h"
#include "HarmonyP.h"
#include "Song.h"

#include <cmath>

JZHarmonyBrowserAnalyzer::JZHarmonyBrowserAnalyzer(JZHarmonyBrowserContext **s, int n)
  : seq(s),
    max_seq(n),
    start_clock(0),
    stop_clock(0),
    eighths_per_chord(1),
    mSteps(0),
    mpFilter(0),
    mpTrack(0),
    count(0),
    delta(0)
{
}


JZHarmonyBrowserAnalyzer::~JZHarmonyBrowserAnalyzer()
{
  Exit();
}


void JZHarmonyBrowserAnalyzer::Init(JZFilter* pFilter, int epc)
{
  Exit();        // cleanup from previous run

  mpFilter = pFilter;
  start_clock = pFilter->GetFromClock();
  stop_clock = pFilter->GetToClock();
  eighths_per_chord = epc;

  if (eighths_per_chord == 0)
  {
    mSteps = max_seq;        // map number of chords to selection
  }
  else
  {
    JZBarInfo BarInfo(*mpFilter->GetSong());
    BarInfo.SetClock(start_clock);
    int start_bar = BarInfo.GetBarIndex();
    BarInfo.SetClock(stop_clock);
    int stop_bar = BarInfo.GetBarIndex();
    mSteps = (stop_bar - start_bar) * 8L / eighths_per_chord;
  }

  count = new int* [mSteps];
  delta = new int* [mSteps];
  for (int i = 0; i < mSteps; i++)
  {
    count[i] = new int[12];
    delta[i] = new int[12];
    for (int j = 0; j < 12; j++)
    {
      count[i][j] = 0;
      delta[i][j] = 0;
    }
  }
}


void JZHarmonyBrowserAnalyzer::Exit()
{
  for (int i = 0; i < mSteps; i++)
  {
    delete [] count[i];
    delete [] delta[i];
  }
  delete [] count;
  delete [] delta;
  count = 0;
  delta = 0;
  mSteps = 0;
}


int JZHarmonyBrowserAnalyzer::Analyze(JZFilter* pFilter, int qbc)
{
  Init(pFilter, qbc);
  if (mSteps < max_seq)
  {
    IterateEvents(&JZHarmonyBrowserAnalyzer::CountEvent);
    CreateChords();
    return mSteps;
  }
  return 0;
}

int JZHarmonyBrowserAnalyzer::Transpose(JZFilter* pFilter, int qbc)
{
  pFilter->GetSong()->NewUndoBuffer();
  Init(pFilter, qbc);
  IterateEvents(&JZHarmonyBrowserAnalyzer::CountEvent);
  GenerateMapping();
  IterateEvents(&JZHarmonyBrowserAnalyzer::TransposeEvent);
  return 0;
}


void JZHarmonyBrowserAnalyzer::IterateEvents(void (JZHarmonyBrowserAnalyzer::*Action)(JZKeyOnEvent*, JZTrack*))
{
  JZTrackIterator Tracks(mpFilter);
  JZTrack *t = Tracks.First();
  while (t)
  {
    if (!t->IsDrumTrack())
    {
      JZEventIterator Events(t);
      JZEvent* pEvent =
        Events.Range(mpFilter->GetFromClock(), mpFilter->GetToClock());
      while (pEvent)
      {
        JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
        if (pKeyOn)
        {
          (this->*Action)(pKeyOn, t);
        }
        pEvent = Events.Next();
      }
      t->Cleanup();
    }
    t = Tracks.Next();
  }
}


int JZHarmonyBrowserAnalyzer::Step2Clock(int step)
{
  int fr = mpFilter->GetFromClock();
  int to = mpFilter->GetToClock();
  return (step * (to - fr)) / mSteps + fr;
}

void JZHarmonyBrowserAnalyzer::CountEvent(JZKeyOnEvent* pKeyOn, JZTrack *t)
{
  for (int i = 0; i < mSteps; i++)
  {
    int start = Step2Clock(i);
    int stop  = Step2Clock(i+1);
    if (
      pKeyOn->GetClock() + pKeyOn->GetEventLength() >= start &&
      pKeyOn->GetClock() < stop)
    {
      if (pKeyOn->GetClock() > start)
      {
        start = pKeyOn->GetClock();
      }
      if (pKeyOn->GetClock() + pKeyOn->GetEventLength() < stop)
      {
        stop = pKeyOn->GetClock() + pKeyOn->GetEventLength();
      }
      count[i][pKeyOn->GetKey() % 12] += stop - start;
    }
  }
}


void JZHarmonyBrowserAnalyzer::TransposeEvent(JZKeyOnEvent* pKeyOn, JZTrack* pTrack)
{
  for (int i = 0; i < mSteps; i++)
  {
    int start = Step2Clock(i);
    int stop  = Step2Clock(i+1);
    if (
      pKeyOn->GetClock() + pKeyOn->GetEventLength() >= start &&
      pKeyOn->GetClock() < stop)
    {
      // key matches this step
      int fr = start;
      int to = stop;
      if (pKeyOn->GetClock() > fr)
      {
        fr = pKeyOn->GetClock();
      }
      if (pKeyOn->GetClock() + pKeyOn->GetEventLength() < to)
      {
        to = pKeyOn->GetClock() + pKeyOn->GetEventLength();
      }

      // transpose if most of key length belongs to this step
      // OR: it covers the whole step
      if (to - fr >= pKeyOn->GetEventLength() / 2 || (fr == start && to == stop))
      {
        JZKeyOnEvent* pKeyOnCopy = (JZKeyOnEvent *)pKeyOn->Copy();
        pKeyOnCopy->SetKey(pKeyOnCopy->GetKey() + delta[i][pKeyOn->GetKey() % 12]);
        pTrack->Kill(pKeyOn);
        pTrack->Put(pKeyOnCopy);

        // do not transpose again
        break;
      }
    }
  }
}


int JZHarmonyBrowserAnalyzer::NumCount(int i)
{
  // count the notes of step i
  int  n = 0;
  for (int k = 0; k < 12; k++)
  {
    if (count[i][k] > 0)
      n++;
  }
  return n;
}


int JZHarmonyBrowserAnalyzer::MaxCount(int i, const JZHarmonyBrowserChord &done)
{
  // find the most used note in step i
  int imax = 0;
  int nmax = 0;
  for (int k = 0; k < 12; k++)
  {
    if (count[i][k] > nmax && !done.Contains(k))
    {
      nmax = count[i][k];
      imax = k;
    }
  }
  return imax;
}


#if 1
void JZHarmonyBrowserAnalyzer::GenerateMapping()
{
  int step;
  for (step = 0; step < mSteps; step++)
  {
    int j;

    int iseq = step % max_seq;
    JZHarmonyBrowserChord chord = seq[iseq]->Chord();
    JZHarmonyBrowserChord scale = seq[iseq]->Scale();
    int notes_remaining = NumCount(step);

    JZHarmonyBrowserChord c;
    JZHarmonyBrowserChord done;
    int     n;

    done.Clear();

    // map chord notes to the most used notes

    c = chord;
    n = c.Count();
    for (j = 0; notes_remaining && j < n; j++)
    {
      int old_key = MaxCount(step, done);
      int new_key = c.Fit(old_key);
      delta[step][old_key] = new_key - old_key;
      c -= new_key;
      notes_remaining --;
      done += old_key;
    }

    // map scale notes to the remaining notes

    c = scale - chord;
    n = c.Count();
    for (j = 0; notes_remaining && j < n; j++)
    {
      int old_key = MaxCount(step, done);
      int new_key = c.Fit(old_key);
      delta[step][old_key] = new_key - old_key;
      c -= new_key;
      notes_remaining --;
      done += old_key;
    }

    // map remaining notes cromatic

    c = chromatic_scale - scale - chord;
    n = c.Count();
    for (j = 0; notes_remaining && j < n; j++)
    {
      int old_key = MaxCount(step, done);
      int new_key = c.Fit(old_key);
      delta[step][old_key] = new_key - old_key;
      c -= new_key;
      notes_remaining --;
      done += old_key;
    }

  }
}

#else

class JZChordMatrix
{
  public:
    JZChordMatrix()
    {
      for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++)
          mat[i][j] = 0;
    }
    double* operator[](int i)
    {
      return mat[i];i
    }
  private:
    double mat[12][12];
};


void JZHarmonyBrowserAnalyzer::GenerateMapping()
{
  int step;
  for (step = 0; step < mSteps; step++)
  {
    int i, j;

    JZChordMatrix mat;

    int iseq = step % max_seq;
    JZHarmonyBrowserChord chord = seq[iseq]->Chord();
    JZHarmonyBrowserChord scale = seq[iseq]->Scale();

    int not_in_chord_costs = 50;
    int not_in_scale_costs = 100;
    int costs_per_semitone = 10;

    // create matrix:
    // mat[i][j] = costs for transposing old key i to new key j
    for (i = 0; i < 12; i++)
    {
      for (j = 0; j < 12; j++)
      {
        double cost = 0;
        if (!chord.Contains(j))
          cost += not_in_chord_costs;
        if (!scale.Contains(j))
          cost += not_in_scale_costs;
        cost += fabs(i-j) * costs_per_semitone;
        cost *= count[step][i];
        mat[i][j] = cost;
      }
    }


    for (i = 0; i < 12; i++)
    {
      cout << i << "\t: ";
      for (j = 0; j < 12; j++)
        cout << mat[i][j] << "\t";
      cout << endl;
    }

    // now we have something similar to a hamilton problem:
    // for each row i compute a column index v[i] so that all
    // v[i] are different and the sum of mat[i][v[i]]
    // is minimal.

    int v[12];
    for (i = 0; i < 12; i++)
      v[i] = i;
  }
}

#endif


void JZHarmonyBrowserAnalyzer::CreateChords()
{
  int* pBest = new int[mSteps];
  for (int i = 0; i < mSteps; i++)
  {
    pBest[i] = -1;
  }

  JZHarmonyBrowserContextIterator iter;
  while (iter())
  {
    const JZHarmonyBrowserContext &ct = iter.Context();
    const JZHarmonyBrowserChord chord = ct.Chord();
    const JZHarmonyBrowserChord scale = ct.Scale();
    for (int i = 0; i < mSteps; i++)
    {
      int err = 0;
      for (int k = 0; k < 12; k++)
      {
        if (!chord.Contains(k))
        {
          err += count[i][k];
        }
        if (!scale.Contains(k))
        {
          err += count[i][k];
        }
      }
      if (pBest[i] == -1 || err < pBest[i])
      {
        *seq[i] = ct;
        seq[i]->SetSeqNr(i + 1);
        pBest[i] = err;
      }
    }
  }
  delete [] pBest;
}
