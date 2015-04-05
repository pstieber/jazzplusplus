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

#include "GuitarSettingsDialog.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZGuitarSettingsDialog, wxDialog)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZGuitarSettingsDialog::JZGuitarSettingsDialog(wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Guitar Settings")),
    mpChordModeCheckBox(nullptr),
    mpBassGuitarCheckBox(nullptr),
    mpShowOctavesCheckBox(nullptr)//,
//    mpFretCountEdit(nullptr)
{
  mpChordModeCheckBox = new wxCheckBox(this, wxID_ANY, "Chord Mode");

  mpBassGuitarCheckBox = new wxCheckBox(this, wxID_ANY, "Bass Guitar");

  mpShowOctavesCheckBox = new wxCheckBox(this, wxID_ANY, "Show Octaves");

//  mpFretCountEdit = new wxTextCtrl(this, wxID_ANY, "");

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* pCheckBoxSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

  pCheckBoxSizer->Add(
    mpChordModeCheckBox,
    0,
    wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL,
    5);
  pCheckBoxSizer->Add(
    mpBassGuitarCheckBox,
    0,
    wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL,
    5);
  pCheckBoxSizer->Add(
    mpShowOctavesCheckBox,
    0,
    wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL,
    5);

  pTopSizer->Add(pCheckBoxSizer, 1, wxGROW | wxALL, 10);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  pOkButton->SetDefault();
  pButtonsSizer->Add(pOkButton, 0, wxALL, 10);
  pButtonsSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 10);
  pButtonsSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 10);

  pTopSizer->Add(pButtonsSizer, 0, wxALIGN_CENTER);

  SetAutoLayout(true);
  SetSizer(pTopSizer);

  pTopSizer->SetSizeHints(this);
  pTopSizer->Fit(this);
}
