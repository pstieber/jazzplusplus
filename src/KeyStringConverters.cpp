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

//#include <locale>
#include <string>
#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
void KeyToString(int Key, string& String)
{
  static string Names[] =
  {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B"
  };

  ostringstream Oss;
  Oss << Names[Key % 12] << Key / 12;
  String = Oss.str();
}

//*****************************************************************************
//*****************************************************************************
int StringToKey(const string& String)
{
  static const char NoteCharacter[] = "cCdDeEfFgGaAbB";
  static int NoteKeyValue[] =
  {
    0,
    0,
    2,
    2,
    4,
    4,
    5,
    5,
    7,
    7,
    9,
    9,
    11,
    11
  };

  int Key = 0;

  for (unsigned i = 0; i < String.length(); ++i)
  {
    if (String[i] == '#')
    {
      // A sharp adds a half step.
      Key += 1;
    }
    else if (isdigit(String[i]))
    {
      // Process the octave digits.
      int n = 0;
      for (; isdigit(String[i]) && i < String.length(); ++i)
      {
        n = 10 * n + String[i] - '0';
      }
      Key += 12 * n;
    }
    else
    {
      // Process the note character.
      int j;
      for (j = 0; NoteCharacter[j]; ++j)
      {
        if (String[i] == NoteCharacter[j])
        {
          Key += NoteKeyValue[j];
          break;
        }
      }
      if (!NoteCharacter[j])
      {
        // This is an error condition.  For now, bogus note characters are
        // ignored.
      }
    }
  }
  return Key;
}
