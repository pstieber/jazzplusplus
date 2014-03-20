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

#include "SampleWindow.h"

#include "Mapper.h"
#include "Player.h"
#include "SampleFrame.h"

#include <wx/dcclient.h>

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZInsertionPoint::Draw(int x)
{
  last_x = x;
  visible ^= 1;
  wxDC *dc = new wxClientDC(mpScrolledWindow);
  int cw, ch;
  mpScrolledWindow->GetClientSize(&cw, &ch);
  dc->SetPen(*wxRED_PEN);
  dc->SetLogicalFunction(wxXOR);
  dc->DrawLine(x, 0, x, ch);
  dc->SetPen(*wxBLACK_PEN);
  dc->SetLogicalFunction(wxCOPY);
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplePlayPosition::StopListen()
{
  Stop();
  if (gpMidiPlayer->IsListening())
    gpMidiPlayer->ListenAudio(-1);
  if (visible)
    Draw();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplePlayPosition::StartListen(int fr, int to)
{
  fr_smpl = fr;
  to_smpl = to;
  gpMidiPlayer->ListenAudio(spl, fr_smpl, to_smpl);
  Start(100);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSamplePlayPosition::IsListening() const
{
  return gpMidiPlayer->IsListening();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplePlayPosition::Draw()
{
  visible ^= 1;
  wxDC *dc = new wxClientDC(&cnvs);
  int cw, ch;
  cnvs.GetClientSize(&cw, &ch);
  dc->SetPen(*wxGREEN_PEN);
  dc->SetLogicalFunction(wxXOR);
  dc->DrawLine(x, 0, x, ch);
  dc->SetPen(*wxBLACK_PEN);
  dc->SetLogicalFunction(wxCOPY);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplePlayPosition::Notify()
{
  int pos = mpPlayer->GetListenerPlayPosition();
  if (pos < 0)
  {
    StopListen();
    return;
  }
  if (visible)
    Draw();
  x = cnvs.Sample2Pixel(fr_smpl + pos);
  Draw();
}

#ifdef OBSOLETE
//*****************************************************************************
//*****************************************************************************
class JZSmplWinSettingsForm : public wxForm
{
  public:
    JZSmplWinSettingsForm(JZSampleFrame& SampleFrame)
      : wxForm( USED_WXFORM_BUTTONS ),
        mpSampleFrame(w)
    {}
    void EditForm(wxPanel *panel)
    {
      Add(wxMakeFormBool("Show Midi Time", &mpSampleFrame.cnvs->midi_time));
      Add(wxMakeFormNewLine());
      AssociatePanel(panel);
    }
    void OnOk()
    {
      mpSampleFrame.settings = 0;
      mpSampleFrame.Redraw();
      wxForm::OnOk();
    }
    void OnCancel()
    {
      mpSampleFrame.settings = 0;
      wxForm::OnCancel();
    }
    void OnHelp()
    {
      JZHelp::Instance().ShowTopic("Settings");
    }
  private:
    JZSampleFrame& mpSampleFrame;
};
#endif

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSampleWindow::JZSampleWindow(JZSampleFrame* pSampleFrame, JZSample& Sample)
  : wxScrolledWindow(pSampleFrame),
    mpSampleFrame(pSampleFrame),
    spl(Sample),
    snapsel(this),
    inspt(this)
{
  sel_fr = sel_to = -1;
  mouse_up_sets_insertion_point = 0;
  playpos = new JZSamplePlayPosition(*this, gpMidiPlayer, spl);
  midi_time = true;
  midi_offs = 0;
  mouse_down = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSampleWindow::~JZSampleWindow()
{
  delete playpos;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::OnSize(int w, int h)
{
  int cw, ch;
  GetClientSize(&cw, &ch);
// snapsel.SetYSnap(0, ch, ch / spl.GetChannelCount());
  snapsel.SetYSnap(0, ch, ch);

  AdjustScrollbars();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::OnEvent(wxMouseEvent& MouseEvent)
{
  // dont accept mouse events as long as the
  // array edit is up
  if (mpSampleFrame->on_accept)
  {
    return;
  }

  wxDC* pDc = new wxClientDC(this);

  // tSnapSel is strange.
  if (MouseEvent.LeftDown())
  {
    mouse_up_sets_insertion_point = 0;
    mouse_down = true;
    if (snapsel.IsSelected())
    {
      snapsel.Draw(*pDc, 0, 0);
      snapsel.SetSelected(false);
    }
    else if (inspt.IsVisible())
    {
      inspt.Draw();
    }
    else
    {
      mouse_up_sets_insertion_point = 1;
    }
    snapsel.ProcessMouseEvent(MouseEvent, 0, 0);
  }
  else if (MouseEvent.LeftUp())
  {
    mouse_down = false;
    snapsel.ProcessMouseEvent(MouseEvent, 0, 0);
    if (snapsel.IsSelected())
    {
      snapsel.Draw(*pDc, 0, 0);
      sel_fr = Pixel2Sample(
        snapsel.GetRectangle().x);
      sel_to = Pixel2Sample(
        snapsel.GetRectangle().x + snapsel.GetRectangle().width);
    }
    else if (mouse_up_sets_insertion_point)
    {
      int x, y;
      MouseEvent.GetPosition(&x, &y);
      sel_fr = sel_to = Pixel2Sample(x);
      inspt.Draw(x);
    }
    else
    {
      sel_fr = sel_to = -1;
    }
  }
  else if (MouseEvent.Dragging() && mouse_down)
  {
    snapsel.ProcessMouseEvent(MouseEvent, 0, 0);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::ClearSelection()
{
  if (snapsel.IsSelected())
  {
    wxDC* pDc = new wxClientDC(this);
    snapsel.Draw(*pDc, 0, 0);
    snapsel.SetSelected(false);
  }
  else if (inspt.IsVisible())
  {
    inspt.Draw();
  }
  sel_fr = sel_to = -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::SetInsertionPoint(int offs)
{
  ClearSelection();
  sel_fr = sel_to = offs;
  int x = Sample2Pixel(offs);
  inspt.Draw(x);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::SetSelection(int fr, int to)
{
  ClearSelection();
  sel_fr = fr;
  sel_to = to;
  JZRectangle r;
  r.SetX(Sample2Pixel(fr));
  r.SetWidth(Sample2Pixel(to) - r.x);
  int cw, ch;
  GetClientSize(&cw, &ch);
  r.SetY(0);
  r.SetHeight(ch);
  snapsel.SetRectangle(r);
  snapsel.SetSelected(true);
  wxDC* pDc = new wxClientDC(this);
  snapsel.Draw(*pDc, 0, 0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleWindow::Sample2Pixel(int sample)
{
  int offs   = mpSampleFrame->GetPaintOffset();
  int length = mpSampleFrame->GetPaintLength();
  int cw, ch;
  GetClientSize(&cw, &ch);
  JZMapper Map(offs, offs + length, 0, cw);
  return static_cast<int>(Map.XToY(sample));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleWindow::Pixel2Sample(float pixel)
{
  int offs   = mpSampleFrame->GetPaintOffset();
  int length = mpSampleFrame->GetPaintLength();
  int cw, ch;
  GetClientSize(&cw, &ch);
  JZMapper Map(0, cw, offs, offs + length);
  int ofs = static_cast<int>(Map.XToY(pixel));
  return spl.Align(ofs);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::OnPaint()
{
  paint_offset = mpSampleFrame->GetPaintOffset();
  paint_length = mpSampleFrame->GetPaintLength();

  wxDC *dc = new wxPaintDC(mpSampleFrame);
//OBSOLETE  dc->BeginDrawing();
  if (inspt.IsVisible())
    inspt.Draw();  // clear insertion point if there
  dc->Clear();

  int cw, ch;
  GetClientSize(&cw, &ch);
  int n = spl.GetChannelCount();
  for (int i = 0; i < n; ++i)
  {
    int x = 0;
    int y = ch * i / n;
    int w = cw;
    int h = ch / n;
    DrawSample(i, x, y, w, h);
    // separate the channels
    dc->DrawLine(x, y, x+w, y);
    if (i > 0)  // not the first one
      DrawTicks(x, y, w);
  }

  if (snapsel.IsSelected())
  {
    JZRectangle r;
    r.SetX(Sample2Pixel(sel_fr));
    r.SetWidth(Sample2Pixel(sel_to) - r.x);
    r.SetY(0);
    r.SetHeight(ch);
    snapsel.SetRectangle(r);
    wxDC* pDc = new wxClientDC(this);
    snapsel.Draw(*pDc, 0, 0);
  }
  else if (sel_fr > 0)
  {
    int x = Sample2Pixel(sel_fr);
    inspt.Draw(x);
  }

//OBSOLETE  dc->EndDrawing();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::DrawTicks(int x, int y, int w)
{
  wxDC *dc = new wxClientDC(this);
  wxFont f = dc->GetFont();
  dc->SetFont(*wxSMALL_FONT);

  int sfr = mpSampleFrame->GetPaintOffset();
  int sto = sfr + mpSampleFrame->GetPaintLength();

  if (!midi_time)
  {
    // display time
    JZMapper Map(sfr, sto, x, x+w);
    int tfr = spl->Samples2Time(sfr) / 1000;
    int tto = spl->Samples2Time(sto) / 1000 + 1;
    for (int sec = tfr; sec < tto; sec++)
    {
      for (int mil = 0; mil < 1000; mil += 100)
      {
        int t = spl->Time2Samples(sec * 1000 + mil);
        int xx = static_cast<int>(Map.XToY(t));
        // draw a tickmark line
        dc->DrawLine(xx, y - 5, xx, y);

        // draw a text
        wxString Time;
        Time << sec << '.' << mil / 100;
        int fw, fh;
        dc->GetTextExtent(Time, &fw, &fh);
        dc->DrawText(Time, xx - fw/2, y + 2);
      }
    }
  }
  else
  {
    // Display midi counts.
    int cfr = static_cast<int>(spl->Samples2Ticks(sfr));
    int cto = static_cast<int>(spl->Samples2Ticks(sto));
    JZMapper Map(cfr, cto, x, x+w);
    JZBarInfo BarInfo(*gpSong);
    BarInfo.SetClock(cfr);
    BarInfo.SetBar(BarInfo.GetBarIndex());
    while (BarInfo.GetClock() < cto)
    {
      int ticks_per_count =
        BarInfo.GetTicksPerBar() / BarInfo.GetCountsPerBar();
      int ticks_per_step = ticks_per_count / 4;
      for (int i = 0; i < BarInfo.GetCountsPerBar(); ++i)
      {
        for (int j = 0; j < 4; j++)
        {
          int clock =
            BarInfo.GetClock() + i * ticks_per_count + j * ticks_per_step;
          int xx = static_cast<int>(Map.XToY(clock));
          // draw a tickmark line
          dc->DrawLine(xx, y - 5, xx, y);

          // draw a text
          if (j == 0)
          {
            wxString String;
            String << i + 1;
            int fw, fh;
            dc->GetTextExtent(String, &fw, &fh);
            dc->DrawText(String, xx - fw/2, y + 2);
          }
        }
      }

      BarInfo.Next();
    }
  }

  dc->SetFont(f);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::DrawSample(int channel, int x, int y, int w, int h)
{
  const short* data = spl.GetData();
  int length = spl.GetLength();
  int step = spl.GetChannelCount();

  // compute display range from position scrollbar
  int xfr = paint_offset + channel;
  int xto = paint_offset + paint_length;
  if (xto > length)
    xto = length;

  if (xfr >= xto)
    return;

  JZMapper XMap(xfr, xto, x, x + w);
  JZMapper YMap(-32767.0, 32767.0, y+h, y);

  wxDC *dc = new wxClientDC(this);

  short prev_ymin = 0;
  short prev_ymax = 0;
  short ymin = 0;
  short ymax = 0;
  int x1 = x;
  for (int n = xfr; n < xto; n += step)
  {
    int x2 = static_cast<int>(XMap.XToY(n));
    short sy = data[n];
    if (x1 != x2)
    {
      // new x-coordinate

      short y1min, y1max;
      if (prev_ymin > ymax)
        y1max = prev_ymin;
      else
        y1max = ymax;

      if (prev_ymax < ymin)
        y1min = prev_ymax;
      else
        y1min = ymin;

      int y1 = static_cast<int>(YMap.XToY(y1min));
      int y2 = static_cast<int>(YMap.XToY(y1max));
      dc->DrawLine(x1, y1, x1, y2);
      prev_ymin = ymin;
      prev_ymax = ymax;
      ymin = sy;
      ymax = sy;
      x1   = x2;
    }
    else
    {
      if (sy > ymax)
        ymax = sy;
      else if (sy < ymin)
        ymin = sy;
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleWindow::Play()
{
  if (playpos->IsListening())
    playpos->StopListen();
  else
  {
    int fr_smpl = sel_fr > 0L ? sel_fr : -1L;
    int to_smpl = sel_to > sel_fr ? sel_to : -1L;
    playpos->StartListen(fr_smpl, to_smpl);
  }
}
