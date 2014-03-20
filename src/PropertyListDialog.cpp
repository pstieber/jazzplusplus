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

#include "PropertyListDialog.h"

#include <cstdlib>
#include <ctype.h>
#include <string.h>

using namespace std;

////////////////////////////////////////////////////////////////
// just wrap the wxPropertyListView so we can intercept the callbacks
class myproplistview : public wxPropertyListView
{
  public:

    myproplistview(JZPropertyListDlg* parent);

    // The OnOk and OnClose handlers are never called, only OnClose, which is
    // bad.  OnClose and OnOk are wrongly documented i think, these are the
    // protos defined in the wxwin src these methods arent even virtual, so i
    // dont know why even OnClose is called. it must be because of event table

    // so i guess the right way is to override wxPropertyListFrame instead,
    // and make evt macros for the buttons like this:

    // BEGIN_EVENT_TABLE(wxPropertyListDialog, wxDialog)
    //   EVT_BUTTON(wxID_CANCEL, wxPropertyListDialog::OnCancel)
    //   EVT_CLOSE(wxPropertyListDialog::OnCloseWindow)
    // END_EVENT_TABLE()

    // BEGIN_EVENT_TABLE(wxPropertyListFrame, wxFrame)
    //   EVT_CLOSE(wxPropertyListFrame::OnCloseWindow)
    // END_EVENT_TABLE()

    // the OnCloseWindow should probably be also in the overriden event table,
    // since event tables don't seem to be inherited.

    // This means all OnClose methods should be renamed onok. this should be
    // called from the JZPropertyListDlg::onok, so then there wouldnt be need
    // for the myproplist class, for this purpose at least

    // or it might be possible to still override here, with the proper evt tbl
    virtual bool OnClose();
    virtual void OnOk(wxCommandEvent& event);
    virtual void OnCancel(wxCommandEvent& event);

    virtual void OnPropertyChanged(wxProperty* pProperty);

  protected:

    JZPropertyListDlg* parent;

};

myproplistview::myproplistview(JZPropertyListDlg* pParent)
  : wxPropertyListView(
      NULL,
      wxPROP_BUTTON_OK | wxPROP_BUTTON_CANCEL | wxPROP_BUTTON_CHECK_CROSS |
        wxPROP_DYNAMIC_VALUE_FIELD | wxPROP_PULLDOWN | wxPROP_SHOWVALUES)
{
  parent = pParent;
}

void myproplistview::OnPropertyChanged(wxProperty* pProperty)
{
  cout << "propchange " << pProperty->GetValue().StringValue() << endl;
  parent->OnPropertyChanged(pProperty);
}

void myproplistview::OnOk(wxCommandEvent& event)
{
  cout << "myproplistview::OnOk" << endl;
  wxPropertyListView::OnOk(event);
}

void myproplistview::OnCancel(wxCommandEvent& event)
{
  cout << "myproplistview::OnCancel" << endl;
  wxPropertyListView::OnCancel(event);
}

// propagate the close event to the parent
bool myproplistview::OnClose()
{
  cout << "myproplistview::OnClose" << endl;

  parent->OnClose();

  // i guess we should call the base class OnClose also

  return false;
}

void JZPropertyListDlg::OnPropertyChanged(wxProperty* pProperty)
{
  cout << "JZPropertyListDlg::OnPropertyChanged parent propchange" << endl;
}

// PAT - Started adding CreateModal.  It doesn't end the modal dialog anywhere
// yet.  When the dialog is closed, it appears to call
// myproplistview::OnClose, but it doesn't call JZPropertyListDlg::OnClose for
// some reason.

void JZPropertyListDlg::CreateModal()
{
  sheet = new wxPropertySheet;
  view = new myproplistview(this);

  view->AddRegistry(myListValidatorRegistry);

  // addproperties is meant to be virtual and cannot be called in constructor
  AddProperties();

  wxPropertyListDialog *propDialog = new wxPropertyListDialog(
    view,
    NULL,
    title,
    wxDefaultPosition,
    wxSize(400, 500));
  view->ShowView(sheet, (wxPanel*)propDialog);

  propDialog->Centre(wxBOTH);
  propDialog->ShowModal();
}

void JZPropertyListDlg::Create()
{
  sheet = new wxPropertySheet;
  view = new myproplistview(this);

//  wxDialog *propDialog = new wxPropertyListDialog(
//    view,
//    NULL,
//    title,
//    wxDefaultPosition,
//    wxSize(300, 200),
//    wxDEFAULT_DIALOG_STYLE | wxDIALOG_MODELESS);

  view->AddRegistry(myListValidatorRegistry);

  // addproperties is meant to be virtual and cannot be called in constructor
  AddProperties();

//  pView->ShowView(sheet, (wxPanel *)propDialog);
//  propDialog->Centre(wxBOTH);
//  //  propDialog->Show(true);
//  propDialog->Show(true);

  wxPropertyListFrame* propFrame = new wxPropertyListFrame(
    view,
    NULL,
    title,
    wxDefaultPosition,
    wxSize(400, 500));

  propFrame->Initialize();

  view->ShowView(sheet, propFrame->GetPropertyPanel());

  propFrame->Centre(wxBOTH);
  propFrame->Show(true);
}

JZPropertyListDlg::JZPropertyListDlg(wxString title)
{
  this->title = title;

  // The validators can be registered once for all.
//  JZPropertyListDlg::myListValidatorRegistry = 0;

//  if (myListValidatorRegistry == 0)
//  {
    myListValidatorRegistry=new wxPropertyValidatorRegistry();
    myListValidatorRegistry->RegisterValidator(
      (wxString)"string",
      new wxStringListValidator);
    myListValidatorRegistry->RegisterValidator(
      (wxString)"real",
      new wxRealListValidator);
    myListValidatorRegistry->RegisterValidator(
      (wxString)"integer",
      new wxIntegerListValidator);
    myListValidatorRegistry->RegisterValidator(
      (wxString)"bool",
      new wxBoolListValidator);
    myListValidatorRegistry->RegisterValidator(
      (wxString)"stringlist",
      new wxListOfStringsListValidator);
//  }
}

bool JZPropertyListDlg::OnClose()
{
  cout << "JZPropertyListDlg::OnClose" << endl;
  return false;
}

// add properties in subclasses here
void JZPropertyListDlg::AddProperties()
{
  cout
    << "JZPropertyListDlg::AddProperties should never be called, override!"
    << endl;
}
