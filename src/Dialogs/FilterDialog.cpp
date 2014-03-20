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

#include "FilterDialog.h"

#include "../Filter.h"
#include "../Help.h"
#include "IntegerEdit.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <string>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZFilterDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZFilterDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZFilterDialog::JZFilterDialog(JZFilter& Filter, wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Event Filter")),
    mFilter(Filter),
    mpFromTimeEdit(0),
    mpToTimeEdit(0),
    mpFromTrackEdit(0),
    mpToTrackEdit(0),
    mpNoteCheckBox(0),
    mpNoteMinEdit(0),
    mpNoteMaxEdit(0),
    mpPolyAftertouchCheckBox(0),
    mpPolyAftertouchMinEdit(0),
    mpPolyAftertouchMaxEdit(0),
    mpControllerCheckBox(0),
    mpControllerMinEdit(0),
    mpControllerMaxEdit(0),
    mpPatchCheckBox(0),
    mpPatchMinEdit(0),
    mpPatchMaxEdit(0),
    mpPitchCheckBox(0),
    mpPitchMinEdit(0),
    mpPitchMaxEdit(0),
    mpMeterCheckBox(0),
    mpChannelAftertouchCheckBox(0),
    mpSysExCheckBox(0),
    mpOtherBox(0)
{
  mpFromTimeEdit = new wxTextCtrl(this, wxID_ANY);
  mpToTimeEdit = new wxTextCtrl(this, wxID_ANY);

  mpFromTrackEdit = new JZIntegerEdit(this, wxID_ANY);
  mpFromTrackEdit->SetValueName("FromTrack");
  mpFromTrackEdit->SetMinAndMax(1, 127);
  mpToTrackEdit = new JZIntegerEdit(this, wxID_ANY);
  mpToTrackEdit->SetValueName("To Track");
  mpToTrackEdit->SetMinAndMax(1, 127);

  mpNoteCheckBox = new wxCheckBox(this, wxID_ANY, "Note");
  mpNoteMinEdit = new JZIntegerEdit(this, wxID_ANY);
  mpNoteMinEdit->SetValueName("Note Minimum");
  mpNoteMinEdit->SetMinAndMax(0, 127);
  mpNoteMaxEdit = new JZIntegerEdit(this, wxID_ANY);
  mpNoteMaxEdit->SetValueName("Note Maximum");
  mpNoteMaxEdit->SetMinAndMax(0, 127);

  mpPolyAftertouchCheckBox = new wxCheckBox(this, wxID_ANY, "Poly Aftertouch");
  mpPolyAftertouchMinEdit = new JZIntegerEdit(this, wxID_ANY);
  mpPolyAftertouchMinEdit->SetValueName("Poly Aftertouch Minimum");
  mpPolyAftertouchMinEdit->SetMinAndMax(0, 127);
  mpPolyAftertouchMaxEdit = new JZIntegerEdit(this, wxID_ANY);
  mpPolyAftertouchMaxEdit->SetValueName("Poly Aftertouch Maximum");
  mpPolyAftertouchMaxEdit->SetMinAndMax(0, 127);

  mpControllerCheckBox = new wxCheckBox(this, wxID_ANY, "Controller");
  mpControllerMinEdit = new JZIntegerEdit(this, wxID_ANY);
  mpControllerMinEdit->SetValueName("Controller Minimum");
  mpControllerMinEdit->SetMinAndMax(0, 127);
  mpControllerMaxEdit = new JZIntegerEdit(this, wxID_ANY);
  mpControllerMaxEdit->SetValueName("Controller Maximum");
  mpControllerMaxEdit->SetMinAndMax(0, 127);

  mpPatchCheckBox = new wxCheckBox(this, wxID_ANY, "Patch");
  mpPatchMinEdit = new JZIntegerEdit(this, wxID_ANY);
  mpPatchMinEdit->SetValueName("Patch Minimum");
  mpPatchMinEdit->SetMinAndMax(0, 127);
  mpPatchMaxEdit = new JZIntegerEdit(this, wxID_ANY);
  mpPatchMaxEdit->SetValueName("Patch Maximum");
  mpPatchMaxEdit->SetMinAndMax(0, 127);

  mpPitchCheckBox = new wxCheckBox(this, wxID_ANY, "Pitch");
  mpPitchMinEdit = new JZIntegerEdit(this, wxID_ANY);
  mpPitchMinEdit->SetValueName("Pitch Minimum");
  mpPitchMinEdit->SetMinAndMax(-8192, 8192);
  mpPitchMaxEdit = new JZIntegerEdit(this, wxID_ANY);
  mpPitchMaxEdit->SetValueName("Pitch Maximum");
  mpPitchMaxEdit->SetMinAndMax(-8192, 8192);

  mpMeterCheckBox = new wxCheckBox(this, wxID_ANY, "Meter");

  mpChannelAftertouchCheckBox =
    new wxCheckBox(this, wxID_ANY, "Channel Aftertouch");

  mpSysExCheckBox = new wxCheckBox(this, wxID_ANY, "SysEx");

  mpOtherBox = new wxCheckBox(this, wxID_ANY, "Other");

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* pFlexGridSizer;

  pFlexGridSizer = new wxFlexGridSizer(2, 4, 4, 2);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "From Time:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpFromTimeEdit,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "To Time:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpToTimeEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "From Track:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpFromTrackEdit,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "To Track:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpToTrackEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 10);

  pFlexGridSizer = new wxFlexGridSizer(5, 5, 4, 2);

  pFlexGridSizer->Add(
    mpNoteCheckBox,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Min:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpNoteMinEdit,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Max:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpNoteMaxEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    mpPolyAftertouchCheckBox,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Min:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpPolyAftertouchMinEdit,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Max:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpPolyAftertouchMaxEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    mpControllerCheckBox,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Min:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpControllerMinEdit,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Max:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpControllerMaxEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    mpPatchCheckBox,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Min:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpPatchMinEdit,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Max:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpPatchMaxEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pFlexGridSizer->Add(
    mpPitchCheckBox,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Min:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpPitchMinEdit,
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    new wxStaticText(this, wxID_ANY, "Max:"),
    0,
    wxALIGN_CENTER_VERTICAL);
  pFlexGridSizer->Add(
    mpPitchMaxEdit,
    0,
    wxALIGN_CENTER_VERTICAL);

  pTopSizer->Add(pFlexGridSizer, 0, wxCENTER | wxALL, 10);

  wxBoxSizer* pCheckBoxSizer = new wxBoxSizer(wxHORIZONTAL);

  pCheckBoxSizer->Add(mpMeterCheckBox, 0, wxALL, 5);

  pCheckBoxSizer->Add(mpChannelAftertouchCheckBox, 0, wxALL, 5);

  pCheckBoxSizer->Add(mpSysExCheckBox, 0, wxALL, 5);

  pCheckBoxSizer->Add(mpOtherBox, 0, wxALL, 5);

  pTopSizer->Add(pCheckBoxSizer, 0, wxCENTER | wxALL, 5);

  wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);
  pButtonSizer->Add(pOkButton, 0, wxALL, 5);
  pButtonSizer->Add(pCancelButton, 0, wxALL, 5);
  pButtonSizer->Add(pHelpButton, 0, wxALL, 5);

  pTopSizer->Add(pButtonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 6);

  SetAutoLayout(true);
  SetSizer(pTopSizer);

  pTopSizer->SetSizeHints(this);
  pTopSizer->Fit(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZFilterDialog::TransferDataToWindow()
{
  string TimeString;

  mFilter.GenerateFromTimeString(TimeString);
  mpFromTimeEdit->ChangeValue(TimeString);

  mFilter.GenerateToTimeString(TimeString);
  mpToTimeEdit->ChangeValue(TimeString);

  mpFromTrackEdit->SetNumber(mFilter.GetFromTrack());
  mpToTrackEdit->SetNumber(mFilter.GetToTrack());

  bool Selected;
  int FromValue, ToValue;

  mFilter.GetFilterEvent(eFilterKeyOn, Selected, FromValue, ToValue);
  mpNoteCheckBox->SetValue(Selected);
  mpNoteMinEdit->SetNumber(FromValue);
  mpNoteMaxEdit->SetNumber(ToValue);

  mFilter.GetFilterEvent(eFilterKeyPressure, Selected, FromValue, ToValue);
  mpPolyAftertouchCheckBox->SetValue(Selected);
  mpPolyAftertouchMinEdit->SetNumber(FromValue);
  mpPolyAftertouchMaxEdit->SetNumber(ToValue);

  mFilter.GetFilterEvent(eFilterControl, Selected, FromValue, ToValue);
  mpControllerCheckBox->SetValue(Selected);
  mpControllerMinEdit->SetNumber(FromValue);
  mpControllerMaxEdit->SetNumber(ToValue);

  mFilter.GetFilterEvent(eFilterProgram, Selected, FromValue, ToValue);
  mpPatchCheckBox->SetValue(Selected);
  mpPatchMinEdit->SetNumber(FromValue);
  mpPatchMaxEdit->SetNumber(ToValue);

  mFilter.GetFilterEvent(eFilterPitch, Selected, FromValue, ToValue);
  mpPitchCheckBox->SetValue(Selected);
  mpPitchMinEdit->SetNumber(FromValue);
  mpPitchMaxEdit->SetNumber(ToValue);

  mpMeterCheckBox->SetValue(mFilter.GetFilterMeter());

  mpChannelAftertouchCheckBox->SetValue(mFilter.GetFilterChannelAftertouch());

  mpSysExCheckBox->SetValue(mFilter.GetFilterSysEx());

  mpOtherBox->SetValue(mFilter.GetFilterOther());

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZFilterDialog::TransferDataFromWindow()
{
  int FromTrack, ToTrack;
  int NoteMin, NoteMax;
  int PolyAftertouchMin, PolyAftertouchMax;
  int ControllerMin, ControllerMax;
  int PatchMin, PatchMax;
  int PitchMin, PitchMax;

  if (
    mpFromTrackEdit->GetNumber(FromTrack) &&
    mpToTrackEdit->GetNumber(ToTrack) &&
    mpNoteMinEdit->GetNumber(NoteMin) &&
    mpNoteMaxEdit->GetNumber(NoteMax) &&
    mpPolyAftertouchMinEdit->GetNumber(PolyAftertouchMin) &&
    mpPolyAftertouchMaxEdit->GetNumber(PolyAftertouchMax) &&
    mpControllerMinEdit->GetNumber(ControllerMin) &&
    mpControllerMaxEdit->GetNumber(ControllerMax) &&
    mpPatchMinEdit->GetNumber(PatchMin) &&
    mpPatchMaxEdit->GetNumber(PatchMax) &&
    mpPitchMinEdit->GetNumber(PitchMin) &&
    mpPitchMaxEdit->GetNumber(PitchMax))
  {
    if (FromTrack > ToTrack)
    {
      ::wxMessageBox(
        "The From Track must be less than or equal to the To Track",
        "Invalid Track Values",
        wxOK | wxICON_EXCLAMATION,
        this);

      mpFromTrackEdit->SetFocus();

      return false;
    }

    if (NoteMin > NoteMax)
    {
      ::wxMessageBox(
        "The Minimum Note must be less than or equal to the Maximum Note",
        "Invalid Note Values",
        wxOK | wxICON_EXCLAMATION,
        this);

      mpNoteMinEdit->SetFocus();

      return false;
    }

    if (PolyAftertouchMin > PolyAftertouchMax)
    {
      ::wxMessageBox(
        "The Minimum Poly Aftertouch must be less than or equal to the Maximum"
        " Poly Aftertouch",
        "Invalid Poly Aftertouch Values",
        wxOK | wxICON_EXCLAMATION,
        this);

      mpPolyAftertouchMinEdit->SetFocus();

      return false;
    }

    if (ControllerMin > ControllerMax)
    {
      ::wxMessageBox(
        "The Minimum Controller must be less than or equal to the Maximum"
        " Controller",
        "Invalid Controller Values",
        wxOK | wxICON_EXCLAMATION,
        this);

      mpControllerMinEdit->SetFocus();

      return false;
    }

    if (PatchMin > PatchMax)
    {
      ::wxMessageBox(
        "The Minimum Patch must be less than or equal to the Maximum Patch",
        "Invalid Patch Values",
        wxOK | wxICON_EXCLAMATION,
        this);

      mpPatchMinEdit->SetFocus();

      return false;
    }

    if (PitchMin > PitchMax)
    {
      ::wxMessageBox(
        "The Minimum Pitch must be less than or equal to the Maximum Pitch",
        "Invalid Pitch Values",
        wxOK | wxICON_EXCLAMATION,
        this);

      mpPitchMinEdit->SetFocus();

      return false;
    }

    string TimeString;

    TimeString = mpFromTimeEdit->GetValue();
    mFilter.SetFromTime(TimeString);

    TimeString = mpToTimeEdit->GetValue();
    mFilter.SetToTime(TimeString);

    mFilter.SetFromTrack(FromTrack);
    mFilter.SetToTrack(ToTrack);

    mFilter.SetFilterEvent(
      eFilterKeyOn,
      mpNoteCheckBox->GetValue(),
      NoteMin,
      NoteMax);

    mFilter.SetFilterEvent(
      eFilterKeyPressure,
      mpPolyAftertouchCheckBox->GetValue(),
      PolyAftertouchMin,
      PolyAftertouchMax);

    mFilter.SetFilterEvent(
      eFilterControl,
      mpControllerCheckBox->GetValue(),
      ControllerMin,
      ControllerMax);

    mFilter.SetFilterEvent(
      eFilterProgram,
      mpPatchCheckBox->GetValue(),
      PatchMin,
      PatchMax);

    mFilter.SetFilterEvent(
      eFilterPitch,
      mpPitchCheckBox->GetValue(),
      PitchMin,
      PitchMax);

    mFilter.SetFilterMeter(mpMeterCheckBox->GetValue());

    mFilter.SetFilterChannelAftertouch(
      mpChannelAftertouchCheckBox->GetValue());

    mFilter.SetFilterSysEx(mpSysExCheckBox->GetValue());

    mFilter.SetFilterOther(mpOtherBox->GetValue());

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZFilterDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Filter Dialog");
}
