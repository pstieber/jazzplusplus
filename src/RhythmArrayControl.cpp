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

#include "RhythmArrayControl.h"

#include "Random.h"

#include <wx/dc.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmArrayControl::JZRhythmArrayControl(
  wxWindow* pParent,
  wxWindowID Id,
  JZRndArray& RandomArray,
  const wxPoint& Position,
  const wxSize& Size,
  long WindowStyle)
  : JZArrayControl(pParent, Id, RandomArray, Position, Size, WindowStyle),
    mStepsPerCount(4),
    mCountPerBar(4)
{
  mStyleBits |= ARED_RHYTHM;

  SetXMinMax(1, mStepsPerCount * mCountPerBar);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmArrayControl::SetMeter(
  int StepsPerCount,
  int CountPerBar,
  int BarCount)
{
  mStepsPerCount = StepsPerCount;
  mCountPerBar = CountPerBar;
  mRandomArray.Resize(StepsPerCount * CountPerBar * BarCount);
  SetXMinMax(1, StepsPerCount * CountPerBar * BarCount);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmArrayControl::DrawXTicks(wxDC& Dc)
{
  if (!(mStyleBits & ARED_RHYTHM))
  {
    JZArrayControl::DrawXTicks(Dc);
    return;
  }

  assert(mStepsPerCount && mCountPerBar);

  Dc.SetFont(*wxSMALL_FONT);

  int TextWidth, TextHeight;
  for (int i = 0; i < mRandomArray.Size(); i += mStepsPerCount)
  {
    int Mark = (i / mStepsPerCount) % mCountPerBar + 1;
    ostringstream Oss;
    Oss << Mark;
    int YPosition = mY + mHeight;
    int XPosition = (int)(mX + (i + 0.5) * mWidth / mRandomArray.Size());
    Dc.GetTextExtent(Oss.str(), &TextWidth, &TextHeight);
    XPosition -= (int)(TextWidth / 2.0);
    Dc.DrawText(Oss.str(), XPosition, YPosition);
  }

  Dc.SetFont(*wxNORMAL_FONT);
}
