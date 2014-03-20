//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2010 Peter J. Stieber, all rights reserved.
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

#include "PitchWheelDialog.h"

#include "../Globals.h"
#include "../Help.h"

#include <wx/button.h>
#include <wx/sizer.h>

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZPitchWheelDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZPitchWheelDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPitchWheelDialog::JZPitchWheelDialog(
  JZPitchEvent* pPitchEvent,
  JZTrack* pTrack,
  wxWindow * pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Pitch Wheel"))
{
  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

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
bool JZPitchWheelDialog::TransferDataToWindow()
{
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZPitchWheelDialog::TransferDataFromWindow()
{
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPitchWheelDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Pitch Wheel Dialog");
}
