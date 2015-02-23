//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2015 Peter J. Stieber
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

#include "JazzPlusPlusApplication.h"

#include "Globals.h"
#include "Help.h"
#include "Project.h"
#include "ProjectManager.h"
#include "TrackFrame.h"

#ifdef _MSC_VER

#ifdef _DEBUG
// This code provides a console window for a GUI windows application.  This
// allows the use of cout for debug or informational messages.
#include "mswin/WindowsConsole.h"
#endif

// This include allows Microsoft leak detection calls like _CrtSetBreakAlloc.
#include <crtdbg.h>

#endif // _MSC_VER

#ifdef __LINUX__

// The following include is required to call feenableexcept on Linux.
#include <fenv.h>

#endif

#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <wx/image.h>
#include <wx/msgdlg.h>

#include <vector>

using namespace std;

//*****************************************************************************
// Description:
//   This is the JazzPlusPlus application class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
// Description:
//   Create a new application object using the wxWidgets macro.  This macro
// will allow wxWidgets to create the application object during program
// execution (it's better than using a static object for many reasons) and
// also declares the accessor function wxGetApp() which will return the
// reference of the right type (i.e. JZJazzPlusPlusApplication and not wxApp).
//-----------------------------------------------------------------------------
IMPLEMENT_APP(JZJazzPlusPlusApplication)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZJazzPlusPlusApplication::JZJazzPlusPlusApplication()
  : wxApp(),
    mpProject(nullptr),
    mpTrackFrame(nullptr)
{
  // When using the Microsoft C++ compiler in debug mode, each heap allocation
  // (i.e. calling new) is counted. The following line will cause the code to
  // generate a user defined break point when the passed allocation index is
  // hit. To find leaks, run in debug and look for the following type of line
  // in the Debug output window.
  //
  // {1494} normal block at 0x00B98FC8, 32 bytes long.
  //  Data: <            Jazz> 01 00 00 00 04 00 00 00 13 00 00 00 4A 61 7A 7A
  //
  // Then use the following line to cause a break point when this allocation
  // occurs:
  //
  // _CrtSetBreakAlloc(1494);

#ifdef __LINUX__
  // This code enables floating point exceptions for
  // 1. Division by zero.
  // 2. Invalid arguments (for example sqrt of a negative number).
  // 3. Overflow.
  // on a Linux box.

  // The scrollbar code in the Ubuntu 12.04 Unity liboveralay-scrollbar code
  // is causing floating point exceptions so I'm commenting out this code.
//  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif // __LINUX__
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZJazzPlusPlusApplication::~JZJazzPlusPlusApplication()
{
}

//-----------------------------------------------------------------------------
// Description:
//   This function is called on application startup and is a good place
// for application initialization.  Initializing here and not in the
// constructor allows an error return.  If OnInit() returns false, the
// application terminates.
//-----------------------------------------------------------------------------
bool JZJazzPlusPlusApplication::OnInit()
{
  // Call base class function.  This is needed for command line parsing.
  if (!wxApp::OnInit())
  {
    return false;
  }

#if defined(_MSC_VER) && defined(_DEBUG)
  RedirectIoToConsole();
#endif // _MSC_VER

  ::wxInitAllImageHandlers();

  SetVendorName("Jazz++");
  SetAppName("Jazz++");

  InsureConfigurationFileExistence();

  // Create the one and only top-level Jazz++ project.
  mpProject = new JZProject;
  gpProject = mpProject;

  // Create the main application window.
  mpTrackFrame = JZProjectManager::Instance()->CreateTrackView();

  gpTrackFrame = mpTrackFrame;

  // Show it and tell the application that it's our main window
  SetTopWindow(mpTrackFrame);

  JZHelp::Instance().ConfigureHelp();

  return true;
}

//-----------------------------------------------------------------------------
// Description:
//   This function checks to see if the user's Jazz++ configuration directory
// files exist and creates the directory and copies default versions if they
// do not.
//-----------------------------------------------------------------------------
void JZJazzPlusPlusApplication::InsureConfigurationFileExistence() const
{
  // Determine the expected location of the user's data directory for Jazz++.
  wxString UserConfigDir = wxStandardPaths::Get().GetUserDataDir();

  // Determine if the directory exists.
  if (!wxDirExists(UserConfigDir))
  {
    // Attempt to create the directory.
    if (!wxMkdir(UserConfigDir))
    {
      wxString String;
      String
        << "Unable to create directory \""
        << UserConfigDir << '"';
      ::wxMessageBox(String, "Directory Creation Error");
    }
  }

  // Setup the wxWidgets configuration file.
  wxFileName WxConfigurationFileName(UserConfigDir, ".jazz");

  wxFileConfig* pFileConfig = new wxFileConfig(
    GetAppName(),
    wxEmptyString,
    WxConfigurationFileName.GetFullPath(),
    wxEmptyString,
    wxCONFIG_USE_LOCAL_FILE);

  delete wxConfigBase::Set(pFileConfig);

  // Make sure all of the configuration files are setup.
  vector<wxString> ConfigurationFileNames;
  ConfigurationFileNames.push_back("README");
  ConfigurationFileNames.push_back("jazz.cfg");
  ConfigurationFileNames.push_back("jazz.mid");
  ConfigurationFileNames.push_back("ctrlnam.jzi");
  ConfigurationFileNames.push_back("e26voice.jzi");
  ConfigurationFileNames.push_back("e26.jzi");
  ConfigurationFileNames.push_back("gm.jzi");
  ConfigurationFileNames.push_back("gmdrmnam.jzi");
  ConfigurationFileNames.push_back("gmdrmset.jzi");
  ConfigurationFileNames.push_back("gmvoices.jzi");
  ConfigurationFileNames.push_back("gs.jzi");
  ConfigurationFileNames.push_back("gsdrmset.jzi");
  ConfigurationFileNames.push_back("gsvoices.jzi");
  ConfigurationFileNames.push_back("jv1000.jzi");
  ConfigurationFileNames.push_back("sc88pdrm.jzi");
  ConfigurationFileNames.push_back("sc88pro.jzi");
  ConfigurationFileNames.push_back("sc88pvoi.jzi");
  ConfigurationFileNames.push_back("xg.jzi");
  ConfigurationFileNames.push_back("xgdrmnam.jzi");
  ConfigurationFileNames.push_back("xgdrmset.jzi");
  ConfigurationFileNames.push_back("xgvoices.jzi");

  for (const auto& ConfigurationFileName : ConfigurationFileNames)
  {
    // Check to see if the user already has a jazz.cfg file in the
    // user configuration directory.
    wxString JazzCfgFile =
      UserConfigDir +
      wxFileName::GetPathSeparator() +
      ConfigurationFileName;

    if (!::wxFileExists(JazzCfgFile))
    {
      // Attempt to copy the default Jazz++ configuration file to this
      // directory.
      wxString DefaultJazzCfgFile =
        wxStandardPaths::Get().GetDataDir() +
        wxFileName::GetPathSeparator() +
        ConfigurationFileName;

      if (::wxFileExists(DefaultJazzCfgFile))
      {
        ::wxCopyFile(DefaultJazzCfgFile, JazzCfgFile);
      }
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZJazzPlusPlusApplication::OnExit()
{
  delete mpProject;

  JZHelp::Instance().CloseHelp();

  // Prevent reported leaks from the configuration class.
  delete wxConfigBase::Set(nullptr);

  JZProjectManager::Destroy();

  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrackFrame* JZJazzPlusPlusApplication::GetMainFrame() const
{
  return mpTrackFrame;
}
