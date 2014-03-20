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

#include "songSettings.h"

JZSongSettingsDlg::JZSongSettingsDlg(tEventWin *w)
: JZPropertyListDlg("Song Settings" )
{
  EventWin = w;
  TicksPerQuarter = EventWin->Song->TicksPerQuarter;
  SongLength      = EventWin->Song->MaxQuarters / 4;
  IntroLength     = EventWin->Song->GetIntroLength();
}

bool JZSongSettingsDlg::OnClose()
{
  EventWin->Song->SetTicksPerQuarter(TicksPerQuarter);
  EventWin->Song->MaxQuarters = SongLength * 4;
  EventWin->Canvas->SetScrollRanges();
  if (EventWin->NextWin)
    EventWin->NextWin->Canvas->SetScrollRanges();
  EventWin->Redraw();
  EventWin->DialogBox = 0;
  EventWin->Song->SetIntroLength(IntroLength);
  return FALSE;
}

void JZSongSettingsDlg::AddProperties()
{
  sheet->AddProperty(new wxProperty("Ticks per Quarter Note: 48, 72, 96, 120, 144, 168, 192", wxPropertyValue(&TicksPerQuarter), "integer", new wxIntegerListValidator(1, 200)));
  sheet->AddProperty(new wxProperty("Song length in Bars", wxPropertyValue(&SongLength), "integer", new wxIntegerListValidator(50, 500)));
  sheet->AddProperty(new wxProperty("Intro length in Bars", wxPropertyValue(&IntroLength), "integer", new wxIntegerListValidator(0, 8)));
}
