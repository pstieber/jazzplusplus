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

#ifndef TRC_INTEGEREDIT_H
#define TRC_INTEGEREDIT_H

#include <wx/textctrl.h>

#include <string>

//*****************************************************************************
// Description:
//   This is the integer edit class declaration.  This is a control class
// that is used to edit integer values with minimum and maximum value
// checking.
//*****************************************************************************
class JZIntegerEdit : public wxTextCtrl
{
  public:

    enum TEConstants
    {
      eStringLength = 11
    };

    JZIntegerEdit(
      wxWindow* pParent,
      wxWindowID Id,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxDefaultSize,
      long WindowStyle = 0,
      const wxValidator& Validator = wxDefaultValidator,
      const wxString& Name = wxTextCtrlNameStr);

    void SetMinAndMax(int Min, int Max);

    void SetValueName(const std::string& ValueName);

    virtual bool GetNumber(int& Value);

    virtual void SetNumber(int Value);

    virtual bool IsValueValid(bool DisplayErrorMessage = true);

  protected:

    void OnChar(wxKeyEvent& Event);

    bool UnlimitedGetNumber(int& Value);

  protected:

    int mMin;

    int mMax;

    std::string mValueName;

  DECLARE_EVENT_TABLE()
};

#endif // !defined(TRC_INTEGEREDIT_H)
