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

#pragma once

#include <map>
#include <vector>
#include <string>

// #define DEBUG(x) x
#define DEBUG(x)

class JZConfiguration;
class JZSong;
class JZSynthesizer;
class JZPlayer;
class JZProject;
class JZRhythmGeneratorFrame;
class JZTrackFrame;
class JZTrackWindow;
class JZHarmonyBrowserInterface;

enum TESizes
{
  // This is the same as the number of keys in the piano window.
  eMaxTrackCount = 127
};

extern JZConfiguration* gpConfig;
extern JZSong* gpSong;
extern JZSynthesizer* gpSynth;
extern JZPlayer* gpMidiPlayer;
extern std::map<int, std::string> gLimitSteps;
extern std::vector<std::pair<std::string, int> > gModes;
extern const int gScaleChromatic;
extern const int gScaleSelected;
extern std::vector<std::pair<std::string, int> > gScaleNames;
extern std::map<int, std::string> gQuantizationSteps;
extern std::vector<std::pair<std::string, int> > gSynthesizerTypes;
extern std::vector<std::pair<std::string, int> > gSynthesierTypeFiles;
extern JZProject* gpProject;
extern JZTrackFrame* gpTrackFrame;
extern JZTrackWindow* gpTrackWindow;
extern JZHarmonyBrowserInterface* gpHarmonyBrowser;
extern JZRhythmGeneratorFrame* gpRhythmGeneratorFrame;
extern const double gDegreesToRadians;
extern const double gRadiansToDegrees;

//*****************************************************************************
//*****************************************************************************
int Tokenize(
  const std::string& String,
  const std::string& Delimiters,
  std::vector<std::string>& Tokens);
