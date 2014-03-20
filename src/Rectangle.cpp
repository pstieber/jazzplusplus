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

#include "Rectangle.h"

//*****************************************************************************
// Description:
//   This is the Jazz++ rectangle class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRectangle::JZRectangle(int XPosition, int YPosition, int Width, int Height)
  : wxRect(XPosition, YPosition, Width, Height)
{
}

//-----------------------------------------------------------------------------
// Description:
//   Add 2 rects together by making a new rect that overlap both (not strictly
// true, since we only take the maximum width and height and do not care about
// start x y).
//-----------------------------------------------------------------------------
void JZRectangle::SetUnion(const JZRectangle& Rectangle)
{
  if (Rectangle.GetWidth() > GetWidth())
  {
    SetWidth(Rectangle.GetWidth());
  }

  if (Rectangle.GetHeight() > GetHeight())
  {
    SetHeight(Rectangle.GetHeight());
  }
}


//-----------------------------------------------------------------------------
// Description:
//   "Normalize" the rectangle, by making width and height always positive,
// and giving it a minimum width and height of 1.
//-----------------------------------------------------------------------------
void JZRectangle::SetNormal()
{
  if (GetWidth() < 0)
  {
    SetWidth(-GetWidth());
    SetX(GetX() - GetWidth());
  }

  if (GetHeight() < 0)
  {
    SetHeight(-GetHeight());
    SetY(GetY() - GetHeight());
  }

  if (GetWidth() == 0)
  {
    SetWidth(1);
  }
  if (GetHeight() == 0)
  {
    SetHeight(1);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Check to see if a point is inside the rectangle.
//-----------------------------------------------------------------------------
bool JZRectangle::IsInside(int XPosition, int YPosition) const
{
  if (XPosition < GetX() || XPosition > GetX() + GetWidth())
  {
    return false;
  }
  if (YPosition < GetY() || YPosition > GetY() + GetHeight())
  {
    return false;
  }
  return true;
}
