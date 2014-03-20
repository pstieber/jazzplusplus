#include "SampleFrame.h"

#include "FileSelector.h"
#include "Help.h"
#include "Mapper.h"
#include "Player.h"
#include "Resources.h"
#include "SampleDialog.h"
#include "SampleWindow.h"

#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/scrolbar.h>

#include "Bitmaps/open.xpm"
#include "Bitmaps/save.xpm"
#include "Bitmaps/play.xpm"
#include "Bitmaps/help.xpm"
#include "Bitmaps/accept.xpm"
#include "Bitmaps/cancel.xpm"
#include "Bitmaps/zoomin.xpm"
#include "Bitmaps/zoomout.xpm"

#define MEN_HELP        5

#define MEN_SILENCE     21
#define MEN_SILENCE_INS 22
#define MEN_SILENCE_APP 23
#define MEN_SILENCE_OVR 24

#define MEN_ACCEPT      42

#define MEN_FLIP        56
#define MEN_FLIP_LEFT   57
#define MEN_FLIP_RIGHT  58

static JZToolDef tdefs[] =
{
  { wxID_OPEN,     false, open_xpm,    "open wave file" },
  { wxID_SAVE,     false, save_xpm,    "save wave file" },
  { JZToolBar::eToolBarSeparator },
  { wxID_ZOOM_IN,  false, zoomin_xpm,  "zoom to selection" },
  { wxID_ZOOM_OUT, false, zoomout_xpm, "zoom out" },
  { MEN_ACCEPT,   false, accept_xpm,  "accept painting" },
  { ID_PAINTER_NONE,   false, cancel_xpm,  "cancel painting" },
  { JZToolBar::eToolBarSeparator },
  { ID_PLAY,     false, play_xpm,    "play sample" },
  { MEN_HELP,     false, help_xpm,    "help" },
  { JZToolBar::eToolBarEnd }
};

int JZSampleFrame::geo[4] =
{
  30,
  30,
  600,
  300
};

