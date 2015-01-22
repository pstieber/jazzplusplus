//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2009-2015 Peter J. Stieber, all rights reserved.
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

#include "VelocityDialog.h"

#include "../Globals.h"
#include "../Help.h"
#include "../Knob.h"
#include "../Resources.h"

#include <wx/button.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZVelocityDialog, wxDialog)

  EVT_KNOB_CHANGED(
    IDC_KB_VELOCITY_START,
    JZVelocityDialog::OnVelocityStartChange)

  EVT_KNOB_CHANGED(
    IDC_KB_VELOCITY_STOP,
    JZVelocityDialog::OnVelocityStopChange)

  EVT_BUTTON(wxID_HELP, JZVelocityDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZVelocityDialog::JZVelocityDialog(
  wxWindow* pParent,
  int& FromValue,
  int& ToValue,
  JEValueAlterationMode& Mode)
  : wxDialog(pParent, wxID_ANY, wxString("Velocity")),
    mFromValue(FromValue),
    mToValue(ToValue),
    mMode(Mode),
    mpVelocityStartKnob(nullptr),
    mpVelocityStartValue(nullptr),
    mpVelocityStopKnob(nullptr),
    mpVelocityStopValue(nullptr),
    mpModeRadioBox(nullptr)
{
  mpVelocityStartKnob = new JZKnob(this, IDC_KB_VELOCITY_START, 0, 0, 127);
  mpVelocityStartValue = new wxStaticText(this, wxID_ANY, "000");

  mpVelocityStopKnob = new JZKnob(this, IDC_KB_VELOCITY_STOP, 0, 0, 127);
  mpVelocityStopValue = new wxStaticText(this, wxID_ANY, "000");

  wxString Choices[] =
  {
    "Set Values",
    "Add To Value",
    "Subtract From Values"
  };
  mpModeRadioBox = new wxRadioBox(
    this,
    wxID_ANY,
    "Value Application Mode",
    wxDefaultPosition,
    wxDefaultSize,
    3,
    Choices,
    1,
    wxRA_SPECIFY_COLS);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* pFlexGridSizer = new wxFlexGridSizer(2, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Start Velocity:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpVelocityStartValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpVelocityStartKnob, 0, wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Stop  Velocity:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpVelocityStopValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpVelocityStopKnob, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxALIGN_CENTER);

  pTopSizer->Add(
    mpModeRadioBox,
    0,
    wxALIGN_CENTER | wxALL,
    5);

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
bool JZVelocityDialog::TransferDataToWindow()
{
  ostringstream Oss;

  Oss << mFromValue;
  mpVelocityStartValue->SetLabel(Oss.str().c_str());

  mpVelocityStartKnob->SetValue(mFromValue);

  Oss.str("");
  Oss << mToValue;
  mpVelocityStopValue->SetLabel(Oss.str().c_str());

  mpVelocityStopKnob->SetValue(mToValue);

  switch (mMode)
  {
    case eSetValues:
      mpModeRadioBox->SetSelection(0);
      break;
    case eAddValues:
      mpModeRadioBox->SetSelection(1);
      break;
    case eSubtractValues:
      mpModeRadioBox->SetSelection(2);
      break;
  }

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZVelocityDialog::TransferDataFromWindow()
{
  mFromValue = mpVelocityStartKnob->GetValue();

  mToValue = mpVelocityStopKnob->GetValue();

  int Selection = mpModeRadioBox->GetSelection();
  if (Selection == 1)
  {
    mMode = eAddValues;
  }
  else if (Selection == 2)
  {
    mMode = eSubtractValues;
  }
  else
  {
    mMode = eSetValues;
  }

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZVelocityDialog::OnVelocityStartChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpVelocityStartValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZVelocityDialog::OnVelocityStopChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpVelocityStopValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZVelocityDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Velocity");
}
