//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2013 Peter J. Stieber
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

#include "DynamicArray.h"

//*****************************************************************************
//*****************************************************************************
class JZBitset
{
  public:
    int operator()(int i)
    {
      return (mArray[index(i)] & mask(i)) != 0;
    }
    void set(int i, int b)
    {
      if (b)
      {
        mArray[index(i)] |= mask(i);
      }
      else
      {
        mArray[index(i)] &= ~mask(i);
      }
    }

  private:

    TTDynamicArray<int> mArray;

    // this works for sizeof(int) >= 4
    int index(int i)
    {
      return i >> 5;
    }
    int mask(int i)
    {
      return 1 << (i & 31);
    }
};
