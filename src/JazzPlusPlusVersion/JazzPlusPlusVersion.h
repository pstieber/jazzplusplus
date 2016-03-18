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

#include <string>

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

    static std::string GetBranchName();

    static std::string GetSha1Sum();

    static std::string GetCommitCount();

    static std::string GetRepositoryUrl();

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
