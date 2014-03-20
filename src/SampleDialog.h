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

#ifndef sampldlg_h
#define sampldlg_h

#include "Random.h"
#include "SliderWindow.h"
#include "SampleCommand.h"
#include "PropertyListDialog.h"

class JZPaintableCommand;
class JZSampleFrame;
class JZSigEqualizer;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxSlider;

/**
 * controls a JZPaintableCommand, that is shows the parameter arrays
 * in samplwin and processes OnAction.
 */

class JZCommandPainter
{
  public:
    JZCommandPainter(JZSampleFrame &w, JZPaintableCommand &cmd);
    virtual ~JZCommandPainter();
    virtual void OnAccept(int fr, int to);
  protected:
    JZSampleFrame &win;
    JZPaintableCommand &cmd;
};


class JZEqualizer : public JZSliderWindow
{
  public:
    JZEqualizer(JZSampleFrame &win);
    virtual ~JZEqualizer();
    virtual void AddItems();
    virtual void AddEdits();
#ifdef OBSOLETE
    virtual void OnItem(wxItem& item, wxCommandEvent& event);
#endif
    double Index2Hertz(int index);
  private:
    void Action();
    JZRndArray array;
    JZSplEqualizer **equ;
    JZSampleFrame &win;
    JZSample    &spl;
    wxButton *action;
    wxButton *cancel;
    static int geo[4];
    int channels;
};

class JZDistortion : public JZSliderWindow
{
  public:
    JZDistortion(JZSampleFrame &win);
    virtual void AddItems();
    virtual void AddEdits();
#ifdef OBSOLETE
    virtual void OnItem(wxItem& item, wxCommandEvent& event);
#endif
  private:
    void Action();
    void MakeCurve(int cvtype);
    void MakeExpo(int degree);
    void MakeSine(int degree);
    JZRndArray arr;
    JZSampleFrame &win;
    wxButton *action;
    wxButton *cancel;
    wxChoice *curve;
    int N, ymin, ymax;  // array dimensions
    static int geo[4];
};

// ----------------------- additive synthesis ---------------------

class JZAddSynth;
class JZRhyArrayEdit;

class JZSynthDlg : public JZSliderWindow
{
  public:
    JZSynthDlg(JZSampleFrame &win);
    virtual ~JZSynthDlg();
    virtual void AddItems();
    virtual void AddEdits();
#ifdef OBSOLETE
    virtual void OnItem(wxItem& item, wxCommandEvent& event);
#endif
    virtual void OnMenuCommand(int id);
    friend std::ostream& operator << (std::ostream& os, JZSynthDlg const &a);
    friend std::istream& operator >> (std::istream& is, JZSynthDlg &a);

  private:

    void Action();
    void SetupEdits();

    JZSampleFrame &win;
    wxButton *action;
    wxButton *cancel;
    wxCheckBox *chk_vol;
    wxCheckBox *chk_pan;
    wxCheckBox *chk_frq;
    wxCheckBox *chk_fft;
    wxCheckBox *chk_noise;
    wxSlider   *num_synths_slider;
    wxSlider   *midi_key_slider;
    wxSlider   *duration_slider;

    enum
    {
      MAXSYNTHS = 6
    };
    JZAddSynth *synths[MAXSYNTHS];
    static int num_synths;
    static int midi_key;
    static int duration;
    static bool vol_enable;
    static bool pan_enable;
    static bool frq_enable;
    static bool fft_enable;
    static bool noise_enable;

    static int geo[4];
    wxString mDefaultFileName;
};


// --------------------------- reverb ----------------------------


class JZReverbForm : public JZPropertyListDlg
{
  public:
    JZReverbForm(JZSampleFrame &win);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  private:
    // space params
    static int roomsize;
    static int brightness;
    static int rvbtime;
    static int volume;

    JZSampleFrame &win;
};


class JZEchoForm : public JZPropertyListDlg
{
  public:
    JZEchoForm(JZSampleFrame &win);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  private:
    static int num_echos;
    static int delay;
    static int ampl;
    static bool rand;
    JZSampleFrame &win;
};

class JZShifterForm : public JZPropertyListDlg
{
  public:
    JZShifterForm(JZSampleFrame &win);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  private:
    static int shift_semis;
    static int shift_frac;
    static bool keep_length;
    static int winsize;
    JZSampleFrame &win;
};


class JZStretcherForm : public JZPropertyListDlg
{
  public:
    JZStretcherForm(JZSampleFrame &win);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  private:
    static int seconds;
    static int centies;
    static int winsize;
    static int oldspeed;
    static int newspeed;
    static bool keep_pitch;
    JZSampleFrame &win;
    JZSample    &spl;
};


class JZSplFilterForm : public JZPropertyListDlg
{
  public:
    JZSplFilterForm(JZSampleFrame &win, bool painter = FALSE);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  protected:
    void ScanResults();
    static int order;
    static int type;
    static int freq;
    static int lo_freq;
    static int hi_freq;
    static int band_width;
    JZSampleFrame &win;
    wxList  typelist;
    char    *typestring;
    bool    painter;
};


class JZWahWah;
class JZWahSettingsForm : public JZSplFilterForm
{
  public:
    JZWahSettingsForm(JZSampleFrame &win, JZWahWah &wah);
    void OnOk();
    void OnHelp();
  private:
    JZWahWah &wah;
};

class JZSplPitch;
class JZSplPitchForm : public JZPropertyListDlg
{
  public:
    JZSplPitchForm(JZSampleFrame &win, JZSplPitch &pitch_painter);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  protected:
    static int range;
    JZSampleFrame &win;
    JZSplPitch  &pitch;
};


class JZChorusForm : public JZPropertyListDlg
{
  public:
    JZChorusForm(JZSampleFrame &win);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  private:
    JZSampleFrame &win;

    static int pitch_freq;
    static int pitch_range;
    static int pan_freq;
    static int pan_spread;
    static int volume;
};

#if 0
class JZStereoForm : public JZPropertyListDlg
{
  public:
    JZStereoForm(JZSampleFrame &win);
    void EditForm(wxPanel *panel);
    void OnOk();
    void OnCancel();
    void OnHelp();
  private:
    JZSampleFrame &win;

    static int delay;
    static int stereo_spread;
};

#endif

#endif

