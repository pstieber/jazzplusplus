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

#include "ToolBar.h"

#include <wx/frame.h>

class wxPanel;
class wxToolBar;
struct JZToolDef;

/**
 * a window containing a panel on the top and some ArrayEdits below
 */

class JZRhyArrayEdit;

class JZSliderWindow : public wxFrame
{
  public:

    JZSliderWindow(
      wxWindow* pParent,
      const wxString& Title,
      int geo[4],
      JZToolDef *tdefs = NULL);

    virtual ~JZSliderWindow();

    void Initialize();
    virtual void OnSize(wxSizeEvent& event);
    virtual bool OnClose();

    virtual void AddItems();
    virtual void AddEdits();

#ifdef OBSOLETE
    virtual void OnItem(wxItem& item, wxCommandEvent& event);
#endif

    virtual void ForceRepaint();

  protected:

#ifdef OBSOLETE
    static void ItemCallback(wxItem& item, wxCommandEvent& event);
#endif

    bool in_constructor;

    wxPanel* panel;

//    JZRhyArrayEdit *edits[100];

    wxWindow* sliders[100];
    int n_sliders;
    int sliders_per_row;
    int* geo;

    JZToolBar* mpToolBar;

  DECLARE_EVENT_TABLE()
};
