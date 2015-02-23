//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2015 Peter J. Stieber
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

#include <wx/window.h>

class JZFilter;
class JZMouseAction;
class JZSnapSelection;
class JZProject;
class wxDialog;

//*****************************************************************************
// Description:
//   This class is derived from a wxWidgets scrolled window, and acts as the
// common base class for JZTrackWindow and JSPianoWindow.
//*****************************************************************************
class JZEventWindow : public wxWindow
{
  public:

    JZEventWindow(
      wxFrame* pParent,
      JZProject* pProject,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxDefaultSize);

    virtual ~JZEventWindow();

    // WARNING: non-constant access.
    JZProject* GetProject() const;

    bool AreEventsSelected() const;

    bool EventsSelected(const wxString& Message) const;

    void Shift(int Units);

    void Quantize();

    void SetChannel();

    void Transpose();

    void Delete();

    void Velocity();

    void Length();

    void ConvertToModulation();

    void Cleanup();

    void SearchReplace();

    void EditMeter();

    void LineText(
      wxDC& Dc,
      int x,
      int y,
      int Width,
      const char* pString,
      int Height = -1,
      bool Down = false);

    //========================================
    // Coordinate conversion member functions.
    //========================================

    int x2Clock(int x);

    int Clock2x(int Clock);

    int x2BarClock(int x, int Next = 0);

  protected:

    int SnapClock(int Clock, bool Up);

    virtual void SnapSelectionStart(wxMouseEvent& MouseEvent) = 0;

    virtual void SnapSelectionStop(wxMouseEvent& MouseEvent) = 0;

    void DrawVerticalLine(wxDC& Dc, int XPosition) const;

    void DrawHorizontalLine(wxDC& Dc, int YPosition) const;

    virtual void GetVirtualEventSize(int& EventWidth, int& EventHeight) const;

    virtual void SetXScrollPosition(int x);

    virtual void SetYScrollPosition(int y);

    int y2yLine(int y, int Up = 0);

    void OnMouseEvent(wxMouseEvent& MouseEvent);

  protected:

    JZSnapSelection* mpSnapSel;

  public:

    JZFilter* mpFilter;

    JZMouseAction* mpMouseAction;

  protected:

    JZProject* mpProject;

    wxColor* mpGreyColor;
    wxBrush* mpGreyBrush;

    int mClockTicsPerPixel;
    int mTopInfoHeight;
    int mLeftInfoWidth;
    int mTrackHeight;
    int mLittleBit;

    int mEventsX, mEventsY, mEventsWidth, mEventsHeight;
    int mCanvasWidth, mCanvasHeight;
    int mFromClock, mToClock;
    int mFromLine, mToLine;

    int mScrolledX, mScrolledY;

  DECLARE_EVENT_TABLE()
};

//*****************************************************************************
// Description:
//   These are the event window inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
// WARNING: non-constant access.
//-----------------------------------------------------------------------------
inline
JZProject* JZEventWindow::GetProject() const
{
  return mpProject;
}
