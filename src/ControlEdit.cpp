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

#include "ControlEdit.h"

#include "EventWindow.h"
#include "Filter.h"
#include "PianoWindow.h"
#include "Project.h"
#include "Track.h"

#include <wx/button.h>
#include <wx/sizer.h>

#include <algorithm>

static const long wbar = 2;
static int bars_state = 2;  // from ArrayEdit

JZCtrlEditBase::JZCtrlEditBase(
  int min,
  int max,
  JZPianoWindow* p,
  char const *label,
  int dx,
  int x,
  int y,
  int w,
  int h,
  int ctrledit)
  : array((w-dx)/wbar, min, max)
{
  ctrlmode = ctrledit;
  selectable = 0;
  Create(p, label, dx, x, y, w, h);
}

void JZCtrlEditBase::Create(
  JZPianoWindow* pPianoWindow,
  char const *label,
  int dx,
  int x,
  int y,
  int w,
  int h)
{
  x_off = dx;
  mpPianoWindow = pPianoWindow;
  track  = 0;
  from_clock = 0;
  to_clock = 1;
  i_max    = 1;
  clocks_per_pixel = 0;
  sticky = 1;

  panel = new JZControlPanel(this, mpPianoWindow, wxPoint(x, y), wxSize(dx, h), 0, "Controller Edit");
  //(void) new wxMessage(panel, (char *)label);
  //panel->NewLine();

  // PORTING: changed the calls a bit so it would compile,
  // need to remake the layout and do the event bindings

  ctrlmode = 0;  // Edit seems stupid to me.

  wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

  if (!ctrlmode)
  {
    topsizer->Add(new wxButton(panel, wxID_ANY, "Apply")) ;
    //(wxFunction)Apply,
    topsizer->Add(new wxButton(panel, wxID_ANY, "Revert")) ;
    //(wxFunction)Revert,

    //(void)new wxButton(panel, (wxFunction)Bars,   "Bars");
  }
  else
  {
    topsizer->Add(new wxButton(panel, wxID_ANY, "Create"));  // create new events (wxFunction)Apply,
    topsizer->Add(new wxButton(panel, wxID_ANY,"Change"));  // change existing events (wxFunction)Edit,
    topsizer->Add(new wxButton(panel, wxID_ANY, "Revert")); //(wxFunction)Revert,
    //(void)new wxButton(panel, (wxFunction)Bars,   "Bars");
  }
  ctrlmode = 0;
  // ctrlmode is used to distinguish between Apply and Edit.

  edit = new JZArrayEdit(
    mpPianoWindow,
    array,
    wxPoint(x + dx, y),
    wxSize(w - dx, h),
    0);
  edit->SetLabel(label);
  edit->SetDrawBars(this);

  panel->SetAutoLayout( TRUE );     // tell dialog to use sizer
  panel->SetSizer( topsizer );      // actually set the sizer

  topsizer->Fit( panel );            // set size to minimum size as calculated by the sizer
  topsizer->SetSizeHints( panel );   // set size hints to honour mininum size
}


JZCtrlEditBase::~JZCtrlEditBase()
{
  delete panel;
  delete edit;
}

// SN++
void JZCtrlEditBase::UpDate()
{
  if (!selectable)
  {
    return;
  }
  OnRevert();
}
//

void JZCtrlEditBase::SetSize(int dx, int x, int y, int w, int h)
{
  array.Resize((long)(w-dx) / wbar);
  // av-- edit->array_val.Resize((long)(w-dx) / wbar);
  panel->SetSize(x, y, dx, h);
  edit->SetSize(x+dx, y, w - dx, h);
  ReInit(track, from_clock, clocks_per_pixel);
}


void JZCtrlEditBase::ReInit(JZTrack *t, long fc, long cpp)
{
  int w, h;
  edit->GetSize(&w, &h);
  track = t;
  from_clock = fc;
  to_clock = from_clock + (long)w * clocks_per_pixel;
  clocks_per_pixel = cpp;
  i_max = Clock2i(to_clock);
  OnRevert();

}

