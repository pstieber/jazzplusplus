//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2008-2013 Peter J. Stieber
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

#include "Knob.h"

#include "Globals.h"

#include <wx/dcbuffer.h>
#include <wx/settings.h>

#include <cmath>

//*****************************************************************************
// Description:
//   This is the knob control event class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
// Define the knob control event types.
// I think the following insures the event IDs are unique.
//-----------------------------------------------------------------------------
DEFINE_EVENT_TYPE(wxEVT_KNOB_CHANGED)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(JZKnobEvent, wxCommandEvent)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKnobEvent::JZKnobEvent()
  : wxCommandEvent(),
    mValue(0)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKnobEvent::JZKnobEvent(
  JZKnob* pKnobCtrl,
  wxEventType Type)
  : wxCommandEvent(Type, pKnobCtrl->GetId()),
    mValue(0)
{
  mValue = pKnobCtrl->GetValue();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKnobEvent::JZKnobEvent(JZKnob* pKnobCtrl, int Value, wxEventType Type)
  : wxCommandEvent(Type, pKnobCtrl->GetId()),
    mValue(Value)
{
}

//*****************************************************************************
// Description:
//   This is the knob class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZKnob, wxControl)
  EVT_SIZE(JZKnob::OnSize)
  EVT_ERASE_BACKGROUND(JZKnob::OnEraseBackground)
  EVT_PAINT(JZKnob::OnPaint)
  EVT_LEFT_DOWN(JZKnob::OnLeftButtonDown)
  EVT_RIGHT_DOWN(JZKnob::OnRightButtonDown)
  EVT_MOTION(JZKnob::OnMouseMove)
  EVT_LEFT_UP(JZKnob::OnLeftButtonUp)
  EVT_LEFT_DCLICK(JZKnob::OnLeftButtonDoubleClick)
  EVT_RIGHT_DCLICK(JZKnob::OnRightButtonDoubleClick)
  EVT_MOUSEWHEEL(JZKnob::OnMouseWheel)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZKnob::mSensitivity = 4;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKnob::JZKnob()
  : wxControl(),
    mMinValue(0),
    mMaxValue(100),
    mSetting(50),
    mRange(300),
    mMaxAngle(300),
    mBuffer(),
    mDragging(false),
    mLastPoint()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKnob::JZKnob(
  wxWindow* pParent,
  wxWindowID Id,
  int Value,
  int MinValue,
  int MaxValue,
  unsigned int MinAngle,
  unsigned int Range,
  const wxPoint& Position,
  const wxSize& Size,
  long WindowStyle,
  const wxValidator& Validator,
  const wxString& Name)
  : wxControl(),
    mMinValue(0),
    mMaxValue(100),
    mSetting(50),
    mRange(300),
    mMaxAngle(300),
    mBuffer(),
    mDragging(false),
    mLastPoint()
{
  Create(
    pParent,
    Id,
    Value,
    MinValue,
    MaxValue,
    MinAngle,
    Range,
    Position,
    Size,
    WindowStyle,
    Validator,
    Name);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::Create(
  wxWindow* pParent,
  wxWindowID Id,
  int Value,
  int MinValue,
  int MaxValue,
  unsigned int MinAngle,
  unsigned int Range,
  const wxPoint& Position,
  const wxSize& Size,
  long WindowStyle,
  const wxValidator& Validator,
  const wxString& Name)
{
  wxControl::Create(
    pParent,
    Id,
    Position,
    Size,
    WindowStyle | wxNO_BORDER,
    Validator,
    Name);

  SetInitialSize(Size);

  mMinValue = MinValue;
  mMaxValue = MaxValue;
  Range %= 360;
  MinAngle %= 360;
  mMaxAngle = (MinAngle + 360 - Range) % 360;

  mRange = Range;
  SetValue(Value);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::SetRange(int MinValue, int MaxValue)
{
  if (MinValue < MaxValue)
  {
    mMinValue = MinValue;
    mMaxValue = MaxValue;
    SetValueWithEvent(mSetting);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZKnob::SetValue(int Value)
{
  if (Value < mMinValue)
  {
    Value = mMinValue;
  }
  if (Value > mMaxValue)
  {
    Value = mMaxValue;
  }

  if (Value != mSetting)
  {
    mSetting = Value;
    Refresh(false);
    Update();
  }
  return mSetting;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZKnob::SetValueWithEvent(int Value)
{
  int ActualValue = SetValue(Value);

  JZKnobEvent Event(this, ActualValue, wxEVT_KNOB_CHANGED);
  Event.SetEventObject(this);
  GetEventHandler()->ProcessEvent(Event);

  return ActualValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::GetCenter(int& x, int& y) const
{
  wxSize Size = GetSize();
  x = Size.x / 2;
  y = Size.y / 2;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnSize(wxSizeEvent& Event)
{
  int Width, Height;
  GetClientSize(&Width, &Height);
  if (Width > 0 && Height > 0)
  {
    mBuffer.Create(Width, Height);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   This code always erases when painting so we override this function to
// avoid flicker.
//-----------------------------------------------------------------------------
void JZKnob::OnEraseBackground(wxEraseEvent& Event)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnPaint(wxPaintEvent& Event)
{
  wxSize Size = GetSize();

  double Theta = gDegreesToRadians *
    (mMaxAngle +
      (((double)mMaxValue - mSetting) / (mMaxValue - mMinValue)) * mRange);

  double DeltaX = cos(Theta);

  // Negate because of the upside down coordinates
  double DeltaY = -sin(Theta);

  wxPaintDC PaintDc(this);

  wxBufferedDC Dc(&PaintDc, mBuffer);

  Dc.SetBackground(
    wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));

  Dc.Clear();

  int XCenter, YCenter;
  GetCenter(XCenter, YCenter);

  int OuterRadius = static_cast<int>(
    (((Size.x < Size.y) ? Size.x : Size.y) * .48) + 0.5);
  int InnerRadius = static_cast<int>(OuterRadius * 0.6 + 0.5);

  wxColour Color(120, 100, 100);
  wxBrush Brush(Color, wxSOLID);
  wxPen Pen(Color);
  int KnobRadius = OuterRadius;
  for (unsigned char Red = 120; KnobRadius > 0 && Red < 250; Red += 5)
  {
    Color.Set(Red, 100, 100);
    Brush.SetColour(Color);
    Pen.SetColour(Color);
    Dc.SetBrush(Brush);
    Dc.SetPen(Pen);
    Dc.DrawCircle(XCenter, YCenter, KnobRadius);
    --KnobRadius;
  }

  wxPen WhitePen(*wxWHITE, 3);
  Dc.SetPen(WhitePen);
  Dc.DrawLine(
    XCenter + static_cast<int>(OuterRadius * DeltaX + 0.5),
    YCenter + static_cast<int>(OuterRadius * DeltaY + 0.5),
    XCenter + static_cast<int>(InnerRadius * DeltaX + 0.5),
    YCenter + static_cast<int>(InnerRadius * DeltaY + 0.5));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnLeftButtonDown(wxMouseEvent& MouseEvent)
{
  SetFocus();

  mLastPoint = MouseEvent.GetPosition();

  SetCursor(wxCursor(wxCURSOR_SIZENS));

  CaptureMouse();

  mDragging = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnRightButtonDown(wxMouseEvent& MouseEvent)
{
  SetFocus();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnMouseMove(wxMouseEvent& MouseEvent)
{
  if (mDragging)
  {
    wxPoint Point = MouseEvent.GetPosition();

    int Delta = (mLastPoint.y - Point.y) / mSensitivity;

    if (Delta)
    {
      int PriorValue = GetValue();
      SetValueWithEvent(PriorValue + Delta);
      if (PriorValue != GetValue())
      {
        mLastPoint = Point;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnLeftButtonUp(wxMouseEvent& MouseEvent)
{
  if (HasCapture())
  {
    ReleaseMouse();
  }

  SetCursor(wxCursor(wxCURSOR_ARROW));

  mDragging = false;

  wxPoint Point = MouseEvent.GetPosition();

  int Delta = (mLastPoint.y - Point.y) / mSensitivity;
  if (Delta)
  {
    SetValueWithEvent(GetValue() + Delta);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnLeftButtonDoubleClick(wxMouseEvent& MouseEvent)
{
  SetValueWithEvent(GetValue() + 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnRightButtonDoubleClick(wxMouseEvent& MouseEvent)
{
  SetValueWithEvent(GetValue() - 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKnob::OnMouseWheel(wxMouseEvent& MouseEvent)
{
  int WheelRotation = MouseEvent.GetWheelRotation();

  if (WheelRotation < 0)
  {
    SetValueWithEvent(GetValue() - 1);
  }
  else if (WheelRotation > 0)
  {
    SetValueWithEvent(GetValue() + 1);
  }
}
