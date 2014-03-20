/*
**  Alacrity Midi Sequencer
**
** Some Code Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
**    I don't know why it says "All Rights Reserved" and then is licensed GPL
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/

#include "midiThruDialog.h"

#ifndef __PORTING

JZMidiThruDlg::JZMidiThruDlg(tEventWin *w)
: wxForm( USED_WXFORM_BUTTONS ),
  InputDeviceChoice("Input Device", Midi->GetInputDevices().AsNamedValue(), &InputDevice),
  OutputDeviceChoice("Output Device", Midi->GetOutputDevices().AsNamedValue(), &OutputDevice)
{
  EventWin = w;
  InputDevice = Midi->GetThruInputDevice();
  OutputDevice = Midi->GetThruOutputDevice();
}


void JZMidiThruDlg::OnHelp()
{
  HelpInstance->ShowTopic("Midi Thru");
}


void JZMidiThruDlg::OnCancel()
{
  EventWin->DialogBox = 0;
  wxForm::OnCancel();
}


void JZMidiThruDlg::OnOk()
{
  InputDeviceChoice.GetValue();
  OutputDeviceChoice.GetValue();
  Config(C_ThruInput) = InputDevice;
  Config(C_ThruOutput) = OutputDevice;
  Midi->SetSoftThru(Config(C_SoftThru), InputDevice, OutputDevice);
  Midi->SetHardThru(Config(C_HardThru), InputDevice, OutputDevice);
  EventWin->Redraw();
  EventWin->DialogBox = 0;
  wxForm::OnOk();
}



void JZMidiThruDlg::EditForm(wxPanel *panel)
{
  if (Midi->SupportsMultipleDevices())
  {
    Add(InputDeviceChoice.mkFormItem(300, 50));
    Add(wxMakeFormNewLine());
    Add(OutputDeviceChoice.mkFormItem(300, 50));
    Add(wxMakeFormNewLine());
  }
#ifdef wx_msw
  Add(wxMakeFormBool( "Software MIDI thru", &Config(C_SoftThru) ));
  Add(wxMakeFormNewLine());
#else
  int driver = Config(C_MidiDriver);
  if (driver == C_DRV_OSS || driver == C_DRV_ALSA)
  {
    Add(wxMakeFormBool( "Software MIDI thru", &Config(C_SoftThru) ));
    Add(wxMakeFormNewLine());
  }
  else
  {
    Add(wxMakeFormBool( "Hardware MIDI thru", &Config(C_HardThru) ));
    Add(wxMakeFormNewLine());
  }
#endif
  AssociatePanel(panel);
}

#endif //Porting

