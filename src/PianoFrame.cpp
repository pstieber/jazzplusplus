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

#include "PianoFrame.h"

#include "Command.h"
#include "ControlEdit.h"
#include "Dialogs.h"
#include "Globals.h"
#include "Harmony.h"
#include "PianoWindow.h"
#include "ProjectManager.h"
#include "Resources.h"
#include "Song.h"
#include "Track.h"
#include "Synth.h"
#include "StandardFile.h"
#include "Player.h"
#include "GuitarFrame.h"
#include "ToolBar.h"
#include "Help.h"
#include "Rectangle.h"

#include <wx/menu.h>
#include <wx/msgdlg.h>

#include <sstream>

using namespace std;

// ************************************************************************
// Menubar
// ************************************************************************

#define ACT_SETTINGS                5
#define MEN_FILTER                  6
#define ACT_HELP_MOUSE              9

#define MEN_LERI                   18
#define MEN_UPDN                   19

#define MEN_VISIBLE                22

#define MEN_CTRL_EDIT              23
#define MEN_CTRL_PITCH             24
#define MEN_CTRL_CONTR             25
#define MEN_CTRL_VELOC             26
#define MEN_CTRL_NONE              27
#define MEN_CTRL_MODUL             28

#define MEN_GUITAR                 29

#define MEN_VIS_ALL_TRK            37

#define ACT_CLOSE                  41
#define MEN_CTRL_TEMPO             42

#define MEN_REDO                   47

#define MEN_CTRL_POLY_AFTER        50
#define MEN_CTRL_CHANNEL_AFTER     51

#define MEN_SEQLENGTH              52
#define MEN_MIDIDELAY              53
#define MEN_CONVERT_TO_MODULATION  54

/*
static JZToolDef tdefs[] =
{
  { ID_SELECT,     TRUE, select_xpm,    "select events"},
  { ID_CHANGE_LENGTH,     TRUE, length_xpm,    "change length"},
  { ID_EVENT_DIALOG,     TRUE, dialog_xpm,    "event dialog"},
  { ID_CUT_PASTE_EVENTS,   TRUE, cutpaste_xpm,  "cut/paste events"},
  { JZToolBar::eToolBarSeparator },
  { ID_SNAP_8,      TRUE, note8_xpm,     "snap 1/8"},
  { ID_SNAP_8D,     TRUE, note83_xpm,    "snap 1/12"},
  { ID_SNAP_16,     TRUE, note16_xpm,    "snap 1/16"},
  { ID_SNAP_16D,    TRUE, note163_xpm,   "snap 1/24"},
  { JZToolBar::eToolBarSeparator },
  { wxID_CUT,        FALSE, cut_xpm,      "cut selection"},
  { wxID_DELETE,     FALSE, delete_xpm,   "delete selection"},
  { ID_QUANTIZE,     FALSE, quantize_xpm, "quantize selection"},
  { ID_SHIFT_LEFT,   FALSE, shiftl_xpm,   "shift selection left"},
  { ID_SHIFT_RIGHT,  FALSE, shiftr_xpm,   "shift selection right"},
  { MEN_VIS_ALL_TRK, TRUE,  evnts_xpm,    "show events from all tracks"},
  { JZToolBar::eToolBarSeparator },
  { wxID_ZOOM_IN,    FALSE, zoomin_xpm,   "zoom in"},
  { wxID_ZOOM_OUT,   FALSE, zoomout_xpm,  "zoom out"},
  { wxID_UNDO,       FALSE, undo_xpm,     "undo"},
  { MEN_REDO,        FALSE, redo_xpm,     "redo"},
  { ID_MISC_RESET_MIDI,   FALSE, panic_xpm,    "all notes off"},
  { ID_HELP_PIANO_WINDOW, FALSE, help_xpm,     "help"},
  { JZToolBar::eToolBarEnd }
};
*/


