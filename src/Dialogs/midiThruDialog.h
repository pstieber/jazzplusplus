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

#ifndef MIDITHRUDIALOG
#define MIDITHRUDIALOG

#include "proplistdlg.h"
#include "../eventwin.h"
#include "../trackwin.h"

#ifndef __PORTING


// ******************************************************************
// Midi thru dialog
// ******************************************************************

class JZMidiThruDlg : public wxForm
{
 public:
  tEventWin *EventWin;
  JZMidiThruDlg(tEventWin *w);
  void EditForm(wxPanel *panel);
  virtual void OnOk();
  virtual void OnCancel();
  virtual void OnHelp();

  JZNamedChoice InputDeviceChoice;
  JZNamedChoice OutputDeviceChoice;
  long InputDevice;
  long OutputDevice;
};

#endif //Porting

#endif // MIDITHRUDIALOG


