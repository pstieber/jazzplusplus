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
#include "Metronome.h"

#include "EventFrame.h"

class JZProject;
class JZToolBar;
class JZTrackWindow;
class JZPianoWindow;

//*****************************************************************************
//*****************************************************************************
class JZTrackFrame : public JZEventFrame, public JZButtonLabelInterface
{
  public:

    JZTrackFrame(
      wxWindow* pParent,
      const wxString& Title,
      JZProject* pProject,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxDefaultSize);

    virtual ~JZTrackFrame();

    JZPianoWindow* GetPianoWindow()
    {
      return 0;
    }

    void NewPlayPosition(int Clock);

    // Overridden JZButtonLabelInterface function.
    virtual void ButtonLabelDisplay(const wxString& Text, bool IsButtonDown);

  private:

    void OnMetroOn(wxCommandEvent& Event);

  private:

    bool OnClose();

    void CreateToolBar();

    void CreateMenu();

    void OnClose(wxCloseEvent& CloseEvent);

    void OnFileNew(wxCommandEvent& Event);

    void OnFileOpenProject(wxCommandEvent& Event);

    void OnFileProjectSave(wxCommandEvent& Event);

    void OnFileProjectSaveAs(wxCommandEvent& Event);

    void OnFileImportMidi(wxCommandEvent& Event);

    void OnFileImportAscii(wxCommandEvent& Event);

    void OnFileExportMidi(wxCommandEvent& Event);

    void OnFileExportAscii(wxCommandEvent& Event);

    void OnFileExportSelectionAsMidi(wxCommandEvent& Event);

    void OnFileExit(wxCommandEvent& Event);

    void OnZoomIn(wxCommandEvent& Event);

    void OnZoomOut(wxCommandEvent& Event);

    void OnPlay(wxCommandEvent& Event);

    void OnPlayLoop(wxCommandEvent& Event);

    void OnRecord(wxCommandEvent& Event);

    void OnPianoWindow(wxCommandEvent& Event);

    void OnToolsHarmonyBrowser(wxCommandEvent& Event);

    void OnToolsRhythmGenerator(wxCommandEvent& Event);

    void OnSettingsMetronome(wxCommandEvent& Event);

    void OnSettingsSynthesizerType(wxCommandEvent& Event);

    void OnSettingsMidiDevice(wxCommandEvent& Event);

    void OnAudioGlobalSettings(wxCommandEvent& Event);

    void OnAudioSampleSettings(wxCommandEvent& Event);

    void OnAudioLoadSampleSet(wxCommandEvent& Event);

    void OnAudioSaveSampleSet(wxCommandEvent& Event);

    void OnAudioSaveSampleSetAs(wxCommandEvent& Event);

    void OnAudioNewSampleSet(wxCommandEvent& Event);

    void OnHelpContents(wxCommandEvent& Event);

    void OnHelpAbout(wxCommandEvent& Event);

    void MousePlay(wxMouseEvent& MouseEvent, TEMousePlayMode Mode);

  private:

    JZToolBar* mpToolBar;

    wxMenu* mpFileMenu;

    wxMenu* mpEditMenu;

    wxMenu* mpToolsMenu;

    JZProject* mpProject;

    JZTrackWindow* mpTrackWindow;

    int mPreviousClock;
    bool mPreviouslyRecording;

  DECLARE_EVENT_TABLE()
};

//*****************************************************************************
// Description:
//   These are the track frame class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZTrackFrame::ButtonLabelDisplay(const wxString& Text, bool IsButtonDown)
{
}
