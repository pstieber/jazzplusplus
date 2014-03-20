//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2009-2013 Peter J. Stieber, all rights reserved.
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

#include <wx/dialog.h>

class JZKnob;
class JZKnobEvent;
class wxCheckBox;
class wxComboBox;
class wxStaticText;

//*****************************************************************************
//*****************************************************************************
class JZTransposeDialog : public wxDialog
{
  public:

    JZTransposeDialog(
      int CurrentScale,
      int Notes,
      int Scale,
      bool FitIntoScale,
      wxWindow* pParent);

  private:

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

    void OnAmountChange(JZKnobEvent& Event);

    void OnHelp(wxCommandEvent& Event);

  private:

    int& mNotes;
    int& mScale;
    bool& mFitIntoScale;

    JZKnob* mpAmountKnob;
    wxStaticText* mpAmountValue;
    wxCheckBox* mpFitIntoScaleCheckBox;
    wxComboBox* mpScaleNamesComboBox;

  DECLARE_EVENT_TABLE();
};