long JZCtrlEditBase::Clock2i(long clock)
{
  return (clock - from_clock) / clocks_per_pixel / wbar;
}

long JZCtrlEditBase::i2Clock(long i)
{
  return i * clocks_per_pixel * wbar + from_clock;
}

int JZCtrlEditBase::Clock2Val(long clock)
{
  long i = Clock2i(clock);
  if (i >= i_max-1)
  {
    //* PAT - The following ifdef was removed due to changes in gcc 3.x.  If
    // it needs to be put back for compatibility purposes, it will need to
    // return in an alternate form.
//    #ifdef FOR_MSW
    return array[(int)(i_max - 1)];
  }
  return array[(int)i];
//#else
//  {
//    return array[i_max-1];
//  }
//  return array[i];
//#endif

#if 0
  long v1 = array[i];
  long v2 = array[i+1];
  long c1 = i2Clock(i);
  long c2 = i2Clock(i+1);
  int  val = (v2 - v1) * (clock - c1) / (c2 - c1) + v1;
  return val;
#endif
}

void JZCtrlEditBase::OnRevert()
{
  int i;

  JZEventIterator iter(track);
  int val = Missing();

  if (sticky && !selectable)
  {
    JZEvent* pEvent = iter.Range(0, from_clock);
    while (pEvent)
    {
      if (IsCtrlEdit(pEvent))
      {
        val = GetValue(pEvent);
      }
      pEvent = iter.Next();
    }
  }

  JZEvent* pEvent = iter.Range(from_clock, to_clock);

  for (i = 0; i < array.Size(); i++)
  {
    array[i] = val;
  }

  i = 0;
  while (pEvent)
  {
    if (IsCtrlEdit(pEvent))
    {
      int k = Clock2i(pEvent->GetClock());
      if (sticky)
      {
        while (i < k)
        {
          array[i++] = val;
        }
      }
      val = GetValue(pEvent);
      array[k] = val;
    }
    pEvent = iter.Next();
  }
  if (sticky && !selectable)
  {
    while (i < array.Size())
    {
      array[i++] = val;
    }
  }

  edit->Refresh();
}


//void JZCtrlEditBase::Revert(wxButton &but, wxCommandEvent& event)
//{
//  JZControlPanel *panel = (JZControlPanel *)but.GetParent();
//  panel->edit->OnRevert();
//}


void JZCtrlEditBase::OnApply()
{
  wxBeginBusyCursor();
  mpPianoWindow->GetProject()->NewUndoBuffer();
  // delete old events, but skip clock 0 to preserve track defaults:
  // (dirty but might work...)
  JZEventIterator iter(track);
  JZEvent* pEvent = iter.Range(std::max(1L, from_clock), to_clock);
  int old_val = Missing();

// SN++ events nur im apply-mode loeschen!
  if (!ctrlmode)
  {
    while (pEvent)
    {
      if (IsCtrlEdit(pEvent))
      {
        track->Kill(pEvent);
      }
      pEvent = iter.Next();
    }

    // find any previous events
    if (sticky)
    {
      pEvent = iter.Range(0, from_clock - 1);
      while (pEvent)
      {
        if (IsCtrlEdit(pEvent))
        {
          old_val = GetValue(pEvent);
        }
        pEvent = iter.Next();
      }
    }

    // SN++ set-Mode
    // create new events
    long clock;
    for (clock = from_clock; clock < to_clock; clock++)
    {
      int new_val = Clock2Val(clock);

      if (old_val != new_val)
      {
        pEvent = NewEvent(clock, new_val);
        track->Put(pEvent);
        old_val = new_val;
      }
    }
  }
  else
  {
    // edit mode: Erzeugt keine neuen Events sondern aendert den Wert
    // bestehender Events.
    // SN++
    JZControlEvent* pControlCopy;
    while (pEvent)
    {
      if (IsCtrlEdit(pEvent))
      {
        if (
          Clock2Val(pEvent->GetClock()) !=
            pEvent->IsControl()->GetControlValue())
        {
          pControlCopy = pEvent->Copy()->IsControl();
          pControlCopy->SetControlValue(Clock2Val(pEvent->GetClock()));
          track->Kill(pEvent);
          track->Put(pControlCopy);
        }
      }
      pEvent = iter.Next();
    }
  }

  // done
  track->Cleanup();
  wxEndBusyCursor();
  OnRevert();

  // SN+ Bug Fix Controller in Piano Fenster updaten.
  mpPianoWindow->Refresh();
}

