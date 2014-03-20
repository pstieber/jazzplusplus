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

#include "FrequencyTable.h"

#include <cmath>
#include <algorithm>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZFrequencyTable::JZFrequencyTable()
{
  double Factor = pow(2.0, 1.0 / 12.0);
  double Frequency = 440 * pow(Factor, 3.0) / 32.0;
  mFrequencyTable.reserve(128);
  for (int i = 0; i < 128; ++i)
  {
    mFrequencyTable.push_back(Frequency);
    Frequency *= Factor;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
size_t JZFrequencyTable::GetKey(double Frequency)
{
  // Do a binary search.
  vector<double>::iterator Position =
    lower_bound(mFrequencyTable.begin(), mFrequencyTable.end(), Frequency);

  // TODO: If Frequency is only a very little bigger than *Position, Frequency
  // is nearer to Position - 1.
  return Position - mFrequencyTable.begin();
}
