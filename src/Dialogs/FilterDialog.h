//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2008-2013 Peter J. Stieber, all rights reserved.
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

class JZFilter;
class JZIntegerEdit;
class wxCheckBox;
class wxTextCtrl;

//*****************************************************************************
//*****************************************************************************
class JZFilterDialog : public wxDialog
{
  public:

    JZFilterDialog(JZFilter& Filter, wxWindow* pParent);

  private:

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

    void OnHelp(wxCommandEvent& Event);

  private:

    JZFilter& mFilter;

    wxTextCtrl* mpFromTimeEdit;
    wxTextCtrl* mpToTimeEdit;

    JZIntegerEdit* mpFromTrackEdit;
    JZIntegerEdit* mpToTrackEdit;

    wxCheckBox* mpNoteCheckBox;
    JZIntegerEdit* mpNoteMinEdit;
    JZIntegerEdit* mpNoteMaxEdit;

    wxCheckBox* mpPolyAftertouchCheckBox;
    JZIntegerEdit* mpPolyAftertouchMinEdit;
    JZIntegerEdit* mpPolyAftertouchMaxEdit;

    wxCheckBox* mpControllerCheckBox;
    JZIntegerEdit* mpControllerMinEdit;
    JZIntegerEdit* mpControllerMaxEdit;

    wxCheckBox* mpPatchCheckBox;
    JZIntegerEdit* mpPatchMinEdit;
    JZIntegerEdit* mpPatchMaxEdit;

    wxCheckBox* mpPitchCheckBox;
    JZIntegerEdit* mpPitchMinEdit;
    JZIntegerEdit* mpPitchMaxEdit;

    wxCheckBox* mpMeterCheckBox;

    wxCheckBox* mpChannelAftertouchCheckBox;

    wxCheckBox* mpSysExCheckBox;

    wxCheckBox* mpOtherBox;

  DECLARE_EVENT_TABLE();
};
