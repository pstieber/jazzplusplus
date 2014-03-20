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

#include "ArrayControl.h"

//*****************************************************************************
//*****************************************************************************
class JZRhythmArrayControl : public JZArrayControl
{
  public:

    JZRhythmArrayControl(
      wxWindow* pParent,
      wxWindowID Id,
      JZRndArray& RandomArray,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxSize(40, 40),
      long WindowStyle = wxNO_BORDER);

    void SetMeter(int StepsPerCount, int CountPerBar, int BarCount);

  protected:

    virtual void DrawXTicks(wxDC& Dc);

  private:

    int mStepsPerCount;
    int mCountPerBar;
};
