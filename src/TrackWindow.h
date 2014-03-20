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

#pragma once

#include "EventWindow.h"
#include "MouseAction.h"
#include "Rectangle.h"

class JZProject;
class JZTrack;
class JZFilter;
class wxFont;

enum TECounterModes
{
  eCmProgram,
  eCmBank,
  eCmVolume,
  eCmPan,
  eCmReverb,
  eCmChorus,
  eCmModes
};

enum TENumberModes
{
  eNmTrackNr,
  eNmMidiChannel,
  eNmModes
};

//*****************************************************************************
//*****************************************************************************
class JZTrackWindow : public JZEventWindow
{
  public:

    JZTrackWindow(
      wxFrame* pParent,
      JZProject* pProject,
      const wxPoint& Position,
      const wxSize& Size);

    virtual ~JZTrackWindow();

    void Create();

    void NewPlayPosition(int Clock);

    void MousePlay(wxMouseEvent& MouseEvent, TEMousePlayMode Mode);

    int EventsSelected(const wxString& Message);

    void ZoomIn();

    void ZoomOut();

    void SetScrollRanges();

  protected:

    virtual void SnapSelectionStart(wxMouseEvent& MouseEvent);

    virtual void SnapSelectionStop(wxMouseEvent& MouseEvent);

  private:

    void OnSize(wxSizeEvent& Event);

    void OnEraseBackground(wxEraseEvent& Event);

    void OnPaint(wxPaintEvent& Event);

    void OnLeftButtonDown(wxMouseEvent& MouseEvent);

    void OnMouseMove(wxMouseEvent& MouseEvent);

    void OnLeftButtonUp(wxMouseEvent& MouseEvent);

    void OnRightButtonUp(wxMouseEvent& MouseEvent);

    void OnScroll(wxScrollWinEvent& Event);

    void HorizontalScroll(wxScrollWinEvent& Event);

    void VerticalScroll(wxScrollWinEvent& Event);

    void IncreaseTrackNumberField(JZTrack* pTrack);

    void DecreaseTrackNumberField(JZTrack* pTrack);

    virtual void OnDraw(wxDC& Dc);

    void Draw(wxDC& Dc);

    void DrawPlayPosition(wxDC& Dc);

    void DrawNumbers(wxDC& Dc);

    void DrawSpeed(wxDC& Dc, int Value = -1, bool Down = false);

    void DrawCounters(wxDC& Dc);

    void DrawEvents(wxDC& Dc);

    void LineText(
      wxDC& Dc,
      int x,
      int y,
      int w,
      const char* pString,
      int h = -1,
      bool Down = false);

//    void Mark(int x, int y);

    void UnMark();

    const char* GetCounterString();

    const char* GetNumberString() const;

//    int x2xBar(int x);

//    int x2wBar(int x);

    int TrackIndex2y(int Track);

    int y2TrackIndex(int y);

    JZTrack* y2Track(int y);

  private:

    int mLeftInfoWidth;
    int mPlayClock;
    bool mUseColors;

    // The values indicate the starting postions and widths of the track fields
    // on the left hand side of the screen.  Note that the position of the
    // first field displayed is always 0, so it doesn't need to be recorded.
    int mNumberWidth;
    int mTrackNameX, mTrackNameWidth;
    int mStateX, mStateWidth;
    int mPatchX, mPatchWidth;

    std::vector<int> mBarX;

    TECounterModes mCounterMode;
    TENumberModes mNumberMode;

    wxFont* mpFixedFont;
    int mFixedFontHeight;

    int mFontSize;
    wxFont* mpFont;

    bool mPreviouslyRecording;
    int mPreviousClock;

    JZRectangle Marked;

    bool mDrawing;

    wxBitmap* mpFrameBuffer;

  DECLARE_EVENT_TABLE()
};