// SN++
void JZCtrlEditBase::Bars(wxButton &but, wxCommandEvent& event)
{
  ((JZControlPanel *)but.GetParent())->edit->OnBars();
}

void JZCtrlEditBase::OnBars()
{
  // Bars und Werte updaten
  if (bars_state < 2)
  {
    bars_state++;
  }
  else
  {
    bars_state=0;
  }
  edit->Refresh();
}

/*void JZCtrlEditBase::Apply(wxButton &but, wxCommandEvent& event)
{
  ((JZControlPanel *)but.GetParent())->edit->OnApply();
}


void JZCtrlEditBase::Edit(wxButton &but, wxCommandEvent& event)
{
  ((JZControlPanel *)but.GetParent())->edit->OnEdit();
}
*/

void JZCtrlEditBase::OnEdit()
{
  ctrlmode = 1;  // edit current events
  OnApply();
  ctrlmode = 0;
}

// SN++ Has 3 Modes (bars_state)  0: no Bars, 1,2: draw Bars
// av: called by JZArrayEdit::OnPaint
void JZCtrlEditBase::DrawBars(wxDC& Dc)
{
  JZBarInfo BarInfo(*mpPianoWindow->GetProject());
  BarInfo.SetClock(from_clock);
  long gclk,x;
  int  ii;
  if (bars_state > 0)
  {
    gclk = BarInfo.GetClock();
    while (gclk < to_clock)
    {
      gclk = BarInfo.GetClock();
      x = mpPianoWindow->Clock2x(gclk-from_clock);
      edit->DrawBarLine(Dc, x - x_off);
      if (bars_state == 2)
      {
        for (ii = 0; ii < BarInfo.GetCountsPerBar(); ++ii)
        {
          gclk += BarInfo.GetTicksPerBar() / BarInfo.GetCountsPerBar();
          x = mpPianoWindow->Clock2x(gclk-from_clock);
          edit->DrawBarLine(Dc, x - x_off);
        }
      }
      BarInfo.Next();
    }
  }
}


// ------------------------------------------------------------------

JZPitchEdit::JZPitchEdit(
  JZPianoWindow* pPianoWindow,
  char const *label,
  int xoff,
  int x,
  int y,
  int w,
  int h)
  : JZCtrlEditBase(-8191, 8191, pPianoWindow, label, xoff, x, y, w, h)
{
}

int JZPitchEdit::Missing()
{
  return 0;
}

int JZPitchEdit::IsCtrlEdit(JZEvent* pEvent)
{
  return pEvent->IsPitch() != 0;
}

int JZPitchEdit::GetValue(JZEvent* pEvent)
{
  return pEvent->IsPitch()->Value;
}

JZEvent * JZPitchEdit::NewEvent(long clock, int val)
{
  return new JZPitchEvent(clock, track->mChannel - 1, val);
}

// ------------------------------------------------------------------

JZControlEdit::JZControlEdit(
  int CtrlNum,
  JZPianoWindow* pPianoWindow,
  char const *label,
  int xoff,
  int x,
  int y,
  int w,
  int h)
  : JZCtrlEditBase(0, 127, pPianoWindow, label, xoff, x, y, w, h, 1)
{
  ctrl_num = CtrlNum;
  if (ctrl_num == 10)  // panpot
  {
    array.SetNull(64);
  }
}

int JZControlEdit::Missing()
{
  if (ctrl_num == 10)
  {
    return 64;
  }
  return 0;
}

int JZControlEdit::IsCtrlEdit(JZEvent* pEvent)
{
  JZControlEvent* pControl = pEvent->IsControl();
  return (pControl && pControl->GetControl() == ctrl_num);
}

int JZControlEdit::GetValue(JZEvent* pEvent)
{
  return pEvent->IsControl()->GetControlValue();
}