// These are the tool bar icons.
#include "Bitmaps/note8.xpm"
#include "Bitmaps/note83.xpm"
#include "Bitmaps/note16.xpm"
#include "Bitmaps/note163.xpm"
#include "Bitmaps/cut.xpm"
#include "Bitmaps/delete.xpm"
#include "Bitmaps/quantize.xpm"
#include "Bitmaps/evnts.xpm"
#include "Bitmaps/undo.xpm"
#include "Bitmaps/redo.xpm"
#include "Bitmaps/zoomin.xpm"
#include "Bitmaps/zoomout.xpm"
#include "Bitmaps/panic.xpm"
#include "Bitmaps/help.xpm"
#include "Bitmaps/shiftl.xpm"
#include "Bitmaps/shiftr.xpm"
#include "Bitmaps/select.xpm"
#include "Bitmaps/length.xpm"
#include "Bitmaps/dialog.xpm"
#include "Bitmaps/cutpaste.xpm"

//*****************************************************************************
// Description:
//   This is the track piano class definition.
//*****************************************************************************
BEGIN_EVENT_TABLE(JZPianoFrame, wxFrame)

  EVT_MENU(wxID_ZOOM_IN, JZPianoFrame::OnZoomIn)
  EVT_MENU(wxID_ZOOM_OUT, JZPianoFrame::OnZoomOut)
  EVT_MENU(ID_SNAP_8, JZPianoFrame::OnSnap8)
  EVT_MENU(ID_SNAP_8D, JZPianoFrame::OnSnap8D)
  EVT_MENU(ID_SNAP_16, JZPianoFrame::OnSnap16)
  EVT_MENU(ID_SNAP_16D, JZPianoFrame::OnSnap16D)

  EVT_MENU(ID_SELECT, JZPianoFrame::OnMSelect)
  EVT_MENU(ID_CHANGE_LENGTH, JZPianoFrame::OnMLength)
  EVT_MENU(ID_EVENT_DIALOG, JZPianoFrame::OnMDialog)
  EVT_MENU(ID_CUT_PASTE_EVENTS, JZPianoFrame::OnMCutPaste)
  EVT_MENU(MEN_GUITAR, JZPianoFrame::OnGuitar)

  EVT_MENU(ID_MISC_RESET_MIDI, JZPianoFrame::OnReset)
  EVT_MENU(MEN_VIS_ALL_TRK, JZPianoFrame::OnVisibleAllTracks)
  EVT_MENU(wxID_DELETE, JZPianoFrame::OnErase)
  EVT_MENU(wxID_CUT, JZPianoFrame::OnCut)
  EVT_MENU(wxID_COPY, JZPianoFrame::OnCopy)
  EVT_MENU(ID_SHIFT, JZPianoFrame::OnShift)
  EVT_MENU(ID_SHIFT_LEFT, JZPianoFrame::OnShiftLeft)
  EVT_MENU(ID_SHIFT_RIGHT, JZPianoFrame::OnShiftRight)
  EVT_MENU(MEN_LERI, JZPianoFrame::OnExchangeLeftRight)
  EVT_MENU(MEN_UPDN, JZPianoFrame::OnExchangeUpDown)
  EVT_MENU(ID_QUANTIZE, JZPianoFrame::OnQuantize)
  EVT_MENU(wxID_UNDO, JZPianoFrame::OnUndo)
  EVT_MENU(wxID_REDO, JZPianoFrame::OnRedo)
  EVT_MENU(MEN_CTRL_PITCH, JZPianoFrame::OnCtrlPitch)
  EVT_MENU(MEN_CTRL_MODUL, JZPianoFrame::OnCtrlModulation)
  EVT_MENU(MEN_CTRL_CONTR, JZPianoFrame::OnSelectController)
  EVT_MENU(MEN_CTRL_VELOC, JZPianoFrame::OnCtrlVelocity)
  EVT_MENU(MEN_CTRL_TEMPO, JZPianoFrame::OnCtrlTempo)
  EVT_MENU(MEN_CTRL_NONE, JZPianoFrame::OnCtrlNone)
  EVT_MENU(MEN_CTRL_POLY_AFTER, JZPianoFrame::OnCtrlPolyAftertouchEdit)
  EVT_MENU(MEN_CTRL_CHANNEL_AFTER, JZPianoFrame::CtrlChannelAftertouchEdit)
