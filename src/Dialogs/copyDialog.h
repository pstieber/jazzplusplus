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

#ifndef COPYDIALOG
#define COPYDIALOG

#include "../commands/copyCommand.h"

#ifndef __PORTING

class JZCopyDlg : public wxForm
{
  JZCopyCommand *cp;

  public:
    JZCopyDlg(JZCopyCommand *c)
      : wxForm( USED_WXFORM_BUTTONS )
    {
      cp = c;
    }

    void OnOk()
    {
      cp->OnOk();
      wxForm::OnOk();
    }

    void OnCancel()
    {
      cp->OnCancel();
      wxForm::OnCancel();
    }

    void OnHelp();

    void EditForm(wxPanel *panel);
};

#endif // Porting

#endif // COPYDIALOG
