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

#include "copyCommand.h"

#include "TrackFrame.h"
#include "Command.h"

bool JZCopyCommand::EraseDestin = 1;
bool JZCopyCommand::RepeatCopy = 0;
bool JZCopyCommand::EraseSource = 0;
bool JZCopyCommand::InsertSpace = 0;

JZCopyCommand::JZCopyCommand(JZTrackFrame *t)
{
  tw = t;
  MarkRepeat = 0;
  Mouse = new JZMarkDestination(tw->Canvas, tw, 0);
  CopyDlg = 0;
}


int JZCopyCommand::Event(wxMouseEvent& e)
{
#ifdef OBSOLETE
  if (!CopyDlg && Mouse && Mouse->Event(e))
  {
    if (Mouse->Aborted)
    {
      if (CopyDlg)
      {
        CopyDlg->OnCancel();
      }
      else
      {
        Execute(0);
      }
    }
    else if (!MarkRepeat)
    {
      e.Position(&StartX, &StartY);
      tw->Mark((long)StartX, (long)StartY);
      wxDialogBox *panel = new wxDialogBox(tw, "Replicate", FALSE );
      CopyDlg = new JZCopyDlg(this);
      CopyDlg->EditForm(panel);
      panel->Fit();
      panel->Show(TRUE);
    }
    else
    {
      e.Position(&StopX, &StopY);
      Execute(1);
    }
  }
#endif // OBSOLETE
  return 0;
}


void JZCopyCommand::OnOk()
{
  CopyDlg = 0;
  if (RepeatCopy)
  {
    delete Mouse;
    Mouse = new JZMarkDestination(tw->Canvas, tw, 1);
    MarkRepeat = 1;
  }
  else
    Execute(1);
}


void JZCopyCommand::OnCancel()
{
  CopyDlg = 0;
  Execute(0);
}


void JZCopyCommand::Execute(int doit)
{

  if (doit)
  {
    long DestTrack = tw->y2Line((long)StartY);
    long DestClock = tw->x2BarClock((long)StartX);
    JZCommandCopy cpy(tw->mpFilter, DestTrack, DestClock);

    if (RepeatCopy)
      cpy.RepeatClock = tw->x2BarClock((long)StopX, 1);
    cpy.EraseSource = EraseSource;
    cpy.EraseDestin = EraseDestin;
    cpy.InsertSpace = InsertSpace;
    cpy.Execute();
  }

  tw->UnMark();
  tw->MouseAction = 0;
  tw->Redraw();
  tw->NextWin->Redraw();
  delete Mouse;
  delete this;
}
