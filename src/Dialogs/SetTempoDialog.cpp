//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2010-2015 Peter J. Stieber, all rights reserved.
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

#include "SetTempoDialog.h"

#include "../Events.h"
#include "../Globals.h"
#include "../Help.h"
#include "IntegerEdit.h"
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
BEGIN_EVENT_TABLE(JZSetTempoDialog, wxDialog)

EVT_BUTTON(wxID_HELP, JZSetTempoDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSetTempoDialog::JZSetTempoDialog(
  JZSetTempoEvent* pSetTempoEvent,
  JZTrack* pTrack,
  wxWindow * pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Set Tempo (for track 0)")),
    mpSetTempoEvent(pSetTempoEvent),
    mpTempoEdit(nullptr),
    mpClockEdit(nullptr)
{
  mpTempoEdit = new JZIntegerEdit(this, wxID_ANY);
  mpTempoEdit->SetValueName("Tempo");
  mpTempoEdit->SetMinAndMax(20, 240);

  mpClockEdit = new wxTextCtrl(this, wxID_ANY);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* pFlexGridSizer;

  pFlexGridSizer = new wxFlexGridSizer(2, 2, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Tempo:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpTempoEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Clock:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(mpClockEdit, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 10);

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
bool JZSetTempoDialog::TransferDataToWindow()
{
  mpTempoEdit->SetNumber(mpSetTempoEvent->GetBPM());

  string ClockString;
  gpProject->ClockToString(mpSetTempoEvent->GetClock(), ClockString);
  mpClockEdit->ChangeValue(ClockString);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSetTempoDialog::TransferDataFromWindow()
{
  int BeatsPerMinute;
  if (mpTempoEdit->GetNumber(BeatsPerMinute))
  {
    mpSetTempoEvent->SetBPM(BeatsPerMinute);

    wxString ClockString = mpClockEdit->GetValue();
    int Clock = gpProject->StringToClock(ClockString);
    mpSetTempoEvent->SetClock(Clock);

    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSetTempoDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Set Tempo Dialog");
}