JZSample *JZSampleFrame::copy_buffer;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSampleFrame::JZSampleFrame(
  wxWindow* pParent,
  JZSampleFrame **ref,
  JZSample& Sample)
  : wxFrame(
      pParent,
      wxID_ANY,
      Sample.GetFileName(),
      wxPoint(geo[0], geo[1]),
      wxSize(geo[2], geo[3])),
    spl(Sample),
    vol_command(Sample),
    pan_command(Sample),
    pitch_command(Sample),
    wah_command(Sample)
{
  this->ref = ref;

  in_constructor = true;

  cnvs         = 0;
  mpToolBar    = 0;
  scrol_panel  = 0;
  pos_scrol    = 0;
  zoom_scrol   = 0;
  num_params   = 0;
  on_accept    = 0;
  equalizer    = 0;
  distortion   = 0;
  reverb       = 0;
  echo         = 0;
  shifter      = 0;
  stretcher    = 0;
  filter       = 0;
  settings     = 0;
  wah_settings = 0;
  pitch_settings = 0;
  chorus       = 0;
  synth        = 0;

  if (copy_buffer == 0)
    copy_buffer = new JZSample(spl.SampleSet());

  mpToolBar = new JZToolBar(this, tdefs);

  // Create a menu bar, the various menus with entries, and attach them to the
  // menu bar.

  wxMenu* pMenu = 0;
  wxMenu* pSubMenu = 0;
  wxMenuBar* pMenuBar = new wxMenuBar;

  // Create and populate the File menu.
  pMenu = new wxMenu;

  pMenu->Append(ID_FILE_REVERT_TO_SAVED, "&Revert to Saved");
  pMenu->Append(wxID_OPEN, "&Load...");
  pMenu->Append(wxID_SAVE, "&Save");
  pMenu->Append(wxID_SAVEAS, "&Save As...");
  pMenu->Append(wxID_CLOSE, "&Close");

  pMenuBar->Append(pMenu, "&File");

  // Create and populate the Edit menu.
  pMenu = new wxMenu;
  pMenu->Append(wxID_CUT, "&Cut");
  pMenu->Append(wxID_COPY, "Co&py");
  pMenu->Append(wxID_PASTE, "&Paste");
  pMenu->Append(ID_EDIT_PASTE_MERGE, "Paste &Merge");
  pSubMenu = new wxMenu;
  pSubMenu->Append(MEN_SILENCE_OVR, "&Replace");
  pSubMenu->Append(MEN_SILENCE_INS, "&Insert");
  pSubMenu->Append(MEN_SILENCE_APP, "&Append");
  pMenu->Append(MEN_SILENCE, "&Silence", pSubMenu);
  pSubMenu = new wxMenu;
  pSubMenu->Append(MEN_FLIP_LEFT, "Left");
  pSubMenu->Append(MEN_FLIP_RIGHT, "Right");
  pMenu->Append(MEN_FLIP, "In&vert Phase", pSubMenu);
  pMenu->Append(ID_EDIT_MAXIMIZE_VOLUME, "&Maximize Volume");
  pMenuBar->Append(pMenu, "&Edit");

  pMenu = new wxMenu;
  pMenu->Append(ID_PAINTERS_VOLUME, "&Volume...");
  pMenu->Append(ID_PAINTER_PAN, "&Panpot...");
  pMenu->Append(ID_PAINTER_PITCH, "&Pitch...");
  pMenu->Append(ID_PAINTER_WAHWAH, "&Filter...");
  pMenu->Append(ID_PAINTER_NONE, "&None...");
  pMenuBar->Append(pMenu, "&Painters");

  pMenu = new wxMenu;
  pMenu->Append(ID_EFFECTS_EQUALIZER, "&Equalizer...");
  pMenu->Append(ID_EFFECTS_FILTER, "&Filter...");
  pMenu->Append(ID_EFFECTS_DISTORTION, "&Distortion...");
  pMenu->Append(ID_EFFECTS_REVERB, "&Reverb...");
  pMenu->Append(ID_EFFECTS_ECHO, "&Echo...");
  pMenu->Append(ID_EFFECTS_CHORUS, "&Chorus...");
  pMenu->Append(ID_EFFECTS_PITCH_SHIFTER, "&Pitch shifter...");
  pMenu->Append(ID_EFFECTS_STRETCHER, "&Time stretcher...");
  pMenu->Append(ID_EFFECTS_REVERSE, "Re&verse");
  pMenu->Append(ID_EFFECTS_SYNTH, "&Synth...");
  pMenuBar->Append(pMenu,         "&Effects");

  pMenu = new wxMenu;
  pMenu->Append(ID_SETTINGS_PITCH_PAINTER, "&Pitch Painter...");
  pMenu->Append(ID_SETTINGS_WAHWAH, "&Filter Painter...");
//  pMenu->Append(wxID_ZOOM_IN,     "Zoom &In");
//  pMenu->Append(wxID_ZOOM_OUT,     "Zoom &Out");
  pMenu->Append(ID_VIEW_SETTINGS, "&View Settings...");
  pMenuBar->Append(pMenu, "&Settings");

  SetMenuBar(pMenuBar);

  // construct a panel containing the scrollbars
  cnvs = new JZSampleWindow(this, spl);
  scrol_panel = new wxPanel(this);

//OBSOLETE  pos_scrol   = new wxScrollBar(scrol_panel, (wxFunction)ScrollCallback);
  pos_scrol = new wxScrollBar(scrol_panel, wxID_ANY);
//  pos_scrol->SetObjectLength(1000);
//  pos_scrol->SetViewLength(1000);
//  pos_scrol->SetValue(0);
  pos_scrol->SetScrollbar(0, 1000, 1000, 1000);

//OBSOLETE  zoom_scrol  = new wxScrollBar(scrol_panel, (wxFunction)ScrollCallback);
  zoom_scrol  = new wxScrollBar(scrol_panel, wxID_ANY);
//  zoom_scrol->SetObjectLength(1000);
//  zoom_scrol->SetViewLength(10);
//  zoom_scrol->SetPageLength(100);
//  zoom_scrol->SetValue(0);
  zoom_scrol->SetScrollbar(0, 10, 1000, 100);

  in_constructor = false;

  // now force a resize for motif
  int cw, ch;
  GetClientSize(&cw, &ch);
  OnSize(cw, ch);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSampleFrame::~JZSampleFrame()
{
  *ref = 0;
  GetPosition(&geo[0], &geo[1]);
  GetSize(&geo[2], &geo[3]);
  delete mpToolBar;
  delete cnvs;
  delete zoom_scrol;
  delete pos_scrol;
  delete scrol_panel;
  for (int i = 0; i < num_params; i++)
    delete params[i];
  delete equalizer;
  delete distortion;
  delete reverb;
  delete echo;
  delete chorus;
  delete synth;
  delete shifter;
  delete stretcher;
  delete filter;
  delete settings;
  delete wah_settings;
  delete pitch_settings;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSampleFrame::OnClose()
{
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::OnSize(int w, int h)
{
  // constructor finished?
  if (in_constructor)
    return;

  int cw, ch;
  GetClientSize(&cw, &ch);

  wxSize ToolBarSize = mpToolBar->GetSize();
  int pw, ph;
  pos_scrol->GetSize(&pw, &ph);
  int zw, zh;
  zoom_scrol->GetSize(&zw, &zh);

//OBSOLETE  mpToolBar->SetSize(0, 0, (int)cw, ToolBarSize.GetHeight());
  scrol_panel->SetSize(0, ch-zh-ph, cw, zh+ph);
  zoom_scrol->SetSize(0, 0, cw, zh);
  pos_scrol->SetSize(0, zh, cw, ph);

  // divide the remaining space on cnvs and params
  int xx = 0;
  int yy = ToolBarSize.GetHeight();
  int ww = cw;
  int hh = ch - ToolBarSize.GetHeight() - zh - ph;
  int nn = spl.GetChannelCount() + num_params;

  int hi = hh * spl.GetChannelCount() / nn;
  cnvs->SetSize(xx, yy, ww, hi);

  hi = hh / nn;
  for (int i = 0; i < num_params; i++)
  {
    int yi = yy + (i + spl.GetChannelCount()) * hh / nn;
    params[i]->SetSize(xx, yi, ww, hi);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::Redraw()
{
  cnvs->Redraw();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSampleFrame::HaveInsertionPoint(int &offs, bool warn)
{
  if (cnvs->sel_fr == cnvs->sel_to && cnvs->sel_fr >= 0)
  {
    offs = cnvs->sel_fr;
    return true;
  }
  else
  {
    offs = -1;
    if (warn)
    {
      ::wxMessageBox("Please set the insertion point first", "Error", wxOK);
    }
    return false;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZSampleFrame::HaveSelection(
  int& fr_smpl,
  int& to_smpl,
  HaveSelectionMode mode)
{
  if (cnvs->sel_fr < cnvs->sel_to && cnvs->sel_fr >= 0)
  {
    fr_smpl = cnvs->sel_fr;
    to_smpl = cnvs->sel_to;
    return true;
  }
  else if (mode == SelAll)
  {
    fr_smpl = 0;
    to_smpl = spl.GetLength();
    return true;
  }
  fr_smpl = to_smpl = -1;
  if (mode == SelWarn)
  {
    ::wxMessageBox("Please select the samples first", "Error", wxOK);
  }
  return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::AddParam(JZRndArray *array, const char *label)
{
  params[num_params] =
    new JZArrayEdit(this, *array, wxPoint(0, 0), wxSize(10, 10), 0);
  params[num_params]->SetLabel(label);
  num_params++;
  int cw, ch;
  GetClientSize(&cw, &ch);
  OnSize(cw, ch);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::ClrParam()
{
  if (num_params > 0)
  {
    int n = num_params;
    num_params = 0;
    for (int i = 0; i < n; ++i)
    {
      delete params[i];
    }
    int cw, ch;
    GetClientSize(&cw, &ch);
    OnSize(cw, ch);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::ClearSelection()
{
  cnvs->ClearSelection();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::LoadError(JZSample& Sample)
{
  wxString Message;
  Message << "Could not load " << Sample.GetFileName();
  ::wxMessageBox(Message, "Error", wxOK);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern int effect(JZSample &spl);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::OnMenuCommand(int id)
{
  if (gpMidiPlayer->IsPlaying())
  {
    return;
  }

  // Player crashes if data disappear.
  if (id != ID_PLAY)
  {
    cnvs->playpos->StopListen();
  }

  switch (id)
  {
    case ID_EFFECTS_EQUALIZER:
      if (equalizer == 0)
        equalizer = new JZEqualizer(*this);
      equalizer->Show(true);
      break;

    case MEN_FLIP_LEFT:
      spl.Flip(0);
      break;
    case MEN_FLIP_RIGHT:
      spl.Flip(1);
      break;

    case ID_EFFECTS_DISTORTION:
      if (distortion == 0)
        distortion = new JZDistortion(*this);
      distortion->Show(true);
      break;

    case ID_EFFECTS_REVERB:
#ifdef OBSOLETE
      if (reverb == 0)
      {
        // Old version was not modal.
        reverb = new wxDialog(this, wxID_ANY, "Reverb");
        tReverbForm *form = new tReverbForm(*this);
        form->EditForm(reverb);
        reverb->Fit();
      }
      ClearSelection();
      SetViewPos(0, spl.GetLength());
      reverb->Show(true);
#endif
      break;

    case ID_EFFECTS_REVERSE:
      {
        int fr, to;
        if (HaveSelection(fr, to))
        {
          spl.Reverse(fr, to);
          Redraw();
        }
      }
      break;

    case ID_EFFECTS_PITCH_SHIFTER:
#ifdef OBSOLETE
      if (shifter == 0)
      {
        shifter = new wxDialogBox(this, "Shifter", false );
        tShifterForm *form = new tShifterForm(*this);
        form->EditForm(shifter);
        shifter->Fit();
      }
      ClearSelection();
      SetViewPos(0, spl.GetLength());
      shifter->Show(true);
#endif
      break;

    case ID_EFFECTS_STRETCHER:
#ifdef OBSOLETE
      if (stretcher == 0)
      {
        stretcher = new wxDialogBox(this, "Stretcher", false );
        tStretcherForm *form = new tStretcherForm(*this);
        form->EditForm(stretcher);
        stretcher->Fit();
      }
      ClearSelection();
      SetViewPos(0, spl.GetLength());
      stretcher->Show(true);
#endif
      break;

    case ID_EFFECTS_FILTER:
#ifdef OBSOLETE
      if (filter == 0)
      {
        filter = new wxDialogBox(this, "Filter", false );
        tSplFilterForm *form = new tSplFilterForm(*this);
        form->EditForm(filter);
        filter->Fit();
      }
      filter->Show(true);
#endif
      break;

    case ID_VIEW_SETTINGS:
#ifdef OBSOLETE
      if (settings == 0)
      {
        settings = new wxDialogBox(this, "Settings", false );
        JZSmplWinSettingsForm *form = new JZSmplWinSettingsForm(*this);
        form->EditForm(settings);
        settings->Fit();
      }
      settings->Show(true);
#endif
      break;

    case ID_EFFECTS_ECHO:
#ifdef OBSOLETE
      if (echo == 0)
      {
        echo = new wxDialogBox(this, "Echo", false );
        tEchoForm *form = new tEchoForm(*this);
        form->EditForm(echo);
        echo->Fit();
      }
      ClearSelection();
      SetViewPos(0, spl.GetLength());
      echo->Show(true);
#endif
      break;

    case ID_EFFECTS_CHORUS:
#ifdef OBSOLETE
      if (chorus == 0)
      {
        chorus = new wxDialogBox(this, "Chorus", false );
        tChorusForm *form = new tChorusForm(*this);
        form->EditForm(chorus);
        chorus->Fit();
      }
      ClearSelection();
      SetViewPos(0, spl.GetLength());
      chorus->Show(true);
#endif
      break;

    case ID_EFFECTS_SYNTH:
      if (synth == 0)
        synth = new JZSynthDlg(*this);
      synth->Show(true);
      break;

    case MEN_ACCEPT:
      if (on_accept)
      {
        int fr = GetPaintOffset();
        int to = fr + GetPaintLength();
        on_accept->OnAccept(fr, to);
        delete on_accept;
        on_accept = 0;
      }
      break;

    case ID_PAINTER_NONE:
      if (on_accept)
      {
        delete on_accept;
        on_accept = 0;
      }
      break;

    case wxID_CUT:
      {
        int fr, to;
        if (HaveSelection(fr, to, SelWarn))
        {
          spl.Cut(*copy_buffer, fr, to);
          cnvs->ClearSelection();
          cnvs->SetInsertionPoint(fr);
          Redraw();
        }
      }
      break;

    case wxID_COPY:
      {
        int fr, to;
        if (HaveSelection(fr, to, SelAll))
          spl.Copy(*copy_buffer, fr, to);
      }
      break;

    case wxID_ZOOM_IN:
      {
        int fr, to;
        if (HaveSelection(fr, to, SelWarn))
          SetViewPos(fr, to);
      }
      break;

    case wxID_ZOOM_OUT:
      SetViewPos(0, spl.GetLength());
      break;

    case ID_EDIT_MAXIMIZE_VOLUME:
      spl.Rescale();
      Redraw();
      break;

    case ID_PAINTERS_VOLUME:
      delete on_accept;
      on_accept = new JZCommandPainter(*this, vol_command);
      break;

    case ID_PAINTER_WAHWAH:
      delete on_accept;
      on_accept = new JZCommandPainter(*this, wah_command);
      break;

    case ID_SETTINGS_WAHWAH:
#ifdef OBSOLETE
      if (wah_settings == 0)
      {
        wah_settings = new wxDialogBox(this, "Filter Painter", false);
        tWahSettingsForm *form = new tWahSettingsForm(*this);
        form->EditForm(wah_settings);
        wah_settings->Fit();
      }
      wah_settings->Show(true);
#endif
      break;

    case ID_SETTINGS_PITCH_PAINTER:
#ifdef OBSOLETE
      if (pitch_settings == 0)
      {
        pitch_settings = new wxDialogBox(this, "Pitch Painter");
        tSplPitchForm *form = new tSplPitchForm(*this);
        form->EditForm(pitch_settings);
        pitch_settings->Fit();
      }
      pitch_settings->Show(true);
#endif
      break;


    case ID_PAINTER_PAN:
      delete on_accept;
      on_accept = new JZCommandPainter(*this, pan_command);
      break;

    case ID_EDIT_PASTE_MERGE:
      {
        int offs;
        if (HaveInsertionPoint(offs))
        {
          spl.PasteMix(*copy_buffer, offs);
          cnvs->SetSelection(offs, offs + copy_buffer->GetLength());
          Redraw();
        }
      }
      break;

    case wxID_PASTE:
      {
        int offs, fr, to;
        if (HaveInsertionPoint(offs, false))
        {
          spl.PasteIns(*copy_buffer, offs);
          cnvs->SetSelection(offs, offs + copy_buffer->GetLength());
          Redraw();
        }
        else if (HaveSelection(fr, to, SelWarn))
        {
          spl.PasteOvr(*copy_buffer, fr, to);
          cnvs->SetInsertionPoint(fr);
          Redraw();
        }
      }
      break;

    case MEN_SILENCE_INS:
      {
        int fr, to;
        if (HaveSelection(fr, to, SelWarn))
        {
          spl.InsertSilence(fr, to - fr);
          Redraw();
        }
      }
      break;

    case MEN_SILENCE_APP:
      {
        int fr, to;
        if (HaveSelection(fr, to, SelWarn))
        {
          spl.InsertSilence(to, to - fr);
          Redraw();
        }
      }
      break;

    case MEN_SILENCE_OVR:
      {
        int fr, to;
        if (HaveSelection(fr, to, SelWarn))
        {
          spl.ReplaceSilence(fr, to - fr);
          Redraw();
        }
      }
      break;

    case ID_PAINTER_PITCH:
      delete on_accept;
      SetViewPos(0, spl.GetLength());
      on_accept = new JZCommandPainter(*this, pitch_command);
      break;

    case ID_FILE_REVERT_TO_SAVED:
      cnvs->ClearSelection();
      if (spl.Load(true))
        LoadError(spl);
      Redraw();
      break;

    case wxID_CLOSE:
//      DELETE_THIS();
      Destroy();
      break;

    case ID_PLAY:
      cnvs->Play();
      break;

    case wxID_OPEN:
      {
        wxString FileName = file_selector(
          spl.GetFileName(),
          "Load Sample",
          false,
          false,
          "*.wav");
        if (!FileName.empty())
        {
          wxBeginBusyCursor();
          cnvs->ClearSelection();
          spl.SetFileName(FileName);
          if (spl.Load(true))
          {
            LoadError(spl);
          }
          spl->RefreshDialogs();
          SetTitle(FileName);
          Redraw();
          wxEndBusyCursor();
        }
      }
      break;

    case wxID_SAVEAS:
      {
        wxString FileName = file_selector(
          spl.GetFileName(),
          "Save Sample",
          true,
          false,
          "*.wav");
        if (!FileName.empty())
        {
          spl.SetFileName(FileName);
          OnMenuCommand(wxID_SAVE);
          spl->RefreshDialogs();
          SetTitle(FileName);
        }
      }
      break;

    case wxID_SAVE:
      {
        if (spl.GetFileName().empty())
        {
          OnMenuCommand(wxID_SAVEAS);
        }
        else
        {
          wxBeginBusyCursor();
          cnvs->ClearSelection();
          int err = spl.Save();
          Redraw();
          wxEndBusyCursor();
          if (err)
          {
            ::wxMessageBox("Writing failed!!", "Error", wxOK);
          }
        }
      }
      break;

    case MEN_HELP:
      JZHelp::Instance().ShowTopic("Sample Editor");
      break;

    default:
      break;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::PlaySample()
{
  cnvs->Play();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleFrame::GetPaintLength()
{
  // return the visible amount of sample data
  double sb = zoom_scrol->GetThumbPosition();
  JZMapper Map(0, 1000, spl.GetLength(), 0);
  int len = static_cast<int>(Map.XToY(sb));
  return spl.Align(len);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleFrame::GetPaintOffset()
{
  // return the visible Offset in sample data
  double sb = pos_scrol->GetThumbPosition();
  JZMapper Map(0, 1000, 0, spl.GetLength());
  int ofs = static_cast<int>(Map.XToY(sb));
  return spl.Align(ofs);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::SetViewPos(int fr, int to)
{
  JZMapper Map(0, spl.GetLength(), 0, 1000);
  int zval = 1000 - (int)Map.XToY(to - fr);
  zoom_scrol->SetThumbPosition(zval);

  int  pval = (int)Map.XToY(fr);
  if (pval > zval)
    pval = zval;

  // avoid motif warnings: by setting a very small length,
  // every position is valid.
//  pos_scrol->SetViewLength(1);
//  pos_scrol->SetValue(pval);
//  pos_scrol->SetViewLength(1000 - zval);
//  pos_scrol->SetPageLength((1000 - zval) * 2 / 3);
  pos_scrol->SetScrollbar(pval, 1, 1000 - zval, (1000 - zval) * 2 / 3);

  Redraw();
}

#ifdef OBSOLETE
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::OnScroll(wxItem &item)
{
  int  zval   = zoom_scrol->GetValue();
  int  pval   = pos_scrol->GetValue();

  if (pval > zval)
    pval = zval;

  pos_scrol->SetValue(pval);
  pos_scrol->SetViewLength(1000 - zval);
  pos_scrol->SetPageLength((1000 - zval) * 2 / 3);

  Redraw();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleFrame::ScrollCallback(wxItem &itm, wxCommandEvent& Event)
{
  ((JZSampleFrame *)(itm.GetParent()->GetParent()))->OnScroll(itm);
}
#endif
