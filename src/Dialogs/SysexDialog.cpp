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

#include "SysexDialog.h"

#include "../Events.h"
#include "../Globals.h"
#include "../Help.h"
#include "../Project.h"
#include "../Resources.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZSysexDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZSysexDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSysexDialog::JZSysexDialog(
  JZSysExEvent* pSysExEvent,
  JZTrack* pTrack,
  wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("System Exclusive")),
    mpSysExEvent(pSysExEvent),
    mpSysExEdit(nullptr),
    mpClockEdit(nullptr)
{
  mpSysExEdit = new wxTextCtrl(this, wxID_ANY);

  mpClockEdit = new wxTextCtrl(this, wxID_ANY);

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  pTopSizer->Add(
    new wxStaticText(
      this,
      wxID_ANY,
      "Example input: f0 7f 7f 04 01 00 7f f7"),
    0,
    wxCENTER | wxALL,
    5);

  pTopSizer->Add(
    new wxStaticText(
      this,
      wxID_ANY,
      "Any DT1/RQ1 checksums will be corrected"),
    0,
    wxCENTER | wxALL,
    5);

  wxFlexGridSizer* pFlexGridSizer;

  pFlexGridSizer = new wxFlexGridSizer(2, 2, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "SysEx (hex):"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(mpSysExEdit, 0, wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Time:"),
    0,
    wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(mpClockEdit, 0, wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 2);

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
bool JZSysexDialog::TransferDataToWindow()
{
  const unsigned char* pSysExData = mpSysExEvent->GetData();
  unsigned short Length = mpSysExEvent->GetDataLength();

  ostringstream Oss;

  for (unsigned short i = 0; i < Length; ++i)
  {
    Oss << hex << pSysExData[i];
    if (i < Length - 1)
    {
      Oss << ' ';
    }
  }

  mpSysExEdit->ChangeValue(Oss.str());

  string ClockString;
  gpProject->ClockToString(mpSysExEvent->GetClock(), ClockString);
  mpClockEdit->ChangeValue(ClockString);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSysexDialog::TransferDataFromWindow()
{
  wxString KeyString = mpSysExEdit->GetValue();

  //TODO  Need to validate the SysEx message.  The Roland SysEx message is
  // made up of nine parts.  All notation is in hex.
  //
  //  [1]  [2]  [3]  [4]  [5]  [6]      [7]  [8]  [9]
  //  F0   41   10   42   12   40007F   00   41   F7
  //
  // Parts [1], [2] and [9] are part of the MIDI specification and are
  // required by all SysEx messages.  What is in between is specific to the
  // manufacturer, identified by part [2], which is 41h in Roland's case.
  //
  // Part [3] is known as the Device ID.  Most Roland MIDI devices use the
  // default of 10h, but is provided for us to change as we see fit.  The idea
  // behind the Device ID is that if you have more than one MIDI device of the
  // same type in a daisy chain (connected to one another) you can change the
  // Device ID on each of them so that you can send SysEx messages that will
  // be accepted by only one of them, not all.
  //
  // Part [4] is the Model ID.  GS synthesizers will all respond to SysEx with
  // a Model ID of 42h, however they generally have their own Model ID as well.
  //
  // Part [5] specifies whether we are sending (12h) or requesting (11h)
  // information.  If a synth receives a SysEx message it recognizes, it will
  // look this part to determine whether it needs to change an internal
  // setting or reply (with its own SysEx message) with the value of a
  // setting.
  //
  // Part [6] is the start address on which the SysEx intends to act.  It is
  // at this address that you may wish to put a value (or values) or retrieve
  // the current value(s).  It always contains three bytes.  Most synthesizer
  // manuals will provide you with a long "address map" table which explains
  // what lives at each address.  Although daunting on a first perusal, once
  // you understand its function it becomes a wonderful resource.
  //
  // Part [7] has two functions.  If part [5] is 12h (sending data) then part
  // [7] contains the data we are sending and can be one byte or many bytes
  // in length.  If it is 11h (requesting data) then it is the number of bytes
  // we want the synth to send us.
  //
  // Part [8] is the Roland checksum.
  //
  // Also need a function to set SysEx event data.
//  gpSynth->FixSysexCheckSum(Copy->IsSysEx());
//  mpSysExEvent->SetData(???);

  wxString ClockString = mpClockEdit->GetValue();
  int Clock = gpProject->StringToClock(ClockString);
  mpSysExEvent->SetClock(Clock);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSysexDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Sysex Dialog");
}
