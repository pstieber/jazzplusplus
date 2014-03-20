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

// JAVE
//
// The proplistdlg class is meant as a convenience wrapper around the wxwin2
// class wxPropertyListForm.
//
// These dialogs aren't exactly beautiful but are meant to make it simpler to
// port simple jazz dialogs to wx2.  Many dialogs in jazz are more advanced
// than can be handled by this class.
//
// In the future this class might be used to implement wxPropertyForms, who
// can be better looking.

#pragma once

#include "DeprecatedWx/proplist.h"
#include "DeprecatedWx/propform.h"

class JZPropertyListDlg
{
  public:

    JZPropertyListDlg(wxString title);
    virtual ~JZPropertyListDlg()
    {
    }
    virtual void AddProperties();
    void Create();
    void CreateModal();
    virtual bool OnClose();
    virtual void OnPropertyChanged(wxProperty* pProperty);

  protected:

    wxPropertySheet* sheet;
    wxPropertyListView* view;
    wxPropertyValidatorRegistry *myListValidatorRegistry;//cannot be a temporary! leads to a crash
    wxString title;
};
