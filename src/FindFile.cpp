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

#include <wx/app.h>
#include <wx/filename.h>

#include <iostream>

using namespace std;

//*****************************************************************************
// Description:
//   This function attempts to find a file.  It checks for the existence of
// the file by
//
// 1. using the passed file name
// 2. appending the passed file name to the path specified by the HOME
//    environment variable, if it exists
// 3. appending the passed file name to the path specified by the JAZZ
//    environment variable, if it exists
// 4. appending the passed file name to the location of the Jazz++ executable
//
// Returns:
//   wxString:
//     A complete path and file name for the found file or wxEmptyString if
//     the file was not found.
//*****************************************************************************
wxString FindFile(const wxString& FileName)
{
  if (::wxFileExists(FileName))
  {
    cout << "FindFile: Immediate hit on file \"" << FileName << '"' << endl;
    return FileName;
  }

  wxString FoundFileName;

  wxString Home;
  if (getenv("HOME") != 0)
  {
    Home = getenv("HOME");
    FoundFileName << Home << wxFileName::GetPathSeparator() << FileName;
    if (wxFileExists(FoundFileName))
    {
      cout << "FindFile: HOME: \"" << FoundFileName << '"' << endl;
      return FoundFileName;
    }
  }

  if (getenv("JAZZ") != 0)
  {
    FoundFileName = "";
    Home = getenv("JAZZ");
    FoundFileName << Home << wxFileName::GetPathSeparator() << FileName;
    if (wxFileExists(FoundFileName))
    {
      cout << "FindFile: JAZZ: \"" << FoundFileName << '"' << endl;
      return FoundFileName;
    }
  }

  // Look where the executable was started.
  FoundFileName = "";
  Home = wxPathOnly(wxTheApp->argv[0]);
  FoundFileName << Home << wxFileName::GetPathSeparator() << FileName;
  if (wxFileExists(FoundFileName))
  {
    cout << "FindFile: Startup directory: \"" << FoundFileName << '"' << endl;
    return FoundFileName;
  }

  cout << "FindFile: File not found: \"" << FileName << '"' << endl;

  return wxEmptyString;
}
