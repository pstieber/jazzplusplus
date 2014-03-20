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

#include "ToolBar.h"

#include <wx/frame.h>
#include <wx/toolbar.h>

//*****************************************************************************
// Description:
//   This is the jazz toolbar class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZToolBar::JZToolBar(wxFrame* pFrame, JZToolDef* pToolDef)
  : mpToolBar(0)
{
  mpToolBar = pFrame->CreateToolBar(
    wxTB_FLAT | wxTB_HORIZONTAL | wxTB_DOCKABLE);

  mpToolBar->SetMargins(5, 5);

//  int BitmapWidth = ToolBarBitmaps[0].GetWidth();
//  int BitmapHeight = ToolBarBitmaps[0].GetHeight();
//  if (mUseLargeToolBarIcons)
//  {
//    BitmapWidth  = 2 * ToolBarBitmaps[0].GetWidth();
//    BitmapHeight = 2 * ToolBarBitmaps[0].GetHeight();
//    for (size_t n = 0; n < WXSIZEOF(ToolBarBitmaps); ++n)
//    {
//      ToolBarBitmaps[n] = wxBitmap(
//        ToolBarBitmaps[n].ConvertToImage().Scale(BitmapWidth, BitmapHeight));
//    }
//  }
//
//  mpToolBar->SetToolBitmapSize(wxSize(BitmapWidth, BitmapHeight));
  mpToolBar->SetToolBitmapSize(wxSize(20, 19));

  while (pToolDef->mId != eToolBarEnd)
  {
    if (pToolDef->mId == eToolBarSeparator)
    {
      mpToolBar->AddSeparator();
    }
    else
    {
      wxBitmap Bitmap = wxBitmap((char **)pToolDef->mpResourceId);

      if (pToolDef->mSticky)
      {
        mpToolBar->AddCheckTool(
          pToolDef->mId,
          wxEmptyString,
          Bitmap,
          wxNullBitmap,
          pToolDef->mpToolTip);
      }
      else
      {
        mpToolBar->AddTool(
          pToolDef->mId,
          wxEmptyString,
          Bitmap,
          pToolDef->mpToolTip);
      }
    }

    ++pToolDef;
  }

  mpToolBar->Realize();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
wxToolBar* JZToolBar::GetDelegateToolBar()
{
  return mpToolBar;
}

//=============================================================================
// Delegated Methods
//=============================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
wxSize JZToolBar::GetSize() const
{
  return mpToolBar->GetSize();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZToolBar::GetToolState(int ToolId) const
{
  return mpToolBar->GetToolState(ToolId);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZToolBar::ToggleTool(int ToolId, const bool Toggle)
{
  mpToolBar->ToggleTool(ToolId, Toggle);
}
