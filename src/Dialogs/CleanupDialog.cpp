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

#include "CleanupDialog.h"

#include "../Globals.h"
#include "../Help.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZCleanupDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZCleanupDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZCleanupDialog::JZCleanupDialog(
    int& ShortestNote,
    bool& ShortenOverlappingNotes,
    wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Cleanup")),
    mShortestNote(ShortestNote),
    mShortenOverlappingNotes(ShortenOverlappingNotes),
    mpShortestNoteChoice(0),
    mpShortenOverlappingNotesCheckBox(0)
{
  mpShortestNoteChoice = new wxChoice(this, wxID_ANY);
  for (const auto& LimitStepPair : gLimitSteps)
  {
    mpShortestNoteChoice->Append(LimitStepPair.second);
  }

  mpShortenOverlappingNotesCheckBox = new wxCheckBox(
    this,
    wxID_ANY,
    "Shorten overlapping notes");

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Delete notes shorter than:"),
    0,
    wxALIGN_CENTER | wxALL,
    5);

  pTopSizer->Add(mpShortestNoteChoice, 0, wxALIGN_CENTER | wxALL, 5);

  pTopSizer->Add(
    mpShortenOverlappingNotesCheckBox,
    0,
    wxALIGN_CENTER | wxALL,
    10);

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
bool JZCleanupDialog::TransferDataToWindow()
{
  int Selection = 0;
  for (const auto& LimitStepPair : gLimitSteps)
  {
    if (LimitStepPair.first >= mShortestNote)
    {
      break;
    }
    ++Selection;
  }
  mpShortestNoteChoice->SetSelection(Selection);

  mpShortenOverlappingNotesCheckBox->SetValue(mShortenOverlappingNotes);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZCleanupDialog::TransferDataFromWindow()
{
  string SelectedValue = mpShortestNoteChoice->GetStringSelection();
  for (const auto& LimitStepPair : gLimitSteps)
  {
    if (SelectedValue == LimitStepPair.second)
    {
      mShortestNote = LimitStepPair.first;
      break;
    }
  }

  mShortenOverlappingNotes = mpShortenOverlappingNotesCheckBox->GetValue();

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZCleanupDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Cleanup");
}
