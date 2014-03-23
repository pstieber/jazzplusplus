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

#include "SynthesizerSettingsDialog.h"

#include "../Configuration.h"
#include "../Globals.h"
#include "../Help.h"

#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZSynthesizerDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZSynthesizerDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSynthesizerDialog::JZSynthesizerDialog(wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Synthesizer Settings")),
    mpSynthesizerListbox(0),
    mpStartListbox(0)
{
  mpSynthesizerListbox = new wxListBox(this, wxID_ANY);

  for (const auto& StringIntPair : gSynthesizerTypes)
  {
    mpSynthesizerListbox->Append(StringIntPair.first);
  }

  mpStartListbox = new wxListBox(this, wxID_ANY);

  mpStartListbox->Append("Never");
  mpStartListbox->Append("Song Start");
  mpStartListbox->Append("Start Play");

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* pListControlSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* pLeftSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* pRightSizer = new wxBoxSizer(wxVERTICAL);

  pLeftSizer->Add(
    new wxStaticText(this, wxID_ANY, "Synthesizer Type"),
    0,
    wxALL,
    2);
  pLeftSizer->Add(mpSynthesizerListbox, 0, wxGROW | wxALL, 2);

  pRightSizer->Add(
    new wxStaticText(this, wxID_ANY, "Send MIDI Reset"),
    0,
    wxALL,
    2);
  pRightSizer->Add(mpStartListbox, 0, wxALL, 2);

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
bool JZSynthesizerDialog::TransferDataToWindow()
{
  int Selection(0), Index(0);
  for (const auto& StringIntPair : gSynthesizerTypes)
  {
    if (StringIntPair.first == gpConfig->GetStrValue(C_SynthType))
    {
      mOldSynthTypeName = StringIntPair.first;
      Selection = Index;
    }
    ++Index;
  }
  mpSynthesizerListbox->SetSelection(Selection);

  mpStartListbox->SetSelection(gpConfig->GetValue(C_SendSynthReset));

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSynthesizerDialog::TransferDataFromWindow()
{
  string SynthTypeName(mOldSynthTypeName);
  wxString SelectionString = mpSynthesizerListbox->GetStringSelection();
  if (!SelectionString.empty())
  {
    SynthTypeName = SelectionString;
  }

  int Selection = mpStartListbox->GetSelection();
  if (Selection != wxNOT_FOUND)
  {
    gpConfig->Put(C_SendSynthReset, Selection);
  }

  if (mOldSynthTypeName != SynthTypeName)
  {
    gpConfig->Put(C_SynthType, SynthTypeName.c_str());

    ::wxMessageBox(
      "Restart jazz for the synthesizer type change to take effect",
      "Info",
      wxOK);
  }

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSynthesizerDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Synthesizer Type Settings");
}
