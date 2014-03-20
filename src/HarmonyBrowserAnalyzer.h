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

class JZFilter;
class JZHarmonyBrowserChord;
class JZHarmonyBrowserContext;
class JZKeyOnEvent;
class JZTrack;

//*****************************************************************************
//*****************************************************************************
class JZHarmonyBrowserAnalyzer
{
  public:

    JZHarmonyBrowserAnalyzer(JZHarmonyBrowserContext **seq, int n_seq);

    ~JZHarmonyBrowserAnalyzer();

    int Analyze(JZFilter *f, int eighth_per_chord = 8);

    int Transpose(JZFilter *f, int eighth_per_chord = 8);

    void Init(JZFilter *f, int steps_per_bar);

    void Exit();

    int Steps() const;

    int Step2Clock(int step);

    JZHarmonyBrowserContext* GetContext(int step) const;

  private:

    void IterateEvents(
      void (JZHarmonyBrowserAnalyzer::*Action)(JZKeyOnEvent *on, JZTrack *t));
    void CountEvent(JZKeyOnEvent *on, JZTrack *t);
    void TransposeEvent(JZKeyOnEvent *on, JZTrack *t);
    void CreateChords();
    int NumCount(int i);
    int MaxCount(int i, const JZHarmonyBrowserChord &done);
    void GenerateMapping();

  private:

    JZHarmonyBrowserContext** seq;
    int max_seq;

    int start_clock, stop_clock;
    int eighths_per_chord;
    int mSteps;
    JZFilter* mpFilter;
    JZTrack* mpTrack;

    int** count;
    int** delta;
};

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZHarmonyBrowserAnalyzer::Steps() const
{
  return mSteps;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
JZHarmonyBrowserContext* JZHarmonyBrowserAnalyzer::GetContext(int step) const
{
  return seq[step % max_seq];
}