// FIXME PAT - We need to bring these back once Dave has figured out what
//             he's doing with them in relation to the track window.
//  EVT_MENU(ID_CLEANUP, JZPianoFrame::OnCleanup)
//  EVT_MENU(ID_SEARCH_AND_REPLACE, JZPianoFrame::OnSearchReplace)
//  EVT_MENU(ID_TRANSPOSE, JZPianoFrame::OnTranspose)
//  EVT_MENU(ID_SET_CHANNEL, JZPianoFrame::OnSetChannel)
//  EVT_MENU(ID_LENGTH, JZPianoFrame::OnLength)
  EVT_MENU(MEN_MIDIDELAY, JZPianoFrame::OnActivateMidiDelayDialog)
  EVT_MENU(MEN_SEQLENGTH, JZPianoFrame::OnActivateSequenceLengthDialog)

//  EVT_MENU(MEN_CONVERT_TO_MODULATION, JZPianoFrame::OnnConvertToModulation)
  EVT_MENU(ACT_SETTINGS, JZPianoFrame::OnActivateSettingsDialog)
  EVT_MENU(MEN_FILTER, JZPianoFrame::OnFilter)
  EVT_MENU(ID_SNAP, JZPianoFrame::OnSnapDlg)

  // These are all "Patrick Approved"
  EVT_CLOSE(JZPianoFrame::ActCloseEvent)
  EVT_MENU(ACT_CLOSE, JZPianoFrame::ActClose)
  EVT_MENU(ACT_HELP_MOUSE, JZPianoFrame::ActHelpMouse)

//  EVT_MENU(wxID_EXIT, JZPianoFrame::OnFileExit)

//  EVT_MENU(wxID_HELP_CONTENTS, JZPianoFrame::OnHelpContents)

//  EVT_MENU(wxID_ABOUT, JZPianoFrame::OnHelpAbout)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPianoFrame::JZPianoFrame(
  wxWindow* pParent,
  const wxString& Title,
  JZProject* pProject,
  const wxPoint& Position,
  const wxSize& Size)
  : wxFrame(
      pParent,
      wxID_ANY,
      Title,
      Position,
      Size,
      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE),
    mpPianoWindow(0),
