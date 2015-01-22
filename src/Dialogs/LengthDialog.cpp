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

#include "LengthDialog.h"

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
BEGIN_EVENT_TABLE(JZLengthDialog, wxDialog)

  EVT_KNOB_CHANGED(
    IDC_KB_LENGTH_START,
    JZLengthDialog::OnLengthStartChange)

  EVT_KNOB_CHANGED(
    IDC_KB_LENGTH_STOP,
    JZLengthDialog::OnLengthStopChange)

  EVT_BUTTON(wxID_HELP, JZLengthDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZLengthDialog::JZLengthDialog(
  wxWindow* pParent,
  int TicksPerQuarter,
  int& FromValue,
  int& ToValue,
  JEValueAlterationMode& Mode)
  : wxDialog(pParent, wxID_ANY, wxString("Length")),
    mFromValue(FromValue),
    mToValue(ToValue),
    mMode(Mode),
    mpLengthStartKnob(nullptr),
    mpLengthStartValue(nullptr),
    mpLengthStopKnob(nullptr),
    mpLengthStopValue(nullptr),
    mpModeRadioBox(nullptr)
{
  mpLengthStartKnob = new JZKnob(
    this,
    IDC_KB_LENGTH_START,
    0,
    0,
    4 * TicksPerQuarter);
  mpLengthStartValue = new wxStaticText(this, wxID_ANY, "000");

  mpLengthStopKnob = new JZKnob(
    this,
    IDC_KB_LENGTH_STOP,
    0,
    0,
    4 * TicksPerQuarter);
  mpLengthStopValue = new wxStaticText(this, wxID_ANY, "000");

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

  wxString String;

  String << "Ticks per Quarter: " << TicksPerQuarter;

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, String),
    0,
    wxALIGN_CENTER | wxALL,
    5);

  wxFlexGridSizer* pFlexGridSizer = new wxFlexGridSizer(2, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Start Length:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpLengthStartValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpLengthStartKnob, 0, wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Stop  Length:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpLengthStopValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpLengthStopKnob, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxALIGN_CENTER | wxALL, 5);

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
bool JZLengthDialog::TransferDataToWindow()
{
  ostringstream Oss;

  Oss << mFromValue;
  mpLengthStartValue->SetLabel(Oss.str().c_str());

  mpLengthStartKnob->SetValue(mFromValue);

  Oss.str("");
  Oss << mToValue;
  mpLengthStopValue->SetLabel(Oss.str().c_str());

  mpLengthStopKnob->SetValue(mToValue);

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
bool JZLengthDialog::TransferDataFromWindow()
{
  mFromValue = mpLengthStartKnob->GetValue();

  mToValue = mpLengthStopKnob->GetValue();

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
void JZLengthDialog::OnLengthStartChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpLengthStartValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZLengthDialog::OnLengthStopChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpLengthStopValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZLengthDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Length");
}
