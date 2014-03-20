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

#pragma once

#include <wx/bitmap.h>
#include <wx/control.h>

class JZKnobEvent;
class JZKnob;

//*****************************************************************************
// Description:
//   Declare knob control event types and macros for handling them.
//*****************************************************************************
//-----------------------------------------------------------------------------
// Description:
//   The second argument to DECLARE_EVENT_TYPE is unused.  wxWidgets assigns a
// unique evant ID at run time.
//-----------------------------------------------------------------------------
BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(wxEVT_KNOB_CHANGED, 1)
END_DECLARE_EVENT_TYPES()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef void (wxEvtHandler::*wxKnobEventFunction)(JZKnobEvent&);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define EVT_KNOB_CHANGED(Id, Function) \
  DECLARE_EVENT_TABLE_ENTRY( \
    wxEVT_KNOB_CHANGED, \
    Id, \
    -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
      (wxKnobEventFunction)& Function, \
    (wxObject *) NULL),

//*****************************************************************************
// Description:
//   This is the knob control event class declaration.
//*****************************************************************************
class JZKnobEvent : public wxCommandEvent
{
  public:

    JZKnobEvent();

    JZKnobEvent(JZKnob* pKnobCtrl, wxEventType Type);

    JZKnobEvent(JZKnob* pKnobCtrl, int Value, wxEventType Type);

    int GetValue() const;

  private:

    int mValue;

  DECLARE_DYNAMIC_CLASS(JZKnobEvent)
};

//*****************************************************************************
// Description:
//   This is the knob class declaration.  This is a custom control that looks
// like a mixer knob.
//*****************************************************************************
class JZKnob : public wxControl
{
  public:

    JZKnob();

    JZKnob(
      wxWindow* pParent,
      wxWindowID Id,
      int Value,
      int MinValue,
      int MaxValue,
      unsigned int MinAngle = 240,
      unsigned int Range = 300,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxSize(40, 40),
      long WindowStyle = wxNO_BORDER,
      const wxValidator& Validator = wxDefaultValidator,
      const wxString& Name = wxT("knob"));

    void Create(
      wxWindow* pParent,
      wxWindowID Id,
      int Value,
      int MinValue,
      int MaxValue,
      unsigned int MinAngle = 240,
      unsigned int Range = 300,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxSize(40, 40),
      long WindowStyle = wxNO_BORDER,
      const wxValidator& Validator = wxDefaultValidator,
      const wxString& Name = wxT("knob"));

    // Retrieve/change the range
    void SetRange(int MinValue, int MaxValue);

    int GetMin() const;

    int GetMax() const;

    void SetMin(int MinValue);

    void SetMax(int MaxValue);

    unsigned int GetMinAngle() const;

    int GetMaxAngle() const;

    int GetValue() const;

    int SetValue(int Value);

    int SetValueWithEvent(int Value);

  private:

    void GetCenter(int& x, int& y) const;

    void OnSize(wxSizeEvent& Event);

    void OnEraseBackground(wxEraseEvent& Event);

    void OnPaint(wxPaintEvent& Event);

    void OnLeftButtonDown(wxMouseEvent& MouseEvent);

    void OnRightButtonDown(wxMouseEvent& MouseEvent);

    void OnMouseMove(wxMouseEvent& MouseEvent);

    void OnLeftButtonUp(wxMouseEvent& MouseEvent);

    void OnLeftButtonDoubleClick(wxMouseEvent& MouseEvent);

    void OnRightButtonDoubleClick(wxMouseEvent& MouseEvent);

    void OnMouseWheel(wxMouseEvent& MouseEvent);

  private:

    static int mSensitivity;

    int mMinValue;

    int mMaxValue;

    int mSetting;

    unsigned int mRange;

    unsigned int mMaxAngle;

    wxBitmap mBuffer;

    bool mDragging;

    wxPoint mLastPoint;

  DECLARE_EVENT_TABLE()
};

//*****************************************************************************
// Description:
//   These are the knob control event inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZKnobEvent::GetValue() const
{
  return mValue;
}

//*****************************************************************************
// Description:
//   These are the knob inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZKnob::GetMin() const
{
  return mMinValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZKnob::GetMax() const
{
  return mMaxValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZKnob::SetMin(int MinValue)
{
  SetRange(MinValue, GetMax());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZKnob::SetMax(int MaxValue)
{
  SetRange(GetMin(), MaxValue);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
unsigned int JZKnob::GetMinAngle() const
{
  return (mMaxAngle - mRange) % 360;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZKnob::GetMaxAngle() const
{
  return mMaxAngle;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZKnob::GetValue() const
{
  return mSetting;
}
