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

#include "NamedChoice.h"

#include "NamedValue.h"
#include "DeprecatedWx/proplist.h"

using namespace std;

JZNamedChoice::JZNamedChoice(
  const char* pTitle,
  const std::vector<std::pair<wxString, int> >& Pairs,
  int* pResult)
  : //mpTitle(pTitle),
    mPairs(Pairs),
    mSelection(),
    mpResult(pResult)
{
}

JZNamedChoice::~JZNamedChoice()
{
}

#ifdef OBSOLETE
wxFormItem *JZNamedChoice::mkFormItem(int w, int h)
{
  SetValue();

  // following adapted from wxwin/src/base/wb_form.cc
  wxList* pList = new wxList;
  for (int i = 0; mpValues[i].Name; i++)
  {
    // Omit empty entries.
    if (*mpValues[i].Name)
    {
      pList->Append((wxObject *)copystring(mpValues[i].Name));
    }
  }

  wxFormItemConstraint *constraint = wxMakeConstraintStrings(pList);
  return wxMakeFormString(
     mpTitle,
     &mpSelection,
     wxFORM_SINGLE_LIST,
     new wxList(constraint, 0),
     0,
     0,
     w,
     h);
}
#endif

// Return a string list validator to use in the wxproplist dialogs.
wxStringListValidator* JZNamedChoice::GetStringListValidator()
{
  wxArrayString* StringList = new wxArrayString();
  for (const auto& StringIntPair : mPairs)
  {
    // Omit empty entries.
    if (!StringIntPair.first.empty())
    {
      StringList->Add(StringIntPair.first);
    }
  }
  return new wxStringListValidator(StringList);
}

void JZNamedChoice::GetValue()
{
  for (const auto& StringIntPair : mPairs)
  {
    if (StringIntPair.first == mSelection)
    {
      *mpResult = StringIntPair.second;
      break;
    }
  }
}

void JZNamedChoice::SetValue()
{
  for (const auto& StringIntPair : mPairs)
  {
    if (*mpResult == StringIntPair.second)
    {
      mSelection = StringIntPair.first;
      break;
    }
  }
}
