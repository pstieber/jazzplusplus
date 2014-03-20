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

#include "GuitarWindow.h"

#include "GuitarFrame.h"

#include <wx/dcclient.h>

#include <string>

using namespace std;

//*****************************************************************************
// Description:
//   This is the guitar window class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZGuitarWindow::mChordMode = false;

//-----------------------------------------------------------------------------
// Start with a regular 6-string guitar: not a bass.
//-----------------------------------------------------------------------------
bool JZGuitarWindow::mBassGuitar = false;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZGuitarWindow::mShowOctaves = true;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZGuitarWindow::mFretCount = 17;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const int JZGuitarWindow::mBassPitches[4] =
{
  28 + 15,
  28 + 10,
  28 +  5,
  28
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const int JZGuitarWindow::mGuitarPitches[6] =
{
  40 + 24,
  40 + 19,
  40 + 15,
  40 + 10,
  40 +  5,
  40
};

//-----------------------------------------------------------------------------
// Description:
//   This is the event table for the guitar window.  The event tables connect
// the wxWidgets events with the functions (event handlers) which process
// them.
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZGuitarWindow, wxScrolledWindow)

  EVT_SIZE(JZGuitarWindow::OnSize)

  EVT_PAINT(JZGuitarWindow::OnPaint)

  EVT_MOTION(JZGuitarWindow::OnMouseMove)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZGuitarWindow::JZGuitarWindow(
  JZGuitarFrame* pParent,
  const wxPoint& Position,
  const wxSize& Size)
  : wxScrolledWindow(
      pParent,
      wxID_ANY,
      Position,
      Size,
      wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE),
//    mpGuitarFrame(pParent),
//    mpPianoWindow(pPianoWindow)
    mMargin(2),
    mActivePitch(0),
    mPlayPitch(0),
    mpFont(0)
{
  mWidth = Size.GetWidth();
  mHeight = Size.GetHeight();
  put_clock = 0;

  if (mBassGuitar)
  {
    mpPitches = mBassPitches;
    mStringCount = 4;
  }
  else
  {
    mpPitches = mGuitarPitches;
    mStringCount = 6;
  }
  mStringHeight = mHeight / (mStringCount + 1);
  mFretWidth = mWidth / mFretCount;

  mpFont = new wxFont(12, wxSWISS, wxNORMAL, wxNORMAL);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZGuitarWindow::~JZGuitarWindow()
{
  delete mpFont;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::ClearBuffer()
{
//  piano->PasteBuffer.Clear();
//  put_clock = 0;
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::OnSize(wxSizeEvent& Event)
{
  int Width, Height;
  GetClientSize(&Width, &Height);
  SetWindowSize(Width, Height);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::ShowPitch(int Pitch)
{
  wxClientDC Dc(this);
  PrepareDC(Dc);

  Dc.SetFont(*mpFont);
  Dc.SetTextBackground(GetForegroundColour());
  ShowPitch(Dc, Pitch);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::OnPaint(wxPaintEvent& Event)
{
  wxPaintDC Dc(this);
  PrepareDC(Dc);

  OnDraw(Dc);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::OnDraw(wxDC& Dc)
{
  Dc.SetFont(*mpFont);

  GetSize(&mWidth, &mHeight);

  DrawBoard(Dc);
  DrawPitch(Dc, mActivePitch, true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::DrawBoard(wxDC& Dc)
{
  int i;

  mStringHeight = mHeight / (mStringCount + 1);
  mFretWidth = mWidth / mFretCount;

  // Paint the guitar strings.
  for (i = 0; i < mStringCount; ++i)
  {
    int y = mHeight * (i + 1) / (mStringCount + 1);
    Dc.DrawLine(0, y, mWidth, y);
  }

  // Paint the guitar frets.
  int y1 = mHeight / (mStringCount + 1);
  int y2 = mHeight * (mStringCount) / (mStringCount + 1);
  int d1 = mHeight / 5;
  for (i = 0; i < mFretCount; i++)
  {
    int x = mWidth * i / mFretCount;
    Dc.DrawLine(x, y1, x, y2);
    switch (i)
    {
      case 0:
      case 11:
      case 23:
        Dc.DrawLine(x + 2, y1, x + 2, y2);
        break;

      case 4:
      case 6:
      case 16:
      case 18:
        Dc.DrawLine(x + 2, y1 + d1, x + 2, y2 - d1);
        break;

      default:
      case 2:
      case 8:
      case 14:
      case 20:
        break;

    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::DrawPitch(wxDC& Dc, int Pitch, int String, bool Show)
{
  static const string KeyNames[12] =
  {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B"
  };

  int x =
    (Pitch - mpPitches[String] - 1) * mWidth / mFretCount + mFretWidth / 3;
  if (x < 0 || x > mWidth)
  {
    return;
  }

  int TextWidth, TextHeight;
  Dc.GetTextExtent(KeyNames[Pitch % 12].c_str(), &TextWidth, &TextHeight);

  int y = mHeight * (String + 1) / (mStringCount + 1) - int(TextHeight / 2);

  wxPen OldPen = Dc.GetPen();
  Dc.SetPen(*wxTRANSPARENT_PEN);
  wxBrush OldBrush = Dc.GetBrush();
  wxBrush Brush(GetBackgroundColour());
  Dc.SetBrush(Brush);
  Dc.DrawRectangle(
    x - mMargin,
    y - mMargin,
    TextWidth + 2 * mMargin,
    TextHeight + 2 * mMargin);
  Dc.SetPen(*wxBLACK_PEN);
  Dc.SetBrush(OldBrush);

  if (Show)
  {
    // Draw the note name.
    Dc.DrawText(KeyNames[Pitch % 12].c_str(), x, y);
  }
  else
  {
    // Draw the guitar string.
    int YPosition = y + TextHeight / 2;
    Dc.DrawLine(
      x - mMargin,
      YPosition,
      x + TextWidth + 2 * mMargin,
      YPosition);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::ShowPitch(wxDC& Dc, int Pitch)
{
  if (Pitch != mActivePitch)
  {
    DrawPitch(Dc, mActivePitch, false);
    mActivePitch = Pitch;
    DrawPitch(Dc, mActivePitch, true);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::DrawPitch(wxDC& Dc, int Pitch, bool Show)
{
  if (Pitch <= 0)
  {
    return;
  }

  if (mShowOctaves)
  {
    for (int i = 0; i < mStringCount; ++i)
    {
      for (int p = Pitch % 12; p < 127; p += 12)
      {
        DrawPitch(Dc, p, i, Show);
      }
    }
  }
  else
  {
    for (int i = 0; i < mStringCount; ++i)
    {
      DrawPitch(Dc, Pitch, i, Show);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarWindow::OnMouseMove(wxMouseEvent& MouseEvent)
{
  wxClientDC Dc(this);
  PrepareDC(Dc);

  wxPoint Position = MouseEvent.GetPosition();
  int x = Dc.DeviceToLogicalX(Position.x);
  int y = Dc.DeviceToLogicalY(Position.y);

  int Pitch = Xy2Pitch(x, y);

  Dc.SetFont(*mpFont);
  Dc.SetTextBackground(GetForegroundColour());
  ShowPitch(Dc, Pitch);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZGuitarWindow::Xy2Pitch(int x, int y)
{
  int string = y2String(y);
  if (string >= 0 && string < mStringCount)
  {
    int Fret = x2Grid(x);
    if (Fret >= 0 && Fret < mFretCount)
    {
      return mpPitches[string] + Fret + 1;
    }
  }
  return 0;
}

