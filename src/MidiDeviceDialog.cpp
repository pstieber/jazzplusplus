//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2013 Peter J. Stieber
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

#include "MidiDeviceDialog.h"

#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/sizer.h>

using namespace std;

//*****************************************************************************
// Description:
//   This is the MIDI device selection dialog class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZMidiDeviceDialog, wxDialog)
  EVT_BUTTON(wxID_OK, JZMidiDeviceDialog::OnOK)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMidiDeviceDialog::JZMidiDeviceDialog(
  const vector<pair<wxString, int> >& MidiDevices,
  int& DeviceIndex,
  wxWindow* pParent,
  const wxString& Title)
  : wxDialog(pParent, wxID_ANY, Title),
    mDeviceIndex(DeviceIndex),
    mpMidiDeviceListBox(nullptr)
{
  mpMidiDeviceListBox = new wxListBox(this, wxID_ANY);

  for (const auto& StringIntPair : MidiDevices)
  {
    mpMidiDeviceListBox->Append(StringIntPair.first);
  }

  if (mDeviceIndex < static_cast<int>(mpMidiDeviceListBox->GetCount()))
  {
    mpMidiDeviceListBox->SetSelection(mDeviceIndex);
  }
  else
  {
    mpMidiDeviceListBox->SetSelection(0);
  }

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  pTopSizer->Add(mpMidiDeviceListBox, 0, wxALIGN_CENTER | wxBOTTOM, 6);

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
void JZMidiDeviceDialog::OnOK(wxCommandEvent& Event)
{
  int Selection = mpMidiDeviceListBox->GetSelection();
  if (Selection == wxNOT_FOUND)
  {
    mDeviceIndex = 0;
  }
  else
  {
    mDeviceIndex = Selection;
  }

  // Let wxWidgets do the rest.
  Event.Skip();
}
