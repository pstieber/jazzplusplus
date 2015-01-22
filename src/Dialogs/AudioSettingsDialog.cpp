#include "AudioSettingsDialog.h"

#include "../Audio.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZAudioSettingsDialog::JZAudioSettingsDialog(
  wxWindow* pParent,
  JZSampleSet& SampleSet)
  : wxDialog(pParent, wxID_ANY, wxString("Audio Settings")),
    mSampleSet(SampleSet),
    mpEnableAudioCheckBox(nullptr),
    mpSamplingRateComboBox(nullptr),
    mpStereoCheckBox(nullptr),
    mpSoftwareMidiAudioSyncCheckBox(nullptr)
{
  mpEnableAudioCheckBox = new wxCheckBox(this, wxID_ANY, "Enable Audio");

  wxArrayString SampleRates;
  SampleRates.push_back("8000");
  SampleRates.push_back("11025");
  SampleRates.push_back("22050");
  SampleRates.push_back("44100");

  mpSamplingRateComboBox = new wxComboBox(
    this,
    wxID_ANY,
    "",
    wxDefaultPosition,
    wxDefaultSize,
    SampleRates);

  mpStereoCheckBox = new wxCheckBox(this, wxID_ANY, "Stereo");

  mpSoftwareMidiAudioSyncCheckBox = new wxCheckBox(
    this,
    wxID_ANY,
    "Software MIDI/Audio Sync");

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer* pFlexGridSizer;

  pTopSizer->Add(mpEnableAudioCheckBox, 0, wxALIGN_CENTER | wxALL, 3);

  pFlexGridSizer = new wxFlexGridSizer(1, 2, 4, 2);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Sample Rate"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpSamplingRateComboBox,
    0,
    wxALIGN_CENTER_VERTICAL);
  pTopSizer->Add(pFlexGridSizer, 0, wxALIGN_CENTER | wxALL, 3);

  pTopSizer->Add(mpStereoCheckBox, 0, wxALIGN_CENTER | wxALL, 3);

  pTopSizer->Add(
    mpSoftwareMidiAudioSyncCheckBox,
    0,
    wxALIGN_CENTER | wxALL,
    3);

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