//    mpFileMenu(0),
//    mpEditMenu(0)
    mpProject(pProject),
    mpToolBar(0)
{
  CreateToolBar();

//  CreateMenu();

  mpDialogBox = 0;
  MixerForm = 0;

  CreateMenu();

  mpToolBar->ToggleTool(ID_SELECT, TRUE);

  mClockTicsPerPixel = 4;

  mpToolBar->ToggleTool(ID_SNAP_16, TRUE);

  mpPianoWindow = new JZPianoWindow(this, pProject);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPianoFrame::~JZPianoFrame()
{
  int XPosition, YPosition, Width, Height;

  GetPosition(&XPosition, &YPosition);
  GetSize(&Width, &Height);

  gpConfig->Put(C_PianoWinXpos, XPosition);
  gpConfig->Put(C_PianoWinYpos, YPosition);
  gpConfig->Put(C_PianoWinWidth, Width);
  gpConfig->Put(C_PianoWinHeight, Height);

  delete mpPianoWindow;

  delete MixerForm;

  delete mpToolBar;

  JZProjectManager::Instance()->Detach(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoFrame::CreateToolBar()
{
  JZToolDef ToolBarDefinitions[] =
  {
    { ID_SELECT,           true, select_xpm,  "select events" },
    { ID_CHANGE_LENGTH,    true, length_xpm,   "change length"},
    { ID_EVENT_DIALOG,     true, dialog_xpm,   "event dialog"},
    { ID_CUT_PASTE_EVENTS, true, cutpaste_xpm, "cut/paste events"},
    { JZToolBar::eToolBarSeparator },
    { ID_SNAP_8,   true, note8_xpm,   "snap 1/8"},
    { ID_SNAP_8D,  true, note83_xpm,  "snap 1/12"},
    { ID_SNAP_16,  true, note16_xpm,  "snap 1/16"},
    { ID_SNAP_16D, true, note163_xpm, "snap 1/24"},
    { JZToolBar::eToolBarSeparator },
    { wxID_CUT,        false, cut_xpm,      "cut selection"},
    { wxID_DELETE,     false, delete_xpm,   "delete selection" },
    { ID_QUANTIZE,     false, quantize_xpm, "quantize selection" },
    { ID_SHIFT_LEFT,   false, shiftl_xpm,   "shift selection left"},
    { ID_SHIFT_RIGHT,                     false, shiftr_xpm, "shift selection right"},
    { ID_SHOW_ALL_EVENTS_FROM_ALL_TRACKS, true,  evnts_xpm,  "show events from all tracks"},
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

// show the guitar edit window.
void JZPianoFrame::OnGuitar(wxCommandEvent& Event)
{
  JZProjectManager::Instance()->CreateGuitarView();
}





void JZPianoFrame::CreateMenu()
{
  wxMenu *win_menu = new wxMenu;
  win_menu->Append(ACT_CLOSE, "&Close");

  wxMenu *edit_menu = new wxMenu("",wxMENU_TEAROFF);
  edit_menu->Append(wxID_DELETE, "&Delete");
  edit_menu->Append(wxID_COPY, "&Copy");
  edit_menu->Append(wxID_CUT, "&Cut");
  edit_menu->Append(ID_SHIFT, "&Shift...");
  edit_menu->Append(ID_QUANTIZE, "&Quantize...");
  edit_menu->Append(ID_SET_CHANNEL, "&Set MIDI Channel...");
  edit_menu->Append(ID_TRANSPOSE, "&Transpose...");
  edit_menu->Append(ID_VELOCITY, "&Velocity...");
  edit_menu->Append(ID_LENGTH, "&Length...");

  edit_menu->Append(MEN_SEQLENGTH, "&Sequence Length...");
  edit_menu->Append(MEN_MIDIDELAY, "&Midi Delay...");
  edit_menu->Append(MEN_CONVERT_TO_MODULATION, "&Convert to Modulation(experimental)");

  edit_menu->Append(MEN_LERI, "&Left <-> Right");
  edit_menu->Append(MEN_UPDN, "&Up <-> Down");
  edit_menu->Append(ID_CLEANUP, "&Cleanup...");
  edit_menu->Append(ID_SEARCH_AND_REPLACE, "&Search Replace...");

  wxMenu *setting_menu = new wxMenu("", wxMENU_TEAROFF);
  setting_menu->Append(MEN_FILTER, "&Filter...");
  setting_menu->Append(ACT_SETTINGS, "&Window...");
  setting_menu->Append(MEN_VISIBLE, "&Events...");
  setting_menu->Append(ID_SNAP, "&Snap...");
  setting_menu->Append(ID_MISC_METER_CHANGE, "&Meter Change...");

  wxMenu *misc_menu = new wxMenu("",wxMENU_TEAROFF);
  misc_menu->Append(wxID_UNDO, "&Undo");
  misc_menu->Append(wxID_REDO, "&Redo");
  misc_menu->Append(MEN_CTRL_PITCH,        "Edit &Pitch");
  misc_menu->Append(MEN_CTRL_VELOC,        "Edit &Velocity");
  misc_menu->Append(MEN_CTRL_MODUL,        "Edit &Modulation");

  misc_menu->Append(MEN_CTRL_POLY_AFTER, "Edit &Key Aftertouch");
  misc_menu->Append(MEN_CTRL_CHANNEL_AFTER,  "Edit &Chn Aftertouch");

  misc_menu->Append(MEN_CTRL_CONTR,        "Edit &Controller...");
  misc_menu->Append(MEN_CTRL_TEMPO,        "Edit &Tempo");
  misc_menu->Append(MEN_CTRL_NONE,        "Edit &None");
  misc_menu->Append(MEN_GUITAR,                "&Guitar board");

  wxMenu *help_menu = new wxMenu("",wxMENU_TEAROFF);
  help_menu->Append(ID_HELP_PIANO_WINDOW, "&Pianowin");
  help_menu->Append(ACT_HELP_MOUSE, "&Mouse");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(win_menu,    "&Window");
  menu_bar->Append(edit_menu,    "&Edit");
  menu_bar->Append(setting_menu, "&Settings");
  menu_bar->Append(misc_menu,    "&Misc");
  menu_bar->Append(help_menu,    "&Help");

  SetMenuBar(menu_bar);
}

void JZPianoFrame::OnFilter(wxCommandEvent& Event)
{
  mpPianoWindow->EditFilter();
}


// Activate velocity edit.
void JZPianoFrame::OnCtrlVelocity(wxCommandEvent& Event)
{
  mpPianoWindow->CtrlVelocity();
}


void JZPianoFrame::CtrlChannelAftertouchEdit(wxCommandEvent& Event)
{
  mpPianoWindow->CtrlChannelAftertouchEdit();
}

void JZPianoFrame::OnCtrlPolyAftertouchEdit(wxCommandEvent& Event)
{
  mpPianoWindow->CtrlPolyAftertouchEdit();
}

void JZPianoFrame::OnCtrlNone(wxCommandEvent& Event)
{
  mpPianoWindow->CtrlNone();
}

void JZPianoFrame::OnCtrlTempo(wxCommandEvent& Event)
{
  mpPianoWindow->CtrlTempo();
}

void JZPianoFrame::OnSelectController(wxCommandEvent& Event)
{
  mpPianoWindow->SelectController();
}

void JZPianoFrame::OnCtrlModulation(wxCommandEvent& Event)
{
  mpPianoWindow->CtrlModulation();
}

void JZPianoFrame::OnCtrlPitch(wxCommandEvent& Event)
{
  mpPianoWindow->CtrlPitch();
}

// Redo undone actions
void JZPianoFrame::OnRedo(wxCommandEvent& Event)
{
  mpPianoWindow->Redo();
}

// Undo actions
void JZPianoFrame::OnUndo(wxCommandEvent& Event)
{
  mpPianoWindow->Undo();
}

// Quantize selected events
void JZPianoFrame::OnQuantize(wxCommandEvent& Event)
{
  mpPianoWindow->Quantize();
}

// Flip events up and down.
void JZPianoFrame::OnExchangeUpDown(wxCommandEvent& Event)
{
  mpPianoWindow->ExchangeUpDown();
}

// Flip events left to right.
void JZPianoFrame::OnExchangeLeftRight(wxCommandEvent& Event)
{
  mpPianoWindow->ExchangeLeftRight();
}

// Shift events snapclock clocks to left.
void JZPianoFrame::OnShiftLeft(wxCommandEvent& Event)
{
  mpPianoWindow->ShiftLeft();
}

// Shift events snapclock clocks to right.
void JZPianoFrame::OnShiftRight(wxCommandEvent& Event)
{
  mpPianoWindow->ShiftRight();
}

void JZPianoFrame::OnShift(wxCommandEvent& Event)
{
  // FIXME PAT - Bring this back once Dave has figured out what's he's doing
  // with the trackwin stuff.
  //MenShift(SnapClocks());
}

void JZPianoFrame::OnCut(wxCommandEvent& Event)
{
  mpPianoWindow->CutOrCopy(wxID_CUT);
}

void JZPianoFrame::OnCopy(wxCommandEvent& Event)
{
  mpPianoWindow->CutOrCopy(wxID_COPY);
}

void JZPianoFrame::OnErase(wxCommandEvent& Event)
{
  mpPianoWindow->Erase();
}

// Toggle the display of events from all tracks, or just from the current track.
void JZPianoFrame::OnVisibleAllTracks(wxCommandEvent& Event)
{
  mpPianoWindow->ToggleVisibleAllTracks();
}

//  Send a midi reset.
void JZPianoFrame::OnReset(wxCommandEvent& Event)
{
  gpMidiPlayer->AllNotesOff(true);
}

void JZPianoFrame::OnMSelect(wxCommandEvent& Event)
{
  mpPianoWindow->MSelect();
}

void JZPianoFrame::OnMLength(wxCommandEvent& Event)
{
  mpPianoWindow->MLength();
}

void JZPianoFrame::OnMDialog(wxCommandEvent& Event)
{
  mpPianoWindow->MDialog();
}

void JZPianoFrame::OnMCutPaste(wxCommandEvent& Event)
{
  mpPianoWindow->MCutPaste();
}

void JZPianoFrame::OnZoomIn(wxCommandEvent& Event)
{
  mpPianoWindow->ZoomIn();
}

void JZPianoFrame::OnZoomOut(wxCommandEvent& Event)
{
  mpPianoWindow->ZoomOut();
}


void JZPianoFrame::OnSnap8(wxCommandEvent& Event)
{
  mpPianoWindow->Snap8();
}

void JZPianoFrame::OnSnap8D(wxCommandEvent& Event)
{
  mpPianoWindow->Snap8D();
}

void JZPianoFrame::OnSnap16(wxCommandEvent& Event)
{
  mpPianoWindow->Snap16();
}

void JZPianoFrame::OnSnap16D(wxCommandEvent& Event)
{
  mpPianoWindow->Snap16D();
}









#ifdef OBSOLETE

class JZVisibleDlg : public wxForm
{
  JZPianoFrame *pPianoWindow;
  public:

    JZVisibleDlg(JZPianoFrame *p)
      : wxForm( USED_WXFORM_BUTTONS ),
        pPianoWindow(p)
    {
    }
    void EditForm(wxPanel *panel);
    virtual void OnOk();
    virtual void OnHelp();
};

void JZVisibleDlg::OnOk()
{
  pPianoWindow->SetVisibleAllTracks(pPianoWindow->VisibleAllTracks);
  // pPianoWindow->Redraw();
  wxForm::OnOk();
}

void JZVisibleDlg::OnHelp()
{
  JZHelp::Instance().ShowTopic("Events");
}


void JZVisibleDlg::EditForm(wxPanel *panel)
{
  Add(wxMakeFormMessage("Select Events to be shown"));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormBool("NoteOn", &pPianoWindow->VisibleKeyOn));
  //Add(wxMakeFormBool("Pitch", &pPianoWindow->VisiblePitch));
  Add(wxMakeFormBool("Controller", &pPianoWindow->VisibleController));
  Add(wxMakeFormBool("Program", &pPianoWindow->VisibleProgram));
  Add(wxMakeFormBool("Tempo", &pPianoWindow->VisibleTempo));
  Add(wxMakeFormBool("SysEx", &pPianoWindow->VisibleSysex));
  Add(wxMakeFormBool("PlayTrack", &pPianoWindow->VisiblePlayTrack));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormBool("Show drumnames on drumtracks", &pPianoWindow->VisibleDrumNames));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormBool("Show events from all Tracks", &pPianoWindow->VisibleAllTracks));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormBool("Show harmonies from Harmony Browser", &pPianoWindow->VisibleHBChord));
  AssociatePanel(panel);
}


void JZPianoFrame::VisibleDialog()
{
  wxDialogBox *panel = new wxDialogBox(this, "Select Events", FALSE );
  JZVisibleDlg * dlg = new JZVisibleDlg(this);
  dlg->EditForm(panel);
  panel->Fit();
  panel->Show(TRUE);
}
#endif // OBSOLETE




// ********************************************************************
// Mouse
// ********************************************************************

/*
 * left drag: Events markieren for Menu
 * left click:
 *   mit shift: copy
 *   ohne shift: cut
 *   auf Leer: paste
 * left click + ctrl:
 *   Note-Dialog
 * right drag:
 *   linke haelfte  : verschieben
 *   rechte halefte : Laenge einstellen
 * right click:
 *   Focus TrackWin
 *
 * PianoRoll:
 *   Left: midiout
 */




// --------------------------------------------------------------------



// ------------------------------------------------------------------------
// dispatch Mouseevent
// ------------------------------------------------------------------------

// **************************************************************************
// Snap
// **************************************************************************


void JZPianoFrame::OnSnapDlg(wxCommandEvent& Event)
{
  mpPianoWindow->SnapDialog();
}


void JZPianoFrame::PressRadio(int Id)
{
  static const int Ids[] =
  {
//    ID_SNAP_8,
//    ID_SNAP_8D,
//    ID_SNAP_16,
//    ID_SNAP_16D,
    ID_SELECT,
    ID_CHANGE_LENGTH,
    ID_EVENT_DIALOG,
    ID_CUT_PASTE_EVENTS,
    0
  };
  for (const int* pId = Ids; *pId; ++pId)
  {
    if (*pId != Id && mpToolBar->GetToolState(*pId))
    {
      mpToolBar->ToggleTool(*pId, false);
    }
  }

  if (Id > 0 && !mpToolBar->GetToolState(Id))
  {
    mpToolBar->ToggleTool(Id, true);
  }
}

void JZPianoFrame::SetToolbarButtonState(int Id)
{
  const int Size = 4;
  int Table[Size] =
  {
    ID_SNAP_8,
    ID_SNAP_8D,
    ID_SNAP_16,
    ID_SNAP_16D
  };

  // toggle toolbar buttons
  for (int i = 0; i < Size; ++i)
  {
    if (Table[i] != Id && mpToolBar->GetToolState(Table[i]))
    {
      mpToolBar->ToggleTool(Table[i], false);
    }
  }

  if (Id > 0 && !mpToolBar->GetToolState(Id))
  {
    mpToolBar->ToggleTool(Id, true);
  }
}

void JZPianoFrame::SetVisibleAllTracks(bool Value)
{
  mpToolBar->ToggleTool(MEN_VIS_ALL_TRK, Value);
  mpPianoWindow->SetVisibleAllTracks(Value);
}

void JZPianoFrame::NewPlayPosition(int Clock)
{
  mpPianoWindow->NewPlayPosition(Clock);
}

void JZPianoFrame::ShowPitch(int Pitch)
{
  mpPianoWindow->ShowPitch(Pitch);
}

void JZPianoFrame::Redraw()
{
  mpPianoWindow->Refresh();
}

void JZPianoFrame::OnActivateSettingsDialog(wxCommandEvent& Event)
{
  mpPianoWindow->ActivateSettingsDialog();
}

// This is a test to see how to implement a dialog with Patrick's system.
// It replaces JZMidiDelayDlg, which isnt necesarily a good idea.
void JZPianoFrame::OnActivateMidiDelayDialog(wxCommandEvent& Event)
{
  mpPianoWindow->ActivateMidiDelayDialog();
}

// Call the sequence length command.
void JZPianoFrame::OnActivateSequenceLengthDialog(wxCommandEvent& Event)
{
  mpPianoWindow->ActivateSequenceLengthDialog();
}

void JZPianoFrame::ActClose(wxCommandEvent& Event)
{
  Show(false);
}

// This event is generated when the window's close button is pressed.
void JZPianoFrame::ActCloseEvent(wxCloseEvent& Event)
{
  // This strange code brought to you by the documentation for wxCloseEvent.
  if (Event.CanVeto())
  {
    Show(false);
    Event.Veto();
  }
  else
  {
    Destroy();
  }
}

void JZPianoFrame::ActHelpMouse(wxCommandEvent& Event)
{
  wxMessageBox(
    "On Top Line:\n"
      "    Left Click:  Start/stop play\n"
      "        +Shift:  Start/stop cycle play\n"
      "    Middle Click:  Start/stop cycle play\n"
      "On Event Area:\n"
      "    Left Click:  Depends on mode\n"
      "        +Shift:  Continue selection\n"
      "        +Ctrl:  Increase velocity\n"
      "        +Ctrl+Shift:  Cut/paste event\n"
      "    Middle Click:  Cut/paste event\n"
      "        +Shift:  Copy event\n"
      "        +Ctrl:  Event dialog\n"
      "    Right Click:  Edit note length / change track\n"
      "        +Shift:  Play pitch\n"
      "        +Ctrl:  Decrease velocity\n"
      "        +Ctrl+Shift:  Copy event",
    "Mouse Help");
}
