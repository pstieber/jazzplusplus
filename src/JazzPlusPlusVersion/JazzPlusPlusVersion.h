//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2013-2016 Peter J. Stieber, all rights reserved.
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

//*****************************************************************************
//*****************************************************************************
class JZJazzPlusPlusVersion
{
  public:

    static const JZJazzPlusPlusVersion& Instance();

    // Description:
    //   This function returns the application's major version number.
    //
    // Returns:
    //   int:
    //     The application's major version number.
    static int GetMajorVersion();

    // Description:
    //   This function returns the application's minor version number.
    //
    // Returns:
    //   int:
    //     The application's minor version number.
    static int GetMinorVersion();

    // Description:
    //   This function returns the application's build number.
    //
    // Returns:
    //   int:
    //     The application's build number.
    static int GetBuildNumber();
};


//*****************************************************************************
// Description:
//   These are the Jazz++ application class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const JZJazzPlusPlusVersion& JZJazzPlusPlusVersion::Instance()
{
  static JZJazzPlusPlusVersion JazzPlusPlusVersion;
  return JazzPlusPlusVersion;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the application's major version number.
// Version 1 was the MFC version developed on the Phase II ONR SBIR.
//
// Ver Date       Description
//  4  ?          Unknown for all prior versions.  Might be filled in by
//                looking at old commit messages.
//  5  1/21/2008  Start of the Jazz++ development revival.
//
// Returns:
//   int:
//     The application's major version number.
//-----------------------------------------------------------------------------
inline
int JZJazzPlusPlusVersion::GetMajorVersion()
{
  return 5;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the application's minor version number.
//
// Ver Date       Description
//  ?  ?          Unknown for all prior versions.  Might be filled in by
//                looking at old commit messages.
//  3  1/21/2008  Major refactoring here to get the code compiling with
//                wxWidgets 2.8.7 and recent compilers including Visual Studio
//                .NET 2005 and GCC 4.
//  4  9/1/2008   Updated to wxWidgets version 2.8.8.
//
// Returns:
//   int:
//     The application's minor version number.
//-----------------------------------------------------------------------------
inline
int JZJazzPlusPlusVersion::GetMinorVersion()
{
  return 4;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the application's build number.
//
// Ver Date       Description
//  ?  ?          Unknown for all prior versions.  Might be filled in by
//                looking at old commit messages.
//  11 1/21/2008  See minor version 3.
//  12 9/1/2008   See minor version 4.
//  13 3/17/2013  Fixed closing while recording and/or playing back.
//  14 3/17/2013  Added "Visit Web Site..." button to the about dialog.
//-----------------------------------------------------------------------------
inline
int JZJazzPlusPlusVersion::GetBuildNumber()
{
  return 14;
}
