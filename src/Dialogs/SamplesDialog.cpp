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

#include "SamplesDialog.h"

#include "../Audio.h"
#include "../Resources.h"
#include "../Sample.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZSamplesDialog, wxDialog)
  EVT_BUTTON(IDC_BN_SD_FILE_SELECT_BROWSE, JZSamplesDialog::OnSelectSampleFile)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSamplesDialog::JZSamplesDialog(wxWindow* pParent, JZSampleSet& SampleSet)
  : wxDialog(pParent, wxID_ANY, wxString("Samples Settings")),
    mSampleSet(SampleSet),
    mpListBox(0),
    mpLabelEdit(0),
    mpFileNameEdit(0),
    mpFileNameBrowseButton(0),
    mpVolumeSlider(0),
    mpPanSlider(0),
    mpPitchSlider(0)
{
  wxArrayString SampleNames;
  for (int Index = 0; Index < JZSampleSet::eSampleCount; ++Index)
  {
    wxString Entry;
    Entry << Index + 1 << ' ' << SampleSet.GetSampleLabel(Index);
    SampleNames.push_back(Entry);
  }

  mpListBox = new wxListBox(
    this,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize,
    SampleNames,
    wxLB_SINGLE);

  mpLabelEdit = new wxTextCtrl(this, wxID_ANY);

  mpFileNameEdit = new wxTextCtrl(this, wxID_ANY);

  mpFileNameBrowseButton = new wxButton(
    this,
    IDC_BN_SD_FILE_SELECT_BROWSE,
    "Browse...");

  mpVolumeSlider = new wxSlider(
    this,
    wxID_ANY,
    64,
    0,
    127,
    wxDefaultPosition,
    wxDefaultSize,
    wxSL_LABELS);

  mpPanSlider = new wxSlider(
    this,
    wxID_ANY,
    0,
    -63,
    63,
    wxDefaultPosition,
    wxDefaultSize,
    wxSL_LABELS);

  mpPitchSlider = new wxSlider(
    this,
    wxID_ANY,
    0,
    -12,
    12,
    wxDefaultPosition,
    wxDefaultSize,
    wxSL_LABELS);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer* pFlexGridSizer;

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Samples"),
    0,
    wxLEFT | wxALL,
    3);
  pTopSizer->Add(mpListBox, 0, wxGROW | wxALL, 3);

  pFlexGridSizer = new wxFlexGridSizer(1, 2, 4, 2);
  pFlexGridSizer->AddGrowableCol(1);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Sample Label:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpLabelEdit,
    1,
    wxGROW | wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxGROW | wxALL, 5);

  pFlexGridSizer = new wxFlexGridSizer(1, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Sample File Name:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpFileNameEdit,
    1,
    wxGROW | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpFileNameBrowseButton,
    0,
    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxGROW | wxALL, 5);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Volume"),
    0,
    wxCENTER | wxALL,
    3);
  pTopSizer->Add(mpVolumeSlider, 0, wxGROW | wxALL, 3);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Pan"),
    0,
    wxCENTER | wxALL,
    3);
  pTopSizer->Add(mpPanSlider, 0, wxGROW | wxALL, 3);

  pTopSizer->Add(
    new wxStaticText(this, wxID_ANY, "Pitch"),
    0,
    wxCENTER | wxALL,
    3);
  pTopSizer->Add(mpPitchSlider, 0, wxGROW | wxALL, 3);

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
void JZSamplesDialog::OnSelectSampleFile(wxCommandEvent& Event)
{
  int Selection = mpListBox->GetSelection();
  if (Selection != wxNOT_FOUND)
  {
    wxFileDialog FileOpenDialog(
      this,
      "Choose a sound file",
      wxEmptyString,
      wxEmptyString,
      "WAV files (*.wav)|*.wav",
      wxFD_OPEN | wxFD_CHANGE_DIR);
    if (FileOpenDialog.ShowModal() == wxID_OK)
    {
      wxFileName FileName = FileOpenDialog.GetPath();
      mSampleSet[Selection].SetFileName(FileName.GetFullPath());
      mSampleSet[Selection].LoadWav();
      mpFileNameEdit->ChangeValue(FileName.GetFullPath());
      mpLabelEdit->ChangeValue(FileName.GetName());
      wxString Label;
      Label << Selection + 1 << ' ' << FileName.GetFullName();
      mpListBox->SetString(Selection, Label);
    }
  }
}
