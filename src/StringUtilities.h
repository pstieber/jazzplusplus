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

#ifndef TRC_STRINGUTILITIES_H
#define TRC_STRINGUTILITIES_H

#include <string>
#include <vector>

namespace TNStringUtilities
{

//-----------------------------------------------------------------------------
// Decsription:
//   This function tokenizes the input string.
//-----------------------------------------------------------------------------
unsigned Tokenize(
  const std::string& Delimiters,
  const std::string& InputString,
  std::vector<std::string>& Tokens);

//-----------------------------------------------------------------------------
// Decsription:
//   This function removes leading and trailing white space the input string.
//-----------------------------------------------------------------------------
std::string TrimLeadingAndTrailingBlanks(const std::string& String);

};

#endif // !defined(TRC_STRINGUTILITIES_H)
