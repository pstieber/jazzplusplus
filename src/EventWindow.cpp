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

#include "EventWindow.h"

#include "Command.h"
#include "Dialogs/CleanupDialog.h"
#include "Dialogs/DeleteDialog.h"
#include "Dialogs/LengthDialog.h"
#include "Dialogs/MeterChangeDialog.h"
#include "Dialogs/MidiChannelDialog.h"
#include "Dialogs/QuantizeDialog.h"
#include "Dialogs/SearchAndReplaceDialog.h"
#include "Dialogs/ShiftDialog.h"
#include "Dialogs/TransposeDialog.h"
#include "Dialogs/VelocityDialog.h"
#include "Dialogs.h"
#include "EventFrame.h"
#include "Filter.h"
#include "Help.h"
#include "MouseAction.h"
#include "Project.h"
#include "ProjectManager.h"

#include <wx/dc.h>
#include <wx/msgdlg.h>

using namespace std;

//*****************************************************************************
// Description:
//   This is the event window class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZEventWindow, wxWindow)
  EVT_MOUSE_EVENTS(JZEventWindow::OnMouseEvent)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZEventWindow::JZEventWindow(
  wxFrame* pParent,
  JZProject* pProject,
  const wxPoint& Position,
  const wxSize& Size)
  : wxWindow(
      pParent,
      wxID_ANY,
      Position,
      Size,
      wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE),
    mpSnapSel(nullptr),
    mpFilter(nullptr),
    mpMouseAction(nullptr),
    mpProject(pProject),
    mpGreyColor(nullptr),
    mpGreyBrush(nullptr),
    mClockTicsPerPixel(36),
    mTopInfoHeight(40),
    mLeftInfoWidth(100),
    mTrackHeight(10),
    mLittleBit(2),
    mEventsX(0),
    mEventsY(mTopInfoHeight),
    mEventsWidth(),
    mEventsHeight(),
    mCanvasWidth(0),
    mCanvasHeight(0),
    mFromClock(0),
    mToClock(0),
    mFromLine(0),
    mToLine(0),
    mScrolledX(0),
    mScrolledY(0)
{
  mpSnapSel = new JZSnapSelection(this);

  mpFilter = new JZFilter(mpProject);

#ifdef __WXMSW__
  mpGreyColor = new wxColor(192, 192, 192);
#else
  mpGreyColor = new wxColor(220, 220, 220);
#endif

  mpGreyBrush = new wxBrush(*mpGreyColor, wxSOLID);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZEventWindow::~JZEventWindow()
{
  delete mpSnapSel;
  delete mpFilter;
  delete mpGreyColor;
  delete mpGreyBrush;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZEventWindow::AreEventsSelected() const
{
  return mpSnapSel->IsSelected();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZEventWindow::EventsSelected(const wxString& Message) const
{
  if (!mpSnapSel->IsSelected())
  {
    wxMessageBox(Message, "Error", wxOK);
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
// Description:
//   Display the "shift events" dialog.
//-----------------------------------------------------------------------------
void JZEventWindow::Shift(int Units)
{
  if (AreEventsSelected())
  {
    int Unit = 30;
    int Shift = 0;
    JZShiftDialog ShiftDialog(*this, *mpFilter, Units, Shift, this);

    if (ShiftDialog.ShowModal() == wxID_OK && Shift != 0)
    {
      JZCommandShift ShiftCommand(mpFilter, Shift * Unit);
      ShiftCommand.Execute();

      JZProjectManager::Instance().UpdateAllViews();
    }
  }
}

//-----------------------------------------------------------------------------
// Quantize selected events.
//-----------------------------------------------------------------------------
void JZEventWindow::Quantize()
{
  if (AreEventsSelected())
  {
    int QuantizationStep = 16;
    bool NoteStart = true;
    bool NoteLength = false;
    int Delay = 0;
    int Groove = 0;

    JZQuantizeDialog QuantizeDialog(
      QuantizationStep,
      NoteStart,
      NoteLength,
      Groove,
      Delay,
      this);

    if (QuantizeDialog.ShowModal() == wxID_OK)
    {
      int Step = mpProject->GetTicksPerQuarter() * 4 / QuantizationStep;

      JZCommandQuantize QuantizeCommand(
        mpFilter,
        QuantizationStep,
        NoteStart,
        NoteLength,
        Groove * Step / 100,
        Delay * Step / 100);

      QuantizeCommand.Execute();
//      QuantizeCommand.Execute(1);

      JZProjectManager::Instance().UpdateAllViews();
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::SetChannel()
{
  int NewChannel = 1;

  JZMidiChannelDialog MidiChannelDialog(NewChannel, this);
  if (MidiChannelDialog.ShowModal() == wxID_OK)
  {
    JZCommandSetChannel SetMidiChannelCommand(mpFilter, NewChannel - 1);
    SetMidiChannelCommand.Execute();
    JZProjectManager::Instance().UpdateAllViews();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::Transpose()
{
  int CurrentScale = JZScale::Analyze(mpFilter);
  int Notes = 0, Scale = gScaleChromatic;
  bool FitIntoScale = false;

  JZTransposeDialog TransposeDialog(
    CurrentScale,
    Notes,
    Scale,
    FitIntoScale,
    this);
  if (TransposeDialog.ShowModal() == wxID_OK)
  {
    JZCommandTranspose TransposeCommand(mpFilter, Notes, Scale, FitIntoScale);
    TransposeCommand.Execute();

    JZProjectManager::Instance().UpdateAllViews();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::Delete()
{
  bool LeaveSpace = true;

  JZDeleteDialog DeleteDialog(this, LeaveSpace);

  if (DeleteDialog.ShowModal() == wxID_OK)
  {
    JZCommandErase EraseCommand(mpFilter, LeaveSpace);
    EraseCommand.Execute();
    JZProjectManager::Instance().UpdateAllViews();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::Velocity()
{
  int FromValue = 64;
  int ToValue = 0;
  JEValueAlterationMode Mode = eSetValues;

  JZVelocityDialog VelocityDialog(this, FromValue, ToValue, Mode);
  if (VelocityDialog.ShowModal() == wxID_OK)
  {
    JZtCommandVelocity VelocityCommand(mpFilter, FromValue, ToValue, Mode);
    VelocityCommand.Execute();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::Length()
{
  int FromValue = 30;
  int ToValue = 0;
  JEValueAlterationMode Mode = eSetValues;

  JZLengthDialog LengthDialog(
    this,
    mpFilter->GetSong()->GetTicksPerQuarter(),
    FromValue,
    ToValue,
    Mode);
  if (LengthDialog.ShowModal() == wxID_OK)
  {
    JZCommandLength LengthCommand(mpFilter, FromValue, ToValue, Mode);
    LengthCommand.Execute();

    JZProjectManager::Instance().UpdateAllViews();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::ConvertToModulation()
{
  JZCommandConvertToModulation cmd(mpFilter);
  cmd.Execute();
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::Cleanup()
{
  int ShortestNote = 48;
  bool ShortenOverlappingNotes = false;

  JZCleanupDialog CleanupDialog(ShortestNote, ShortenOverlappingNotes, this);
  if (CleanupDialog.ShowModal() == wxID_OK)
  {
    int LengthLimit =
      mpFilter->GetSong()->GetTicksPerQuarter() * 4 / ShortestNote;

    JZCommandCleanup CleanupCommand(
      mpFilter,
      LengthLimit,
      ShortenOverlappingNotes);

    CleanupCommand.Execute();

    JZProjectManager::Instance().UpdateAllViews();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::SearchReplace()
{
  short From = 1, To = 1;

  JZSearchAndReplaceDialog SearchAndReplaceDialog(From, To, this);
  if (SearchAndReplaceDialog.ShowModal() == wxID_OK)
  {
    JZCommandSearchReplace SearchAndReplaceCommand(mpFilter, From - 1, To - 1);

    SearchAndReplaceCommand.Execute();

    JZProjectManager::Instance().UpdateAllViews();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::EditMeter()
{
  JZMeterChangeDialog MeterChangeDialog(this);
  MeterChangeDialog.ShowModal();
}

//-----------------------------------------------------------------------------
// Description:
//   Only consider the event portion of the window when computing the virtual
// size.  Do not consider the static information on the left or top portion of
// the screen.
//-----------------------------------------------------------------------------
void JZEventWindow::GetVirtualEventSize(
  int& EventWidth,
  int& EventHeight) const
{
  int TotalClockTics =
    mpProject->GetMaxQuarters() * mpProject->GetTicksPerQuarter();
  EventWidth = TotalClockTics / mClockTicsPerPixel;
  EventHeight = 127 * mTrackHeight;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::SetXScrollPosition(int x)
{
  // The following line converts an x position in window coordinates to an
  // x position in scrolled coordinates.
  int ScrolledX = x - mEventsX + mScrolledX;

  if (mScrolledX != ScrolledX)
  {
    mScrolledX = ScrolledX;

    // Set the new from clock and to clock positions based on the new scroll
    // position.
    mFromClock = mScrolledX * mClockTicsPerPixel;
    mToClock = x2Clock(mCanvasWidth);

    SetScrollPos(wxHORIZONTAL, mScrolledX);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::SetYScrollPosition(int y)
{
  // The following line converts a y position in window coordinates to a
  // y position in scrolled coordinates.
  int ScrolledY = y - mEventsY + mScrolledY;

  if (mScrolledY != y)
  {
    mScrolledY = ScrolledY;

    // Set the new from line and to line positions based on the new scroll
    // position.
    mFromLine = mScrolledY / mTrackHeight;
    mToLine = 1 + (mScrolledY + mCanvasHeight - mTopInfoHeight) / mTrackHeight;

    SetScrollPos(wxVERTICAL, mScrolledY);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   This function takes an x-pixel value in window coordinates and converts
// it to clock tics.
//-----------------------------------------------------------------------------
int JZEventWindow::x2Clock(int x)
{
  return (x - mEventsX) * mClockTicsPerPixel + mFromClock;
}

//-----------------------------------------------------------------------------
// Description:
//   This function takes clock tics and converts the value into an x-pixel
// location on the screen in window coordinates.
//-----------------------------------------------------------------------------
int JZEventWindow::Clock2x(int Clock)
{
  return mEventsX + (Clock - mFromClock) / mClockTicsPerPixel;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZEventWindow::x2BarClock(int x, int Next)
{
  int Clock = x2Clock(x);
  JZBarInfo BarInfo(*mpProject);
  BarInfo.SetClock(Clock);
  while (Next--)
  {
    BarInfo.Next();
  }
  return BarInfo.GetClock();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZEventWindow::y2yLine(int y, int Up)
{
  if (Up)
  {
    y += mTrackHeight;
  }
  y -= mTopInfoHeight;
  y -= y % mTrackHeight;
  y += mTopInfoHeight;
  return y;
}

//-----------------------------------------------------------------------------
// Was the VLine macro
//-----------------------------------------------------------------------------
void JZEventWindow::DrawVerticalLine(wxDC& Dc, int XPosition) const
{
  Dc.DrawLine(XPosition, 0, XPosition, mEventsY + mEventsHeight);
}

//-----------------------------------------------------------------------------
// Was the HLine macro
//-----------------------------------------------------------------------------
void JZEventWindow::DrawHorizontalLine(wxDC& Dc, int YPosition) const
{
  Dc.DrawLine(0, YPosition, mCanvasWidth, YPosition);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZEventWindow::LineText(
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
    y = y2yLine(y);
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
// Descriptions:
//   This mouse handler delegates to the subclassed event window.
//-----------------------------------------------------------------------------
void JZEventWindow::OnMouseEvent(wxMouseEvent& MouseEvent)
{
  if (!mpMouseAction)
  {
    int x, y;
    MouseEvent.GetPosition(&x, &y);
    if (
      mEventsX < x && x < mEventsX + mEventsWidth &&
      mEventsY < y && y < mEventsY + mEventsHeight)
    {
      if (MouseEvent.LeftDown())
      {
        {
          SnapSelectionStart(MouseEvent);

//          if (mpSnapSel->IsSelected())
//          {
            // Redraw the whole window instead (inefficient, we should rather
            // invalidate a rect).
            Refresh();
//          }
          mpSnapSel->ProcessMouseEvent(MouseEvent, mScrolledX, mScrolledY);
          mpMouseAction = mpSnapSel;
        }
      }
    }
  }
  else
  {
    // mpMouseAction active

    if (mpMouseAction->ProcessMouseEvent(MouseEvent, mScrolledX, mScrolledY))
    {
      // mpMouseAction finished

      if (mpMouseAction == mpSnapSel)
      {
        SnapSelectionStop(MouseEvent);

        // inefficient, invalidate rect first instead.
        Refresh();
      }
      else
      {
        delete mpMouseAction;
      }

      mpMouseAction = nullptr;
    }
  }
}
