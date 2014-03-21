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

#include "QuantizeDialog.h"

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
BEGIN_EVENT_TABLE(JZQuantizeDialog, wxDialog)

  EVT_KNOB_CHANGED(
    IDC_KB_GROOVE,
    JZQuantizeDialog::OnGrooveChange)

  EVT_KNOB_CHANGED(
    IDC_KB_DELAY,
    JZQuantizeDialog::OnDelayChange)

  EVT_BUTTON(wxID_HELP, JZQuantizeDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZQuantizeDialog::JZQuantizeDialog(
  int& QuantizationStep,
  bool& NoteStart,
  bool& NoteLength,
  int& Groove,
  int& Delay,
  wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Length")),
    mQuantizationStep(QuantizationStep),
    mNoteStart(NoteStart),
    mNoteLength(NoteLength),
    mGroove(Groove),
    mDelay(Delay),
    mpStepSizeComboBox(0),
    mpNoteStartCheckBox(0),
    mpNoteLengthCheckBox(0),
    mpGrooveKnob(0),
    mpGrooveValue(0),
    mpDelayKnob(0),
    mpDelayValue(0)
{
  mpStepSizeComboBox = new wxComboBox(this, wxID_ANY);

  for (const auto& StepNamePair : gQuantizationSteps)
  {
    mpStepSizeComboBox->Append(StepNamePair.second.c_str());
  }

  mpNoteStartCheckBox = new wxCheckBox(this, wxID_ANY, "Note Start");
  mpNoteLengthCheckBox = new wxCheckBox(this, wxID_ANY, "Note Length");
  mpGrooveKnob = new JZKnob(this, IDC_KB_GROOVE, 0, -100, 100);
  mpGrooveValue = new wxStaticText(this, wxID_ANY, "-000");
  mpDelayKnob = new JZKnob(this, IDC_KB_DELAY, 0, -100, 100);
  mpDelayValue = new wxStaticText(this, wxID_ANY, "-000");

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);
  pButtonSizer->Add(pOkButton, 0, wxALL, 5);
  pButtonSizer->Add(pCancelButton, 0, wxALL, 5);
  pButtonSizer->Add(pHelpButton, 0, wxALL, 5);

  pTopSizer->Add(mpStepSizeComboBox, 0, wxALIGN_CENTER | wxALL, 6);

  pTopSizer->Add(mpNoteStartCheckBox, 0, wxALIGN_CENTER | wxALL, 6);

  pTopSizer->Add(mpNoteLengthCheckBox, 0, wxALIGN_CENTER | wxALL, 6);

  wxFlexGridSizer* pFlexGridSizer = new wxFlexGridSizer(2, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Groove:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpGrooveValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpGrooveKnob, 0, wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Delay:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpDelayValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpDelayKnob, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxALIGN_CENTER | wxALL, 5);

  pTopSizer->Add(pButtonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 6);

  SetAutoLayout(true);
  SetSizer(pTopSizer);

  pTopSizer->SetSizeHints(this);
  pTopSizer->Fit(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZQuantizeDialog::TransferDataToWindow()
{
  int Selection = 0;
  for (const auto& StepNamePair : gQuantizationSteps)
  {
    if (StepNamePair.first <= mQuantizationStep)
    {
      break;
    }
    ++Selection;
  }
  mpStepSizeComboBox->SetSelection(Selection);

  mpNoteStartCheckBox->SetValue(mNoteStart);
  mpNoteLengthCheckBox->SetValue(mNoteLength);

  ostringstream Oss;

  Oss << mGroove;
  mpGrooveValue->SetLabel(Oss.str().c_str());
  mpGrooveKnob->SetValue(mGroove);

  Oss.str("");
  Oss << mDelay;
  mpDelayValue->SetLabel(Oss.str().c_str());
  mpDelayKnob->SetValue(mDelay);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZQuantizeDialog::TransferDataFromWindow()
{
  string SelectedValue = mpStepSizeComboBox->GetValue();
  for (const auto& StepNamePair : gQuantizationSteps)
  {
    if (SelectedValue == StepNamePair.second)
    {
      mQuantizationStep = StepNamePair.first;
      break;
    }
  }

  mNoteStart = mpNoteStartCheckBox->GetValue();
  mNoteLength = mpNoteLengthCheckBox->GetValue();
  mGroove = mpGrooveKnob->GetValue();
  mDelay = mpDelayKnob->GetValue();

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZQuantizeDialog::OnGrooveChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpGrooveValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZQuantizeDialog::OnDelayChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpDelayValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZQuantizeDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Quantize");
}
