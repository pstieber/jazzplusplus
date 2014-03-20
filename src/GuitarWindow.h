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

#include <wx/scrolwin.h>

class JZGuitarFrame;

//*****************************************************************************
// Description:
//   This is the guitar window class declaration.
//*****************************************************************************
class JZGuitarWindow : public wxScrolledWindow
{
  public:

    JZGuitarWindow(
      JZGuitarFrame* pParent,
      const wxPoint& Position,
      const wxSize& Size);

    virtual ~JZGuitarWindow();

    virtual void OnDraw(wxDC& Dc);

    void ClearBuffer();

    virtual void SetWindowSize(int Width, int Height)
    {
      mWidth = Width;
      mHeight = Height;
      wxScrolledWindow::SetSize(0, 0, Width, Height);
      Refresh();
    }

    void ShowPitch(int Pitch);

  private:

    void DrawBoard(wxDC& Dc);

    void DrawPitch(wxDC& Dc, int Pitch, int String, bool Show);

    void DrawPitch(wxDC& Dc, int Pitch, bool Show);

    void ShowPitch(wxDC& Dc, int Pitch);

    int y2String(int y);

    int x2Grid(int x);

    int Xy2Pitch(int x, int y);

    void OnSize(wxSizeEvent& Event);

    void OnPaint(wxPaintEvent& Event);

    void OnMouseMove(wxMouseEvent& MouseEvent);

  private:

//    JZGuitarFrame* mpGuitarFrame;
//    JZPianoFrame* mpPianoWindow;

    static bool mChordMode;
    static bool mBassGuitar;
    static bool mShowOctaves;
    static int  mFretCount;

    int mStringCount;
    const int* mpPitches;
    static const int mGuitarPitches[6];
    static const int mBassPitches[4];

    int mWidth, mHeight;
    int mStringHeight, mFretWidth;        // rounded values
    int mMargin;
    int mActivePitch;     // mouse move
    int mPlayPitch;       // sound
    int put_clock;        // left up

    wxFont* mpFont;

    DECLARE_EVENT_TABLE()
};

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZGuitarWindow::y2String(int y)
{
  return (y + mStringHeight / 2) * (mStringCount + 1) / mHeight - 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZGuitarWindow::x2Grid(int x)
{
  return x * mFretCount / mWidth;
}
