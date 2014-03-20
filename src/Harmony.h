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

#include "ToolBar.h"

#include <wx/frame.h>

class wxObject;
class JZHarmonyBrowserAnalyzer;
class JZHarmonyBrowserCanvas;
class JZGenMelody;

//*****************************************************************************
//*****************************************************************************
class JZHarmonyBrowserInterface
{
  public:

    virtual ~JZHarmonyBrowserInterface()
    {
    }

    // true = yes
    virtual bool IsSequenceDefined() = 0;

    // Return number of keys in out

    virtual int GetChordKeys(int* out, int step, int n_steps) = 0;

    virtual int GetBassKeys(int* out, int step, int n_steps) = 0;

    virtual int GetSelectedChord(int* out) = 0; // returns # keys

    virtual int GetSelectedScale(int* out) = 0; // returns # keys

    virtual JZHarmonyBrowserAnalyzer* GetAnalyzer() = 0;

    virtual void TransposeSelection() = 0;
};

//*****************************************************************************
//*****************************************************************************
class JZHarmonyBrowserFrame : public wxFrame, public JZHarmonyBrowserInterface
{
  public:

    JZHarmonyBrowserFrame();

    ~JZHarmonyBrowserFrame();

    bool IsSequenceDefined();

    int GetChordKeys(int* out, int step, int n_steps);

    int GetSelectedChord(int* out);

    int GetSelectedScale(int* out);

    int GetBassKeys(int* out, int step, int n_steps);

    JZHarmonyBrowserAnalyzer* GetAnalyzer();

    void TransposeSelection();

  protected:

    void OnClose(wxCloseEvent& Event);

    void OnCloseWindow(wxCommandEvent& Event);

    void OnUpdateMajorScale(wxUpdateUIEvent& Event);
    void OnUpdateHarmonicMinorScale(wxUpdateUIEvent& Event);
    void OnUpdateMelodicMinorScale(wxUpdateUIEvent& Event);
    void OnUpdateIonicScale(wxUpdateUIEvent& Event);

    void OnUpdateFourEqualNotes(wxUpdateUIEvent& Event);
    void OnUpdateThreeEqualNotes(wxUpdateUIEvent& Event);
    void OnUpdateTwoEqualNotes(wxUpdateUIEvent& Event);
    void OnUpdateOneEqualNotes(wxUpdateUIEvent& Event);
    void OnUpdateZeroEqualNotes(wxUpdateUIEvent& Event);

    void OnToolBarSelect(wxCommandEvent& Event);

    void OnFileLoad(wxCommandEvent& Event);

    void OnFileSaveAs(wxCommandEvent& Event);

    void OnSettingsChord(wxCommandEvent& Event);

    void OnSettingsMidi(wxCommandEvent& Event);

    void OnUpdateHaunschildLayout(wxUpdateUIEvent& Event);
    void OnSettingsHaunschild(wxCommandEvent& Event);

    void OnActionClearSequence(wxCommandEvent& Event);

    void OnMouseHelp(wxCommandEvent& Event);

    void OnHelp(wxCommandEvent& Event);

  private:

    int SeqSelected();

  private:

    JZHarmonyBrowserCanvas* mpHbWindow;

    JZToolBar* mpToolBar;

    JZGenMelody* genmeldy;

  DECLARE_EVENT_TABLE()
};

extern void CreateHarmonyBrowser();
