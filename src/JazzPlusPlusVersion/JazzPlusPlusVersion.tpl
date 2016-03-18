//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2016 Peter J. Stieber, all rights reserved.
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

#include "JazzPlusPlusVersion.h"

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
// Description:
//   This function returns the application's major version number.
//
// Returns:
//   int:
//     The application's major version number.
//-----------------------------------------------------------------------------
int JZJazzPlusPlusVersion::GetMajorVersion()
{
  return MAJOR_VERSION;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the application's minor version number.
//
// Returns:
//   int:
//     The application's minor version number.
//-----------------------------------------------------------------------------
int JZJazzPlusPlusVersion::GetMinorVersion()
{
  return MINOR_VERSION;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the application's build number.
//
// Returns:
//   int:
//     The application's build number.
//-----------------------------------------------------------------------------
int JZJazzPlusPlusVersion::GetBuildNumber()
{
  return BUILD_NUMBER;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the git branch name associated with the HEAD.
//
// Returns:
//   string:
//     The git branch name.
//-----------------------------------------------------------------------------
string JZJazzPlusPlusVersion::GetBranchName()
{
  string BranchNameString = "BRANCH_NAME";
  return BranchNameString;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the Git repository SHA1 sum.
//
// Returns:
//   int:
//     The Git repository SHA1 sum.
//-----------------------------------------------------------------------------
string JZJazzPlusPlusVersion::GetSha1Sum()
{
  string Sha1String = "SHA1_SUM";
  return Sha1String;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the result of
//
// git log --pretty=format:'' | wc -l
//
// Returns:
//   string:
//     The repository commit count.
//-----------------------------------------------------------------------------
string JZJazzPlusPlusVersion::GetCommitCount()
{
  string CommitCountString = "COMMIT_COUNT";
  return CommitCountString;
}

//-----------------------------------------------------------------------------
// Description:
//   This function returns the repository URL.
//
// Returns:
//   string:
//     The repository URL.
//-----------------------------------------------------------------------------
string JZJazzPlusPlusVersion::GetRepositoryUrl()
{
  string RepositoryUrl("https://github.com/pstieber/jazzplusplus.git");
  return RepositoryUrl;
}
