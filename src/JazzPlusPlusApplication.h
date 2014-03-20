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

#pragma once

#include <wx/app.h>

class JZProject;
class JZTrackFrame;

//*****************************************************************************
// Description:
//   This is the JazzPlusPlus application class declaration.
//*****************************************************************************
class JZJazzPlusPlusApplication : public wxApp
{
  public:

    JZJazzPlusPlusApplication();

    ~JZJazzPlusPlusApplication();

    // Override the base class virtual functions.

    // Description:
    //   This function is called on application startup and is a good place
    // for application initialization.  Initializing here and not in the
    // constructor allows an error return.  If OnInit() returns false, the
    // application terminates.
    virtual bool OnInit();

    virtual int OnExit();

    // Description:
    //   This virtual function returns a pointer to the application's main
    // frame.
    //
    // Returns:
    //   JZTrackFrame*:
    //     A pointer to the application's main frame.
    JZTrackFrame* GetMainFrame() const;

  private:

    void InsureConfigurationFileExistence() const;

  private:

    JZProject* mpProject;

    JZTrackFrame* mpTrackFrame;
};

//*****************************************************************************
//*****************************************************************************
DECLARE_APP(JZJazzPlusPlusApplication)
