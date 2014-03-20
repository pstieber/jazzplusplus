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

#include <wx/window.h>

#include <iosfwd>
#include <vector>

//*****************************************************************************
//*****************************************************************************
class JZRandomGenerator
{
  public:

    double asDouble();
};

extern JZRandomGenerator rnd;

//*****************************************************************************
// Description:
//   Array of probabilities.
//*****************************************************************************
class JZRndArray
{
  friend class JZArrayEdit;

  public:

    JZRndArray(int Size, int Min, int Max);
    JZRndArray & operator = (const JZRndArray &);
    JZRndArray(JZRndArray const &);

    virtual ~JZRndArray();

    int GetNull()
    {
      return mNull;
    }
    void SetNull(int Null)
    {
      mNull = Null;
    }

    int &operator[] (int i)
    {
      return mArray[i];
    }
    int  operator[] (int i) const
    {
      return mArray[i];
    }
    double operator[](double f);
    float operator[](float f)
    {
      return (float)operator[]((double)f);
    }
    int Size() const
    {
      return mArray.size();
    }
    int GetMin() const
    {
      return mMin;
    }
    int GetMax() const
    {
      return mMax;
    }
    void SetMinMax(int Min, int Max);
    void Resize(int nn)
    {
      mArray.resize(nn);
    }

    friend std::ostream& operator << (std::ostream &, JZRndArray const &);
    friend std::istream& operator >> (std::istream &, JZRndArray &);

    // Returns index 0..n-1 (arrayvalues -> empiric distribution)
    int Random();

    // returns index 0..n-1 (arrayvalues -> empiric distribution)
    int Random(double rndval);

      // return 0/1
    int Random(int i);

    int Interval(int seed);

    void SetUnion(JZRndArray &o, int fuzz);
    void SetDifference(JZRndArray &o, int fuzz);
    void SetIntersection(JZRndArray &o, int fuzz);
    void SetInverse(int fuzz);
    int Fuzz(int fuzz, int v1, int v2) const;
    void Clear();

  protected:

    std::vector<int> mArray;
    int mNull, mMin, mMax;
};

//*****************************************************************************
//*****************************************************************************
#define ARED_GAP            1
#define ARED_XTICKS         2
#define ARED_YTICKS         4
#define ARED_MINMAX         8
#define ARED_RHYTHM        16
#define ARED_BLOCKS        32
#define ARED_LINES         64

//*****************************************************************************
//*****************************************************************************
class JZArrayEditDrawBars
{
  public:

    virtual ~JZArrayEditDrawBars()
    {
    }

    virtual void DrawBars(wxDC& Dc) = 0;
};

//*****************************************************************************
//*****************************************************************************
class JZArrayEdit : public wxWindow
{
  public:

    JZArrayEdit(
      wxWindow* pParent,
      JZRndArray& Array,
      const wxPoint& Position,
      const wxSize& Size,
      int StyleBits = (ARED_GAP | ARED_XTICKS));

    virtual ~JZArrayEdit();

    virtual void OnDraw(wxDC& Dc);
    virtual void OnSize(wxSizeEvent& event);
    virtual void OnMouseEvent(wxMouseEvent& MouseEvent);
    virtual int Dragging(wxMouseEvent& MouseEvent);
    virtual int ButtonDown(wxMouseEvent& MouseEvent);
    virtual int ButtonUp(wxMouseEvent& MouseEvent);

    virtual void SetLabel(const std::string& Label);

    void SetEnabled(bool Enabled = true);

    void SetStyle(int StyleBits)
    {
      mStyleBits = StyleBits;
    }

    // Minimum and maximum value in array (both values inclusive)
    void SetYMinMax(int Min, int Max);

    // For display x-axis only, does not resize the array (both values inclusive)
    void SetXMinMax(int XMin, int XMax);

    void DrawBarLine(wxDC& Dc, int xx);
    void SetDrawBars(JZArrayEditDrawBars* pDrawBars)
    {
      mpDrawBars = pDrawBars;
    }

    void Init()
    {
    }

  protected:

    void DrawBar(wxDC& Dc, int i, int black);

    virtual void DrawXTicks(wxDC& Dc);
    virtual void DrawYTicks(wxDC& Dc);
    virtual void DrawLabel(wxDC& Dc);
    virtual void DrawNull(wxDC& Dc);
    int GetIndex(wxMouseEvent& MouseEvent);

    virtual std::string GetXText(int XValue);  // Text for x-tickmarks
    virtual std::string GetYText(int YValue);  // Text for y-tickmarks

  protected:

    JZRndArray& mArray;

    // Shorthand for mArray.mMin, mArray.mMax, ...
    int& mMin;
    int& mMax;
    int& mNull;

    std::string mLabel;
    JZArrayEditDrawBars* mpDrawBars;

    // paint position
    int mX, mY, mWidth, mHeight, mYNull;

    // Dragging flag.
    bool mDragging;

    // If ctrl is pushed: drag this one.
    int mIndex;

    // Array size is mapped to this range for x-tick marks.
    int mXMin, mXMax;

    bool mEnabled;
    int mStyleBits;

  DECLARE_EVENT_TABLE()
};

//*****************************************************************************
//*****************************************************************************
class JZRhyArrayEdit : public JZArrayEdit
{
  public:

    JZRhyArrayEdit(
      wxWindow* pParent,
      JZRndArray& Array,
      const wxPoint& Position,
      const wxSize& Size,
      int StyleBits = ARED_GAP | ARED_XTICKS | ARED_RHYTHM);

    void SetMeter(int StepsPerCount, int CountPerBar, int BarCount);

  protected:

    virtual void DrawXTicks(wxDC& Dc);

  private:

    int mStepsPerCount;
    int mCountPerBar;
};
