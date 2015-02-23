//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2015 Peter J. Stieber
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

#include "TrackFrame.h"

#include "AboutDialog.h"
#include "Configuration.h"
#include "Dialogs/MetronomeSettingsDialog.h"
#include "Dialogs/SynthesizerSettingsDialog.h"
#include "Globals.h"
#include "Harmony.h"
#include "Help.h"
#include "JazzPlusPlusApplication.h"
#include "Player.h"
#include "Project.h"
#include "ProjectManager.h"
#include "RecordingInfo.h"
#include "Resources.h"
#include "Rhythm.h"
#include "TrackWindow.h"
#include "ToolBar.h"

#ifdef __WXMSW__
#include "mswin/WindowsPlayer.h"
#endif

// These are the tool bar icons.
#include "Bitmaps/open.xpm"
#include "Bitmaps/save.xpm"
#include "Bitmaps/new.xpm"
#include "Bitmaps/repl.xpm"
#include "Bitmaps/delete.xpm"
#include "Bitmaps/quantize.xpm"
#include "Bitmaps/mixer.xpm"
#include "Bitmaps/play.xpm"
#include "Bitmaps/undo.xpm"
#include "Bitmaps/redo.xpm"
#include "Bitmaps/zoomin.xpm"
#include "Bitmaps/zoomout.xpm"
#include "Bitmaps/panic.xpm"
#include "Bitmaps/help.xpm"
#include "Bitmaps/pianowin.xpm"
#include "Bitmaps/metro.xpm"
#include "Bitmaps/playloop.xpm"
#include "Bitmaps/record.xpm"

#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

//DEBUG#include <iostream>

using namespace std;

