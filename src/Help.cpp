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

#include "Help.h"

#include <wx/filedlg.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>

#include <fstream>

using namespace std;

//*****************************************************************************
// Description:
//   This is the help class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
wxString JZHelp::mHelpFileName = "jazz.hhp";

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZHelp::JZHelp()
  : mpHelp(0)
{
  mpHelp = new wxHtmlHelpController(wxHF_DEFAULT_STYLE | wxHF_OPEN_FILES);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZHelp::~JZHelp()
{
  CloseHelp();
  delete mpHelp;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZHelp::ShowTopic(const wxString& TopicString)
{
  mpHelp->LoadFile(mHelpFileName);
  mpHelp->KeywordSearch(TopicString);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZHelp::DisplayHelpContents()
{
  mpHelp->LoadFile(mHelpFileName);
  mpHelp->DisplayContents();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZHelp::CloseHelp()
{
  // GetFrame returns NULL if there is no help frame active.
  if (mpHelp->GetFrame())
  {
    // Close the help frame; this will cause the config data to get written.
    mpHelp->GetFrame()->Close(true);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZHelp::ConfigureHelp()
{
  wxConfigBase* pConfig = wxConfigBase::Get();

  // Let the help system store the Jazz++ help configuration info.
  mpHelp->UseConfig(pConfig);

  // This code should be distributed with a HelpFiles subdirectory under
  // the directory the executable is stored in on Windows and under the
  // ${prefix}/shared/${appname} on Linux.
  wxString HelpFileDirectoryGuess =
    wxStandardPaths::Get().GetDataDir() +
    wxFileName::GetPathSeparator() +
    "HelpFiles" +
    wxFileName::GetPathSeparator();

  // Attempt to obtain the path to the help file from configuration data.
  wxString HelpFilePath;
  bool WasHelpPathRead = false;
  if (pConfig)
  {
    WasHelpPathRead = pConfig->Read(
      "/Paths/Help",
      &HelpFilePath,
      HelpFileDirectoryGuess);
  }

  // Construct a full file name.
  wxString HelpFileNameAndPath = HelpFilePath + mHelpFileName;

  // Test for the existence of the help file.
  bool HelpFileFound = false;
  ifstream Is;
  Is.open(HelpFileNameAndPath.mb_str());
  if (!Is)
  {
    // Ask the user to find the help file.
    if (FindAndRegisterHelpFilePath(HelpFilePath))
    {
      HelpFileNameAndPath = HelpFilePath + mHelpFileName;

      // Try one more time.
      Is.close();
      Is.clear();
      Is.open(HelpFileNameAndPath.mb_str());
      if (!Is)
      {
        wxString Message = "Failed to add the Jazz++ book " + mHelpFileName;
        ::wxMessageBox(Message);
      }
      else
      {
        HelpFileFound = true;
      }
    }
  }
  else
  {
    HelpFileFound = true;
  }

  // GetUserDataDir returns the directory for the user-dependent application
  // data files.  The value is $HOME/.appname on Linux,
  // c:\Documents and Settings\username\Application Data\appname on
  // Windows, and ~/Library/Application Support/appname on the Mac.
  // The cached version of the help file will be placed in this location.
  mpHelp->SetTempDir(wxStandardPaths::Get().GetUserDataDir());

  if (HelpFileFound)
  {
    // Add the Jazz++ help file the the help system.
    mpHelp->AddBook(HelpFileNameAndPath);

    if (!WasHelpPathRead && pConfig)
    {
      // Register the help path.
      pConfig->Write("/Paths/Help", HelpFilePath);
    }
  }
}

//-----------------------------------------------------------------------------
// Description:
//   This function walks the user through a top-level help file search.  If
// the help file is found, create a configuration entry so the code can find
// the help file path the next time the application starts.
//
// Outputs:
//   wxString& HelpFilePath:
//     A user-selected path to the help file.  The calling code should check
//     to insure the help file is actually in this path.
//-----------------------------------------------------------------------------
bool JZHelp::FindAndRegisterHelpFilePath(wxString& HelpFilePath) const
{
  wxString Message;
  Message =
    "Unable to find " + mHelpFileName + "\n" +
    "Would you like to locate this file?";
  int Response = ::wxMessageBox(
    Message,
    "Cannnot Find Help File",
    wxOK | wxCANCEL);

  if (Response == wxOK)
  {
    // Use an open dialog to find the help file.
    wxFileDialog OpenDialog(
      0,
      "Open the Help File",
      HelpFilePath,
      mHelpFileName,
      "*.hhp",
      wxFD_OPEN);

    if (OpenDialog.ShowModal() == wxID_OK)
    {
      // Generate a string that contains a path to the help file.
      wxString TempHelpFilePath;
      TempHelpFilePath = ::wxPathOnly(OpenDialog.GetPath());
      TempHelpFilePath += ::wxFileName::GetPathSeparator();

      wxConfigBase* pConfig = wxConfigBase::Get();
      if (pConfig)
      {
        pConfig->Write("/Paths/Help", TempHelpFilePath);
      }

      // Return the user selected help file path.
      HelpFilePath = TempHelpFilePath;

      return true;
    }
  }

  return false;
}
