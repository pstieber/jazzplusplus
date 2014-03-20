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

#include "SliderWindow.h"

#include "Random.h"
#include "ToolBar.h"

#include <wx/panel.h>

// These two lines are here to allow cout usage.
#include <iostream>

using namespace std;

JZSliderWindow::JZSliderWindow(
  wxWindow* pParent,
  const wxString& Title,
  int geo[4],
  JZToolDef *tdefs)
  : wxFrame(
      pParent,
      wxID_ANY,
      Title,
      wxPoint(geo[0], geo[1]),
      wxSize(geo[2], geo[3]))
{
  this->geo = geo;
  in_constructor = true;
  n_sliders   = 0;
  sliders_per_row = 1;
  panel = new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(1000, 1000));

  if (tdefs != NULL)
    mpToolBar=new JZToolBar(this, tdefs);
  else
    mpToolBar = 0;
}

void JZSliderWindow::Initialize()
{
  AddItems();
  panel->Fit();
  AddEdits();
  in_constructor = false;

  int cw, ch;
  GetClientSize(&cw, &ch);

#ifdef OBSOLETE
  OnSize(cw, ch);
#endif
}


JZSliderWindow::~JZSliderWindow()
{
  GetPosition(&geo[0], &geo[1]);
  GetSize(&geo[2], &geo[3]);
  for (int i = 0; i < n_sliders; ++i)
  {
    delete sliders[i];
  }
}


BEGIN_EVENT_TABLE(JZSliderWindow, wxFrame)
  EVT_SIZE(JZSliderWindow::OnSize)
END_EVENT_TABLE()

  /**called from the event table whenever the window is resized

  the resizing algorithm here no longer works for some reason


*/
void JZSliderWindow::OnSize(wxSizeEvent& Event)
{
  cout <<"JZSliderWindow::OnSize "<<in_constructor<<endl;
//  wxSize sz = Event.GetSize();

  if (in_constructor)
  {
    return;
  }

  int cw, ch;
  GetClientSize(&cw, &ch);
  int pw = 0;
  int ph = 0;
  panel->GetSize(&pw, &ph);
//  if (mpToolBar)
//  {
//    int tw = 0;
//    int th = 0;
//    mpToolBar->GetMaxSize(&tw, &th);
//    mpToolBar->SetSize(0, 0, (int)cw, (int)th);
//    panel->SetSize(0, (int)th, (int)cw, (int)ph);
//    ph += (int)th; // add toolbar height to panel height
//  }
//  else
//  {
    panel->SetSize(0, 0, cw, ph);
//  }

  int n_rows = (n_sliders - 1) /  sliders_per_row + 1;
  for (int row = 0; row < n_rows; row++)
  {
    float y0 = ph + row * (ch - ph) / n_rows;
    float y1 = y0 + (ch - ph) / n_rows;
    int n_cols = sliders_per_row;
    if (row == n_rows - 1)
      n_cols = (n_sliders - 1) % sliders_per_row + 1;
    for (int col = 0; col < n_cols; col++)
    {
      int k = row * sliders_per_row + col;
      float x0 = col * (float)cw / n_cols;
      float x1 = x0 + (float)cw / n_cols;
      sliders[k]->SetSize((int)x0, (int)y0, (int)(x1 - x0), (int)(y1 - y0));
//      sliders[k]->SetSize((int)x0, 0, (int)(x1 - x0), (int)(y0-y1));
      cout
        << "slider " << k << " size:" << (int)x0 << ' ' << (int)y0
        << ' ' << (int)(x1 - x0) << ' ' << (int)(y1 - y0)
        << endl;
    }
  }

}

bool JZSliderWindow::OnClose()
{
  return true;
}

void JZSliderWindow::AddItems()
{
#ifdef OBSOLETE
 (void)new wxButton(panel, (wxFunction)ItemCallback, "MyButton") ;
#endif
}


void JZSliderWindow::AddEdits()
{
  n_sliders = 2;
  sliders_per_row = 2;
  for (int i = 0; i < n_sliders; ++i)
  {
    sliders[i] = new JZRhyArrayEdit(
      this,
      *new JZRndArray(20, 0, 100),
      wxPoint(10, 10),
      wxSize(10, 10),
      ARED_GAP | ARED_XTICKS);
  }
}

#ifdef OBSOLETE
void JZSliderWindow::OnItem(wxItem& item, wxCommandEvent& event)
{
}


void JZSliderWindow::ItemCallback(wxItem& item, wxCommandEvent& event)
{
  ((JZSliderWindow *)(item.GetParent()->GetParent()))->OnItem(item, event);
}

#endif

void JZSliderWindow::ForceRepaint()
{
#ifdef OBSOLETE
  int cw, ch;
  GetClientSize(&cw, &ch);
  OnSize(cw, ch);

  OnPaint();

  for (int i = 0; i < n_sliders; i++)
    sliders[i]->OnPaint();
#endif
}
