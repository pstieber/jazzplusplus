/*
 * Copyright (C) 2004, Patrick Earl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ResourceDialog.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/fs_zip.h>
#include <wx/msgdlg.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

//WX_DEFINE_LIST(jppResourceElementList);

using namespace std;

/******************************************************************************
 * jppResourceElement
 *****************************************************************************/

jppResourceElement::jppResourceElement()
{
  // Set all pointer fields to zero.
  string = 0;
  intptr = 0;
  longptr = 0;
  boolptr = 0;
}


/******************************************************************************
 * jppResourceDialog
 *****************************************************************************/

// Static Members

bool jppResourceDialog::initialized;

void jppResourceDialog::LoadResource(const wxString& xrcfile)
{
  if (!initialized)
  {
    wxFileSystem::AddHandler(new wxZipFSHandler);
    wxXmlResource::Get()->InitAllHandlers();
    initialized = true;
  }

  wxXmlResource::Get()->Load(xrcfile);
}


// Instance Methods

jppResourceDialog::jppResourceDialog(wxWindow* parent, const wxString& name)
  : links()
{
  dialogName = name;

  // The system will report any errors in loading, assuming we don't crash
  // ourselves first.
  dialog = wxXmlResource::Get()->LoadDialog(parent, name);

  // Make the list delete its data items when the jppResourceDialog is deleted.
//  links.DeleteContents(true);
}

jppResourceDialog::~jppResourceDialog()
{
  if (dialog)
  {
    dialog->Destroy();
  }
  for (
    list<jppResourceElement*>::iterator iResourceElement = links.begin();
    iResourceElement != links.end();
    ++iResourceElement)
  {
    jppResourceElement* pResourceElement = *iResourceElement;
    delete pResourceElement;
  }
}

void jppResourceDialog::Attach(const wxString& name, wxString *data)
{
  jppResourceElement *pResourceElement = new jppResourceElement;
  pResourceElement->resource = name;
  pResourceElement->string = data;
//  links.Append(pResourceElement);
  links.push_back(pResourceElement);
}

void jppResourceDialog::Attach(const wxString& name, bool *data)
{
  jppResourceElement *pResourceElement = new jppResourceElement;
  pResourceElement->resource = name;
  pResourceElement->boolptr = data;
//  links.Append(pResourceElement);
  links.push_back(pResourceElement);
}

void jppResourceDialog::Attach(const wxString& name, long *data)
{
  jppResourceElement *pResourceElement = new jppResourceElement;
  pResourceElement->resource = name;
  pResourceElement->longptr = data;
//  links.Append(pResourceElement);
  links.push_back(pResourceElement);
}

void jppResourceDialog::Attach(const wxString& name, int *data)
{
  jppResourceElement *pResourceElement = new jppResourceElement;
  pResourceElement->resource = name;
  pResourceElement->intptr = data;
//  links.Append(pResourceElement);
  links.push_back(pResourceElement);
}

void jppResourceDialog::Attach(const wxString& name, long *data, wxArrayLong a)
{
  jppResourceElement *pResourceElement = new jppResourceElement;
  pResourceElement->resource = name;
  pResourceElement->longptr = data;
  pResourceElement->longarr = a;
  links.push_back(pResourceElement);
}

void jppResourceDialog::Attach(const wxString& name, long *data, long *a)
{
  wxArrayLong arr;
  while (*a != -1)
  {
    arr.Add(*a);
    a++;
  }
  Attach(name, data, arr);
}

void jppResourceDialog::Attach(const wxString& name, int *data, wxArrayInt a)
{
  jppResourceElement *pResourceElement = new jppResourceElement;
  pResourceElement->resource = name;
  pResourceElement->intptr = data;

  for (unsigned i = 0; i < a.GetCount(); ++i)
  {
    pResourceElement->longarr.Add(a[i]);
  }

  links.push_back(pResourceElement);
}

void jppResourceDialog::Attach(const wxString& name, int *data, int *a)
{
  wxArrayInt arr;
  while (*a != -1)
  {
    arr.Add(*a);
    a++;
  }
  Attach(name, data, arr);
}


int jppResourceDialog::ShowModal()
{

  // Don't bother with dialogs that don't exist.  An error message will be
  // produced by the system at some point.
  if (!dialog)
  {
    return wxID_CANCEL;
  }

  // Iterate through the list of attachments.  If we can't find the resource,
  // let the user know.  Upon finding the resource, attempt to load the
  // current value(s) into it.

  for (
    list<jppResourceElement*>::iterator iResourceElement = links.begin();
    iResourceElement != links.end();
    ++iResourceElement)
  {
    jppResourceElement* pResourceElement = *iResourceElement;

    wxWindow* win = wxWindow::FindWindowByName(
      pResourceElement->resource, dialog);

    if (!win)
    {
      wxMessageBox("Unable to locate widget named:\n"
                   "    " + pResourceElement->resource + "\n"
                   "Tried to find it in the dialog named:\n"
                   "    " + dialogName,
                   "Error Finding Resource",
                   wxOK | wxICON_ERROR);
      return wxID_CANCEL;
    }

    if (!LoadData(pResourceElement, win))
    {
      return wxID_CANCEL;
    }
  }

  int res = dialog->ShowModal();

  if (res == wxID_OK)
  {
    // Iterate through list of attached links.  For each link, try and move
    // the data from the wxWidget to the location of the relevant pointer.
    for (
      list<jppResourceElement*>::iterator iResourceElement = links.begin();
      iResourceElement != links.end();
      ++iResourceElement)
    {
      jppResourceElement* pResourceElement = *iResourceElement;

      wxWindow *win = wxWindow::FindWindowByName(
        pResourceElement->resource,
        dialog);

      if (!StoreData(pResourceElement, win))
      {
        return wxID_CANCEL;
      }
    }
  }

  return res;
}

