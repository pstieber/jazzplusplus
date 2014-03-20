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

#ifndef COPYRIGHTDIALOG
#define COPYRIGHTDIALOG

#include "proplistdlg.h"

// new JZCopyrightDlg using new baseclass JZPropertyListDlg
class JZCopyrightDlg : public JZPropertyListDlg
{
  public:

    JZCopyrightDlg(JZSong* song);
    virtual void AddProperties();
    virtual bool OnClose();

  protected:

    JZSong* pSong;
    wxProperty* pCopyrightProp;
    char* pString;
};

#endif //COPYRIGHTDIALOG
