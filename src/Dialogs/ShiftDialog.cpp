//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2008-2015 Peter J. Stieber, all rights reserved.
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

#include "ShiftDialog.h"

#include "../Command.h"
#include "../Filter.h"
#include "../Help.h"
#include "../ProjectManager.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZShiftDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZShiftDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZShiftDialog::JZShiftDialog(
  JZEventWindow& EventWindow,
  JZFilter& Filter,
  int Unit,
  int& Shift,
  wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Shift")),
    mFilter(Filter),
    mUnit(Unit),
    mShift(Shift),
    mpStepsSlider(nullptr)
{
  mpStepsSlider = new wxSlider(
    this,
    wxID_ANY,
    mShift,
    -16,
    16,
    wxDefaultPosition,
    wxDefaultSize,
    wxSL_LABELS | wxSL_BOTTOM);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Shift events left or right"),
    0,
    wxCENTER | wxALL,
    5);

  ostringstream Oss;
  Oss << "Snap is currently " << mUnit << " clocks";
  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, Oss.str().c_str()),
    0,
    wxCENTER | wxALL,
    5);

  pTopSizer->Add(mpStepsSlider, 0, wxCENTER | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);
  pButtonSizer->Add(pOkButton, 0, wxALL, 5);
  pButtonSizer->Add(pCancelButton, 0, wxALL, 5);
  pButtonSizer->Add(pHelpButton, 0, wxALL, 5);

  pTopSizer->Add(pButtonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 6);

  SetAutoLayout(true);
  SetSizer(pTopSizer);

  pTopSizer->SetSizeHints(this);
  pTopSizer->Fit(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZShiftDialog::TransferDataToWindow()
{
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZShiftDialog::TransferDataFromWindow()
{
  mShift = mpStepsSlider->GetValue();

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZShiftDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Shift Dialog");
}
