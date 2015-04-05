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

#include <cassert>

//*****************************************************************************
// Description:
//   Template version of the macro code macro code listed above.
//*****************************************************************************
template <typename TAType>
class TTDynamicArray
{
  public:

    TTDynamicArray();

    TTDynamicArray(TAType InitialValue, int InitialSize = 0);

    TTDynamicArray(const TTDynamicArray& Other);

    virtual ~TTDynamicArray();

    TTDynamicArray& operator = (const TTDynamicArray& Rhs);

    TAType& operator[](int i);

    const TAType operator[](int i) const;

    int GetSize() const;

  protected:

    void Resize(int NewSize);

    // Delete all arrays.
    void Clear();

  protected:

    // Number of arrays.
    int mArrayCount;

    // Number of elements per array.
    int mBlockSize;

    TAType mInitialValue;
    TAType** mppArray;
};

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
TTDynamicArray<TAType>::TTDynamicArray()
  : mArrayCount(0),
    mBlockSize(16),
    mInitialValue(0),
    mppArray(nullptr)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
TTDynamicArray<TAType>::TTDynamicArray(TAType InitialValue, int InitialSize)
  : mArrayCount(0),
    mBlockSize(16),
    mInitialValue(InitialValue),
    mppArray(nullptr)
{
  if (InitialSize)
  {
    Resize(InitialSize);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
TTDynamicArray<TAType>::TTDynamicArray(const TTDynamicArray<TAType>& Other)
  : mArrayCount(0),
    mBlockSize(16),
    mInitialValue(Other.mInitialValue),
    mppArray(nullptr)
{
  for (int i = 0; i < Other.mArrayCount * mBlockSize; ++i)
  {
    (*this)[i] = Other[i];
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
void TTDynamicArray<TAType>::Clear()
{
  for (int i = 0; i < mArrayCount; ++i)
  {
    delete [] mppArray[i];
  }
  delete [] mppArray;
  mArrayCount = 0;
  mppArray = nullptr;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
TTDynamicArray<TAType>::~TTDynamicArray()
{
  Clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
TTDynamicArray<TAType>& TTDynamicArray<TAType>::operator = (
  const TTDynamicArray<TAType>& Rhs)
{
  if (&Rhs != this)
  {
    Clear();
    mInitialValue = Rhs.mInitialValue;
    mBlockSize = Rhs.mBlockSize;
    for (int i = 0; i < Rhs.mArrayCount * Rhs.mBlockSize; ++i)
    {
      (*this)[i] = Rhs[i];
    }
  }
  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
TAType& TTDynamicArray<TAType>::operator[](int i)
{
  assert(i >= 0);
  Resize(i);
  return mppArray[i / mBlockSize][i % mBlockSize];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
const TAType TTDynamicArray<TAType>::operator[](int i) const
{
  assert(i >= 0);
  int k = i / mBlockSize;
  if (k >= mArrayCount || mppArray[k] == 0)
  {
    return mInitialValue;
  }
  return mppArray[k][i % mBlockSize];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
void TTDynamicArray<TAType>::Resize(int NewSize)
{
  int k = NewSize / mBlockSize;
  if (k >= mArrayCount)
  {
    int i, n = k + 1;
    TAType** ppTemp = new TAType * [n];
    for (i = 0; i < mArrayCount; i++)
    {
      ppTemp[i] = mppArray[i];
    }
    for (; i < n; ++i)
    {
      ppTemp[i] = nullptr;
    }
    delete [] mppArray;
    mppArray = ppTemp;
  }

  if (mppArray[k] == nullptr)
  {
    mppArray[k] = new TAType [mBlockSize];
    for (int i = 0; i < mBlockSize; ++i)
    {
      mppArray[k][i] = mInitialValue;
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename TAType>
int TTDynamicArray<TAType>::GetSize() const
{
  return mArrayCount * mBlockSize;
}