//*****************************************************************************
// Description:
//   This is the track frame class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZTrackFrame, JZEventFrame)

  EVT_CLOSE(JZTrackFrame::OnClose)

  EVT_MENU(wxID_NEW, JZTrackFrame::OnFileNew)

  EVT_MENU(wxID_OPEN, JZTrackFrame::OnFileOpenProject)

  EVT_MENU(wxID_SAVE, JZTrackFrame::OnFileProjectSave)

  EVT_MENU(wxID_SAVEAS, JZTrackFrame::OnFileProjectSaveAs)

  EVT_MENU(ID_IMPORT_MIDI, JZTrackFrame::OnFileImportMidi)

  EVT_MENU(ID_IMPORT_ASCII_MIDI, JZTrackFrame::OnFileImportAscii)

  EVT_MENU(ID_EXPORT_MIDI, JZTrackFrame::OnFileExportMidi)

  EVT_MENU(ID_EXPORT_ASCII_MIDI, JZTrackFrame::OnFileExportAscii)

  EVT_MENU(
    ID_EXPORT_SELECTION_AS_MIDI,
    JZTrackFrame::OnFileExportSelectionAsMidi)

  EVT_MENU(wxID_EXIT, JZTrackFrame::OnFileExit)

  EVT_MENU(ID_PLAY, JZTrackFrame::OnPlay)

  EVT_MENU(ID_PLAY_LOOP, JZTrackFrame::OnPlayLoop)

  EVT_MENU(ID_RECORD, JZTrackFrame::OnRecord)

  EVT_MENU(ID_PIANOWIN, JZTrackFrame::OnPianoWindow)

  EVT_MENU(ID_METRONOME_TOGGLE, JZTrackFrame::OnMetroOn)

  EVT_MENU(wxID_ZOOM_IN, JZTrackFrame::OnZoomIn)

  EVT_MENU(wxID_ZOOM_OUT, JZTrackFrame::OnZoomOut)

  EVT_MENU(ID_TOOLS_HARMONY_BROWSER, JZTrackFrame::OnToolsHarmonyBrowser)

  EVT_MENU(ID_TOOLS_RHYTHM_GENERATOR, JZTrackFrame::OnToolsRhythmGenerator)

  EVT_MENU(ID_SETTINGS_METRONOME, JZTrackFrame::OnSettingsMetronome)

  EVT_MENU(ID_SETTINGS_SYNTHESIZER, JZTrackFrame::OnSettingsSynthesizerType)

  EVT_MENU(ID_SETTINGS_MIDI_DEVICE, JZTrackFrame::OnSettingsMidiDevice)

  EVT_MENU(ID_AUDIO_GLOBAL_SETTINGS, JZTrackFrame::OnAudioGlobalSettings)

  EVT_MENU(ID_AUDIO_SAMPLE_SETTINGS, JZTrackFrame::OnAudioSampleSettings)

  EVT_MENU(ID_AUDIO_LOAD_SAMPLE_SET, JZTrackFrame::OnAudioLoadSampleSet)

  EVT_MENU(ID_AUDIO_SAVE_SAMPLE_SET, JZTrackFrame::OnAudioSaveSampleSet)

  EVT_MENU(ID_AUDIO_SAVE_SAMPLE_SET_AS, JZTrackFrame::OnAudioSaveSampleSetAs)

  EVT_MENU(ID_AUDIO_NEW_SAMPLE_SET, JZTrackFrame::OnAudioNewSampleSet)

  EVT_MENU(wxID_HELP_CONTENTS, JZTrackFrame::OnHelpContents)

  EVT_MENU(wxID_ABOUT, JZTrackFrame::OnHelpAbout)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrackFrame::JZTrackFrame(
  wxWindow* pParent,
  const wxString& Title,
  JZProject* pProject,
  const wxPoint& Position,
  const wxSize& Size)
  : JZEventFrame(pParent, Title, pProject, Position, Size),
    mpToolBar(0),
    mpFileMenu(0),
    mpEditMenu(0),
    mpToolsMenu(0),
    mpProject(pProject),
    mpTrackWindow(0),
    mPreviousClock(0),
    mPreviouslyRecording(false)
{
  CreateToolBar();

  CreateMenu();

  mpTrackWindow = new JZTrackWindow(
    this,
    pProject,
    wxPoint(0, 0),
    wxSize(600, 120));

  gpTrackWindow = mpTrackWindow;

  mpTrackWindow->Create();

  SetEventWindow(mpTrackWindow);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrackFrame::~JZTrackFrame()
{
  int XPosition, YPosition, Width, Height;

  GetPosition(&XPosition, &YPosition);
  GetSize(&Width, &Height);

  gpConfig->Put(C_TrackWinXpos, XPosition);
  gpConfig->Put(C_TrackWinYpos, YPosition);
  gpConfig->Put(C_TrackWinWidth, Width);
  gpConfig->Put(C_TrackWinHeight, Height);

  delete mpToolBar;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::CreateToolBar()
{
  JZToolDef ToolBarDefinitions[] =
  {
    { wxID_OPEN, false, open_xpm, "load song" },
    { wxID_SAVE, false, save_xpm, "save song" },
    { wxID_NEW,  false, new_xpm,  "new song" },
    { JZToolBar::eToolBarSeparator },
    { wxID_DUPLICATE, false, repl_xpm,     "duplicate selection" },
    { wxID_DELETE,    false, delete_xpm,   "delete selection" },
    { ID_QUANTIZE,    false, quantize_xpm, "quantize selection" },
    { ID_MIXER,       false, mixer_xpm,    "mixer" },
    { ID_PIANOWIN,    false, pianowin_xpm, "show piano window" },
    { JZToolBar::eToolBarSeparator },
    { ID_PLAY,             false, play_xpm,     "start play"},
    { ID_PLAY_LOOP,        false, playloop_xpm, "loop play"},
    { ID_RECORD,           false, record_xpm,   "record"},
    { ID_METRONOME_TOGGLE, true, metro_xpm,     "metronome" },
    { JZToolBar::eToolBarSeparator },
    { wxID_ZOOM_IN,       false, zoomin_xpm,  "zoom in" },
    { wxID_ZOOM_OUT,      false, zoomout_xpm, "zoom out"},
    { wxID_UNDO,          false, undo_xpm,    "undo"},
    { wxID_REDO,          false, redo_xpm,    "redo"},
    { wxID_RESET,         false, panic_xpm,   "all notes off"},
    { wxID_HELP_CONTENTS, false, help_xpm,    "help" },
    { JZToolBar::eToolBarEnd }
  };

  mpToolBar = new JZToolBar(this, ToolBarDefinitions);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::CreateMenu()
{
  // Create the file menu.
  mpFileMenu = new wxMenu;

  mpFileMenu->Append(wxID_NEW, "&New");
  mpFileMenu->Append(wxID_OPEN, "&Open Project...");
  mpFileMenu->Append(wxID_CLOSE, "&Close");
  mpFileMenu->Append(wxID_SAVE, "&Save Project");
  mpFileMenu->Append(wxID_SAVEAS, "Save Project &As...");

  mpFileMenu->AppendSeparator();

  mpFileMenu->Append(ID_IMPORT_MIDI, "Import MIDI...");
//DEBUG  mpFileMenu->Append(ID_IMPORT_ASCII_MIDI, "Import ASCII...");
  mpFileMenu->Append(ID_EXPORT_MIDI, "Export as MIDI...");
//DEBUG  mpFileMenu->Append(ID_EXPORT_ASCII_MIDI, "Export as ASCII...");
  mpFileMenu->Append(
    ID_EXPORT_SELECTION_AS_MIDI,
    "Export Selection as MIDI...");

  mpFileMenu->AppendSeparator();

  mpFileMenu->Append(wxID_PREFERENCES, "&Preferences...");

  mpFileMenu->AppendSeparator();

  mpFileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4");

  mpEditMenu = new wxMenu;

  mpEditMenu->Append(wxID_UNDO, "&Undo...");
  mpEditMenu->Append(wxID_REDO, "&Redo...");

  mpEditMenu->AppendSeparator();

  mpEditMenu->Append(wxID_CUT, "&Cut");
  mpEditMenu->Append(wxID_COPY, "C&opy");
  mpEditMenu->Append(wxID_PASTE, "&Paste");
  mpEditMenu->Append(ID_TRIM, "&Trim");

  mpEditMenu->AppendSeparator();

  mpEditMenu->Append(ID_QUANTIZE, "&Quantize...");
  mpEditMenu->Append(ID_SET_CHANNEL, "&Set MIDI Channel...");
  mpEditMenu->Append(ID_TRANSPOSE, "&Transpose...");
  mpEditMenu->Append(ID_VELOCITY, "&Velocity...");
  mpEditMenu->Append(ID_LENGTH, "&Length...");
  mpEditMenu->Append(ID_SHIFT, "Shi&ft...");
  mpEditMenu->Append(ID_CLEANUP, "C&leanup...");
  mpEditMenu->Append(ID_SEARCH_AND_REPLACE, "Search Re&place...");

  mpEditMenu->AppendSeparator();

  mpEditMenu->Append(wxID_DELETE, "&Delete");
  mpEditMenu->Append(wxID_DELETE, "&Silence");

#if 0
  mpEditMenu->AppendSeparator();

  mpEditMenu->Append(MEN_SPLIT, "Split");
  mpEditMenu->Append(MEN_REPLICATE, "&Duplicate");

  mpEditMenu->AppendSeparator();

  mpEditMenu->Append(MEN_SELECTIONSUB, "Select (needs submenu)");

  mpEditMenu->AppendSeparator();
#endif

  wxMenu* pMiscMenu = new wxMenu;
  pMiscMenu->Append(ID_MISC_TRACK_MERGE, "Mer&ge Tracks...");
  pMiscMenu->Append(ID_MISC_SPLIT_TRACKS, "&Split Tracks...");
  pMiscMenu->Append(ID_MISC_METER_CHANGE, "&Meter Change...");
  pMiscMenu->Append(ID_MISC_RESET_MIDI, "&Reset Midi");
#if 0
  pMiscMenu->Append(MEN_RHYTHM,      "Random R&hythm...");
  pMiscMenu->Append(MEN_SHUFFLE,     "Random Sh&uffle...");
  pMiscMenu->Append(MEN_GENMELDY,    "Random Melod&y...");
  pMiscMenu->Append(MEN_ARPEGGIO,    "Random Arpeggio...");
  pMiscMenu->Append(MEN_MAPPER,      "Ma&pper...");
  pMiscMenu->Append(MEN_EVENTLIST,   "Event &List...");
#endif
  pMiscMenu->Append(ID_MISC_SET_COPYRIGHT, "&Set Music Copyright...");

  mpToolsMenu = new wxMenu;
  mpToolsMenu->Append(ID_TOOLS_HARMONY_BROWSER, "&Harmony Browser...");
  mpToolsMenu->Append(ID_TOOLS_RHYTHM_GENERATOR, "&Rhythm Generator...");

#if 0
  // Move to Project Menu
  mpFileMenu->Append(MEN_LOAD_TMPL,     "Load &Template...");
  mpFileMenu->Append(MEN_LOADPATTERN,   "Load Pattern...");
  mpFileMenu->Append(MEN_SAVEPATTERN,   "Save Pattern...");
  mpFileMenu->AppendSeparator();

  parts_menu = new wxMenu;
  parts_menu->Append(ID_MIXER,    "&Mixer...");
  parts_menu->Append(MEN_MASTER,   "Mas&ter...");
  parts_menu->Append(MEN_SOUND,    "&Sound...");
  parts_menu->Append(MEN_VIBRATO,  "&Vibrato...");
  parts_menu->Append(MEN_ENVELOPE, "&Envelope...");

  bender_menu = new wxMenu;
  bender_menu->Append(MEN_BEND_BASIC,    "&Bender Basic...");
  bender_menu->Append(MEN_BEND_LFO1,    "&Bender LFO1...");
  bender_menu->Append(MEN_BEND_LFO2,    "&Bender LFO2...");
  parts_menu->Append(MEN_SUB_BENDER, "&Bender...", bender_menu );

  modulation_menu = new wxMenu;
  modulation_menu->Append(MEN_MOD_BASIC,    "&Modulation Basic...");
  modulation_menu->Append(MEN_MOD_LFO1,    "&Modulation LFO1...");
  modulation_menu->Append(MEN_MOD_LFO2,    "&Modulation LFO2...");
  parts_menu->Append(MEN_SUB_MODUL, "&Modulation...", modulation_menu );

  caf_menu = new wxMenu;
  caf_menu->Append(MEN_CAF_BASIC,    "&CAf Basic...");
  caf_menu->Append(MEN_CAF_LFO1,    "&CAf LFO1...");
  caf_menu->Append(MEN_CAF_LFO2,    "&CAf LFO2...");
  parts_menu->Append(MEN_SUB_CAF, "&CAf...", caf_menu );

  paf_menu = new wxMenu;
  paf_menu->Append(MEN_PAF_BASIC,    "&PAf Basic...");
  paf_menu->Append(MEN_PAF_LFO1,    "&PAf LFO1...");
  paf_menu->Append(MEN_PAF_LFO2,    "&PAf LFO2...");
  parts_menu->Append(MEN_SUB_PAF, "&PAf...", paf_menu );

  cc1_menu = new wxMenu;
  cc1_menu->Append(MEN_CC1_BASIC,    "&CC1 Basic...");
  cc1_menu->Append(MEN_CC1_LFO1,    "&CC1 LFO1...");
  cc1_menu->Append(MEN_CC1_LFO2,    "&CC1 LFO2...");
  parts_menu->Append(MEN_SUB_CC1, "&CC1...", cc1_menu );

  cc2_menu = new wxMenu;
  cc2_menu->Append(MEN_CC2_BASIC,    "&CC2 Basic...");
  cc2_menu->Append(MEN_CC2_LFO1,    "&CC2 LFO1...");
  cc2_menu->Append(MEN_CC2_LFO2,    "&CC2 LFO2...");
  parts_menu->Append(MEN_SUB_CC2, "&CC2...", cc2_menu );

  parts_menu->Append(MEN_DRUM_PARAM,    "&Drum Parameters...");
  parts_menu->Append(MEN_PART_RSRV,    "&Partial Reserve...");
  parts_menu->Append(MEN_PART_MODE,    "&Part Mode...");
#endif

  wxMenu* pSettingMenu = new wxMenu;
#if 0
  pSettingMenu->Append(MEN_FILTER,    "&Filter...");
  pSettingMenu->Append(MEN_TWSETTING, "&Window...");
  pSettingMenu->Append(MEN_SONG,      "&Song...");
#endif
  pSettingMenu->Append(ID_SETTINGS_METRONOME, "&Metronome...");
#if 0
  pSettingMenu->Append(MEN_EFFECTS,   "&Effects...");
  pSettingMenu->Append(MEN_TIMING,    "&Timing...");
  pSettingMenu->Append(MEN_MIDI_THRU, "&Midi Thru...");
#endif
  pSettingMenu->Append(ID_SETTINGS_SYNTHESIZER, "&Synthesizer Type...");

#ifdef __WXMSW__
  pSettingMenu->Append(ID_SETTINGS_MIDI_DEVICE, "&Midi Device...");
#else
  if (
    gpConfig->GetValue(C_MidiDriver) == eMidiDriverOss ||
    gpConfig->GetValue(C_MidiDriver) == eMidiDriverAlsa)
  {
    pSettingMenu->Append(ID_SETTINGS_MIDI_DEVICE, "&Midi Device...");
  }
#endif

#if 0

  save_settings_menu = new wxMenu;
  save_settings_menu->Append( MEN_SAVE_THRU, "&Midi Thru" );
  save_settings_menu->Append( MEN_SAVE_TIM, "&Timing" );
  save_settings_menu->Append( MEN_SAVE_EFF, "&Effect Macros" );
  save_settings_menu->Append( MEN_SAVE_GEO, "&Window Geometry" );
  save_settings_menu->Append( MEN_SAVE_METRO, "&Metronome" );
  // save_settings_menu->Append( MEN_SAVE_SYNTH, "&Synth Type" );
  save_settings_menu->Append( MEN_SAVE_ALL, "&Save All" );
  pSettingMenu->Append(MEN_SAVE_SET, "&Save settings", save_settings_menu );
#endif

  wxMenu* pAudioMenu = new wxMenu;
  pAudioMenu->Append(ID_AUDIO_GLOBAL_SETTINGS, "&Global Audio Settings...");
  pAudioMenu->Append(ID_AUDIO_SAMPLE_SETTINGS, "Sample Set Se&ttings... ");
  pAudioMenu->Append(ID_AUDIO_LOAD_SAMPLE_SET, "&Load Sample Set...");
  pAudioMenu->Append(ID_AUDIO_SAVE_SAMPLE_SET, "&Save Sample Set");
  pAudioMenu->Append(ID_AUDIO_SAVE_SAMPLE_SET_AS, "Save Sample Set &As");
  pAudioMenu->Append(ID_AUDIO_NEW_SAMPLE_SET, "&New Sample Set");

  wxMenu* mpHelpMenu = new wxMenu;
  mpHelpMenu->Append(wxID_HELP_CONTENTS, "&Contents");
//  mpHelpMenu->Append(MEN_HELP_JAZZ, "&Jazz");
//  mpHelpMenu->Append(MEN_HELP_TWIN, "&Trackwin");
//  mpHelpMenu->Append(MEN_HELP_MOUSE, "&Mouse");
  mpHelpMenu->Append(wxID_ABOUT, "&About");

  // Create a menu bar and add entries.
  wxMenuBar* pMenuBar = new wxMenuBar();
  pMenuBar->Append(mpFileMenu, "&File");
  pMenuBar->Append(mpEditMenu, "&Edit");
#if 0
  pMenuBar->Append(parts_menu, "&Parts");
#endif
  pMenuBar->Append(mpToolsMenu, "&Tools");
  pMenuBar->Append(pSettingMenu, "&Settings");
  pMenuBar->Append(pMiscMenu, "&Misc");
  pMenuBar->Append(pAudioMenu, "&Audio");
  pMenuBar->Append(mpHelpMenu , "&Help");

  SetMenuBar(pMenuBar);

//  EnableDisableMenus();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::NewPlayPosition(int Clock)
{
  mpTrackWindow->NewPlayPosition(Clock);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZTrackFrame::OnClose()
{
//  if (JZTrack::changed)
//  {
//    if (
//      ::wxMessageBox(
//        "Song has changed. Quit anyway?",
//        "Quit ?",
//        wxYES_NO) == wxNO)
//    {
//      return false;
//    }
//  }
//  if (gpProject->IsPlaying())
//  {
//    gpProject->Stop();
//#ifndef WX_MSW
//    sleep(1);
//#endif
//  }

  delete gpHarmonyBrowser;
  delete gpRhythmGeneratorFrame;

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnClose(wxCloseEvent& CloseEvent)
{
  if (CloseEvent.CanVeto() && mpProject && mpProject->IsPlaying())
  {
    if (
      wxMessageBox(
        "Currently playing the MIDI file... continue closing?",
        "Please confirm",
        wxICON_QUESTION | wxYES_NO) != wxYES )
    {
      CloseEvent.Veto();
      return;
    }
  }

  if (mpProject && mpProject->IsPlaying())
  {
    // Since we cannont veto the close event, stop a playing project.
    mpProject->Stop();
  }

  // At this point, we can either call Destroy(), or CloseEvent.Skip()
  // since the default event handler also calls Destroy().
  CloseEvent.Skip();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileNew(wxCommandEvent& Event)
{
  if (wxMessageBox("Clear Song?", "Sure?", wxOK | wxCANCEL) == wxOK)
  {
    gpProject->Clear();
    mpTrackWindow->Refresh(false);
//    NextWin->NewPosition(1, 0);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileOpenProject(wxCommandEvent& Event)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileProjectSave(wxCommandEvent& Event)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileProjectSaveAs(wxCommandEvent& Event)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileImportMidi(wxCommandEvent& Event)
{
  wxFileDialog OpenDialog(
    0,
    "Load MIDI File",
    "",
    "",
    "MIDI files (MID, MIDI)|*.mid;*.midi|All files (*.*)|*.*",
    wxFD_OPEN | wxFD_CHANGE_DIR);
  if (OpenDialog.ShowModal() == wxID_OK)
  {
    wxString MidiFileName = OpenDialog.GetPath();
    gpProject->OpenSong(MidiFileName);
    SetTitle(MidiFileName);
//    NextWin->NewPosition(1, 0);
    mpTrackWindow->SetScrollRanges();
//    mpTrackWindow->SetScrollPosition(0, 0);
//    NextWin->Canvas->SetScrollRanges();
    mpTrackWindow->Refresh(false);
//    JZTrack::changed = false;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileImportAscii(wxCommandEvent&)
{
  wxFileDialog OpenDialog(
    0,
    "Load ASCII File",
    "",
    "",
    "ASCII MIDI files (txt)|*.txt|All files (*.*)|*.*",
    wxFD_OPEN | wxFD_CHANGE_DIR);
  if (OpenDialog.ShowModal() == wxID_OK)
  {
    wxString AsciiMidiFileName = OpenDialog.GetPath();
    gpProject->OpenAndReadAsciiMidiFile(AsciiMidiFileName);
    SetTitle(AsciiMidiFileName);
    mpTrackWindow->SetScrollRanges();
    mpTrackWindow->Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileExportMidi(wxCommandEvent& Event)
{
  // wxFD_OVERWRITE_PROMPT - For save dialog only: prompt for a confirmation
  // if a file will be overwritten.
  wxFileDialog SaveAsDialog(
    0,
    "Save MIDI File",
    "",
    "",
    "MIDI files (MID, MIDI)|*.mid;*.midi|All files (*.*)|*.*",
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (SaveAsDialog.ShowModal() == wxID_OK)
  {
    wxString FileName = SaveAsDialog.GetPath();
    gpProject->ExportMidiFile(FileName);
    SetTitle(FileName);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileExportAscii(wxCommandEvent&)
{
  // wxFD_OVERWRITE_PROMPT - For save dialog only: prompt for a confirmation
  // if a file will be overwritten.
  wxFileDialog SaveAsDialog(
    0,
    "Save MIDI File as ASCII",
    "",
    "",
    "ASCII MIDI files (txt)|*.txt|All files (*.*)|*.*",
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (SaveAsDialog.ShowModal() == wxID_OK)
  {
    wxString FileName = SaveAsDialog.GetPath();
    gpProject->ExportAsciiMidiFile(FileName);
    SetTitle(FileName);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileExportSelectionAsMidi(wxCommandEvent& Event)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnFileExit(wxCommandEvent& Event)
{
  if (OnClose() == false)
  {
    return;
  }
  Close();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnPlay(wxCommandEvent& Event)
{
  wxMouseEvent MouseEvent;
  MousePlay(MouseEvent, ePlayButton);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnPlayLoop(wxCommandEvent& Event)
{
  wxMouseEvent MouseEvent;
  MousePlay(MouseEvent, ePlayLoopButton);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnRecord(wxCommandEvent& Event)
{
  wxMouseEvent MouseEvent;
  MousePlay(MouseEvent, eRecordButton);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnPianoWindow(wxCommandEvent& Event)
{
  JZProjectManager::Instance().CreatePianoView();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnMetroOn(wxCommandEvent& Event)
{
  gpProject->ToggleMetronome();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnZoomIn(wxCommandEvent& Event)
{
  mpTrackWindow->ZoomIn();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnZoomOut(wxCommandEvent& Event)
{
  mpTrackWindow->ZoomOut();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnToolsHarmonyBrowser(wxCommandEvent& Event)
{
  CreateHarmonyBrowser();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnToolsRhythmGenerator(wxCommandEvent& Event)
{
  CreateRhythmGenerator(mpTrackWindow, mpProject);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnSettingsMetronome(wxCommandEvent& Event)
{
  JZMetronomeInfo MetronomeInfo = gpProject->GetMetronomeInfo();
  JZMetronomeSettingsDialog MetronomeSettingsDialog(
    this,
    MetronomeInfo);
  if (
    MetronomeSettingsDialog.ShowModal() == wxID_OK &&
    MetronomeInfo != gpProject->GetMetronomeInfo())
  {
    gpProject->SetMetronomeInfo(MetronomeInfo);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnSettingsSynthesizerType(wxCommandEvent& Event)
{
  JZSynthesizerDialog SynthesizerDialog(this);
  SynthesizerDialog.ShowModal();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnSettingsMidiDevice(wxCommandEvent& Event)
{
#ifdef __WXMSW__
  int InputDevice, OutputDevice;
  gpConfig->Get(C_WinInputDevice, InputDevice);
  gpConfig->Get(C_WinOutputDevice, OutputDevice);
  JZWindowsPlayer::SettingsDlg(InputDevice, OutputDevice);
  ::wxMessageBox(
    "Restart Jazz++ to activate changes in device settings",
    "Info",
    wxOK);
#else
  if (gpConfig->GetValue(C_MidiDriver) == eMidiDriverOss)
  {
    int Device = gpMidiPlayer->FindMidiDevice();
    if (Device >= 0)
    {
      gpConfig->Put(C_Seq2Device, Device);
      ::wxMessageBox(
        "Restart Jazz++ to activate changes in device settings",
        "Info",
        wxOK);
    }
    else
    {
      ::wxMessageBox("No midi device found", "Info", wxOK);
    }
  }
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnAudioGlobalSettings(wxCommandEvent& Event)
{
  mpProject->EditAudioGlobalSettings(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnAudioSampleSettings(wxCommandEvent& Event)
{
  mpProject->EditAudioSamples(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnAudioLoadSampleSet(wxCommandEvent& Event)
{
  mpProject->LoadSampleSet(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnAudioSaveSampleSet(wxCommandEvent& Event)
{
  mpProject->SaveSampleSet(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnAudioSaveSampleSetAs(wxCommandEvent& Event)
{
  mpProject->SaveSampleSetAs(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnAudioNewSampleSet(wxCommandEvent& Event)
{
  mpProject->ClearSampleSet(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnHelpContents(wxCommandEvent& Event)
{
  JZHelp::Instance().DisplayHelpContents();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZTrackFrame::OnHelpAbout(wxCommandEvent& Event)
{
  JZAboutDialog AboutDialog(this);
  AboutDialog.ShowModal();
}

//-----------------------------------------------------------------------------
// Description:
//   Handle clicks in the play bar.
// not playing:
//   events selected:
//     left : start rec/play
//     right: mute + start rec/play
//   no events selected:
//     left+right: start play
// playing:
//   left+right: stop
//-----------------------------------------------------------------------------
void JZTrackFrame::MousePlay(wxMouseEvent& MouseEvent, TEMousePlayMode Mode)
{
  mpTrackWindow->MousePlay(MouseEvent, Mode);
}
