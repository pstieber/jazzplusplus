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

#include "Random.h"

#include "Mapper.h"

#include <wx/dcclient.h>

#include <cassert>
#include <cstdlib>
#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
double JZRandomGenerator::asDouble()
{
  return double(rand()) / double(RAND_MAX);
}

JZRandomGenerator rnd;

//*****************************************************************************
// Description:
//   Array of probabilities
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRndArray::JZRndArray(int Size, int Min, int Max)
  : mArray(Size),
    mNull(0),
    mMin(Min),
    mMax(Max)
{
  for (size_t i = 0; i < mArray.size(); ++i)
  {
    mArray[i] = Min;
  }
  mNull = mMin > 0 ? mMin : 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRndArray::JZRndArray(const JZRndArray& Other)
  : mArray(Other.mArray),
    mNull(Other.mNull),
    mMin(Other.mMin),
    mMax(Other.mMax)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRndArray::~JZRndArray()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRndArray::SetMinMax(int Min, int Max)
{
  mMin = Min;
  mMax = Max;
  mNull = mMin > 0 ? mMin : 0;
  for (size_t i = 0; i < mArray.size(); ++i)
  {
    if (mArray[i] < mMin)
    {
      mArray[i] = mMin;
    }
    else if (mArray[i] > mMax)
    {
      mArray[i] = mMax;
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
double JZRndArray::operator[](double f)
{
  int i = (int)f;
  if (i < 0)
  {
    i = 0;
  }
  else if (i >= mArray.size() - 2)
  {
    i = mArray.size() - 2;
  }
  JZMapper Map(i, i + 1, mArray[i], mArray[i + 1]);
  return Map.XToY(f);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRndArray& JZRndArray::operator = (const JZRndArray& Rhs)
{
  if (this != &Rhs)
  {
    mArray = Rhs.mArray;
    mMin = Rhs.mMin;
    mMax = Rhs.mMax;
    mNull = Rhs.mNull;
  }
  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZRndArray::Random()
{
  return Random(rnd.asDouble());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZRndArray::Random(double RandonValue)
{
  assert(!mArray.empty());

  double Sum = 0.0;
  for (size_t i = 0; i < mArray.size(); ++i)
  {
    assert(mArray[i] >= 0);
    Sum += mArray[i];
  }
  if (Sum <= 0)
  {
    return 0;
  }

  double dec = Sum * RandonValue * 0.99999;
  assert(dec < Sum);

  int i = 0;
  while (dec >= 0.0)
  {
    dec -= mArray[i++];
  }
  i--;

  assert(i >= 0 && i < mArray.size());
  return i;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZRndArray::Interval(int seed)
{
  // Test to see if this is the initial call.
  if (seed < 0)
  {
    seed = int(rnd.asDouble() * mArray.size());
  }
  int delta = Random();
  if (rnd.asDouble() < 0.5)
  {
    delta = -delta;
  }
  seed = (seed + mArray.size() + delta) % mArray.size();
  return seed;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZRndArray::Random(int i)
{
  return rnd.asDouble() * (mMax - mMin) < mArray[i];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRndArray::SetUnion(JZRndArray& Other, int fuzz)
{
  for (size_t i = 0; i < mArray.size(); ++i)
  {
    int Value = mArray[i];
    if (Other.mArray[i] > Value)
    {
      Value = Other.mArray[i];
    }
    mArray[i] = Fuzz(fuzz, mArray[i], Value);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRndArray::SetIntersection(JZRndArray& Other, int fuzz)
{
  for (size_t i = 0; i < mArray.size(); ++i)
  {
    int Value = mArray[i];
    if (Other.mArray[i] < Value)
    {
      Value = Other.mArray[i];
    }
    mArray[i] = Fuzz(fuzz, mArray[i], Value);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRndArray::SetDifference(JZRndArray& Other, int fuzz)
{
  JZRndArray tmp(Other);
  tmp.SetInverse(tmp.GetMax());
  SetIntersection(tmp, fuzz);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRndArray::SetInverse(int fuzz)
{
  for (size_t i = 0; i < mArray.size(); ++i)
  {
    mArray[i] = Fuzz(fuzz, mArray[i], mMin + mMax - mArray[i]);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZRndArray::Fuzz(int fuz, int v1, int v2) const
{
  // interpolate between v1 and v2
  return (fuz - mMin) * v2 / (mMax - mMin) + (mMax - fuz) * v1 / (mMax - mMin);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRndArray::Clear()
{
  for (size_t i = 0; i < mArray.size(); ++i)
  {
    mArray[i] = mMin;
  }
}

//*****************************************************************************
//*****************************************************************************
ostream& operator << (ostream& Os, const JZRndArray& RndArray)
{
  Os
    << RndArray.mArray.size()
    << ' ' << RndArray.mMin
    << ' ' << RndArray.mMax
    << '\n';
  for (size_t i = 0; i < RndArray.mArray.size(); ++i)
  {
    Os << RndArray.mArray[i] << ' ';
  }
  Os << endl;
  return Os;
}

//*****************************************************************************
//*****************************************************************************
istream & operator >> (istream& Is, JZRndArray& RndArray)
{
  unsigned Size;
  Is >> Size >> RndArray.mMin >> RndArray.mMax;
  RndArray.mArray.resize(Size);
  for (size_t i = 0; i < RndArray.mArray.size(); ++i)
  {
    Is >> RndArray.mArray[i];
  }
  return Is;
}

//*****************************************************************************
// length of tickmark line
//*****************************************************************************
#define TICK_LINE 0

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZArrayEdit, wxWindow)
  EVT_SIZE(JZArrayEdit::OnSize)
  EVT_MOUSE_EVENTS(JZArrayEdit::OnMouseEvent)
END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZArrayEdit::JZArrayEdit(
  wxWindow* pParent,
  JZRndArray& Array,
  const wxPoint& Position,
  const wxSize& Size,
  int StyleBits)
  : wxWindow(pParent, wxID_ANY, Position, Size),
    mArray(Array),
    mMin(Array.mMin),
    mMax(Array.mMax),
    mNull(Array.mNull),
    mLabel(),
    mpDrawBars(0),
    mX(0),
    mY(0),
    mWidth(Size.GetWidth()),
    mHeight(Size.GetHeight()),
    mYNull(0),
    mDragging(false),
    mIndex(-1),
    mXMin(0),
    mXMax(mArray.Size()),
    mEnabled(true),
    mStyleBits(StyleBits)
{
  int TextWidth, TextHeight;

  wxClientDC Dc(this);
  Dc.SetFont(*wxSMALL_FONT);
  Dc.GetTextExtent("123", &TextWidth, &TextHeight);

  if (mStyleBits & ARED_XTICKS)
  {
    // Leave space for the bottom line.
    mHeight -= TextHeight;
  }

  if (mStyleBits & (ARED_MINMAX | ARED_YTICKS))
  {
    // Leave space to display the minimum and maximum
    mX = (int)(TextWidth + TICK_LINE);
    mWidth -= (int)(TextWidth + TICK_LINE);
  }

  mYNull = mY + mHeight - mHeight * (mNull - mMin) / (mMax - mMin);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::OnSize(wxSizeEvent& Event)
{
  mWidth = Event.GetSize().GetWidth();
  mHeight = Event.GetSize().GetHeight();

  Event.Skip();

  int TextWidth, TextHeight;

  wxClientDC Dc(this);
  Dc.GetTextExtent("123", &TextWidth, &TextHeight);

  if (mStyleBits & ARED_XTICKS)
  {
    mHeight -= TextHeight;
  }
  if (mStyleBits & (ARED_MINMAX | ARED_YTICKS))
  {
    mX = (int)(TextWidth + TICK_LINE);
    mWidth -= (int)(TextWidth + TICK_LINE);
  }
  mYNull = mY + mHeight - mHeight * (mNull - mMin) / (mMax - mMin);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZArrayEdit::~JZArrayEdit()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::DrawBar(wxDC& Dc, int i, int black)
{
  if (mStyleBits & ARED_LINES)
  {
    if (!black)
    {
      Dc.SetPen(*wxWHITE_PEN);
    }

    JZMapper XMap(0, mArray.Size(), 0, mWidth);
    JZMapper YMap(mMin, mMax, mHeight, 0);

    int x1 = (int)XMap.XToY(i + 0.5);
    int y1 = (int)YMap.XToY(mArray[i]);
    if (i > 0)
    {
      // draw line to prev position
      int x0 = (int)XMap.XToY(i - 0.5);
      int y0 = (int)YMap.XToY(mArray[i-1]);
      Dc.DrawLine(x0, y0, x1, y1);
    }
    if (i < mArray.Size() - 1)
    {
      // draw line to next position
      int x2 = (int)XMap.XToY(i + 1.5);
      int y2 = (int)YMap.XToY(mArray[i + 1]);
      Dc.DrawLine(x1, y1, x2, y2);
    }

    if (!black)
    {
      Dc.SetPen(*wxBLACK_PEN);
    }
    return;
  }

  int gap = 0;
  if (mStyleBits & ARED_GAP)
  {
    gap = mWidth / mArray.Size() / 6;
    if (!gap && mWidth / mArray.Size() > 3)
    {
      gap = 1;
    }
  }
  int xbar, ybar, wbar, hbar;

  wbar = mWidth / mArray.Size() - 2 * gap;
  xbar = mX + i * mWidth / mArray.Size() + gap;
  hbar = mHeight * (mArray[i] - mNull) / (mMax - mMin);

  if (mStyleBits & ARED_BLOCKS)
  {
    /*
    ybar = mYNull - hbar;
    if (hbar < 0)
      hbar = -hbar;
    hbar = (hbar < 2) ? hbar : 2;
    */
    int hblk = 12;

    ybar = mYNull - hbar - hblk/2;
    hbar = hblk;
    if (ybar < mY)
    {
      int d = mY - ybar;
      ybar += d;
      hbar -= d;
    }
    if (ybar + hbar > mY + mHeight)
    {
      int d = (ybar + hbar) - (mY + mHeight);
      hbar -= d;
    }
    if (hbar < 2)
      hbar = 2;
  }
  else

  if (hbar < 0)
  {
    ybar = mYNull;
    hbar = -hbar;
  }
  else
    ybar = mYNull - hbar;

  if (ybar == mY)
  {
    ++ybar, --hbar;
  }

  if (!black)
  {
    Dc.SetBrush(*wxWHITE_BRUSH);
    Dc.SetPen(*wxWHITE_PEN);
  }
  if (wbar && hbar)
  {
    Dc.DrawRectangle(xbar, ybar, wbar, hbar);
  }
  if (!black)
  {
    Dc.SetBrush(*wxBLACK_BRUSH);
    Dc.SetPen(*wxBLACK_PEN);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
string JZArrayEdit::GetXText(int XValue)
{
  ostringstream Oss;
  Oss << XValue;
  return Oss.str();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
string JZArrayEdit::GetYText(int YValue)
{
  ostringstream Oss;
  Oss << YValue;
  return Oss.str();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::DrawXTicks(wxDC& Dc)
{
  int TextWidth, TextHeight;

  if (!(mStyleBits & ARED_XTICKS))
  {
    return;
  }

  Dc.SetFont(*wxSMALL_FONT);

  // compute tickmark x-distance
  Dc.GetTextExtent("-123", &TextWidth, &TextHeight);
  int MaxLabels = (int)(mWidth / (TextWidth + TextWidth/2));
  if (MaxLabels > 0)
  {
    int Step = (mXMax - mXMin + 1) / MaxLabels;
    if (Step <= 0)
    {
      Step = 1;
    }
    for (int Value = mXMin; Value <= mXMax; Value += Step)
    {
      string String = GetXText(Value);
      Dc.GetTextExtent(String, &TextWidth, &TextHeight);
      int YPosition = mY + mHeight;
      float XPosition = mX + mWidth * (Value - mXMin) / (mXMax - mXMin + 1);

      // Center text.
      XPosition -= TextWidth / 2;

      // Middle of bar.
      XPosition += 0.5 * mWidth / mArray.Size();

      Dc.DrawText(String, (int)XPosition, YPosition);
//      Dc.DrawLine(mX - TICK_LINE, YPosition, mX, YPosition);
    }
  }

  Dc.SetFont(*wxNORMAL_FONT);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::DrawYTicks(wxDC& Dc)
{

  Dc.SetFont(*wxSMALL_FONT);

  if (mStyleBits & ARED_YTICKS)
  {
    // Compute tickmark y-distance.
    int TextWidth, TextHeight;
    Dc.GetTextExtent("-123", &TextWidth, &TextHeight);
    int MaxLabels = (int)(mHeight / (TextHeight + TextHeight / 2));
    if (MaxLabels > 0)
    {
      int Step = (mMax - mMin) / MaxLabels;
      if (Step <= 0)
      {
        Step = 1;
      }
      for (int Value = mMin; Value < mMax; Value += Step)
      {
        string String = GetYText(Value);
        Dc.GetTextExtent(String, &TextWidth, &TextHeight);
        int YPosition =
          mY + mHeight - mHeight * (Value - mMin) / (mMax - mMin) -
          TextHeight / 2;
        Dc.DrawText(String, mX - TextWidth - TICK_LINE, YPosition);
//        Dc.DrawLine(mX - TICK_LINE, YPosition, mX, YPosition);
      }
    }
  }
  else if (mStyleBits & ARED_MINMAX)
  {
    // mMin/mMax
    int TextWidth, TextHeight;
    ostringstream Oss;

    Oss << mMax;
    Dc.GetTextExtent(Oss.str(), &TextWidth, &TextHeight);
    Dc.DrawText(Oss.str(), mX - TextWidth, mY);

    Oss.str("");

    Oss << mMin;
    Dc.GetTextExtent(Oss.str(), &TextWidth, &TextHeight);
    Dc.DrawText(Oss.str(), mX - TextWidth, mY + mHeight - TextHeight);
  }

  Dc.SetFont(*wxNORMAL_FONT);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::DrawLabel(wxDC& Dc)
{
  Dc.SetFont(*wxSMALL_FONT);
  if (!mLabel.empty())
  {
    Dc.DrawText(mLabel, mX + 5, mY + 2);
  }
  Dc.SetFont(*wxNORMAL_FONT);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::OnDraw(wxDC& Dc)
{
  int i;

  // surrounding rectangle
  Dc.Clear();
  if (mEnabled)
  {
    Dc.SetBrush(*wxWHITE_BRUSH);
  }
  else
  {
    Dc.SetBrush(*wxGREY_BRUSH);
  }

  Dc.SetPen(*wxBLACK_PEN);
  if (mWidth && mHeight)
  {
    Dc.DrawRectangle(mX, mY, mWidth, mHeight);
  }

  // sliders
  Dc.SetBrush(*wxBLACK_BRUSH);
  for (i = 0; i < mArray.Size(); ++i)
  {
    DrawBar(Dc, i, 1);
  }

  DrawXTicks(Dc);
  DrawLabel(Dc);
  DrawYTicks(Dc);
  DrawNull(Dc);
  if (mpDrawBars)
  {
    mpDrawBars->DrawBars(Dc);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::DrawNull(wxDC& Dc)
{
  Dc.SetPen(*wxCYAN_PEN);

  // Draw y-null line.
  if (mMin < mNull && mNull < mMax)
  {
    Dc.DrawLine(mX, mYNull, mX + mWidth, mYNull);
  }

  // Draw x-null line.
  if (mXMin < 0 && 0 < mXMax)
  {
    int x0 = mWidth * (0 - mXMin) / (mXMax - mXMin);
    Dc.DrawLine(x0, mY, x0, mY + mHeight);
  }

  Dc.SetPen(*wxBLACK_PEN);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::SetXMinMax(int XMin, int XMax)
{
  mXMin = XMin;
  mXMax = XMax;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZArrayEdit::GetIndex(wxMouseEvent& MouseEvent)
{
  int EventX, EventY;
  MouseEvent.GetPosition(&EventX, &EventY);
  int Index = (int)((EventX - mX) * mArray.Size() / mWidth);
  if (Index < 0)
  {
    Index = 0;
  }
  if (Index >= mArray.Size())
  {
    Index = mArray.Size() - 1;
  }
  return Index;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZArrayEdit::Dragging(wxMouseEvent& MouseEvent)
{
  if (!mDragging)
  {
    return 0;
  }

  if (mIndex < 0)
  {
    mIndex = GetIndex(MouseEvent);
  }

  wxClientDC Dc(this); // PORTING this is evil and shoud go

  int Value = mNull;
  if (MouseEvent.LeftIsDown())
  {
    int EventX, EventY;
    MouseEvent.GetPosition(&EventX, &EventY);

    Value = (int)((double)(mY + mHeight - EventY) * (mMax - mMin) / mHeight +
      mMin + 0.5);
    Value = Value > mMax ? mMax : Value;
    Value = Value < mMin ? mMin : Value;
  }

  if (MouseEvent.ShiftDown())
  {
    for (int k = 0; k < mArray.Size(); ++k)
    {
      DrawBar(Dc, k, 0);
      mArray[k] = Value;
      DrawBar(Dc, k, 1);
    }
  }
  else if (MouseEvent.ControlDown())
  {
    DrawBar(Dc, mIndex, 0);
    mArray[mIndex] = Value;
    DrawBar(Dc, mIndex, 1);
  }
  else
  {
    int i = GetIndex(MouseEvent);
    int k = i;
    if (i < mIndex)
      for (; i <= mIndex; ++i)
      {
        DrawBar(Dc, i, 0);
        mArray[i] = Value;
        DrawBar(Dc, i, 1);
      }
    else
      for (; i >= mIndex; --i)
      {
        DrawBar(Dc, i, 0);
        mArray[i] = Value;
        DrawBar(Dc, i, 1);
      }
    mIndex = k;
  }

  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZArrayEdit::ButtonDown(wxMouseEvent& MouseEvent)
{
#ifdef __WXMSW__
  CaptureMouse();
#endif
  mDragging = true;
  mIndex = GetIndex(MouseEvent);
  Dragging(MouseEvent);
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZArrayEdit::ButtonUp(wxMouseEvent& MouseEvent)
{
#ifdef __WXMSW__
  ReleaseMouse();
#endif
  mDragging = false;
  mIndex    = -1;
  Refresh();
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::OnMouseEvent(wxMouseEvent& MouseEvent)
{
  if (!mEnabled)
  {
    return;
  }
  if (MouseEvent.ButtonDown())
  {
    ButtonDown(MouseEvent);
  }
  else if (MouseEvent.Dragging())
  {
    Dragging(MouseEvent);
  }
  else if (MouseEvent.ButtonUp())
  {
    ButtonUp(MouseEvent);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::SetEnabled(bool Enabled)
{
  mEnabled = Enabled;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::SetLabel(const string& Label)
{
  mLabel = Label;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::SetYMinMax(int YMin, int YMax)
{
  mArray.SetMinMax(YMin, YMax);
  mYNull = mY + mHeight - mHeight * (mNull - mMin) / (mMax - mMin);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZArrayEdit::DrawBarLine(wxDC& Dc, int XPosition)
{
  if (XPosition > mX && XPosition + 1 < mX + mWidth)
  {
    Dc.SetPen(*wxLIGHT_GREY_PEN);
    Dc.DrawLine(XPosition, mY + 1, XPosition, mY + mHeight - 2);
    Dc.SetPen(*wxBLACK_PEN);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhyArrayEdit::JZRhyArrayEdit(
  wxWindow* pParent,
  JZRndArray& Array,
  const wxPoint& Position,
  const wxSize& Size,
  int StyleBits)
  : JZArrayEdit(pParent, Array, Position, Size, StyleBits),
    mStepsPerCount(4),
    mCountPerBar(4)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhyArrayEdit::SetMeter(int StepsPerCount, int CountPerBar, int BarCount)
{
  mStepsPerCount = StepsPerCount;
  mCountPerBar = CountPerBar;
  mArray.Resize(StepsPerCount * CountPerBar * BarCount);
  SetXMinMax(1, StepsPerCount * CountPerBar * BarCount);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhyArrayEdit::DrawXTicks(wxDC& Dc)
{
  if (!(mStyleBits & ARED_RHYTHM))
  {
    JZArrayEdit::DrawXTicks(Dc);
    return;
  }

  assert(mStepsPerCount && mCountPerBar);

  Dc.SetFont(*wxSMALL_FONT);

  // tick marks
  int TextWidth, TextHeight;
  for (int i = 0; i < mArray.Size(); i += mStepsPerCount)
  {
    int mark = (i / mStepsPerCount) % mCountPerBar + 1;
    ostringstream Oss;
    Oss << mark;
    int YPosition = mY + mHeight;
    int XPosition = (int)(mX + (i + 0.5) * mWidth / mArray.Size());
    Dc.GetTextExtent(Oss.str(), &TextWidth, &TextHeight);
    XPosition -= (int)(TextWidth / 2.0);
    Dc.DrawText(Oss.str(), XPosition, YPosition);
  }
  Dc.SetFont(*wxNORMAL_FONT);
}