JZEvent * JZControlEdit::NewEvent(long clock, int val)
{
  return new JZControlEvent(clock, track->mChannel - 1, ctrl_num, val);
}

// ------------------------------------------------------------------

JZVelocityEdit::JZVelocityEdit(
  JZPianoWindow* pParent,
  char const *label,
  int xoff,
  int x,
  int y,
  int w,
  int h)
  : JZCtrlEditBase(1, 127, pParent, label, xoff, x, y, w, h)
{
  sticky = 0;
  selectable = 1;
}

int JZVelocityEdit::Missing()
{
  return 1;
}

int JZVelocityEdit::IsCtrlEdit(JZEvent* pEvent)
{
  // SN++ Falls im PianoWin Events selektiert sind, werden nur diese
  //      Events geaendert
  if (!mpPianoWindow->AreEventsSelected())
  {
    return (pEvent->IsKeyOn() != 0);
  }
  else
  {
    if (pEvent->IsKeyOn())
    {
      return (
        mpPianoWindow->GetFilter()->IsSelected(pEvent) &&
        (pEvent->GetClock() >= mpPianoWindow->GetFilter()->GetFromClock() &&
          pEvent->GetClock() <= mpPianoWindow->GetFilter()->GetToClock()));
    }
  }
  return 0;
}

int JZVelocityEdit::GetValue(JZEvent* pEvent)
{
  return pEvent->IsKeyOn()->GetVelocity();
}

void JZVelocityEdit::OnApply()
{
  static long from_clk, to_clk;

  wxBeginBusyCursor();
  mpPianoWindow->GetProject()->NewUndoBuffer();

  JZEventIterator iter(track);

  if (mpPianoWindow->AreEventsSelected())
  {
    from_clk = mpPianoWindow->GetFilter()->GetFromClock();
    to_clk   = mpPianoWindow->GetFilter()->GetToClock();
  }
  else
  {
    from_clk = from_clock;
    to_clk   = to_clock;
  }

  JZEvent* pEvent = iter.Range(from_clk, to_clk);

  while (pEvent)
  {
    // SN++ Falls im PianoWin Events selektiert sind, werden nur diese
    //      Events geaendert
    if (
      !mpPianoWindow->AreEventsSelected() ||
      mpPianoWindow->GetFilter()->IsSelected(pEvent))
    {

      JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
      if (pKeyOn)
      {
        JZKeyOnEvent* pKeyOnCopy = pKeyOn->Copy()->IsKeyOn();

        int i = Clock2i(pKeyOnCopy->GetClock());
        pKeyOnCopy->SetVelocity(array[i]);
        track->Kill(pKeyOn);
        track->Put(pKeyOnCopy);
      }
    }
    pEvent = iter.Next();
  }
  track->Cleanup();
  wxEndBusyCursor();
  OnRevert();

  // SN+ for Color Darstellung
  mpPianoWindow->Refresh();
}

// ------------------------------------------------------------------

JZPolyAfterEdit::JZPolyAfterEdit(
  JZPianoWindow* pPianoWindow,
  char const *label,
  int xoff,
  int x,
  int y,
  int w,
  int h)
  : JZCtrlEditBase(0, 127, pPianoWindow, label, xoff, x, y, w, h, 1)
{
  sticky = 0;  // SN must be set for proper editing!
  selectable = 1;
}


int JZPolyAfterEdit::Missing()
{
  return 0;
}

int JZPolyAfterEdit::IsCtrlEdit(JZEvent* pEvent)
{
  // SN++ Falls im PianoWin Events selektiert sind, werden nur diese
  //      Events geaendert

  if (!mpPianoWindow->AreEventsSelected())
  {
    return pEvent->IsKeyPressure() != 0;
  }
  else
  {
    if (pEvent->IsKeyPressure())
    {
      return (
        mpPianoWindow->GetFilter()->IsSelected(pEvent) &&
        (pEvent->GetClock() >= mpPianoWindow->GetFilter()->GetFromClock() &&
        pEvent->GetClock() <= mpPianoWindow->GetFilter()->GetToClock()));
    }
  }
  return 0;
}

