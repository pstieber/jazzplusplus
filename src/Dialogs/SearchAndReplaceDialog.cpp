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

#include "SearchAndReplaceDialog.h"

#include "../Configuration.h"
#include "../Globals.h"
#include "../Help.h"

#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZSearchAndReplaceDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZSearchAndReplaceDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSearchAndReplaceDialog::JZSearchAndReplaceDialog(
  short From,
  short To,
  wxWindow* pParent)
  : wxDialog(
      pParent,
      wxID_ANY,
      wxString("Search and replace controller types")),
    mFrom(From),
    mTo(To),
    mpFromListBox(0),
    mpToListBox(0)

{
  mpFromListBox = new wxListBox(this, wxID_ANY);
  mpToListBox = new wxListBox(this, wxID_ANY);

  const vector<pair<string, int> >& ControlNames =
    gpConfig->GetControlNames();
  for (
    vector<pair<string, int> >::const_iterator iControlName =
      ControlNames.begin();
    iControlName != ControlNames.end();
    ++iControlName)
  {
    const string& ControlName = iControlName->first;
    if (!ControlName.empty())
    {
      mpFromListBox->Append(ControlName);
      mpToListBox->Append(ControlName);
    }
  }

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Search and replace controller types"),
    0,
    wxALIGN_CENTER | wxALL,
    6);

  wxBoxSizer* pLeftSizer = new wxBoxSizer(wxVERTICAL);
  pLeftSizer->Add(
    new wxStaticText(this, wxID_ANY, "Search for"),
    0,
    wxALL,
    3);
  pLeftSizer->Add(mpFromListBox, 0, wxALIGN_CENTER | wxALL, 3);

  wxBoxSizer* pRightSizer = new wxBoxSizer(wxVERTICAL);
  pRightSizer->Add(
    new wxStaticText(this, wxID_ANY, "Replace with"),
    0,
    wxALL,
    3);
  pRightSizer->Add(mpToListBox, 0, wxALIGN_CENTER | wxALL, 3);

  wxBoxSizer* pListBoxSizer = new wxBoxSizer(wxHORIZONTAL);
  pListBoxSizer->Add(pLeftSizer, 0, wxALL, 3);
  pListBoxSizer->Add(pRightSizer, 0, wxALL, 3);

  pTopSizer->Add(pListBoxSizer, 0, wxALL, 3);

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
void JZSearchAndReplaceDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Search Replace");
}
