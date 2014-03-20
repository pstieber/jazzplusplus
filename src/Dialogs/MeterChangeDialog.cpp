#include "MeterChangeDialog.h"

#include "../Globals.h"
#include "../Help.h"

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/sizer.h>

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZMeterChangeDialog, wxDialog)

  EVT_BUTTON(wxID_HELP, JZMeterChangeDialog::OnHelp)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMeterChangeDialog::JZMeterChangeDialog(wxWindow* pParent)
  : wxDialog(pParent, wxID_ANY, wxString("Meter Change")),
    mpNumeratorComboBox(0),
    mpDenominatorComboBox(0)
{
  mpNumeratorComboBox = new wxComboBox(this, wxID_ANY);
  mpNumeratorComboBox->Append("1");
  mpNumeratorComboBox->Append("2");
  mpNumeratorComboBox->Append("3");
  mpNumeratorComboBox->Append("4");
  mpNumeratorComboBox->Append("5");
  mpNumeratorComboBox->Append("6");
  mpNumeratorComboBox->Append("7");
  mpNumeratorComboBox->Append("8");
  mpNumeratorComboBox->Append("9");
  mpNumeratorComboBox->Append("10");
  mpNumeratorComboBox->Append("11");
  mpNumeratorComboBox->Append("12");
  mpNumeratorComboBox->Append("13");
  mpNumeratorComboBox->Append("14");
  mpNumeratorComboBox->Append("15");
  mpNumeratorComboBox->Append("16");
  mpNumeratorComboBox->Append("17");

  mpDenominatorComboBox = new wxComboBox(this, wxID_ANY);
  mpDenominatorComboBox->Append("2");
  mpDenominatorComboBox->Append("4");
  mpDenominatorComboBox->Append("8");
  mpDenominatorComboBox->Append("16");
  mpDenominatorComboBox->Append("32");

  wxButton* pOkButton = new wxButton(this, wxID_OK, "&OK");
  wxButton* pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton* pHelpButton = new wxButton(this, wxID_HELP, "Help");
  pOkButton->SetDefault();

  wxBoxSizer* pTopSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* pButtonSizer = new wxBoxSizer(wxHORIZONTAL);
  pButtonSizer->Add(pOkButton, 0, wxALL, 5);
  pButtonSizer->Add(pCancelButton, 0, wxALL, 5);
  pButtonSizer->Add(pHelpButton, 0, wxALL, 5);

  pTopSizer->Add(mpNumeratorComboBox, 0, wxALIGN_CENTER | wxALL, 6);
  pTopSizer->Add(mpDenominatorComboBox, 0, wxALIGN_CENTER | wxALL, 6);

  pTopSizer->Add(pButtonSizer, 0, wxALIGN_CENTER | wxBOTTOM, 6);

  SetAutoLayout(true);
  SetSizer(pTopSizer);

  pTopSizer->SetSizeHints(this);
  pTopSizer->Fit(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMeterChangeDialog::OnHelp(wxCommandEvent& Event)
{
  JZHelp::Instance().ShowTopic("Meterchange");
}
