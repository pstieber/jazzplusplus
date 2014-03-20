//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2010 Peter J. Stieber
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

#ifndef COPYCOMMAND
#define COPYCOMMAND

#include "MouseAction.h"

class JZTrackFrame;
class JZMarkDestination;
class JZCopyDlg;

//*****************************************************************************
// Description:
//   This class handles the GUI copy operation.
//*****************************************************************************
class JZCopyCommand : public wxObject, public JZMouseAction
{
  public:

    static bool RepeatCopy;
    static bool EraseSource;
    static bool EraseDestin;
    static bool InsertSpace;

    int Event(wxMouseEvent& e);
    JZCopyCommand(JZTrackFrame* t);
    void EditForm(wxPanel* panel);
    void OnOk();
    void OnCancel();
    void Execute(int doit);

  private:

    JZTrackFrame* tw;
    int MarkRepeat;
    float StartX, StartY, StopX, StopY;

    JZMarkDestination* Mouse;
    JZCopyDlg* CopyDlg;
};

#endif // Copycommand