int JZPolyAfterEdit::GetValue(JZEvent* pEvent)
{
  JZKeyPressureEvent* pKeyPressure = pEvent->IsKeyPressure();
  if (pKeyPressure)
  {
    return pKeyPressure->GetPressureValue();
  }
  return -1;
}


void JZPolyAfterEdit::OnApply()
{
  static long from_clk, to_clk;
  JZEvent* pEvent;

  // SN++ Apply works only if some events are selected !!
  if (!mpPianoWindow->AreEventsSelected())
  {
    OnRevert();
    return;
  }

  wxBeginBusyCursor();
  mpPianoWindow->GetProject()->NewUndoBuffer();

  JZEventIterator iter(track);

  if (mpPianoWindow->AreEventsSelected())
  {
    from_clk = mpPianoWindow->GetFilter()->GetFromClock();
    to_clk   = mpPianoWindow->GetFilter()->GetToClock();
  }
  else
  {
    from_clk = from_clock;
    to_clk   = to_clock;
  }
  JZKeyOnEvent* pKeyOn;

  if (!ctrlmode)
  {
    // OnApply

    // SN++ Alle selektierten AfterTouch events loeschen
    pEvent = iter.Range(from_clk, to_clk);
    while (pEvent)
    {
      if (
        !mpPianoWindow->AreEventsSelected() ||
        mpPianoWindow->GetFilter()->IsSelected(pEvent))
      {
        JZKeyPressureEvent* pKeyPressure = pEvent->IsKeyPressure();
        if (pKeyPressure)
        {
          track->Kill(pKeyPressure);
        }
      }
      pEvent = iter.Next();
    }
    // SN++ Neue Aftertouch's von KeyOn bis KeyLength einfuehgen;
    long key_end(-1), key_clk(-1);
    int  key_val = -1;
    int  key_cha(-1);
    JZKeyPressureEvent* pKeyPressure;
    pEvent = iter.Range(from_clk, to_clk);
    while (pEvent)
    {
      if (
        !mpPianoWindow->AreEventsSelected() ||
        mpPianoWindow->GetFilter()->IsSelected(pEvent))
      {
        pKeyOn = pEvent->IsKeyOn();
        if (pKeyOn)
        {
          key_clk = pKeyOn->GetClock() + 1;
          key_end = pKeyOn->GetClock() + pKeyOn->GetEventLength();
          key_val = pKeyOn->GetKey();
          key_cha = pKeyOn->GetChannel();
        }
        if (key_val>0)
        {
          int i,temp=0;
          for (long iclk=key_clk;iclk<key_end && iclk<to_clk;iclk +=8)
          {
            i = Clock2i(iclk);

            // SN++ Ein neues Event wird nur erzeut wenn sich der Wert aendert
            //      und der Wert groesser als 0 ist.
            if (array[i] > 0 && array[i] != temp)
            {
              pKeyPressure = new JZKeyPressureEvent(
                iclk,
                key_cha,
                key_val,
                array[i]);
              track->Put(pKeyPressure);
              temp = array[i];
            }
          }
          key_val = -1;
        }
      }
      pEvent = iter.Next();
    }
  }
  else
  {
    // OnEdit
    // edit mode: Erzeugt keine neuen Events sondern aendert den Wert
    // bestehender Events.
    // SN++
    pEvent = iter.Range(from_clk, to_clk);
    JZKeyPressureEvent* pKeyPressureCopy;
    while (pEvent)
    {
      if (
        !mpPianoWindow->AreEventsSelected() ||
        mpPianoWindow->GetFilter()->IsSelected(pEvent))
      {
        if (pEvent->IsKeyPressure())
        {
          if (
            Clock2Val(pEvent->GetClock()) !=
              pEvent->IsKeyPressure()->GetPressureValue())
          {
            pKeyPressureCopy = pEvent->Copy()->IsKeyPressure();
            pKeyPressureCopy->SetPressureValue(Clock2Val(pEvent->GetClock()));
            track->Kill(pEvent);
            track->Put(pKeyPressureCopy);
          }
        }
      }
      pEvent = iter.Next();
    }
  }

  track->Cleanup();
  wxEndBusyCursor();
  OnRevert();
  // SN+ for Color Darstellung
  mpPianoWindow->Refresh();
}

