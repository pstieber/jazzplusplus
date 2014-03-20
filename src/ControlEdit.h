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

#include "Random.h"

#include <wx/panel.h>

class JZPianoWindow;
class JZTrack;
class JZEvent;
class JZCtrlEditBase;
class wxButton;

// to access JZCtrlEditBase from Buttons etc

class JZControlPanel : public wxPanel
{
  public:

    friend class JZCtrlEditBase;

    JZControlPanel(
      JZCtrlEditBase* e,
      wxWindow* pParent,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxDefaultSize,
      long style=0,
      const char* pName = "panel")
      : wxPanel(pParent, wxID_ANY, Position, Size, style, pName)
    {
      edit = e;
    }

    JZCtrlEditBase *edit;
};




class JZCtrlEditBase : public JZArrayEditDrawBars
{
  public:

    JZCtrlEditBase(
      int min,
      int max,
      JZPianoWindow* pPianoWindow,
      char const* pLabel,
      int xoff,
      int x,
      int y,
      int w,
      int h,
      int mode = 0);

    virtual ~JZCtrlEditBase();

    void SetSize(int xoff, int x, int y, int w, int h);

    void ReInit(JZTrack *track, long FromClock, long ClocksPerPixel);

// SN++ Default = 0, 1 bedeutet der Editor arbeitet auch auf Selektionen.
//      Dieser Patch zusammen mit dem "selectable Patch" im PianoWin
//      ist for VelocEdit und AftertouchEdit Updates.
    int selectable;

    virtual void UpDate();

  protected:

    virtual int Missing()  = 0;

    virtual int IsCtrlEdit(JZEvent *e)  = 0;

    virtual int GetValue(JZEvent *e) = 0;

    virtual JZEvent* NewEvent(long clock, int val)
    {
      return 0;
    }

    virtual void OnApply();
    virtual void OnRevert();
    virtual void OnEdit();
    virtual void OnBars();

    long Clock2i(long clock);
    long i2Clock(long i);
    int  Clock2Val(long clock);
    int  sticky;

// SN++
    int x_off;
    int ctrlmode;
    JZTrack *track;
    long   from_clock;
    long   to_clock;
    long   i_max;
    long   clocks_per_pixel;
    JZRndArray  array;

    JZArrayEdit* edit;
    JZPianoWindow* mpPianoWindow;
    JZControlPanel* panel;

  private:

    void Create(
      JZPianoWindow* p,
      char const* pLabel,
      int dx,
      int x,
      int y,
      int w,
      int h);

    static void Apply(wxButton& but, wxCommandEvent& event);
    static void Revert(wxButton& but, wxCommandEvent& event);
// SN++
    static void Edit(wxButton &but, wxCommandEvent& event);
    static void Bars(wxButton &but, wxCommandEvent& event);
    void DrawBars(wxDC& Dc);
};

class JZPitchEdit : public JZCtrlEditBase
{
  public:

    JZPitchEdit(
      JZPianoWindow* pPianoWindow,
      char const* pLabel,
      int xoff,
      int x,
      int y,
      int w,
      int h);

  protected:

    virtual int Missing();
    virtual int IsCtrlEdit(JZEvent *e);
    virtual int GetValue(JZEvent *e);
    virtual JZEvent * NewEvent(long clock, int val);
};

// SN++ Key Aftertouch
class JZPolyAfterEdit : public JZCtrlEditBase
{
  public:

    JZPolyAfterEdit(
      JZPianoWindow* pPianoWindow,
      char const* pLabel,
      int xoff,
      int x,
      int y,
      int w,
      int h);

  protected:

    virtual int Missing();
    virtual int IsCtrlEdit(JZEvent *e);
    virtual int GetValue(JZEvent *e);
    virtual void OnApply();
};

// SN++ Channel Aftertouch
class JZChannelAftertouchEdit : public JZCtrlEditBase
{
  public:

    JZChannelAftertouchEdit(
      JZPianoWindow* pPianoWindow,
      char const* pLabel,
      int xoff,
      int x,
      int y,
      int w,
      int h);

  protected:

    virtual int Missing();
    virtual int IsCtrlEdit(JZEvent *e);
    virtual int GetValue(JZEvent *e);
    virtual JZEvent * NewEvent(long clock, int val);
    virtual void OnApply();
    virtual void UpDate();
};

class JZControlEdit : public JZCtrlEditBase
{
  public:

    JZControlEdit(
      int CtrlNum,
      JZPianoWindow* pPianoWindow,
      char const* pLabel,
      int xoff,
      int x,
      int y,
      int w,
      int h);

  protected:

    virtual int Missing();
    virtual int IsCtrlEdit(JZEvent *e);
    virtual int GetValue(JZEvent *e);
    virtual JZEvent * NewEvent(long clock, int val);
  private:
    int ctrl_num;
};

class JZVelocityEdit : public JZCtrlEditBase
{
  public:

    JZVelocityEdit(
      JZPianoWindow* pPianoWindow,
      char const* pLabel,
      int xoff,
      int x,
      int y,
      int w,
      int h);

  protected:

    virtual int Missing();
    virtual int IsCtrlEdit(JZEvent *e);
    virtual int GetValue(JZEvent *e);
    virtual void OnApply();
};

class JZTempoEdit : public JZCtrlEditBase
{
  public:

    JZTempoEdit(
      int min,
      int max,
      JZPianoWindow* pPianoWindow,
      char const* pLabel,
      int xoff,
      int x,
      int y,
      int w,
      int h);

  protected:

    virtual int Missing();
    virtual int IsCtrlEdit(JZEvent *e);
    virtual int GetValue(JZEvent *e);
    virtual JZEvent * NewEvent(long clock, int val);
};
