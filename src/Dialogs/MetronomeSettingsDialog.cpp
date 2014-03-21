//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2008-2013 Peter J. Stieber, all rights reserved.
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

#include "MetronomeSettingsDialog.h"

#include "../Configuration.h"
#include "../Globals.h"
#include "../Help.h"
#include "../Knob.h"
#include "../Metronome.h"
#include "../Resources.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <iostream>
#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZMetronomeSettingsDialog, wxDialog)

  EVT_KNOB_CHANGED(IDC_KB_VOLUME, JZMetronomeSettingsDialog::OnVolumeChange)

  EVT_BUTTON(wxID_HELP, JZMetronomeSettingsDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMetronomeSettingsDialog::JZMetronomeSettingsDialog(
  wxWindow* pParent,
  JZMetronomeInfo& MetronomeInfo)
  : wxDialog(pParent, wxID_ANY, wxString("Metronome Settings")),
    mMetronomeInfo(MetronomeInfo),
    mIndexToName(),
    mIndexToPitch(),
    mPitchToIndex(),
    mKeyNormalName(),
    mKeyAccentedName(),
    mpVelocityKnob(0),
    mpVelocityValue(0),
    mpAccentedCheckBox(0),
    mpNormalListbox(0),
    mpAccentedListbox(0)
{
  int Index = 0;
  const vector<pair<string, int> >& DrumNames = gpConfig->GetDrumNames();
  for (const auto& NameValuePair : gpConfig->GetDrumNames())
  {
    const string& DrumName = NameValuePair.first;
    const int& Value = NameValuePair.second;

    if (!DrumName.empty())
    {
      mIndexToName.push_back(DrumName);
      mIndexToPitch.push_back(Value - 1);
      mPitchToIndex.insert(make_pair(Value - 1, Index++));
    }
  }

  if (mMetronomeInfo.GetKeyNormal() < mPitchToIndex.size())
  {
    mKeyNormalName =
      mIndexToName[mPitchToIndex[mMetronomeInfo.GetKeyNormal()]];
  }

  if (mMetronomeInfo.GetKeyAccented() < mPitchToIndex.size())
  {
    mKeyAccentedName =
      mIndexToName[mPitchToIndex[mMetronomeInfo.GetKeyAccented()]];
  }

  mpVelocityKnob = new JZKnob(this, IDC_KB_VOLUME, 100, 0, 127);

  mpVelocityValue = new wxStaticText(this, wxID_ANY, "127");

  mpAccentedCheckBox = new wxCheckBox(this, wxID_ANY, "Use Accented Click");

  mpNormalListbox = new wxListBox(this, wxID_ANY);

  mpAccentedListbox = new wxListBox(this, wxID_ANY);

  for (const auto& DrumName : mIndexToName)
  {
    mpNormalListbox->Append(DrumName.c_str());
    mpAccentedListbox->Append(DrumName.c_str());
  }

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* pListControlSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Velocity"),
    0,
    wxCENTER | wxALL,
    2);

  pTopSizer->Add(mpVelocityKnob, 0, wxCENTER | wxALL, 2);

  pTopSizer->Add(mpVelocityValue, 0, wxCENTER | wxALL, 2);

  pTopSizer->Add(mpAccentedCheckBox, 0, wxCENTER | wxALL, 2);

  wxBoxSizer* pLeftSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* pRightSizer = new wxBoxSizer(wxVERTICAL);

  pLeftSizer->Add(
    new wxStaticText(this, wxID_ANY, "Normal Click"),
    0,
    wxALL,
    2);
  pLeftSizer->Add(mpNormalListbox, 0, wxGROW | wxALL, 2);

  pRightSizer->Add(
    new wxStaticText(this, wxID_ANY, "Accented Click:"),
    0,
    wxALL,
    2);
  pRightSizer->Add(mpAccentedListbox, 0, wxALL, 2);

  pListControlSizer->Add(pLeftSizer, 0, wxALL, 3);
  pListControlSizer->Add(pRightSizer, 0, wxALL, 3);

  pTopSizer->Add(pListControlSizer, 0, wxCENTER);

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
bool JZMetronomeSettingsDialog::TransferDataToWindow()
{
  mpVelocityKnob->SetValueWithEvent(mMetronomeInfo.GetVelocity());
  mpAccentedCheckBox->SetValue(mMetronomeInfo.IsAccented());

  unsigned Selection, Index;

  Selection = 0;
  Index = 0;
  for (const auto& DrumName : mIndexToName)
  {
    if (DrumName == mKeyNormalName)
    {
      Selection = Index;
      break;
    }
    ++Index;
  }

  if (Selection < mpNormalListbox->GetCount())
  {
    mpNormalListbox->SetSelection(Selection);
  }

  Selection = 0;
  Index = 0;
  for (const auto& DrumName : mIndexToName)
  {
    if (DrumName == mKeyAccentedName)
    {
      Selection = Index;
      break;
    }
    ++Index;
  }

  if (Selection < mpAccentedListbox->GetCount())
  {
    mpAccentedListbox->SetSelection(Selection);
  }

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZMetronomeSettingsDialog::TransferDataFromWindow()
{
  mMetronomeInfo.SetVelocity(static_cast<unsigned char>(
    mpVelocityKnob->GetValue()));
  mMetronomeInfo.SetIsAccented(mpAccentedCheckBox->GetValue());

  int Selection;

  Selection = mpNormalListbox->GetSelection();
  if (Selection != wxNOT_FOUND)
  {
    mMetronomeInfo.SetKeyNormal(mIndexToPitch[Selection]);
  }

  Selection = mpAccentedListbox->GetSelection();
  if (Selection != wxNOT_FOUND)
  {
    mMetronomeInfo.SetKeyAccented(mIndexToPitch[Selection]);
  }

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeSettingsDialog::OnVolumeChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpVelocityValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeSettingsDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Metronome Settings");
}
