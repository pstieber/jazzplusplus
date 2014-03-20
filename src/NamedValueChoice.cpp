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

#include "NamedValueChoice.h"

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZNamedValueChoice::JZNamedValueChoice(
  wxWindow* pParent,
  const map<int, string>& Map)
  : wxChoice(pParent, wxID_ANY),
    mMap(Map)
{
  for (
    map<int, string>::const_iterator iMap = mMap.begin();
    iMap != mMap.end();
    ++iMap)
  {
    Append(iMap->second);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZNamedValueChoice::GetValue()
{
  int Selection = GetSelection();
  if (Selection >= 0)
  {
    int i = 0;
    for (
      map<int, string>::const_iterator iMap = mMap.begin();
      iMap != mMap.end();
      ++iMap, ++i)
    {
      if (i == Selection)
      {
        return iMap->first;
      }
    }
  }
  return 16;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZNamedValueChoice::SetValue(int Measure)
{
  int i = 0;
  for (
    map<int, string>::const_iterator iMap = mMap.begin();
    iMap != mMap.end();
    ++iMap, ++i)
  {
    if (iMap->first == Measure)
    {
      SetSelection(i);
      break;
    }
  }
}
