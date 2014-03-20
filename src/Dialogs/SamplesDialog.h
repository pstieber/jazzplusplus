//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2010 Peter J. Stieber, all rights reserved.
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

#include <wx/dialog.h>

class JZSampleSet;
class wxButton;
class wxListBox;
class wxSlider;
class wxTextCtrl;

//*****************************************************************************
// Description:
//   This is the sample dialog class declaration.
//*****************************************************************************
class JZSamplesDialog : public wxDialog
{
  public:

    JZSamplesDialog(wxWindow* pParent, JZSampleSet& SampleSet);

  private:

    void OnSelectSampleFile(wxCommandEvent& Event);

  private:

    JZSampleSet& mSampleSet;

    wxListBox* mpListBox;
    wxTextCtrl* mpLabelEdit;
    wxTextCtrl* mpFileNameEdit;
    wxButton* mpFileNameBrowseButton;
    wxSlider* mpVolumeSlider;
    wxSlider* mpPanSlider;
    wxSlider* mpPitchSlider;

  DECLARE_EVENT_TABLE()
};
