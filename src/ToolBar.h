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

#pragma once

#include <wx/gdicmn.h>

class wxFrame;
class wxToolBar;

//*****************************************************************************
// Description:
//   This struct is used to initialize the JZToolBar class, which is a
// wxToolBar wrapper.  An array of JZToolDef items must be created, and the ID
// of the last entry in the array must be set to JZToolBar::eToolBarEnd.
//*****************************************************************************
struct JZToolDef
{
  int mId;
  bool mSticky;
  const void* mpResourceId;
  const char* mpToolTip;
};

//*****************************************************************************
// Description:
//   This is the jazz toolbar class declaration.
//*****************************************************************************
class JZToolBar
{
  public:

    enum TEToolBarFlags
    {
      eToolBarSeparator = -2,
      eToolBarEnd       = -1
    };

    // Description:
    //   Pass the frame on which the toolbar is to be created.  Also pass in
    // the list of toolbar definitions terminated by eToolBarEnd.
    JZToolBar(wxFrame* pFrame, JZToolDef* pToolDef);

    // To retrieve the wxToolBar we're delegating to
    wxToolBar* GetDelegateToolBar();

    // Delegated functions from wxToolBar
    void ToggleTool(int ToolId, const bool Toggle);
    wxSize GetSize() const;
    bool GetToolState(int ToolId) const;

  private:

    wxToolBar* mpToolBar;
};
