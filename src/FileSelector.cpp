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

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/string.h>

//*****************************************************************************
// Description:
//   Add a supplied extension to a file name if the file name doesn't already
// have an extension.
//   Extension is assumed to have a leading dot. (.mid for example)
//*****************************************************************************
wxString add_default_ext(const wxString fn, const wxString Extension)

{
  // Is any extension already there?
  {
    const wxString x = wxFileNameFromPath(fn);
    if (x.find('.', TRUE) != 0)
    {
      // if there is a dot, assume its an extension and return.
      return fn;
    }
  }

  // Otherwise append the supplied extension and return
  wxString RevisedFileName = fn;
  RevisedFileName += Extension;
  return RevisedFileName;
}

//*****************************************************************************
// Description:
//   Presents the user with a file selector.  Some default values are used.
// This can be used to present a load or save selector.  If save, handles
// situations like showing a "really save over..." question box.
//
// Returns:
//   wxString:
//     This is the file name.
//*****************************************************************************
wxString file_selector(
  wxString DefaultFileName,
  const wxString title,
  bool save,
  bool changed,
  const wxString Extension)
{
   wxString file;
   wxString path;

   if (save)
   {
     file = wxFileNameFromPath(DefaultFileName);
   }

   path = wxPathOnly(DefaultFileName);

   int flags = save ? wxFD_SAVE : wxFD_OPEN;
   wxString s = wxFileSelector(title, path, file, wxEmptyString, Extension, flags);

  // add extension if missing
   if (!s.empty() && !Extension.empty())
  {
    s = add_default_ext(s, Extension);
  }

  // warn if overwriting existent file
  if (!s.empty() && save)
  {
    if (wxFileExists(s))
    {
      wxString buf;
      //sprintf(buf, "overwrite %s?", (char*)s);
      buf << "overwrite "<<s<<"?";
      if (wxMessageBox(buf, "Save ?", wxYES_NO) == wxNO)
      {
        s = wxEmptyString;
      }
    }
  }

  if (!s.empty() && !save && changed)
  {
    wxString buf;
    buf << DefaultFileName;
    buf <<" has changed. Load anyway?";
    if (wxMessageBox(buf, "Load ?", wxYES_NO) == wxNO)
    {
      s = wxEmptyString;
    }
  }

  if (!s.empty() && !save)
  {
    if (!wxFileExists(s))
    {
      wxString buf;
      buf<< "Cannot find file ";
      buf<< s;
      wxMessageBox( buf, "Error", wxOK );
      s = wxEmptyString;
    }
  }

  if (!s.empty())
  {
    //DefaultFileName = s;
    //return DefaultFileName;
    //i dont understand the point of the above original construct
//     wxString rv=*(new wxString(s));
//     return rv;//copy the local string and return it
    return s; //should be safe to return locally allocated string
  }

  return wxEmptyString;
}
