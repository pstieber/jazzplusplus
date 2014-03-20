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

//*****************************************************************************
// Description:
//   This class maps values in the range [x0, x1] to values in the range
// [y0, y1].
//*****************************************************************************
class JZMapper
{

  public:

    JZMapper()
      : x0(0),
        x1(0),
        y0(0),
        y1(0)
    {
    }

    JZMapper(double xx0, double xx1, double yy0, double yy1)
    {
      x0 = xx0;
      x1 = xx1;
      y0 = yy0;
      y1 = yy1;
    }

    void Initialize(double xx0, double xx1, double yy0, double yy1)
    {
      x0 = xx0;
      x1 = xx1;
      y0 = yy0;
      y1 = yy1;
    }

    // Description:
    //    Transform a value.
    double XToY(double x) const
    {
      return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
    }

    // Description:
    //   Perform the reverse transformation.
    double YToX(double y)
    {
      return x0 + (y - y0) * (x1 - x0) / (y1 - y0);
    }

  private:

    double x0, x1, y0, y1;
};

//*****************************************************************************
// Description:
//   This class maps values in the range [-x, x] to values in the range
// [1/y, y] using the exp function (i.e. map(0) == 1).
//*****************************************************************************
class JZExponentialMapper
{
  public:

    JZExponentialMapper(double x, double y);

    double XToY(double x) const;

  private:

    JZMapper mMapper;
};
