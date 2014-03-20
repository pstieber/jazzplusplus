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

#include "Globals.h"

#include "NamedValue.h"
#include "Player.h"
#include "Project.h"
#include "Song.h"
#include "Synth.h"

using namespace std;

JZConfiguration* gpConfig = 0;

JZSong* gpSong = 0;

JZSynthesizer* gpSynth = 0;

JZPlayer* gpMidiPlayer = 0;

map<int, string> gLimitSteps;

vector<pair<string, int> > gModes;

// 0..11 are the C...B major keys.
const int gScaleChromatic = 12;
const int gScaleSelected  = 13;

vector<pair<string, int> > gScaleNames;

map<int, string> gQuantizationSteps;

vector<pair<string, int> > gSynthesizerTypes;

vector<pair<string, int> > gSynthesierTypeFiles;

JZProject* gpProject = 0;

JZTrackFrame* gpTrackFrame = 0;

JZTrackWindow* gpTrackWindow = 0;

JZHarmonyBrowserInterface* gpHarmonyBrowser = 0;

JZRhythmGeneratorFrame* gpRhythmGeneratorFrame = 0;

const double gDegreesToRadians = 0.01745329251994330212;
const double gRadiansToDegrees = 57.2957795130823;

//*****************************************************************************
// Decsription:
//   This function tokenizes the input string.
//*****************************************************************************
int Tokenize(
  const string& String,
  const string& Delimiters,
  vector<string>& Tokens)
{
  string::size_type Begin, End;

  // Initialize the token index.
  int TokenIndex = 0;

  // Search the beginning of the line for the first token.
  Begin = String.find_first_not_of(Delimiters);

  // While at the beginning of a word found.
  while (Begin != string::npos)
  {
    // Search for the end of the actual token.
    End = String.find_first_of(Delimiters, Begin);

    if (End == string::npos)
    {
      End = String.length();
    }

    Tokens.push_back(String.substr(Begin, End - Begin));

    ++TokenIndex;

    Begin = String.find_first_not_of(Delimiters, End);
  }

  return TokenIndex;
}
