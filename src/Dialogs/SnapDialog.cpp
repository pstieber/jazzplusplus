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

#include "SnapDialog.h"

#include "../Globals.h"
#include "../Help.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZSnapDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZSnapDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSnapDialog::JZSnapDialog(int& SnapDenominator, wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Snap")),
    mSnapDenominator(SnapDenominator),
    mpSnapValueChoice(0)
{
  mpSnapValueChoice = new wxChoice(this, wxID_ANY);
  for (const auto& IntNamePair : gLimitSteps)
  {
    mpSnapValueChoice->Append(IntNamePair.second);
  }

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Quantize Cut and Paste Events"),
    0,
    wxALIGN_CENTER | wxALL,
    5);

  pTopSizer->Add(mpSnapValueChoice, 0, wxALIGN_CENTER | wxALL, 5);

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
bool JZSnapDialog::TransferDataToWindow()
{
  int Selection = 0;
  for (const auto& IntNamePair : gLimitSteps)
  {
    if (IntNamePair.first >= mSnapDenominator)
    {
      break;
    }
    ++Selection;
  }
  mpSnapValueChoice->SetSelection(Selection);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSnapDialog::TransferDataFromWindow()
{
  string SelectedValue = mpSnapValueChoice->GetStringSelection();
  for (const auto& IntNamePair : gLimitSteps)
  {
    const string& String = IntNamePair.second;
    if (SelectedValue == String)
    {
      mSnapDenominator = IntNamePair.first;
      break;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSnapDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Snap");
}
