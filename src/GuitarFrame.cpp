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

#include "GuitarFrame.h"

#include "GuitarWindow.h"
#include "GuitarSettingsDialog.h"
#include "Help.h"
#include "ProjectManager.h"
#include "Resources.h"

#include <wx/menu.h>

//*****************************************************************************
// Description:
//   This is the guitar frame class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
// Description:
//   This is the guitar frame event table.  The event table connects the
// wxWidgets events with the functions (event handlers) which process them. It
// can be also done at run-time, but for the simple menu events like this the
// static method is much simpler.
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZGuitarFrame, wxFrame)

  EVT_MENU(MEN_CLEAR, JZGuitarFrame::OnClear)

  EVT_MENU(ID_VIEW_SETTINGS, JZGuitarFrame::OnSettings)

  EVT_MENU(wxID_HELP, JZGuitarFrame::OnHelp)

  EVT_MENU(wxID_CLOSE, JZGuitarFrame::OnClose)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
// Description:
//   This is the guitar frame constructor.
//-----------------------------------------------------------------------------
JZGuitarFrame::JZGuitarFrame(wxWindow* pParent)
  : wxFrame(
      pParent,
      wxID_ANY,
      "Guitar board",
      wxPoint(20, 20),
      wxSize(600, 150),
      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
  // set the frame icon
//  SetIcon(wxICON(mondrian));

  wxMenu* pMenu = 0;
  wxMenuBar* pMenuBar = new wxMenuBar;

  pMenu = new wxMenu;
  pMenu->Append(wxID_CLOSE, "&Close");
  pMenuBar->Append(pMenu, "&File");

  pMenu = new wxMenu;
  pMenu->Append(MEN_CLEAR, "C&lear");
  pMenu->Append(ID_VIEW_SETTINGS, "&Settings");
  pMenuBar->Append(pMenu, "&View");

  pMenu = new wxMenu;
  pMenu->Append(wxID_HELP, "&Help");
  pMenuBar->Append(pMenu, "&Help");

  SetMenuBar(pMenuBar);

  mpFretBoardWindow = new JZGuitarWindow(this, wxPoint(0, 0), wxSize(600, 120));
//  mpFretBoardWindow->SetScrollbars(10, 10, 100, 240);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZGuitarFrame::~JZGuitarFrame()
{
  JZProjectManager::Instance()->Detach(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarFrame::PrepareDC(wxDC& Dc)
{
}

//-----------------------------------------------------------------------------
// These are the wxWidgets event handlers.

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarFrame::OnClear(wxCommandEvent& Event)
{
  mpFretBoardWindow->ClearBuffer();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarFrame::OnSettings(wxCommandEvent& Event)
{
  JZGuitarSettingsDialog GuitarSettingsDialog(this);

  if (GuitarSettingsDialog.ShowModal() == wxID_OK)
  {
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarFrame::OnHelp(wxCommandEvent& event)
{
  JZHelp::Instance().ShowTopic("Guitar board");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarFrame::OnClose(wxCommandEvent& Event)
{
  // true is to force the frame to close
  Close(true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZGuitarFrame::ShowPitch(int Pitch)
{
  mpFretBoardWindow->ShowPitch(Pitch);
}

