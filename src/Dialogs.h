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

#if 0
#include "CommandUtilities.h"
#include "PropertyListDialog.h"
#endif

//class JZFilter;
class JZEvent;
//class JZEventFrame;
//class JZEventWindow;
class JZPianoWindow;
//class JZSong;
class JZTrack;

//class JZShiftDlg : public JZPropertyListDlg
//{
//  public:
//
//    long mSteps;        // 0 was static
//    long mUnit;
//
//    JZFilter* mpFilter;
//    JZSong* mpSong;
//
//    JZShiftDlg(JZEventFrame* pEventWindow, JZFilter* pFilter, long Unit);
//    void AddProperties();
//    bool OnClose();
//    void OnHelp();
//};

//class JZSearchReplaceDlg : public JZPropertyListDlg
//{
//  public:
//
//    static int frCtrl;
//    static int toCtrl;
///*     JZNamedChoice frList; */
///*     JZNamedChoice toList; */
//
//    JZFilter *Filter;
//    JZSong   *Song;
//
//    JZSearchReplaceDlg(JZEventWindow* w, JZFilter *f);
//    void AddProperties();
//    bool OnClose();
//    void OnHelp();
//};

#if 0
// seqLength
class JZSeqLengthDlg : public JZPropertyListDlg
{
  public:


    static double scale;

    JZFilter   *Filter;
    JZSong     *Song;

    JZSeqLengthDlg(JZEventFrame *win, JZFilter *f);
    void AddProperties();
    bool OnClose();
    void OnHelp();
};

// midiDelay
class JZMidiDelayDlg : public JZPropertyListDlg
{
  public:

    static double scale;
    static long clockDelay;
    static int repeat;

    JZFilter   *Filter;
    JZSong     *Song;

    JZMidiDelayDlg(JZEventFrame *win, JZFilter *f);
    void AddProperties();
    bool OnClose();
    void OnHelp();
};
#endif

void EventDialog(
  JZEvent*&,
  JZPianoWindow*,
  JZTrack*,
  long Clock,
  int Channel,
  int Pitch);
