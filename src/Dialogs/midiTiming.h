/*
**  Alacrity Midi Sequencer
**
** Some Code Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
**    I don't know why it says "All Rights Reserved" and then is licensed GPL
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/

#ifndef MIDITIMING
#define MIDITIMING
#include "../eventwin.h"
#include "../trackwin.h"

#ifndef __PORTING


// ******************************************************************
// Midi Timing Dialog
// ******************************************************************

class JZMidiButton;

class JZTimingDlg : public wxForm
{
 public:
  tEventWin *EventWin;
  JZTimingDlg(tEventWin *w);
  void EditForm(wxPanel *panel);
  virtual void OnOk();
  virtual void CloseWindow();
  virtual void OnHelp();
  char *ClkSrcArray[4];
  char *MtcTypeArray[4];
  wxListBox *ClkSrcListBox;
  wxListBox *MtcTypeListBox;
  wxCheckBox *RealTimeCheckBox;
  wxText *MtcOffsetEntry;
  void MtcInitRec();
  void MtcFreezeRec();
  static void MtcRecFunc( JZMidiButton& button, wxCommandEvent& event );
  static void OkFunc( JZMidiButton& button, wxCommandEvent& event );
  static void CancelFunc( JZMidiButton& button, wxCommandEvent& event );
  static void HelpFunc( JZMidiButton& button, wxCommandEvent& event );
};

class JZMidiButton : public wxButton
{
  public:
    JZMidiButton(
      JZTimingDlg *dlg,
      wxPanel *panel,
      wxFunction func,
      char *label,
      int x = -1,
      int y = -1,
      int width = -1,
      int height = -1,
      long style = 0,
      char *name = "button")
    : wxButton( panel, func, label, x, y, width, height, style, name )
    {
      midiDlg = dlg;
    }

    void OnOk()
    {
      midiDlg->OnOk();
    }

    void OnCancel()
    {
      midiDlg->CloseWindow();
    }

    void OnHelp()
    {
      midiDlg->OnHelp();
    }

    void OnInitRec()
    {
      midiDlg->MtcInitRec();
    }

    void OnFreezeRec()
    {
      midiDlg->MtcFreezeRec();
    }
  private:
    JZTimingDlg *midiDlg;
};



#endif // Porting

#endif // Miditiming