bool jppResourceDialog::LoadData(jppResourceElement *pResourceElement, wxWindow *win)
{
  bool used = 0;

  if (pResourceElement->string)
  {
    if (win->IsKindOf(CLASSINFO(wxTextCtrl)))
    {
      used = 1;
      ((wxTextCtrl*)win)->SetValue(*pResourceElement->string);
    }
  }
  else if (pResourceElement->boolptr)
  {
    if (win->IsKindOf(CLASSINFO(wxCheckBox)))
    {
      used = 1;
      ((wxCheckBox*)win)->SetValue(*pResourceElement->boolptr);
    }
  }
  else if (pResourceElement->longptr || pResourceElement->intptr)
  {
    long value;
    if (pResourceElement->longptr) value = *pResourceElement->longptr;
    if (pResourceElement->intptr) value = *pResourceElement->intptr;

    if (win->IsKindOf(CLASSINFO(wxChoice)))
    {
      wxChoice *choice = (wxChoice*)win;
      if (choice->GetCount() != pResourceElement->longarr.GetCount())
      {
        wxMessageBox(
          "Error handling data for this widget:\n"
          "    dialog = " + dialogName + "\n"
          "    control = " + pResourceElement->resource + "\n"
          "Length mismatch with translation array.",
          "Error Loading Data",
          wxOK | wxICON_ERROR);
      }
      else
      {
        unsigned i;
        for (i = 0; i < pResourceElement->longarr.GetCount(); ++i)
        {
          if (value == pResourceElement->longarr[i]) break;
        }

        if (i == pResourceElement->longarr.GetCount())
        {
          // We couldn't find the value in the list.
          wxMessageBox(
            "Error handling data for this widget:\n"
            "    dialog = " + dialogName + "\n"
            "    control = " + pResourceElement->resource + "\n"
            "Initial value not found in translation array.",
            "Error Loading Data",
            wxOK | wxICON_ERROR);
          return false;
        }

        choice->SetSelection(i);
        used = 1;
      }
    } else if (win->IsKindOf(CLASSINFO(wxSlider)))
    {
      used = 1;
      ((wxSlider*)win)->SetValue((int)value);
    }
  }

  if (!used)
  {
    wxMessageBox("Unable to locate a mapping for this widget:\n"
                 "    dialog = " + dialogName + "\n"
                 "    widget = " + pResourceElement->resource,
                 "Error Loading Data",
                 wxOK | wxICON_ERROR);
  }

  return used;
}

bool jppResourceDialog::StoreData(
  jppResourceElement *pResourceElement,
  wxWindow *win)
{
  // We don't need to do as much error checking here.  Most of that will have
  // been done in LoadData before things have had opportunity to modify
  // themselves.

  if (pResourceElement->string)
  {
    if (win->IsKindOf(CLASSINFO(wxTextCtrl)))
    {
      *(pResourceElement->string) = ((wxTextCtrl*)win)->GetValue();
    }
  }
  else if (pResourceElement->boolptr)
  {
    if (win->IsKindOf(CLASSINFO(wxCheckBox)))
    {
      *(pResourceElement->boolptr) = ((wxCheckBox*)win)->GetValue();
    }
  }
  else if (pResourceElement->longptr || pResourceElement->intptr)
  {
    if (win->IsKindOf(CLASSINFO(wxChoice)))
    {
      int sel = ((wxChoice*)win)->GetSelection();

      if (sel < 0 || sel >= (int)pResourceElement->longarr.GetCount())
      {
        wxMessageBox(
          "Error handling data for this widget:\n"
          "    dialog = " + dialogName + "\n"
          "    control = " + pResourceElement->resource + "\n"
          "Selection value out of range.",
          "Error Loading Data",
          wxOK | wxICON_ERROR);
        return false;
      }

      if (pResourceElement->longptr)
      {
        *(pResourceElement->longptr) = pResourceElement->longarr[sel];
      }
      if (pResourceElement->intptr)
      {
        *(pResourceElement->intptr) = (int)pResourceElement->longarr[sel];
      }

    }
    else if (win->IsKindOf(CLASSINFO(wxSlider)))
    {
      int value = ((wxSlider*)win)->GetValue();

      if (pResourceElement->longptr)
      {
        *(pResourceElement->longptr) = value;
      }
      if (pResourceElement->intptr)
      {
        *(pResourceElement->intptr) = value;
      }
    }
  }

  return true;
}
