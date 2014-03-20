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

#include "TextDialog.h"

#include "../Events.h"
#include "../Globals.h"
#include "../Help.h"
#include "../Project.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <string>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZTextDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZTextDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTextDialog::JZTextDialog(
  JZTextEvent* pTextEvent,
  JZTrack* pTrack,
  wxWindow * pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Text")),
    mpTextEvent(pTextEvent),
    mpTextEdit(0),
    mpClockEdit(0)
{
  mpTextEdit = new wxTextCtrl(this, wxID_ANY);
  mpClockEdit = new wxTextCtrl(this, wxID_ANY);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* pFlexGridSizer;

  pFlexGridSizer = new wxFlexGridSizer(2, 2, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Text:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(mpTextEdit, 0, wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Time:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(mpClockEdit, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 2);

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
bool JZTextDialog::TransferDataToWindow()
{
  const unsigned char* pText = mpTextEvent->GetData();

  mpTextEdit->ChangeValue(pText);

  string ClockString;
  gpProject->ClockToString(mpTextEvent->GetClock(), ClockString);
  mpClockEdit->ChangeValue(ClockString);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZTextDialog::TransferDataFromWindow()
{
  wxString Text = mpTextEdit->GetValue();
  mpTextEvent->SetText(Text.c_str());

  wxString ClockString = mpClockEdit->GetValue();
  int Clock = gpProject->StringToClock(ClockString);
  mpTextEvent->SetClock(Clock);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTextDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Text Dialog");
}