// ----------------------------------------------------------------------

JZChannelAftertouchEdit::JZChannelAftertouchEdit(
  JZPianoWindow* pPianoWindow,
  char const *label,
  int xoff,
  int x,
  int y,
  int w,
  int h)
  : JZCtrlEditBase(0, 127, pPianoWindow, label, xoff, x, y, w, h, 1)
{
}


int JZChannelAftertouchEdit::Missing()
{
  return 0;
}

int JZChannelAftertouchEdit::IsCtrlEdit(JZEvent* pEvent)
{
  return pEvent->IsChnPressure() != 0;
}

int JZChannelAftertouchEdit::GetValue(JZEvent* pEvent)
{
  return pEvent->IsChnPressure()->Value;
}


JZEvent *JZChannelAftertouchEdit::NewEvent(long clock, int val)
{
  return new JZChnPressureEvent(clock, track->mChannel - 1, val);
}

void JZChannelAftertouchEdit::UpDate()
{
  OnRevert();
}


void JZChannelAftertouchEdit::OnApply()
{
  wxBeginBusyCursor();
  mpPianoWindow->GetProject()->NewUndoBuffer();

  // delete old events, but skip clock 0 to preserve track defaults:
  // (dirty but might work...)
  JZEventIterator iter(track);
  JZEvent* pEvent = iter.Range(std::max(1L, from_clock), to_clock);
  int old_val = Missing();

  // SN++ events nur im apply-mode loeschen!
  if (!ctrlmode)
  {
    while (pEvent)
    {
      if (IsCtrlEdit(pEvent))
      {
        track->Kill(pEvent);
      }
      pEvent = iter.Next();
    }

    // find any previous events
    if (sticky)
    {
      pEvent = iter.Range(0, from_clock - 1);
      while (pEvent)
      {
        if (IsCtrlEdit(pEvent))
        {
          old_val = GetValue(pEvent);
        }
        pEvent = iter.Next();
      }
    }

    // SN++ set-Mode
    // create new events
    long clock;
    for (clock = from_clock; clock < to_clock; clock += 8)
    {
      int new_val = Clock2Val(clock);

      if (old_val != new_val)
      {
        pEvent = NewEvent(clock, new_val);
        track->Put(pEvent);
        old_val = new_val;
      }
    }
  }
  else
  {
    // edit mode: Erzeugt keine neuen Events sondern aendert den Wert
    // bestehender Events.
    // SN++
    JZChnPressureEvent* pChnPressureCopy;
    while (pEvent)
    {
      if (IsCtrlEdit(pEvent))
      {
        if (Clock2Val(pEvent->GetClock()) != GetValue(pEvent))
        {
          pChnPressureCopy = pEvent->Copy()->IsChnPressure();
          pChnPressureCopy->Value = Clock2Val(pEvent->GetClock());
          track->Kill(pEvent);
          track->Put(pChnPressureCopy);
        }
      }
      pEvent = iter.Next();
    }
  }

  // done
  track->Cleanup();
  wxEndBusyCursor();
  OnRevert();
}



// ------------------------------------------------------------------

JZTempoEdit::JZTempoEdit(
  int min,
  int max,
  JZPianoWindow* pPianoWindow,
  char const *label,
  int xoff,
  int x,
  int y,
  int w,
  int h)
  : JZCtrlEditBase(min, max, pPianoWindow, label, xoff, x, y, w, h)
{
}

int JZTempoEdit::Missing()
{
  return track->GetDefaultSpeed();
}

int JZTempoEdit::IsCtrlEdit(JZEvent* pEvent)
{
  return pEvent->IsSetTempo() != 0;
}

int JZTempoEdit::GetValue(JZEvent* pEvent)
{
  return pEvent->IsSetTempo()->GetBPM();
}

JZEvent * JZTempoEdit::NewEvent(long clock, int val)
{
  return new JZSetTempoEvent(clock, val);
}
