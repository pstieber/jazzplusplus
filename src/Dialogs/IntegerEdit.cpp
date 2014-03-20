//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2008-2013 Peter J. Stieber, all rights reserved.
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

#include "IntegerEdit.h"

#include <wx/msgdlg.h>

#include <cstdlib>
#include <limits>
#include <sstream>

using namespace std;

//*****************************************************************************
// Description:
//   This is the integer field edit class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZIntegerEdit, wxTextCtrl)
  EVT_CHAR(JZIntegerEdit::OnChar)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZIntegerEdit::JZIntegerEdit(
  wxWindow* pParent,
  wxWindowID Id,
  const wxPoint& Position,
  const wxSize& Size,
  long WindowStyle,
  const wxValidator& Validator,
  const wxString& Name)
  : wxTextCtrl(
      pParent,
      Id,
      wxEmptyString,
      Position,
      Size,
      WindowStyle,
      Validator,
      Name),
    mMin(numeric_limits<int>::min()),
    mMax(numeric_limits<int>::max()),
    mValueName()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZIntegerEdit::SetMinAndMax(int Min, int Max)
{
  if (Min <= Max)
  {
    mMin = Min;
    mMax = Max;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZIntegerEdit::SetValueName(const string& ValueName)
{
  mValueName = ValueName;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZIntegerEdit::GetNumber(int& Value)
{
  int TestValue;
  bool Status = UnlimitedGetNumber(TestValue);

  ostringstream Oss;
  if (mValueName.empty())
  {
    Oss << "Value";
  }
  else
  {
    Oss << mValueName;
  }

  if (!Status)
  {
    Oss << " is not a valid number";
  }
  else if (TestValue < mMin)
  {
    Oss << " must be greater than or equal to " << mMin;
    Status = false;
  }
  else if (TestValue > mMax)
  {
    Oss << " must be less than or equal to " << mMax;
    Status = false;
  }

  if (!Status)
  {
    Oss << '.';
    ::wxMessageBox(
      Oss.str().c_str(),
      "Invalid Integer Value",
      wxOK | wxICON_EXCLAMATION,
      this);
    SetFocus();
    SetSelection(0, GetLastPosition());
  }
  else
  {
    Value = TestValue;
  }

  return Status;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZIntegerEdit::IsValueValid(bool DisplayErrorMessage)
{
  int TestValue;
  if (DisplayErrorMessage)
  {
    bool Status = UnlimitedGetNumber(TestValue);
    // Prepare the initial portion of the error message.
    ostringstream Oss;
    if (mValueName.empty())
    {
      Oss << "Value";
    }
    else
    {
      Oss << mValueName;
    }
    if (!Status)
    {
      Oss << " is not a valid number";
    }
    else if (TestValue < mMin)
    {
      Oss << " must be greater than or equal to " << mMin;
      Status = false;
    }
    else if (TestValue > mMax)
    {
      Oss << " must be less than or equal to " << mMax;
      Status = false;
    }
    if (!Status)
    {
      Oss << '.';
      ::wxMessageBox(
        Oss.str().c_str(),
        "Invalid Integer Value",
        wxOK | wxICON_EXCLAMATION,
        this);
      SetFocus();
      SetSelection(0, GetLastPosition());
    }
    return Status;
  }
  bool Status = UnlimitedGetNumber(TestValue);
  if (!Status)
  {
    return false;
  }
  if (TestValue < mMin)
  {
    return false;
  }
  if (TestValue > mMax)
  {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZIntegerEdit::UnlimitedGetNumber(int& Value)
{
  bool Status = true;
  wxString ValueString = GetValue();
  long Test;
  if (!ValueString.ToLong(&Test))
  {
    Status = false;
    Value = 0;
  }
  else
  {
    Value = static_cast<int>(Test);
  }
  return Status;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZIntegerEdit::SetNumber(int Value)
{
  if (Value < mMin)
  {
    Value = mMin;
  }
  if (Value > mMax)
  {
    Value = mMax;
  }
  ostringstream Oss;
  Oss << Value;
  wxString ValueString(Oss.str().c_str());

  // Quickly check to see if the text is really going to change.
  // This reduces flicker in the control and prevents false modification
  // messages from being sent.
  int NewLength = ValueString.Length();
  wxString OldString = GetValue();
  int OldLength = OldString.Length();
  if (
    NewLength > eStringLength ||
    OldLength != NewLength    ||
    OldString != ValueString)
  {
    // Go ahead and change the text without sending a wxWidgets text changing
    // event.
    ChangeValue(ValueString);
  }
}

//-----------------------------------------------------------------------------
// Filter the keys processed by the control.
//-----------------------------------------------------------------------------
void JZIntegerEdit::OnChar(wxKeyEvent& Event)
{
  //-------------------------------------
  // Check for non character adding keys.
  // WARNING: Paste Ctrl-V is a problem!
  // Should check the content and length
  // of the paste, but don't know how!
  //-------------------------------------
  // The (Key >= 1 && Key <= 26) logic was added to to allow control keys
  // through for cut an paste operations inside the control.  The control
  // key/operation pairs include
  //    3  Ctrl-C for copy.
  //   22  Ctrl-V for paste.
  //   24  Ctrl-X for cut.
  int Key = Event.GetKeyCode();
  if (
    (Key == WXK_BACK)   || (Key == WXK_DELETE) ||
    (Key == WXK_LEFT)   || (Key == WXK_RIGHT)  ||
    (Key == WXK_UP)     || (Key == WXK_DOWN)   ||
    (Key == WXK_HOME)   || (Key == WXK_END)    ||
    (Key == WXK_INSERT) ||
    (Key >= 1 && Key <= 26))
  {
    wxTextCtrl::OnChar(Event);
  }

  // Limit the number of characters allowed.
  wxString ValueString = GetValue();
  if (ValueString.Length() >= eStringLength)
  {
    return;
  }

  // Only allow certain keys.
  if ((Key >= '0' && Key <= '9') || Key == '-')
  {
    wxTextCtrl::OnChar(Event);
  }
}
