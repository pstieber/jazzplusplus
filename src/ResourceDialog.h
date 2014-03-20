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

#pragma once

#include <list>

#include <wx/dynarray.h>
#include <wx/string.h>

class wxDialog;
class wxWindow;

/// Used by jppResourceDialog to store "Attach" entries.

/** This class links together a named resource and a pointer to a data item.
    It will be used to retrieve the data from the named resource and store that
    data into the memory location represented by the pointer.  The pointer
    fields should all be set to zero, except the one that needs to be filled.
    The constructor will set all the fields to zero. */

class jppResourceElement
{
 public:
  /// ID of the widget in the XRC resource.
  wxString resource;

  /// Reference to wxString data.
  wxString *string;
  /// Reference to bool data.
  bool *boolptr;
  /// Reference to long data.
  long *longptr;
  /// Reference to int data.
  int *intptr;

  /// List of longs for mapping selection indices.
  wxArrayLong longarr;

  /// Sets all pointer entries to zero.
  jppResourceElement();
};


/* Create a list that can contain jppResourceElements. */

//WX_DECLARE_LIST(jppResourceElement, jppResourceElementList);


/// Implements XRC (XML resource) dialog system.
/** This class is used to create a dialog from a XRC XML file.  The dialog
    should be configured to return wxID_OK when the dialog contents are
    accepted.  Normally, you would place OK and Cancel buttons on the dialog.
    Those buttons would have IDs of wxID_OK and wxID_CANCEL.

    The LoadResource function is used to initialize the system and to load
    the various resource files.  It should be called once for each resource
    file that needs loaded from the filesystem.  Generally, this would be
    done at application init time.

    A new dialog based on a resource can be created using the jppResourceDialog
    constructor.  Pass in the parent of the dialog and the ID of the dialog
    from the XRC file.  If the jppResourceDialog creation is too slow, try
    pre-loading any dialog on application init.  This appears to load stuff
    needed by the system, making subsequent dialog displays much faster.

    Once the dialog is created, you will likely want to run one or more of the
    Attach methods to attach certain named XRC resources to your own
    variables.  Once a variable is attached, any accepted changes to the dialog
    will cause the appropriate change in the variable.

    ShowModal() is used to spawn the dialog in a modal fashion, updating the
    variables before it returns.  Just as wxDialog::ShowModal(), it will return
    the result of the dialog box.

    Here is an example:

    \code
    // At application init time:
    tResourceDialog::LoadResource("mydialog.xrc");

    // At dialog creation time:
    tResourceDialog dialog(parent_window, "myDialog");
    dialog.Attach("myTextWidget",&someWxString);
    dialog.Attach("myCheckWidget",&someBool);
    dialog.ShowModal();
    \endcode

    The XRC file itself would contain a dialog with an ID of "myDialog".  That
    dialog would contain a wxTextCtrl with an ID of "myTextWidget" and a
    wxCheckBox with an ID of "myCheckWidget".  When the dialog has been OKed,
    the values will automatically be transferred into the variables and
    ShowModal() will return wxID_OK;

    FIXME - Add notes about which widgets support which exact types.  Note that
            some type conversions are performed automatically, though effects
            like wraparound, truncation, and rounding, are not checked for.

    FIXME - The non-modal code has not yet been implemented.  Perhaps we should
            use the event table system instead of plain subclassing.
*/

class jppResourceDialog
{
 public:
  /// Load an XRC or XRS resource file.
  /** This function should be called once for each resource file in the system.
      It it usually called from application init. */

  static void LoadResource(const wxString& xrcfile);


  /// Create a dialog with the given parent and resource name.

  jppResourceDialog(wxWindow* parent, const wxString& name);


  /// Calls Destroy() on the previously created dialog.

  virtual ~jppResourceDialog();


  /// Show the dialog.
  /** Shows the dialog and also sets any "Attached" data values if the dialog
      contents have been accepted.  The dialog must return wxID_OK to indicate
      acceptance. */

  int ShowModal();


  /// For attaching to a wxString.
  /** Connects a wxString variable to a named resource.  Currently supports
      wxTextCtrl widgets. */

  void Attach(const wxString& name, wxString *data);


  /// For attaching to a bool.
  /** Connects a bool variable to a named resource.  Currently supports
      wxCheckBox widgets. */

  void Attach(const wxString& name, bool *data);


  /// For attaching to a long.
  /** Connects a long variable to a named resource.  Currently supports
      wxSlider widgets. */

  void Attach(const wxString& name, long *data);

  /// Duplicate of long version, but with int restrictions.

  void Attach(const wxString& name, int *data);

  /// For attaching a list of items to a long.
  /** Connects a long variable to some list of items.  Currently supports
      wxChoice widgets.  The actual text displayed in the GUI is defined by the
      XRC file.  The indices in the GUI correspond to the indicies in the
      passed in array.  If the user selects the item with index 2, the long
      data will be set to the value at index 2 of the given array. */

  void Attach(const wxString& name, long *data, wxArrayLong array);

  /// For attaching a list of items to a long.
  /* Similar to the above function, only it accepts a -1 terminated array of
     longs for convenience. */

  void Attach(const wxString& name, long *data, long *a);


  /// Duplicate of long version, but with int restrictions.

  void Attach(const wxString& name, int *data, int *a);

  /// Duplicate of long version, but with int restrictions.

  void Attach(const wxString& name, int *data, wxArrayInt array);


  /// Ok event handler
  /** Called when a dialog produced by Show() has caused a wxID_OK event.
      If the function returns true, the Attached data will be set and the
      dialog will be hidden. */

  virtual bool OnOk()
  {
    return true;
  }

  /// Cancel event handler
  /** Called when a dialog produced by Show() has caused a wxID_CANCEL event.
      If the function returns true, the dialog will be hidden. */

  virtual bool OnCancel()
  {
    return true;
  }

  /// Apply event handler
  /** Called when a dialog produced by Show() has caused a wxID_APPLY event.
      If the function returns true, the Attached data will be set. */

  virtual bool OnApply()
  {
    return true;
  }


 private:
  /// Loads data from the data pointers into the widgets.
  bool LoadData(jppResourceElement *elem, wxWindow *win);
  /// Stores the data from the widgets into the data pointers.
  bool StoreData(jppResourceElement *elem, wxWindow *win);

  /// If the resource system has been initialized.
  static bool initialized;

  /// List of associations created by Attach.
//  jppResourceElementList links;
  std::list<jppResourceElement*> links;

  /// Reference to the dialog created from the XRC resource.
  wxDialog* dialog;

  /// The name of the dialog resource.  Used in error reporting.
  wxString dialogName;
};
