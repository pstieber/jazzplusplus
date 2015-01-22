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

#include "KeyOnDialog.h"

#include "../Events.h"
#include "../Globals.h"
#include "../Help.h"
#include "../KeyStringConverters.h"
#include "../Knob.h"
#include "../Project.h"
#include "../Resources.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZKeyOnDialog, wxDialog)

  EVT_KNOB_CHANGED(IDC_KB_VELOCITY, JZKeyOnDialog::OnVelocityChange)

  EVT_KNOB_CHANGED(IDC_KB_OFF_VELOCITY, JZKeyOnDialog::OnOffVelocityChange)

  EVT_KNOB_CHANGED(IDC_KB_CHANNEL, JZKeyOnDialog::OnChannelChange)

  EVT_BUTTON(wxID_HELP, JZKeyOnDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKeyOnDialog::JZKeyOnDialog(
  JZKeyOnEvent* pEvent,
  JZTrack* pTrack,
  wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Key On")),
    mpEvent(pEvent),
    mpPitchEdit(nullptr),
    mpVelocityValue(nullptr),
    mpVelocityKnob(nullptr),
    mpOffVelocityValue(nullptr),
    mpOffVelocityKnob(nullptr),
    mpLengthEdit(nullptr),
    mpChannelValue(nullptr),
    mpChannelKnob(nullptr),
    mpClockEdit(nullptr)
{
  mpPitchEdit = new wxTextCtrl(this, wxID_ANY);

  mpVelocityValue = new wxStaticText(this, wxID_ANY, "000");

  mpVelocityKnob = new JZKnob(this, IDC_KB_VELOCITY, 0, 0, 127);

  mpOffVelocityValue = new wxStaticText(this, wxID_ANY, "000");

  mpOffVelocityKnob = new JZKnob(this, IDC_KB_OFF_VELOCITY, 0, 0, 127);

  mpLengthEdit = new wxTextCtrl(this, wxID_ANY);

  mpChannelValue = new wxStaticText(this, wxID_ANY, "00");

  mpChannelKnob = new JZKnob(this, IDC_KB_CHANNEL, 0, 1, 16);

  mpClockEdit = new wxTextCtrl(this, wxID_ANY);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* pFlexGridSizer;

  pFlexGridSizer = new wxFlexGridSizer(1, 2, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Pitch:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(mpPitchEdit, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 2);

  pFlexGridSizer = new wxFlexGridSizer(2, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Velocity:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpVelocityValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpVelocityKnob, 0, wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Off Velocity:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpOffVelocityValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpOffVelocityKnob, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxALIGN_CENTER);

  pFlexGridSizer = new wxFlexGridSizer(1, 2, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Length:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(mpLengthEdit, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 2);

  pFlexGridSizer = new wxFlexGridSizer(1, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Channel:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpChannelValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpChannelKnob, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 2);

  pFlexGridSizer = new wxFlexGridSizer(1, 2, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Clock:"),
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
bool JZKeyOnDialog::TransferDataToWindow()
{
  string KeyString;
  KeyToString(mpEvent->GetKey(), KeyString);
  mpPitchEdit->ChangeValue(KeyString);

  ostringstream Oss;

  Oss << (int)mpEvent->GetVelocity();
  mpVelocityValue->SetLabel(Oss.str());

  mpVelocityKnob->SetValue(mpEvent->GetVelocity());

  Oss.str("");
  Oss << (int)mpEvent->GetOffVelocity();
  mpOffVelocityValue->SetLabel(Oss.str());

  mpOffVelocityKnob->SetValue(mpEvent->GetOffVelocity());

  wxString LengthString;
  LengthString << mpEvent->GetEventLength();
  mpLengthEdit->ChangeValue(LengthString);

  Oss.str("");
  Oss << (int)mpEvent->GetChannel() + 1;
  mpChannelValue->SetLabel(Oss.str());

  mpChannelKnob->SetValue(mpEvent->GetChannel() + 1);

  string ClockString;
  gpProject->ClockToString(mpEvent->GetClock(), ClockString);
  mpClockEdit->ChangeValue(ClockString);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZKeyOnDialog::TransferDataFromWindow()
{
  wxString KeyString = mpPitchEdit->GetValue();
  mpEvent->SetKey(StringToKey(KeyString));

  mpEvent->SetVelocity(mpVelocityKnob->GetValue());

  mpEvent->SetOffVelocity(mpOffVelocityKnob->GetValue());

  wxString LengthString = mpLengthEdit->GetValue();
  istringstream Iss(LengthString);
  unsigned short Length;
  Iss >> Length;
  mpEvent->SetLength(Length);

  mpEvent->SetChannel(mpChannelKnob->GetValue() - 1);

  wxString ClockString = mpClockEdit->GetValue();
  int Clock = gpProject->StringToClock(ClockString);
  mpEvent->SetClock(Clock);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKeyOnDialog::OnVelocityChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpVelocityValue->SetLabel(Oss.str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKeyOnDialog::OnOffVelocityChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpOffVelocityValue->SetLabel(Oss.str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKeyOnDialog::OnChannelChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpChannelValue->SetLabel(Oss.str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZKeyOnDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Key On Dialog");
}
