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

#include "Rectangle.h"

#include <vector>

#include <wx/timer.h>

class JZEventWindow;

//*****************************************************************************
//*****************************************************************************
enum TEMousePlayMode
{
  eMouse,
  eSpaceBar,
  ePlayButton,
  ePlayLoopButton,
  eRecordButton
};

//*****************************************************************************
// Description:
//   This is the mouse mapper class declaration.  This class maps the state of
// the mouse and certain keyboard keys into user defined integer codes.  There
// are 12 possible mouse button / keyboard key combinations.  They are:
//
// 0,  1,  2 = left, middle, right down
// 3,  4,  5 = left, middle, right down + shift
// 6,  7,  8 = left, middle, right down + ctrl
// 9, 10, 11 = left, middle, right down + ctrl + shift
//
// Note that combinations or mouse buttons are not considered.
//   The function GetAction converts a wxWidgets mouse event into a user
// defined code.  The default code for all mouse actions is 0.
//*****************************************************************************
class JZMouseMapper
{
  public:

    JZMouseMapper(const int Actions[12]);

    enum TEButton
    {
      eLeft,
      eMiddle,
      eRight
    };

    int GetAction(wxMouseEvent& MouseEvent);

    void SetAction(
      int Action,
      TEButton Button = eLeft,
      bool Shift = false,
      bool Ctrl = false);

    void SetLeftAction(int Action = 0)
    {
      mLeftAction = Action;
    }

  private:

    int mActions[12];

    int mLeftAction;
};

//*****************************************************************************
// Description:
//   This is the mouse action base class declaration.  Derived classes are
// instantiated in the mouse handler of the event window, for example, to
// retain state during mouse operations, like drag and drop and so on.
//   The ProcessMouseEvent() function is used to determine what to do with an
// incoming event.  Normally, if the event is a left button down event, call
// the LeftDown function of the class, and so on.
//*****************************************************************************
class JZMouseAction
{
  public:

    virtual ~JZMouseAction();

    virtual int LeftDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int LeftUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int RightDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int RightUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int MiddleDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int MiddleUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int Dragging(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int ProcessMouseEvent(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);
};

//*****************************************************************************
// Description:
//   This is the selection class declaration.  This class selects events using
// the mouse and draws indicating the selected events.
//*****************************************************************************
class JZSelection : public JZMouseAction
{
  public:

    JZSelection(wxWindow* pWindow);

    virtual ~JZSelection();

    virtual bool IsActive() const
    {
      return mActive;
    }

    virtual bool IsSelected() const
    {
      return mSelected;
    }

    virtual void SetSelected(bool Selected)
    {
      mSelected = Selected;
    }

    virtual const JZRectangle& GetRectangle() const
    {
      return mRectangle;
    }

    virtual void SetRectangle(const JZRectangle& Rectangle)
    {
      mRectangle = Rectangle;
    }

    virtual void Snap(
      int& x,
      int& y,
      int ScrolledX,
      int ScrolledY,
      bool Up)
    {
    }

    virtual int Dragging(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int ProcessMouseEvent(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int ButtonDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int ButtonUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual void Draw(wxDC& Dc, int ScrolledX, int ScrolledY);

    // Drawing with clipping.
    virtual void Draw(
      wxDC& Dc,
      int ScrolledX,
      int ScrolledY,
      int ClipX,
      int ClipY,
      int ClipWidth,
      int ClipHeight);

    void Select(const JZRectangle& Rectangle);

  private:

    bool mActive;

    // The following indicates if the rectangle is valid.
    bool mSelected;

    JZRectangle mRectangle;

    wxWindow* mpWindow;

    wxBrush* mpBackgroundBrush;
};

//*****************************************************************************
// Description:
//   This is the snap selection class declaration.
//*****************************************************************************
class JZSnapSelection : public JZSelection
{
  public:

    JZSnapSelection(wxWindow* pWindow);

    virtual void Snap(
      int& x,
      int& y,
      int ScrolledX,
      int ScrolledY,
      bool Up);

    void SetXSnap(const std::vector<int>& XVector, int ScrolledX);

    void SetYSnap(const std::vector<int>& YVector, int ScrolledY);

    void SetXSnap(int XMin, int XMax, int XStep);

    void SetYSnap(int YMin, int YMax, int YStep);

  private:

    void SnapToXVector(int& Coordinate, int Scrolled, bool Up) const;

    void SnapToYVector(int& Coordinate, int Scrolled, bool Up) const;

    static void SnapMod(
      int& Coordinate,
      int Min,
      int Max,
      int Step,
      int Scrolled,
      bool Up);

  protected:

    std::vector<int> mXCoordinates;

    std::vector<int> mYCoordinates;

    int mXMin, mXMax, mXStep, mYMin, mYMax, mYStep;
};


//*****************************************************************************
//  JZButtonLabelInterface
//
//  Specifies an interface for displaying a text string within another widget.
//  The other widget would inherit from this interface and implement the Display
//  method to print the string somewhere appropriate.  The down argument
//  indicates if the text should be displayed in a depressed button or a normal
//  button.
//*****************************************************************************
class JZButtonLabelInterface
{
  public:

    virtual ~JZButtonLabelInterface()
    {
    }

    virtual void ButtonLabelDisplay(
      const wxString& Text,
      bool IsButtonDown) = 0;
};


//*****************************************************************************
//  MouseCounter - let you enter numbers with left/right mouse button
//*****************************************************************************
class JZMouseCounter : public wxTimer, public JZMouseAction
{
  public:

    JZRectangle r;

    int Value;

    JZMouseCounter(
      JZButtonLabelInterface *win,
      JZRectangle *rec,
      int val,
      int min,
      int max,
      int wait = 0);

  private:

    int Min, Max, Delta;
    int Timeout;
    int Wait;        // don't inc/dec at Init
    JZButtonLabelInterface *win;

    virtual int LeftDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int LeftUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int RightDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int RightUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual void Notify();

    virtual void ShowValue(bool down);
};


//*****************************************************************************
// JZMarkDestination - mark destination of some operation
//*****************************************************************************
class JZMarkDestination : public JZMouseAction
{
  public:

    int Aborted;
    float x, y;

    virtual int LeftDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    virtual int RightDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    JZMarkDestination(wxWindow* canvas, wxFrame* frame, int left);

  private:

    wxWindow *Canvas;
    wxFrame  *Frame;

    int ButtonDown(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);
};

#if 0

//*****************************************************************************
// Description:
//   This is the mouse button class declaration.  This class simulates a 3D
// button.
//*****************************************************************************
class JZMouseButton : public JZMouseAction
{
  public:

    JZMouseButton(
      JZEventWindow* pEventWindow,
      JZRectangle* pRectangle,
      const char* pDownString,
      const char* pUpString = 0);

    virtual ~JZMouseButton();

    virtual int ProcessMouseEvent(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

  protected:

    virtual void Action()
    {
    }

  private:

    JZEventWindow* mpEventWindow;

    JZRectangle mRectangle;

    wxString mDownString;

    wxString mUpString;
};

#endif

//*****************************************************************************
// Description:
//  These are the mouse action class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZMouseAction::LeftDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZMouseAction::LeftUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZMouseAction::RightDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZMouseAction::RightUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZMouseAction::MiddleDown(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZMouseAction::MiddleUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZMouseAction::Dragging(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  return 0;
}
