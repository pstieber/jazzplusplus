#pragma once

#include "SampleCommand.h"

#include <wx/frame.h>

class JZRndArray;
class JZToolBar;
class JZArrayEdit;
class JZCommandPainter;
class JZDistortion;
class JZEqualizer;
class JZSample;
class JZSampleWindow;
class JZSynthDlg;
class wxDialog;
class wxPanel;
class wxScrollBar;

//*****************************************************************************
//*****************************************************************************
class JZSampleFrame : public wxFrame
{
  friend class JZSampleWindow;
  friend class JZCommandPainter;
  friend class JZSmplWinSettingsForm;

  public:

    JZSampleFrame(wxWindow* pParent, JZSampleFrame** ref, JZSample& Sample);
    ~JZSampleFrame();
    virtual void OnSize(int w, int h);
    virtual bool OnClose();
    virtual void OnMenuCommand(int id);
    void Redraw();
    bool HaveInsertionPoint(int &offs, bool warn = TRUE);
    enum HaveSelectionMode
    {
      SelWarn,
      SelNoWarn,
      SelAll
    };
    bool HaveSelection(int &fr_smpl, int &to_smpl, HaveSelectionMode = SelAll);

    void AddParam(JZRndArray *array, const char *label);
    void ClrParam();
    void ClearSelection();
    JZSample &GetSample()
    {
      return spl;
    }
    void PlaySample();

  private:

    int GetPaintLength();
    int GetPaintOffset();
#ifdef OBSOLETE
    static void ScrollCallback(wxItem &itm, wxCommandEvent& event);
    void OnScroll(wxItem &item);
#endif
    void SetViewPos(int fr, int to);
    void LoadError(JZSample &spl);

  private:

    JZSample& spl;
    JZSampleWindow* cnvs;
    wxPanel* scrol_panel;
    wxScrollBar* pos_scrol;
    wxScrollBar* zoom_scrol;
    JZToolBar* mpToolBar;
    int in_constructor;
    JZSampleFrame** ref;
    static int geo[4];

    static JZSample* copy_buffer;

    enum
    {
      MAXPARAM = 4
    };
    JZArrayEdit* params[MAXPARAM];
    int num_params;

    JZCommandPainter* on_accept;
    JZSplVolume vol_command;
    JZSplPan pan_command;
    JZSplPitch pitch_command;
    JZWahWah wah_command;

    JZEqualizer* equalizer;
    JZDistortion* distortion;
    JZSynthDlg* synth;
    wxDialog* reverb;
    wxDialog* echo;
    wxDialog* chorus;
    wxDialog* shifter;
    wxDialog* stretcher;
    wxDialog* filter;
    wxDialog* settings;
    wxDialog* wah_settings;
    wxDialog* pitch_settings;
};
