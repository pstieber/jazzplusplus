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

#include "MidiChannelDialog.h"

#include "../Globals.h"
#include "../Help.h"
#include "../Knob.h"
#include "../Resources.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZMidiChannelDialog, wxDialog)

  EVT_KNOB_CHANGED(
    IDC_KB_MIDI_CHANNEL,
    JZMidiChannelDialog::OnMidiChannelChange)

  EVT_BUTTON(wxID_HELP, JZMidiChannelDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMidiChannelDialog::JZMidiChannelDialog(int& MidiChannel, wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Set MIDI Channel")),
    mMidiChannel(MidiChannel),
    mpMidiChannelKnob(nullptr),
    mpMidiChannelValue(nullptr)
{
  mpMidiChannelKnob = new JZKnob(
    this,
    IDC_KB_MIDI_CHANNEL,
    0,
    0,
    16);
  mpMidiChannelValue = new wxStaticText(this, wxID_ANY, "00");

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* pFlexGridSizer = new wxFlexGridSizer(1, 3, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "MIDI Channel:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpMidiChannelValue,
    0,
    wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE);
  pFlexGridSizer->Add(mpMidiChannelKnob, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxALIGN_CENTER);

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
bool JZMidiChannelDialog::TransferDataToWindow()
{
  ostringstream Oss;

  Oss << mMidiChannel;
  mpMidiChannelValue->SetLabel(Oss.str().c_str());

  mpMidiChannelKnob->SetValue(mMidiChannel);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZMidiChannelDialog::TransferDataFromWindow()
{
  mMidiChannel = mpMidiChannelKnob->GetValue();

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMidiChannelDialog::OnMidiChannelChange(JZKnobEvent& Event)
{
  int Value = Event.GetValue();
  ostringstream Oss;
  Oss << Value;
  mpMidiChannelValue->SetLabel(Oss.str().c_str());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMidiChannelDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Set MIDI Channel");
}
