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

#include "MouseAction.h"

#include "EventWindow.h"

#include <wx/brush.h>
#include <wx/dcclient.h>

//DEBUG#include <iostream>

using namespace std;

//*****************************************************************************
// Description:
//   This is the mouse mapper class declaration.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMouseMapper::JZMouseMapper(const int Actions[12])
  : mActions(),
    mLeftAction(0)
{
  for (int i = 0; i < 12; i++)
  {
    mActions[i] = Actions[i];
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMouseMapper::SetAction(
  int Action,
  TEButton Button,
  bool Shift,
  bool Ctrl)
{
  int i = 0;
  switch (Button)
  {
    case eLeft:
      i = 0;
      break;
    case eMiddle:
      i = 1;
      break;
    case eRight:
      i = 2;
      break;
  }
  if (Shift)
  {
    i += 3;
  }
  if (Ctrl)
  {
    i += 6;
  }
  mActions[i] = Action;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMouseMapper::GetAction(wxMouseEvent& MouseEvent)
{
  if (!MouseEvent.ButtonDown())
  {
    return 0;
  }

  if (
    mLeftAction > 0 &&
    MouseEvent.LeftDown() &&
    !MouseEvent.ShiftDown() &&
    !MouseEvent.ControlDown())
  {
    return mLeftAction;
  }

  // Assume the left button is down.
  int i = 0;
  if (MouseEvent.MiddleDown())
  {
    i = 1;
  }
  else if (MouseEvent.RightDown())
  {
    i = 2;
  }

  if (MouseEvent.ShiftDown())
  {
    i += 3;
  }

  if (MouseEvent.ControlDown())
  {
    i += 6;
  }

  return mActions[i];
}

//*****************************************************************************
// Description:
//   This is the mouse action base class definition.  Derived classes are
// instantiated in the mouse handler of the event window, for example, to
// retain state during mouse operations, like drag and drop and so on.
//   The ProcessMouseEvent() function is used to determine what to do with an
// incoming event.    Normally, if the event is a left button down event, call
// the LeftDown function of the class, and so on.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMouseAction::~JZMouseAction()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMouseAction::ProcessMouseEvent(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (MouseEvent.Dragging())
  {
    return Dragging(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.LeftDown())
  {
    return LeftDown(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.LeftUp())
  {
    return LeftUp(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.MiddleDown())
  {
    return MiddleDown(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.MiddleUp())
  {
    return MiddleUp(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.RightDown())
  {
    return RightDown(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.RightUp())
  {
    return RightUp(MouseEvent, ScrolledX, ScrolledY);
  }
  return 0;
}

//*****************************************************************************
// Description:
//  This is the selection class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSelection::JZSelection(wxWindow* pWindow)
  : mActive(false),
    mSelected(false),
    mRectangle(),
    mpWindow(pWindow),
    mpBackgroundBrush(nullptr)
{
//  mpBackgroundBrush = new wxBrush(wxColor(192, 192, 192), wxSOLID);
  mpBackgroundBrush = new wxBrush(wxColor(100, 100, 100), wxSOLID);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSelection::~JZSelection()
{
  delete mpBackgroundBrush;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSelection::ProcessMouseEvent(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (MouseEvent.ButtonDown())
  {
    return ButtonDown(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.ButtonUp())
  {
    return ButtonUp(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.Dragging())
  {
    return Dragging(MouseEvent, ScrolledX, ScrolledY);
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSelection::ButtonDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (!mActive)
  {
    mActive = true;
    if (mSelected && MouseEvent.ShiftDown())
    {
      // Continue selection
      JZRectangle Rectangle = mRectangle;
      Rectangle.SetNormal();
      Dragging(MouseEvent, ScrolledX, ScrolledY);
    }
    else
    {
      mSelected = false;
      int x = MouseEvent.GetX() + ScrolledX;
      int y = MouseEvent.GetY() + ScrolledY;
      Snap(x, y, ScrolledX, ScrolledY, false);
      mRectangle.x = x;
      mRectangle.y = y;
      mRectangle.width = 1;
      mRectangle.height = 1;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSelection::ButtonUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (mActive)
  {
    mActive = false;
    mRectangle.SetNormal();

    // Only select if the rectangle is larger than 3x3 pixels.
    mSelected = (mRectangle.width > 3 && mRectangle.height > 3);
    return 1;
  }

  mpWindow->Refresh();
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSelection::Dragging(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (!mActive)
  {
    ButtonDown(MouseEvent, ScrolledX, ScrolledY);
  }

  if (mActive)
  {
    int x = MouseEvent.GetX() + ScrolledX;
    int y = MouseEvent.GetY() + ScrolledY;
    if (x < 0)
    {
      x = 0;
    }
    if (y < 0)
    {
      y = 0;
    }
    Snap(x, y, ScrolledX, ScrolledY, true);

    mRectangle.width = x - mRectangle.x;
    mRectangle.height = y - mRectangle.y;
  }

  return 0;
}

//-----------------------------------------------------------------------------
// Description:
//   Draw the selected rectangle.
//-----------------------------------------------------------------------------
void JZSelection::Draw(wxDC& Dc, int ScrolledX, int ScrolledY)
{
//  if (mSelected)
  {
    JZRectangle Rectangle = mRectangle;

    Dc.SetLogicalFunction(wxXOR);
    Dc.SetBrush(*mpBackgroundBrush);

    Rectangle.SetNormal();
    if (Rectangle.width && Rectangle.height)
    {
      Dc.DrawRectangle(
        Rectangle.x - ScrolledX,
        Rectangle.y - ScrolledY,
        Rectangle.width,
        Rectangle.height);
    }
    Dc.SetLogicalFunction(wxCOPY);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Draw, but use clipping to redruce drawing.
//-----------------------------------------------------------------------------
void JZSelection::Draw(
  wxDC& Dc,
  int ScrolledX,
  int ScrolledY,
  int ClipX,
  int ClipY,
  int ClipWidth,
  int ClipHeight)
{
//  if (mSelected)
  {
    Dc.SetClippingRegion(ClipX, ClipY, ClipWidth, ClipHeight);
    Draw(Dc, ScrolledX, ScrolledY);
    Dc.DestroyClippingRegion();
  }
}

//-----------------------------------------------------------------------------
//   I think this one is meant to select a rectangle and repaint it.
// It did this by drawing directly in the device context.  This is bad, so I
// tried changing it to invalidation instead.
//-----------------------------------------------------------------------------
void JZSelection::Select(const JZRectangle& Rectangle)
{
  mRectangle = Rectangle;
  mSelected = true;

  // Inefficient because should invalidate only the rectangle.
  mpWindow->Refresh();
}

//*****************************************************************************
// Description:
//   This is the snap selection class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSnapSelection::JZSnapSelection(wxWindow* pWindow)
  : JZSelection(pWindow),
    mXCoordinates(),
    mYCoordinates(),
    mXMin(0),
    mXMax(0),
    mXStep(0),
    mYMin(0),
    mYMax(0),
    mYStep(0)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::Snap(
  int& x,
  int& y,
  int ScrolledX,
  int ScrolledY,
  bool Up)
{
  if (!mXCoordinates.empty())
  {
    SnapToXVector(x, ScrolledX, Up);
  }
  else if (mXStep)
  {
    SnapMod(x, mXMin, mXMax, mXStep, ScrolledX, Up);
  }

  if (!mYCoordinates.empty())
  {
    SnapToYVector(y, ScrolledY, Up);
  }
  else if (mYStep)
  {
    SnapMod(y, mYMin, mYMax, mYStep, ScrolledY, Up);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::SetXSnap(const vector<int>& XVector, int ScrolledX)
{
  mXCoordinates.clear();
  for (const auto& XValue : XVector)
  {
    mXCoordinates.push_back(XValue + ScrolledX);
  }
  mXStep = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::SetYSnap(const vector<int>& YVector, int ScrolledY)
{
  mYCoordinates.clear();
  for (const auto& YValue : YVector)
  {
    mYCoordinates.push_back(YValue + ScrolledY);
  }
  mYStep = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::SetXSnap(int XMin, int XMax, int XStep)
{
  mXMin = XMin;
  mXMax = XMax;
  mXStep = XStep;
  mXCoordinates.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::SetYSnap(int YMin, int YMax, int YStep)
{
  mYMin = YMin;
  mYMax = YMax;
  mYStep = YStep;
  mYCoordinates.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::SnapToXVector(
  int& Coordinate,
  int Scrolled,
  bool Up) const
{
//DEBUG  cout << "In: " << Coordinate;
  for (unsigned i = 0; i < mXCoordinates.size(); ++i)
  {
    if (mXCoordinates[i] > Coordinate)
    {
      if (Up || i == 0)
      {
        Coordinate = mXCoordinates[i];
      }
      else
      {
        Coordinate = mXCoordinates[i - 1];
      }
//DEBUG      cout << "     Out: " << Coordinate  << endl;
      return;
    }
  }
//DEBUG  cout << "     Out: " << Coordinate  << endl;
  Coordinate = mXCoordinates[mXCoordinates.size() - 1];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::SnapToYVector(
  int& Coordinate,
  int Scrolled,
  bool Up) const
{
//DEBUG  cout << "In: " << Coordinate;
  for (unsigned i = 0; i < mYCoordinates.size(); ++i)
  {
    if (mYCoordinates[i] > Coordinate)
    {
      if (Up || i == 0)
      {
        Coordinate = mYCoordinates[i];
      }
      else
      {
        Coordinate = mYCoordinates[i - 1];
      }
//DEBUG      cout << "     Out: " << Coordinate  << endl;
      return;
    }
  }
//DEBUG  cout << "     Out: " << Coordinate  << endl;
  Coordinate = mYCoordinates[mYCoordinates.size() - 1];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapSelection::SnapMod(
  int& Coordinate,
  int Min,
  int Max,
  int Step,
  int Scrolled,
  bool Up)
{
//DEBUG  cout << "In: " << Coordinate;
  if (Coordinate <= Min)
  {
    Coordinate = Min;
    return;
  }
  if (Coordinate >= Max)
  {
    Coordinate = Max;
//DEBUG    cout << "Max: " << Coordinate << endl;
    return;
  }
  Coordinate -= (Coordinate - Min) % Step - (Scrolled % Step);
  if (Up)
  {
    Coordinate += Step;
  }
//DEBUG  cout << "     Out: " << Coordinate  << endl;
}


// *************************************************************************
// JZMouseCounter
// *************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMouseCounter::JZMouseCounter(
  JZButtonLabelInterface* wwin,
  JZRectangle* Rectangle,
  int val,
  int min,
  int max,
  int wait)
{
  win = wwin;
  r = *Rectangle;
  Value = val;
  Min = min;
  Max = max;
  Timeout = 500;
  Wait = wait;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMouseCounter::LeftDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  Delta = MouseEvent.ShiftDown() ? 10 : 1;
  Start(Timeout);
  if (Wait)
  {
    ShowValue(TRUE);
  }
  else
  {
    Notify();
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMouseCounter::LeftUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  Stop();
  ShowValue(FALSE);
  return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMouseCounter::RightDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  Delta = MouseEvent.ShiftDown() ? -10 :  -1;
  Start(Timeout);
  if (Wait)
  {
    ShowValue(TRUE);
  }
  else
  {
    Notify();
  }
  return 0;

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMouseCounter::RightUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  Stop();
  ShowValue(FALSE);
  return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMouseCounter::Notify()
{
  Value += Delta;
  if (Value > Max)
    Value = Max;
  if (Value < Min)
    Value = Min;
  ShowValue(TRUE);
  if (Timeout > 50)
  {
    Stop();
    Timeout >>= 1;
    Start(Timeout);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMouseCounter::ShowValue(bool down)
{
  char buf[20];
  sprintf(buf, "%3d", Value);
  win->ButtonLabelDisplay(buf, down);
}

// -------------------------------------------------------------------------
// JZMarkDestination
// -------------------------------------------------------------------------


JZMarkDestination::JZMarkDestination(wxWindow* canvas, wxFrame *frame, int left)
{
  wxCursor c;
  Canvas = canvas;
  Frame  = frame;
  if (left)
    c =  wxCursor(wxCURSOR_POINT_LEFT);
  else
    c =  wxCursor(wxCURSOR_POINT_RIGHT);
  Canvas->SetCursor(c);
  Aborted = 1;
  //Frame->SetStatusText("Click Destination point");
}

int JZMarkDestination::ButtonDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  wxCursor c =  wxCursor(wxCURSOR_ARROW);
  Canvas->SetCursor(c);

  //converts physical coords to logical(scrolled) coords
  wxClientDC* scrolledDC=new wxClientDC(Canvas);
  Canvas->PrepareDC(*scrolledDC);
  wxPoint point = MouseEvent.GetLogicalPosition(*scrolledDC);
  delete scrolledDC;

  x=point.x;
  y=point.y;
//DEBUG  cout << "JZMarkDestination::ButtonDown " << x << ' ' << y <<endl;
  return 1;
}

int JZMarkDestination::RightDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  ButtonDown(MouseEvent, ScrolledX, ScrolledY);
  Aborted = 1;
  //Frame->SetStatusText("Operation aborted");
  return 1;
}

int JZMarkDestination::LeftDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  ButtonDown(MouseEvent, ScrolledX, ScrolledY);
  Aborted = 0;
  //Frame->SetStatusText("");
  return 1;
}

#if 0

//*****************************************************************************
// Description:
//   This is the mouse button class definition.  This class simulates a 3D
// button.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMouseButton::JZMouseButton(
  JZEventWindow* pEventWindow,
  JZRectangle* pRectangle,
  const char* pDownString,
  const char* pUpString)
  : mpEventWindow(pEventWindow),
    mRectangle(*pRectangle),
    mDownString(),
    mUpString()
{
  if (pDownString)
  {
    mDownString = pDownString;
  }

  if (pUpString)
  {
    mUpString = pUpString;
  }
  else
  {
    mUpString = mDownString;
  }

  wxClientDC Dc(mpEventWindow);

  mpEventWindow->LineText(
    Dc,
    mRectangle.x,
    mRectangle.y,
    mRectangle.GetWidth(),
    mDownString.c_str(),
    mRectangle.GetHeight(),
    true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMouseButton::~JZMouseButton()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMouseButton::ProcessMouseEvent(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (MouseEvent.ButtonUp())
  {
    Action();

    wxClientDC Dc(mpEventWindow);

    mpEventWindow->LineText(
      Dc,
      mRectangle.x,
      mRectangle.y,
      mRectangle.GetWidth(),
      mUpString.c_str(),
      mRectangle.GetHeight(),
      false);

    delete this;

    return 1;
  }
  return 0;
}

#endif
