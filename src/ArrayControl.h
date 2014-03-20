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

#include <wx/control.h>

#include <string>

class JZRndArray;

//*****************************************************************************
//*****************************************************************************
class JZArrayControl : public wxControl
{
  public:

    JZArrayControl(
      wxWindow* pParent,
      wxWindowID Id,
      JZRndArray& RandomArray,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxSize(40, 40),
      long WindowStyle = wxNO_BORDER);

    virtual ~JZArrayControl();

    void SetLabel(const std::string& Label);

    void SetXMinMax(int XMin, int XMax);

  private:

    void OnSize(wxSizeEvent& Event);

    void OnEraseBackground(wxEraseEvent& Event);

    void OnPaint(wxPaintEvent& Event);

    void OnMouseEvent(wxMouseEvent& MouseEvent);

    void OnMouseCaptureLost(wxMouseCaptureLostEvent& Event);

    void ButtonDown(wxMouseEvent& MouseEvent);

    void Dragging(wxMouseEvent& MouseEvent);

    void ButtonUp(wxMouseEvent& MouseEvent);

    int GetIndex(wxMouseEvent& MouseEvent);

    void DrawBar(wxDC& Dc, int i, bool black);

    void DrawLabel(wxDC& Dc);

  protected:

    virtual void DrawXTicks(wxDC& Dc);

  private:

    void DrawYTicks(wxDC& Dc);

    void DrawNull(wxDC& Dc);

  protected:

    JZRndArray& mRandomArray;

    long mStyleBits;

    bool mEnabled;

    std::string mLabel;

    int mX, mY, mYNull;
    int mWidth, mHeight;

    // Dragging flag.
    bool mDragging;

    // If ctrl is pushed: drag this one.
    int mIndex;

    // Array size is mapped to this range for x-tick marks.
    int mXMin, mXMax;

  DECLARE_EVENT_TABLE()
};
