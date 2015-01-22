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

#include "TransposeDialog.h"

#include "../Globals.h"
#include "../Help.h"
#include "../Knob.h"
#include "../Resources.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZTransposeDialog, wxDialog)

  EVT_KNOB_CHANGED(
    IDC_KB_AMOUNT,
    JZTransposeDialog::OnAmountChange)

  EVT_BUTTON(wxID_HELP, JZTransposeDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTransposeDialog::JZTransposeDialog(
  int CurrentScale,
  int Notes,
  int Scale,
  bool FitIntoScale,
  wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Transpose")),
    mNotes(Notes),
    mScale(Scale),
    mFitIntoScale(FitIntoScale),
    mpAmountKnob(nullptr),
    mpAmountValue(nullptr),
    mpFitIntoScaleCheckBox(nullptr),
    mpScaleNamesComboBox(nullptr)
{
  wxString CurrentSelectionText;
  CurrentSelectionText
    << "Current selection looks like " << gScaleNames[CurrentScale + 2].first;

  mpAmountKnob = new JZKnob(this, IDC_KB_AMOUNT, 0, -12, 12);
  mpAmountValue = new wxStaticText(this, wxID_ANY, "-00");

  mpFitIntoScaleCheckBox = new wxCheckBox(this, wxID_ANY, "Fit into Scale");

  mpScaleNamesComboBox = new wxComboBox(this, wxID_ANY);

  for (const auto& StringIntPair : gScaleNames)
  {
    mpScaleNamesComboBox->Append(StringIntPair.first);
  }

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, CurrentSelectionText),
    0,
    wxALIGN_CENTER | wxALL,
    5);

  wxFlexGridSizer* pFlexGridSizer = new wxFlexGridSizer(1, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Amount:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpAmountValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpAmountKnob, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxALIGN_CENTER | wxALL, 5);

  pTopSizer->Add(mpFitIntoScaleCheckBox, 0, wxALIGN_CENTER | wxALL, 5);

  pTopSizer->Add(mpScaleNamesComboBox, 0, wxALIGN_CENTER | wxALL, 5);

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
bool JZTransposeDialog::TransferDataToWindow()
{
  mpFitIntoScaleCheckBox->SetValue(mFitIntoScale);

  ostringstream Oss;

  Oss << mNotes;
  mpAmountValue->SetLabel(Oss.str().c_str());
  mpAmountKnob->SetValue(mNotes);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZTransposeDialog::TransferDataFromWindow()
{
  mFitIntoScale = mpFitIntoScaleCheckBox->GetValue();

  mNotes = mpAmountKnob->GetValue();

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTransposeDialog::OnAmountChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpAmountValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTransposeDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Transpose");
}
