//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2009-2013 Peter J. Stieber, all rights reserved.
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

#include "../CommandUtilities.h"

#include <wx/dialog.h>

class JZKnob;
class JZKnobEvent;
class wxRadioBox;
class wxStaticText;

//*****************************************************************************
//*****************************************************************************
class JZVelocityDialog : public wxDialog
{
  public:

    JZVelocityDialog(
      wxWindow* pParent,
      int& FromValue,
      int& ToValue,
      JEValueAlterationMode& Mode);

  private:

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

    void OnVelocityStartChange(JZKnobEvent& Event);

    void OnVelocityStopChange(JZKnobEvent& Event);

    void OnHelp(wxCommandEvent& Event);

  private:

    int& mFromValue;
    int& mToValue;
    JEValueAlterationMode& mMode;

    JZKnob* mpVelocityStartKnob;
    wxStaticText* mpVelocityStartValue;
    JZKnob* mpVelocityStopKnob;
    wxStaticText* mpVelocityStopValue;

    wxRadioBox* mpModeRadioBox;

  DECLARE_EVENT_TABLE();
};
