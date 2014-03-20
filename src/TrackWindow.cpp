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

#include "TrackWindow.h"

#include "TrackFrame.h"
#include "Filter.h"
#include "Project.h"
#include "Player.h"
#include "RecordingInfo.h"
#include "Globals.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/msgdlg.h>

#include <iomanip>
//DEBUG#include <iostream>
#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZTrackWindow, JZEventWindow)

  EVT_SIZE(JZTrackWindow::OnSize)

  EVT_ERASE_BACKGROUND(JZTrackWindow::OnEraseBackground)

  EVT_PAINT(JZTrackWindow::OnPaint)

  EVT_LEFT_DOWN(JZTrackWindow::OnLeftButtonDown)

  EVT_MOTION(JZTrackWindow::OnMouseMove)

  EVT_LEFT_UP(JZTrackWindow::OnLeftButtonUp)

  EVT_RIGHT_UP(JZTrackWindow::OnRightButtonUp)

  EVT_SCROLLWIN(JZTrackWindow::OnScroll)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrackWindow::JZTrackWindow(
  wxFrame* pParent,
  JZProject* pProject,
  const wxPoint& Position,
  const wxSize& Size)
  : JZEventWindow(pParent, pProject, Position, Size),
    mPlayClock(-1),
    mUseColors(true),
    mNumberWidth(),
    mTrackNameX(),
    mTrackNameWidth(),
    mStateX(),
    mStateWidth(),
    mPatchX(0),
    mPatchWidth(0),
    mBarX(),
    mCounterMode(eCmProgram),
    mNumberMode(eNmMidiChannel),
    mpFixedFont(0),
    mFixedFontHeight(0),
    mFontSize(12),
    mpFont(0),
    mPreviouslyRecording(false),
    mPreviousClock(0),
    mDrawing(false),
    mpFrameBuffer(0)
{
  SetBackgroundColour(*wxWHITE);

  mpFrameBuffer = new wxBitmap;

  SetScrollRanges();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrackWindow::~JZTrackWindow()
{
  delete mpFixedFont;
  delete mpFont;
  delete mpFrameBuffer;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::Create()
{
  wxClientDC Dc(this);

  Dc.SetFont(wxNullFont);

  delete mpFixedFont;
  mpFixedFont = new wxFont(12, wxSWISS, wxNORMAL, wxNORMAL);
  Dc.SetFont(*mpFixedFont);

  int Width, Height;
  Dc.GetTextExtent("M", &Width, &mFixedFontHeight);

  delete mpFont;
  mpFont = new wxFont(mFontSize, wxSWISS, wxNORMAL, wxNORMAL);
  Dc.SetFont(*mpFont);

  Dc.GetTextExtent("M", &Width, &Height);
  mLittleBit = Width / 2;

  mTopInfoHeight = mFixedFontHeight + 2 * mLittleBit;

  Dc.GetTextExtent("HXWjgi", &Width, &Height);
  mTrackHeight = Height + 2 * mLittleBit;

  Dc.GetTextExtent("999", &Width, &Height);
  mNumberWidth = Width + 2 * mLittleBit;

  Dc.GetTextExtent("Normal Track Name", &Width, &Height);
  mTrackNameWidth = Width + 2 * mLittleBit;

  Dc.GetTextExtent("m", &Width, &Height);
  mStateWidth = Width + 2 * mLittleBit;

  Dc.GetTextExtent("999", &Width, &Height);
  mPatchWidth = Width + 2 * mLittleBit;

  mLeftInfoWidth =
    mNumberWidth + mTrackNameWidth + mStateWidth + mPatchWidth + 1;

//DEBUG  cout
//DEBUG    << ' ' << mNumberWidth
//DEBUG    << ' ' << mTrackNameWidth
//DEBUG    << ' ' << mStateWidth
//DEBUG    << ' ' << mPatchWidth
//DEBUG    << ' ' << mLeftInfoWidth
//DEBUG    << endl;

  UnMark();
}

//-----------------------------------------------------------------------------
// Description:
//   Update the play position to the clock argument, and trigger a redraw so
// the play bar will be drawn.
//-----------------------------------------------------------------------------
void JZTrackWindow::NewPlayPosition(int Clock)
{
  int scroll_clock = (mFromClock + 5 * mToClock) / 6;

  if (
    !mpSnapSel->IsActive() &&
    (Clock > scroll_clock || Clock < mFromClock) && Clock >= 0)
  {
    // Avoid permanent redraws when end of scroll range is reached.
    if (
      Clock > mFromClock &&
      mToClock >= mpProject->GetMaxQuarters() * mpProject->GetTicksPerQuarter())
    {
      return;
    }

    int x = Clock2x(Clock);
    SetXScrollPosition(x);
  }

  if (!mpSnapSel->IsActive())  // sets clipping
  {
    if (mPlayClock != Clock)
    {
//      int OldPlayClock = mPlayClock;
      mPlayClock = Clock;
//      wxRect InvalidateRect;
//      InvalidateRect.x = Clock2x(OldPlayClock) - 1;
//      InvalidateRect.y = 0;
//      InvalidateRect.width = 3;
//      InvalidateRect.height= 100000000;

//      Refresh(true, &InvalidateRect);

//      InvalidateRect.x = Clock2x(mPlayClock) - 1;

//      Refresh(true, &InvalidateRect);

      Refresh(false);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//void JZTrackWindow::Mark(int x, int y)
//{
//  Marked.SetX(x2xBar(x));
//  Marked.SetY(y2yLine(y));
//  Marked.SetWidth(x2wBar(x));
//  Marked.SetHeight(mTrackHeight);
//
//  wxDC* pDc = new wxClientDC(this);
//  LineText(*pDc, Marked.GetX(), Marked.GetY(), Marked.GetWidth(), ">");
//  delete pDc;
//}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::UnMark()
{
  Marked.SetX(-1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnSize(wxSizeEvent& Event)
{
  GetClientSize(&mCanvasWidth, &mCanvasHeight);
  if (mCanvasWidth > 0 && mCanvasHeight > 0)
  {
    mpFrameBuffer->Create(mCanvasWidth, mCanvasHeight);
    SetScrollRanges();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Do nothing, to avoid flickering.
//-----------------------------------------------------------------------------
void JZTrackWindow::OnEraseBackground(wxEraseEvent& Event)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnPaint(wxPaintEvent& Event)
{
  // One must always create a wxPaintDC object, even if it is not used.
  // Otherwise, under MS Windows, refreshing for this and other windows will
  // fail.
  wxPaintDC Dc(this);
  PrepareDC(Dc);

  OnDraw(Dc);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnLeftButtonDown(wxMouseEvent& MouseEvent)
{
  wxPoint Point = MouseEvent.GetPosition();

  if (Point.x < mNumberWidth && Point.y >= mTopInfoHeight)
  {
    JZRectangle Rectangle(
      0,
      y2yLine(Point.y + mScrolledY),
      Clock2x(mpProject->GetMaxQuarters() * mpProject->GetTicksPerQuarter()) +
        mScrolledX,
      mTrackHeight);
    mpSnapSel->Select(Rectangle);
    SnapSelectionStop(MouseEvent);
  }
  else if (
    Point.x >= mEventsX && Point.x < mEventsX + mEventsWidth &&
    Point.y >= mEventsY && Point.y < mEventsY + mEventsHeight)
  {
    SnapSelectionStart(MouseEvent);

    mpSnapSel->ButtonDown(MouseEvent, mScrolledX, mScrolledY);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnMouseMove(wxMouseEvent& MouseEvent)
{
  if (MouseEvent.LeftIsDown())
  {
    mpSnapSel->Dragging(MouseEvent, mScrolledX, mScrolledY);
//    SnapSelectionStop(MouseEvent);
    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnLeftButtonUp(wxMouseEvent& MouseEvent)
{
  wxPoint Point = MouseEvent.GetPosition();

  // Check to see if the mouse was clicked in the top header.
  if (Point.y < mTopInfoHeight)
  {
    if (Point.x < mNumberWidth)
    {
      // The point is inside of the number mode indicator, so toggle the first
      // column between track index and MIDI channel.
      if (mNumberMode == eNmTrackNr)
      {
        mNumberMode = eNmMidiChannel;
        Refresh(false);
      }
      else
      {
        mNumberMode = eNmTrackNr;
        Refresh(false);
      }
    }
    else if (
      Point.x >= mTrackNameX &&
      Point.x < mTrackNameX + mTrackNameWidth)
    {
      // The point is inside of the track name header.  This cell indicates
      // the song tempo.

      // Bump up the speed value one tick.
      int SpeedBpm = gpProject->GetTrack(0)->GetDefaultSpeed();
      ++SpeedBpm;
      if (SpeedBpm > 0 && SpeedBpm < 300)
      {
        gpProject->GetTrack(0)->SetDefaultSpeed(SpeedBpm);
      }
      Refresh(false);
    }
    else if (Point.x >= mPatchX && Point.x < mPatchX + mPatchWidth)
    {
      // The point is inside the patch header.

      // Toggle the patch type.
      switch (mCounterMode)
      {
        case eCmProgram:
          mCounterMode = eCmBank;
          break;
        case eCmBank:
          mCounterMode = eCmVolume;
          break;
        case eCmVolume:
          mCounterMode = eCmPan;
          break;
        case eCmPan:
          mCounterMode = eCmReverb;
          break;
        case eCmReverb:
          mCounterMode = eCmChorus;
          break;
        case eCmChorus:
        default:
          mCounterMode = eCmProgram;
          break;
      }
      Refresh(false);
    }
  }
  else
  {
    // The point is not in the top header row.

    // Get the track associated with the y position.
    JZTrack* pTrack = y2Track(Point.y);
    if (pTrack)
    {
      if (Point.x < mNumberWidth)
      {
        // The point is inside the number field.
      }
      else if (
        Point.x >= mTrackNameX &&
        Point.x < mTrackNameX + mTrackNameWidth)
      {
        // The point is inside of a track name column.  Edit the track
        // settings.
        pTrack->Edit(this);
        Refresh(false);
      }
      else if (Point.x >= mStateX && Point.x < mStateX + mStateWidth)
      {
        // The point is inside the track name field.  Toggle the track state.
        pTrack->ToggleState(1);
        Refresh(false);
      }
      else if (Point.x >= mPatchX && Point.x < mPatchX + mPatchWidth)
      {
        IncreaseTrackNumberField(pTrack);
      }
      else if (
        Point.x >= mEventsX && Point.x < mEventsX + mEventsWidth &&
        Point.y >= mEventsY && Point.y < mEventsY + mEventsHeight)
      {
        mpSnapSel->ButtonUp(MouseEvent, mScrolledX, mScrolledY);

        // The point is in event area.
        SnapSelectionStop(MouseEvent);
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnRightButtonUp(wxMouseEvent& MouseEvent)
{
  wxPoint Point = MouseEvent.GetPosition();

  if (Point.y < mTopInfoHeight)
  {
    // The point is inside the top header line.

    if (Point.x >= mTrackNameX && Point.x < mTrackNameX + mTrackNameWidth)
    {
      // The point is inside the track name field.
      int SpeedBpm = gpProject->GetTrack(0)->GetDefaultSpeed();

      // Knock down the speed value one tick.
      --SpeedBpm;

      if (SpeedBpm > 0 && SpeedBpm < 300)
      {
        gpProject->GetTrack(0)->SetDefaultSpeed(SpeedBpm);
      }
      Refresh(false);
    }
  }
  else
  {
    // The point is below the top header line.

    // Get the track associated with the y position.
    JZTrack* pTrack = y2Track(Point.y);
    if (pTrack)
    {
      if (Point.x < mNumberWidth)
      {
        // The point is inside the number field.
      }
      else if (Point.x >= mStateX && Point.x < mStateX + mStateWidth)
      {
        // The point is inside the track name field.
      }
      else if (Point.x >= mPatchX && Point.x < mPatchX + mPatchWidth)
      {
        DecreaseTrackNumberField(pTrack);
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::IncreaseTrackNumberField(JZTrack* pTrack)
{
  bool UpdateFlag = false;

  switch (mCounterMode)
  {
    case eCmProgram:
      break;
    case eCmBank:
      break;
    case eCmVolume:
      UpdateFlag = pTrack->IncreaseVolume();
      break;
    case eCmPan:
      break;
    case eCmReverb:
      break;
    case eCmChorus:
    default:
      break;
  }
  if (UpdateFlag)
  {
    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::DecreaseTrackNumberField(JZTrack* pTrack)
{
  bool UpdateFlag = false;

  switch (mCounterMode)
  {
    case eCmProgram:
      break;
    case eCmBank:
      break;
    case eCmVolume:
      UpdateFlag = pTrack->DecreaseVolume();
      break;
    case eCmPan:
      break;
    case eCmReverb:
      break;
    case eCmChorus:
    default:
      break;
  }
  if (UpdateFlag)
  {
    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::ZoomIn()
{
  if (mClockTicsPerPixel >= 2)
  {
    mClockTicsPerPixel /= 2;
    mScrolledX *= 2;

    SetScrollRanges();

    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::ZoomOut()
{
  if (mClockTicsPerPixel <= 120)
  {
    mClockTicsPerPixel *= 2;
    mScrolledX /= 2;

    SetScrollRanges();

    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnDraw(wxDC& Dc)
{
  Draw(Dc);
}

//-----------------------------------------------------------------------------
// Description:
//   Draw the static portion of the scenario in the static bitmap.
//-----------------------------------------------------------------------------
void JZTrackWindow::Draw(wxDC& Dc)
{
  if (!mpFrameBuffer->Ok() || mDrawing)
  {
    return;
  }

  mDrawing = true;

  // Create a memory device context and select the frame bitmap into it.
  wxMemoryDC LocalDc;
  LocalDc.SelectObject(*mpFrameBuffer);

  LocalDc.SetFont(*mpFont);

  // Setup the brush that is used to clear the background.
  LocalDc.SetBackground(*wxWHITE_BRUSH);

  // Clear the background using the brush that was just setup, in case the
  // following drawing calls fail.
  LocalDc.Clear();

  GetClientSize(&mCanvasWidth, &mCanvasHeight);

  mEventsX = mLeftInfoWidth;
  mEventsY = mTopInfoHeight;

  mEventsWidth = mCanvasWidth - mLeftInfoWidth;
  mEventsHeight = mCanvasHeight - mTopInfoHeight;

  mFromLine = mScrolledY / mTrackHeight;
  mToLine = 1 + (mScrolledY + mCanvasHeight - mTopInfoHeight) / mTrackHeight;

  mFromClock = mScrolledX * mClockTicsPerPixel;
  mToClock = x2Clock(mCanvasWidth);

  mTrackNameX = mNumberWidth;
  mStateX = mTrackNameX + mTrackNameWidth;
  mPatchX = mStateX  + mStateWidth;

  LocalDc.DestroyClippingRegion();

  LocalDc.SetPen(*wxBLACK_PEN);

  // Draw the vertical lines.
  DrawVerticalLine(LocalDc, 0);
  DrawVerticalLine(LocalDc, mTrackNameX);
  DrawVerticalLine(LocalDc, mStateX);
  DrawVerticalLine(LocalDc, mPatchX);

  DrawVerticalLine(LocalDc, mEventsX - 1);
  DrawHorizontalLine(LocalDc, mEventsY);
  DrawHorizontalLine(LocalDc, mEventsY - 1);

  if (mpProject)
  {
    JZBarInfo BarInfo(*mpProject);

//DEBUG    cout
//DEBUG      << "mLeftInfoWidth:                " << mLeftInfoWidth << '\n'
//DEBUG      << "mCanvasWidth - mLeftInfoWidth: " << mCanvasWidth - mLeftInfoWidth << '\n'
//DEBUG      << "BarInfo.TicksPerBar            " << BarInfo.TicksPerBar << '\n'
//DEBUG      << "From Clock:                    " << mFromClock << '\n'
//DEBUG      << "To Clock:                      " << mToClock << '\n'
//DEBUG      << "Clocks/Pixel:                  " << mClockTicsPerPixel << '\n'
//DEBUG      << "From Measure:                  " << mFromClock / BarInfo.TicksPerBar << '\n'
//DEBUG      << "To Measure:                    " << mToClock / BarInfo.TicksPerBar << '\n'
//DEBUG      << "From X:                        " << Clock2x(mFromClock) << '\n'
//DEBUG      << "To X:                          " << Clock2x(mToClock) << '\n'
//DEBUG      << endl;

    BarInfo.SetClock(mFromClock);

    mBarX.clear();
    int Intro = gpProject->GetIntroLength();
    LocalDc.SetPen(*wxGREY_PEN);
    while (1)
    {
      int x = Clock2x(BarInfo.GetClock());
      if (x > mScrolledX + mCanvasWidth)
      {
        break;
      }

      if (x >= mEventsX)
      {
        int c;
        if (mClockTicsPerPixel > 48)
        {
          c = 8;
        }
        else
        {
          c = 4;
        }
        if (((BarInfo.GetBarIndex() - Intro + 96) % c) == 0)
        {
          LocalDc.SetPen(*wxBLACK_PEN);
          ostringstream Oss;
          Oss << BarInfo.GetBarIndex() + 1 - Intro;
          LocalDc.DrawText(
            Oss.str().c_str(),
            x + mLittleBit,
            mEventsY - mTrackHeight);
          LocalDc.SetPen(*wxGREY_PEN);
          LocalDc.DrawLine(
            x,
            mEventsY + 1 - mTrackHeight,
            x,
            mEventsY + mEventsHeight);
        }
        else
        {
          LocalDc.SetPen(*wxLIGHT_GREY_PEN);
          LocalDc.DrawLine(x, mEventsY + 1, x, mEventsY + mEventsHeight);
        }

        // x-coordinate for MouseAction->Snap()
        mBarX.push_back(x);
//DEBUG        LocalDc.SetPen(*wxRED_PEN);
//DEBUG        LocalDc.DrawLine(x, 0, x, mCanvasHeight);
      }
      BarInfo.Next();
    }
    LocalDc.SetPen(*wxBLACK_PEN);
  }

  // For each track show the MIDI channel, name, state, prg.
  int TrackNumber = mFromLine;

  for (
    int y = TrackIndex2y(TrackNumber);
    y < mEventsY + mEventsHeight;
    y += mTrackHeight)
  {
    LocalDc.SetClippingRegion(
      0,
      mEventsY,
      mCanvasWidth,
      mEventsHeight);

    LocalDc.SetPen(*wxGREY_PEN);
    LocalDc.DrawLine(mEventsX + 1, y, mCanvasWidth, y);
    LocalDc.SetPen(*wxBLACK_PEN);
    LocalDc.DrawLine(0, y, mEventsX, y);

    LocalDc.DestroyClippingRegion();

    JZTrack* pTrack = gpProject->GetTrack(TrackNumber);
    if (pTrack)
    {
      LocalDc.SetClippingRegion(
        mTrackNameX,
        mEventsY,
        mTrackNameWidth + mStateWidth,
        mEventsHeight);

      // Draw the track name.
      if (pTrack->IsEditing())
      {
        // Show the button pressed when the dialog box is open.
        LineText(
          LocalDc,
          mTrackNameX,
          y,
          mTrackNameWidth,
          pTrack->GetName(),
          -1,
          true);
      }
      else
      {
        LineText(
          LocalDc,
          mTrackNameX,
          y,
          mTrackNameWidth,
          pTrack->GetName(),
          -1,
          false);
      }

      // Draw the track status.
      LineText(LocalDc, mStateX, y, mStateWidth, pTrack->GetStateChar());

      LocalDc.DestroyClippingRegion();
    }
    else
    {
      LineText(LocalDc, mTrackNameX, y, mTrackNameWidth, "", -1, false);
      LineText(LocalDc, mStateX, y, mStateWidth, "");
    }

    ++TrackNumber;
  }

  DrawNumbers(LocalDc);
  DrawSpeed(LocalDc);
  DrawCounters(LocalDc);

  LineText(LocalDc, mStateX, -1, mStateWidth, "", mTopInfoHeight);

  DrawEvents(LocalDc);

  if (Marked.x > 0)
  {
    LineText(LocalDc, Marked.x, Marked.y, Marked.width, ">");
  }

  LocalDc.DestroyClippingRegion();

  DrawPlayPosition(LocalDc);

  // Draw the selection box.
  mpSnapSel->Draw(
    LocalDc,
    mScrolledX,
    mScrolledY,
    mEventsX,
    mEventsY,
    mEventsWidth,
    mEventsHeight);

  Dc.Blit(
    0,
    0,
    mCanvasWidth,
    mCanvasHeight,
    &LocalDc,
    0,
    0,
    wxCOPY);

  LocalDc.SetFont(wxNullFont);
  LocalDc.SelectObject(wxNullBitmap);

  mDrawing = false;
}

//-----------------------------------------------------------------------------
// Description:
//   This function draws the "numbers" column (leftmost one), which either
// represents track numbers or midi channel depending on mode.
//-----------------------------------------------------------------------------
void JZTrackWindow::DrawNumbers(wxDC& Dc)
{
  const char* pString = GetNumberString();
  LineText(Dc, 0, -1, mNumberWidth, pString, mTopInfoHeight);

  Dc.SetClippingRegion(0, mEventsY, mNumberWidth, mEventsHeight);
  for (int i = mFromLine; i < mToLine; ++i)
  {
    JZTrack* pTrack = gpProject->GetTrack(i);
    if (pTrack != 0)
    {
      if (pTrack->GetAudioMode())
      {
        LineText(Dc, 0, TrackIndex2y(i), mNumberWidth, "Au");
      }
      else
      {
        int Value;
        switch (mNumberMode)
        {
          case eNmTrackNr:
            Value = i + 1;
            break;
          case eNmMidiChannel:
            Value = pTrack->mChannel;
            break;
          default:
            Value = 0;
            break;
        }
        ostringstream Oss;
        Oss << setw(2) << Value;
        LineText(Dc, 0, TrackIndex2y(i), mNumberWidth, Oss.str().c_str());
      }
    }
    else
    {
      LineText(Dc, 0, TrackIndex2y(i), mNumberWidth, "");
    }
  }
  Dc.DestroyClippingRegion();
}

//-----------------------------------------------------------------------------
// Description:
//   This function draws the "speed" tempo indicator in the top left part of
// the canvas.
//-----------------------------------------------------------------------------
void JZTrackWindow::DrawSpeed(wxDC& Dc, int Value, bool Down)
{
  if (Value < 0)
  {
    Value = gpProject->GetTrack(0)->GetDefaultSpeed();
  }

  ostringstream Oss;
  Oss << "speed: " << setw(3) << Value;

  LineText(
    Dc,
    mTrackNameX,
    -1,
    mTrackNameWidth,
    Oss.str().c_str(),
    mTopInfoHeight,
    Down);
}

//-----------------------------------------------------------------------------
// Description:
//   Draw the "play position", by placing a vertical line where the
// "play clock" is.
//-----------------------------------------------------------------------------
void JZTrackWindow::DrawPlayPosition(wxDC& Dc)
{
  if (
    !mpSnapSel->IsActive() &&
    mPlayClock >= mFromClock &&
    mPlayClock < mToClock)
  {
    Dc.SetBrush(*wxBLACK_BRUSH);
    Dc.SetPen(*wxBLACK_PEN);

    int x = Clock2x(mPlayClock);

    // Draw a line, 2 pixels wide.
    Dc.DrawLine(x,     0, x,     mEventsY + mEventsHeight);
    Dc.DrawLine(x + 1, 0, x + 1, mEventsY + mEventsHeight);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::LineText(
  wxDC& Dc,
  int x,
  int y,
  int Width,
  const char* pString,
  int Height,
  bool Down)
{
  if (Height <= 0)
  {
    Height = mTrackHeight;
  }
  if (Width && Height)
  {
    Dc.SetBrush(*mpGreyBrush);
    Dc.SetPen(*wxGREY_PEN);
#ifdef __WXMSW__
    Dc.DrawRectangle(x, y, Width + 1, Height + 1);
#else
    Dc.DrawRectangle(x, y, Width, Height);
#endif
    x += 1;
    y += 1;
    Width -= 2;
    Height -= 2;
    if (Down)
    {
      Dc.SetPen(*wxBLACK_PEN);
      Dc.DrawLine(x, y, x + Width, y);
      Dc.DrawLine(x, y, x,         y + Height);
      Dc.SetPen(*wxWHITE_PEN);
      Dc.DrawLine(x + Width, y,          x + Width, y + Height);
      Dc.DrawLine(x,         y + Height, x + Width, y + Height);
    }
    else
    {
      Dc.SetPen(*wxWHITE_PEN);
      Dc.DrawLine(x, y, x + Width, y);
      Dc.DrawLine(x, y, x,         y + Height);
      Dc.SetPen(*wxBLACK_PEN);
      Dc.DrawLine(x + Width, y,          x + Width, y + Height);
      Dc.DrawLine(x,         y + Height, x + Width, y + Height);
    }
    Dc.SetPen(*wxBLACK_PEN);
    x -= 2;
    y -= 2;
  }

  if (pString && strlen(pString) > 0)
  {
    wxColor TextBackgroundColor = Dc.GetTextBackground();
    Dc.SetTextBackground(*mpGreyColor);
    int TextWidth, TextHeight;
    Dc.GetTextExtent(pString, &TextWidth, &TextHeight);
    int Margin = (Width - TextWidth) / 2;
    if (Margin < mLittleBit)
    {
      Margin = mLittleBit;
    }
    Dc.DrawText(pString, x + Margin, y + mLittleBit);
    Dc.SetTextBackground(TextBackgroundColor);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::DrawCounters(wxDC& Dc)
{
  int i;
  const char* pString = GetCounterString();
  LineText(Dc, mPatchX, -1, mPatchWidth, pString, mTopInfoHeight);

  Dc.SetClippingRegion(mPatchX, mEventsY, mPatchWidth, mEventsHeight);
  for (i = mFromLine; i < mToLine; i++)
  {
    JZTrack* pTrack = gpProject->GetTrack(i);
    if (pTrack)
    {
      int Value;
      switch (mCounterMode)
      {
        case eCmProgram:
          Value = pTrack->GetPatch();
          break;
        case eCmBank:
          Value = pTrack->GetBank();
          break;
        case eCmVolume:
          Value = pTrack->GetVolume();
          break;
        case eCmPan:
          Value = pTrack->GetPan();
          break;
        case eCmReverb:
          Value = pTrack->GetReverb();
          break;
        case eCmChorus:
          Value = pTrack->GetChorus();
          break;
        default:
          Value = 0;
          break;
      }
      ostringstream Oss;
      Oss << setw(3) << Value;
      LineText(Dc, mPatchX, TrackIndex2y(i), mPatchWidth, Oss.str().c_str());
    }
    else
    {
      LineText(Dc, mPatchX, TrackIndex2y(i), mPatchWidth, "?");
    }
  }
  Dc.DestroyClippingRegion();
}

//-----------------------------------------------------------------------------
// Description:
//   Draw the MIDI events.
//-----------------------------------------------------------------------------
void JZTrackWindow::DrawEvents(wxDC& Dc)
{
  if (!mpProject)
  {
    return;
  }

  JZBarInfo BarInfo(*mpProject);

  Dc.SetClippingRegion(mEventsX, mEventsY, mEventsWidth, mEventsHeight);

  int TrackNumber = mFromLine;
  for (
    int y = TrackIndex2y(TrackNumber);
    y < mEventsY + mEventsHeight;
    y += mTrackHeight)
  {
    JZTrack *Track = gpProject->GetTrack(TrackNumber);
    if (Track)
    {
      JZEventIterator Iterator(Track);
      int StopClk = x2Clock(mCanvasWidth);
      JZEvent* pEvent = Iterator.Range(mFromClock, StopClk);
      int y0 = y + mLittleBit;
      int y1 = y + mTrackHeight - mLittleBit;

      if (mUseColors)
      {
#if 0
        while (pEvent)       // slow!
        {
          float x = Clock2x(pEvent->GetClock());
          Dc.SetPen(pEvent->GetPen());
          Dc.DrawLine(x, y0, x, y1);
          pEvent = Iterator.Next();
        }
#else
        int xdone = -1;
        int h = y1 - y0;
        while (pEvent)       // very slow!
        {
          int x1 = Clock2x(pEvent->GetClock() + pEvent->GetLength());
          if (x1 > xdone)
          {
            int x0 = Clock2x(pEvent->GetClock());
            if (x0 < xdone)
            {
              x0 = xdone;
            }
            int w = x1 - x0;
            if (w < 2)
            {
              w = 2;
            }
            xdone = x0 + w;
            Dc.SetPen(*pEvent->GetPen());
            Dc.SetBrush(*pEvent->GetBrush());
            Dc.DrawRectangle(x0, y0, w, h);
          }
          pEvent = Iterator.Next();
        }
#endif
        Dc.SetPen(*wxBLACK_PEN);
      }
      else
      {
        float xblack = -1.0;
        while (pEvent)
        {
          int x = Clock2x(pEvent->GetClock());

          // Avoid painting events ON the bar
          if ( !(pEvent->GetClock() % BarInfo.GetTicksPerBar()) ) x = x + 1;

          if (x > xblack)
          {
            Dc.DrawLine(x, y0, x, y1);
#ifndef SLOW_MACHINE
            xblack = x;
#else
            xblack = x + 4;
#endif
          }
          pEvent = Iterator.Next();
        }
      }
    }
    ++TrackNumber;
  }

  Dc.DestroyClippingRegion();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char* JZTrackWindow::GetCounterString()
{
  const char* pString;
  switch (mCounterMode)
  {
    case eCmProgram:
      pString = "Prg";
      break;
    case eCmBank:
      pString = "Bnk";
      break;
    case eCmVolume:
      pString = "Vol";
      break;
    case eCmPan:
      pString = "Pan";
      break;
    case eCmReverb:
      pString = "Rev";
      break;
    case eCmChorus:
      pString = "Cho";
      break;
    default:
      pString = "???";
      break;
  }
  return pString;
}

//-----------------------------------------------------------------------------
//   Returns a string indicating the current use of the "numbers" column,
// which is the leftmost one.
// T means track number, M means midi channel
//-----------------------------------------------------------------------------
const char* JZTrackWindow::GetNumberString() const
{
  const char* pString;
  switch (mNumberMode)
  {
    case eNmTrackNr:
      pString = "T";
      break;
    case eNmMidiChannel:
      pString = "M";
      break;
    default:
      pString = "?";
      break;
  }
  return pString;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//int JZTrackWindow::x2xBar(int x)
//{
//  for (int i = 1; i < mBarX.size(); ++i)
//  {
//    if (x < mBarX[i])
//    {
//      return mBarX[i - 1];
//    }
//  }
//  return -1;
//}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//int JZTrackWindow::x2wBar(int x)
//{
//  for (int i = 1; i < mBarX.size(); ++i)
//  {
//    if (x < mBarX[i])
//    {
//      return mBarX[i] - mBarX[i - 1];
//    }
//  }
//  return 0;
//}

//-----------------------------------------------------------------------------
// Description:
//   Convert a track index into a y-pixel location in the visible window.
//-----------------------------------------------------------------------------
int JZTrackWindow::TrackIndex2y(int TrackIndex)
{
  return TrackIndex * mTrackHeight + mTopInfoHeight - mScrolledY;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZTrackWindow::y2TrackIndex(int y)
{
  return (y + mScrolledY - mTopInfoHeight) / mTrackHeight;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrack* JZTrackWindow::y2Track(int y)
{
  return mpProject->GetTrack(y2TrackIndex(y));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZTrackWindow::EventsSelected(const wxString& Message)
{
  if (!mpSnapSel->IsSelected())
  {
    wxMessageBox(Message, "Error", wxOK);
    return 0;
  }
  return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::SetScrollRanges()
{
  int EventWidth, EventHeight;
  GetVirtualEventSize(EventWidth, EventHeight);

  // Must add the thumb size to the passed range to reach the maximum
  // desired value.
  int ThumbSize;

  ThumbSize = EventWidth / 10;
  SetScrollbar(wxHORIZONTAL, mScrolledX, ThumbSize, EventWidth + ThumbSize);

  ThumbSize = EventHeight / 10;
  SetScrollbar(wxVERTICAL, mScrolledY, ThumbSize, EventHeight + ThumbSize);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::MousePlay(wxMouseEvent& MouseEvent, TEMousePlayMode Mode)
{
  if (Mode == eMouse && !MouseEvent.ButtonDown())
  {
    return;
  }

  // This is a little hack to keep it working for now.
  // All this stuff needs to be moved.
  JZRecordingInfo* pRecInfo = gpProject->GetRecInfo();

  if (!gpProject->IsPlaying())
  {
    switch (Mode)
    {
      case eMouse:
        int x, y;
        MouseEvent.GetPosition(&x, &y);
        gpProject->SetPlayPosition(x2BarClock(x));
        gpProject->Mute((MouseEvent.RightDown() != 0));
        if (
          mpSnapSel->IsSelected() &&
          (MouseEvent.ShiftDown() || MouseEvent.MiddleDown()))
        {
          gpProject->SetLoop(true);
        }
        else
        {
          gpProject->SetLoop(false);
          mPreviouslyRecording = mpSnapSel->IsSelected();
        }
        break;

      case eSpaceBar:
        break;

      case ePlayButton:
        gpProject->SetLoop(false);
        gpProject->SetRecord(false);
        break;

      case ePlayLoopButton:
        if (!EventsSelected("Please select loop range first."))
        {
          return;
        }
        gpProject->SetLoop(true);
        gpProject->SetRecord(false);
        break;

      case eRecordButton:
        if (!EventsSelected("Please select record track/bar first."))
        {
          return;
        }
        JZBarInfo BarInfo(*gpProject);

        BarInfo.SetClock(mpFilter->GetFromClock());

        if (BarInfo.GetBarIndex() > 0)
        {
          BarInfo.SetBar(BarInfo.GetBarIndex() - 1);
        }
        gpProject->SetPlayPosition(BarInfo.GetClock());
        gpProject->SetRecord(true);
        gpProject->SetLoop(false);
        break;
    }

    // todo: Figure out if we should have getters for these instead
    // and make them private jppProject members
    bool loop   = gpProject->mLoop;
    bool muted  = gpProject->mMuted;
    bool Record = gpProject->mRecord;

    // Is it possible to record?
    if (Record && mpSnapSel->IsSelected())
    {
      pRecInfo->mTrackIndex = mpFilter->GetFromTrack();

      pRecInfo->mpTrack = gpProject->GetTrack(pRecInfo->mTrackIndex);

      pRecInfo->mFromClock = mpFilter->GetFromClock();
      pRecInfo->mToClock   = mpFilter->GetToClock();

      if (muted)
      {
        pRecInfo->mIsMuted = true;
        pRecInfo->mpTrack->SetState(tsMute);
#ifdef OBSOLETE
        LineText(
          *pDc,
          mStateX,
          TrackIndex2y(pRecInfo.mTrackIndex),
          mStateWidth,
          pRecInfo.Track->GetStateChar());
#endif
      }
      else
      {
        pRecInfo->mIsMuted = false;
      }
    }
    else
    {
      pRecInfo->mpTrack = 0;
    }

    // Is it possible to loop?
    int loop_clock = 0;
    if (loop && mpSnapSel->IsSelected())
    {
      mPreviousClock = mpFilter->GetFromClock();
      loop_clock = mpFilter->GetToClock();
    }

    // GO!

    if (pRecInfo->mpTrack)  // recording?
    {
      gpMidiPlayer->SetRecordInfo(pRecInfo);
    }
    else
    {
      gpMidiPlayer->SetRecordInfo(0);
    }

    gpProject->mStartTime = mPreviousClock;
    gpProject->mStopTime = loop_clock;
    gpProject->Play();

  }
  else
  {
    gpProject->Stop();
    if (pRecInfo->mpTrack)
    {
      if (pRecInfo->mIsMuted)
      {
//        wxDC* pDc = new wxClientDC(mpTrackWindow);

        pRecInfo->mpTrack->SetState(tsPlay);
//        LineText(
//          *pDc,
//          mStateX,
//          TrackIndex2y(pRecInfo->mTrackIndex),
//          mStateWidth,
//          pRecInfo->mpTrack->GetStateChar());

//        delete pDc;
      }
      if (
        !pRecInfo->mpTrack->GetAudioMode() &&
        !gpProject->GetPlayer()->IsRecordBufferEmpty())
      {
        //int choice = wxMessageBox("Keep recorded events?", "You played", wxOK | wxCANCEL);
        //if (choice == wxOK)
        {
          wxBeginBusyCursor();

          gpProject->NewUndoBuffer();

          pRecInfo->mpTrack->MergeRange(
            gpProject->GetPlayer()->GetRecordBuffer(),
            pRecInfo->mFromClock,
            pRecInfo->mToClock,
            pRecInfo->mIsMuted);

          wxEndBusyCursor();

          Refresh(false);
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::SnapSelectionStart(wxMouseEvent& MouseEvent)
{
  mpSnapSel->SetXSnap(mBarX, mScrolledX);
  mpSnapSel->SetYSnap(
    TrackIndex2y(mFromLine),
    mEventsY + mEventsHeight + mScrolledY,
    mTrackHeight);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::SnapSelectionStop(wxMouseEvent& MouseEvent)
{
  if (mpSnapSel->IsSelected())
  {
    mpFilter->SetFromTrack(y2TrackIndex(mpSnapSel->GetRectangle().y));
    mpFilter->SetToTrack(y2TrackIndex(
      mpSnapSel->GetRectangle().y +
      mpSnapSel->GetRectangle().GetHeight() - 1));
    mpFilter->SetFromClock(x2BarClock(mpSnapSel->GetRectangle().x + 1));
    mpFilter->SetToClock(x2BarClock(
      mpSnapSel->GetRectangle().x + mpSnapSel->GetRectangle().GetWidth() + 1));
//    NextWin->NewPosition(mpFilter->GetFromTrack(), mpFilter->GetFromClock());
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::OnScroll(wxScrollWinEvent& Event)
{
  if (Event.GetOrientation() == wxHORIZONTAL)
  {
    HorizontalScroll(Event);
  }
  else if (Event.GetOrientation() == wxVERTICAL)
  {
    VerticalScroll(Event);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::HorizontalScroll(wxScrollWinEvent& Event)
{
  int EventWidth, EventHeight;
  GetVirtualEventSize(EventWidth, EventHeight);

  int NewScrolledX = mScrolledX;

  if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
  {
    --NewScrolledX;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
  {
    ++NewScrolledX;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP)
  {
    NewScrolledX -= 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN)
  {
    NewScrolledX += 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_TOP)
  {
    NewScrolledX = 0;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_BOTTOM)
  {
    NewScrolledX = EventWidth - 1;
  }
  else if (
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK ||
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE)
  {
    NewScrolledX = Event.GetPosition();
  }

  if (NewScrolledX < 0)
  {
    NewScrolledX = 0;
  }
  if (NewScrolledX > EventWidth - 1)
  {
    NewScrolledX = EventWidth - 1;
  }

  if (NewScrolledX != mScrolledX)
  {
    mScrolledX = NewScrolledX;
    SetScrollPos(wxHORIZONTAL, mScrolledX, true);
    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackWindow::VerticalScroll(wxScrollWinEvent& Event)
{
  int EventWidth, EventHeight;
  GetVirtualEventSize(EventWidth, EventHeight);

  int NewScrolledY = mScrolledY;

  if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
  {
    --NewScrolledY;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
  {
    ++NewScrolledY;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP)
  {
    NewScrolledY -= 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN)
  {
    NewScrolledY += 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_TOP)
  {
    NewScrolledY = 0;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_BOTTOM)
  {
    NewScrolledY = EventHeight - 1;
  }
  else if (
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK ||
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE)
  {
    NewScrolledY = Event.GetPosition();
  }

  if (NewScrolledY < 0)
  {
    NewScrolledY = 0;
  }
  if (NewScrolledY > EventHeight - 1)
  {
    NewScrolledY = EventHeight - 1;
  }

  if (NewScrolledY != mScrolledY)
  {
    mScrolledY = NewScrolledY;
    SetScrollPos(wxVERTICAL, mScrolledY, true);
    Refresh(false);
  }
}
