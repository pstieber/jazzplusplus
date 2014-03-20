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

#include "MouseAction.h"

#include <wx/scrolwin.h>

class JZPlayer;
class JZSample;
class JZSampleFrame;
class JZSampleWindow;

//*****************************************************************************
//*****************************************************************************
class JZInsertionPoint
{
  public:

    JZInsertionPoint(wxScrolledWindow* pScrolledWindow)
      : mpScrolledWindow(pScrolledWindow)
    {
      last_x = 0;
      visible = 0;
    }

    void Draw(int x);

    void Draw()
    {
      Draw(last_x);
    }

    int IsVisible() const
    {
      return visible;
    }

    float GetX() const
    {
      return last_x;
    }

  private:

    int last_x;
    int visible;
    wxScrolledWindow* mpScrolledWindow;
};

//*****************************************************************************
//*****************************************************************************
class JZSamplePlayPosition : public wxTimer
{
  public:

    JZSamplePlayPosition(
      JZSampleWindow& SampleWindow,
      JZPlayer* pPlayer,
      JZSample& Sample)
      : cnvs(SampleWindow),
        mpPlayer(pPlayer),
        spl(Sample)
    {
      visible = false;
      x = 0;
    }

    ~JZSamplePlayPosition()
    {
      Stop();
      if (visible)
        Draw();
    }

    void StopListen();

    void StartListen(int fr, int to);

    bool IsListening() const;

    void Draw();

    virtual void Notify();

  private:
    JZSampleWindow& cnvs;
    JZPlayer* mpPlayer;
    JZSample& spl;
    bool visible;
  int x;
    int fr_smpl;
    int to_smpl;
};

//*****************************************************************************
//*****************************************************************************
class JZSampleWindow : public wxScrolledWindow
{
  friend class JZSampleFrame;
  friend class JZSmplWinSettingsForm;

  public:

    JZSampleWindow(JZSampleFrame* pSampleFrame, JZSample& Sample);

    virtual ~JZSampleWindow();

    void Redraw()
    {
      OnPaint();
    }

    virtual void OnPaint();

    virtual void OnSize(int w, int h);

    virtual void OnEvent(wxMouseEvent& MouseEvent);

    void ClearSelection();

    void SetInsertionPoint(int offs);

    void SetSelection(int fr, int to);

    int Sample2Pixel(int sample);

    int Pixel2Sample(float pixel);

    void Play();

  private:

    void DrawSample(int channel, int x, int y, int w, int h);

    void DrawTicks(int x, int y, int w);

  private:

    JZSampleFrame* mpSampleFrame;

    JZSample& spl;

    int paint_offset;
    int paint_length;

    JZSnapSelection snapsel;

    // sel_fr == 0: no selection and no insertion point
    // sel_fr >  0 && sel_fr == sel_to: insertion point
    // sel_fr >  0 && sel_fr <  sel_to: selected range
    int sel_fr, sel_to;
    JZInsertionPoint inspt;
    int mouse_up_sets_insertion_point;
    JZSamplePlayPosition *playpos;

    // for tickmark display
    bool midi_time;
    int midi_offs;

    bool mouse_down;
};
