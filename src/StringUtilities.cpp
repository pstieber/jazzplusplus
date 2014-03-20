//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2008-2013 Peter J. Stieber, all rights reserved.
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

#include "StringUtilities.h"

using namespace std;

//-----------------------------------------------------------------------------
// Decsription:
//   This function tokenizes the input string.
//-----------------------------------------------------------------------------
unsigned TNStringUtilities::Tokenize(
  const string& Delimiters,
  const string& InputString,
  vector<string>& Tokens)
{
  // make sure the vector of tokens is empty.
  Tokens.clear();

  string::size_type Begin, End;

  // Initialize the token index.
  unsigned TokenIndex = 0;

  // Search the beginning of the string for the first token.
  Begin = InputString.find_first_not_of(Delimiters);

  // While at the beginning of a word found.
  while (Begin != string::npos)
  {
    // Search for the end of the actual token.
    End = InputString.find_first_of(Delimiters, Begin);

    if (End == string::npos)
    {
      End = InputString.length();
    }

    Tokens.push_back(InputString.substr(Begin, End - Begin));

    ++TokenIndex;

    Begin = InputString.find_first_not_of(Delimiters, End);
  }
  return TokenIndex;
}

//-----------------------------------------------------------------------------
// Decsription:
//   This function removes leading and trailing white space the input string.
//-----------------------------------------------------------------------------
string TNStringUtilities::TrimLeadingAndTrailingBlanks(const string& String)
{
  const string WhiteSpaceCharacters = " \t\n";

  string::size_type Start = String.find_first_not_of(WhiteSpaceCharacters);
  if (Start == string::npos)
  {
    return "";
  }
  string::size_type Stop = String.find_last_not_of(WhiteSpaceCharacters);
  return string(String, Start, Stop - Start + 1);
}
