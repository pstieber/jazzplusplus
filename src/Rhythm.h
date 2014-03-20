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

#include "Globals.h"
#include "Random.h"

#include <wx/frame.h>
#include <wx/panel.h>

#include <iosfwd>
#include <vector>

class JZArrayControl;
class JZBarInfo;
class JZEventWindow;
class JZRhythmArrayControl;
class JZSong;
class JZToolBar;
class JZTrack;
class wxButton;
class wxCheckBox;
class wxListBox;
class wxPanel;
class wxSlider;

#define MAX_GROUPS  5
#define MAX_KEYS   20

//*****************************************************************************
//*****************************************************************************
class JZRhythmGroup
{
  public:

    JZRhythmGroup();

    void Write(std::ostream& Os) const;
    void Read(std::istream& Is, int Version);

    int GetContrib() const
    {
      return mContrib;
    }

    int GetListen() const
    {
      return mListen;
    }

  public:

    int mListen;
    int mContrib;
};

//*****************************************************************************
//*****************************************************************************
class JZRhythmGroups
{
  public:

    const JZRhythmGroup& operator [] (int i) const
    {
      return mRhythmGroups[i];
    }

    JZRhythmGroup& operator [] (int i)
    {
      return mRhythmGroups[i];
    }

    void Write(std::ostream& Os) const;
    void Read(std::istream& Is, int Version);

  public:

    JZRhythmGroup mRhythmGroups[MAX_GROUPS];
};

//*****************************************************************************
//*****************************************************************************
class JZRhythm
{
    friend class JZRhythmWindow;
    friend class JZRhythmGeneratorWindow;

  public:

    JZRhythm(int Key);
    JZRhythm(const JZRhythm& Other);
    JZRhythm& operator = (const JZRhythm& Other);
    virtual ~JZRhythm();

    const std::string& GetLabel() const
    {
      return mLabel;
    }

    void SetLabel(const std::string& Label);

    void Generate(
      JZTrack* pTrack,
      int FromClock,
      int ToClock,
      int TicksPerBar);

    void Generate(
      JZTrack* pTrack,
      const JZBarInfo& BarInfo,
      JZRhythm* rhy[],
      int RhythmCount);

    void Generate(
      JZTrack* pTrack,
      const JZBarInfo& BarInfo,
      const std::vector<JZRhythm*>& Instruments);

    void GenInit(int StartClock);

    void GenerateEvent(
      JZTrack* pTrack,
      int Clock,
      short vel,
      short len);

    void Write(std::ostream& Os) const;

    void Read(std::istream& Is, int Version);

    const JZRhythmGroup& GetRhythmGroup(int Index) const
    {
      return mRhythmGroups[Index];
    }

  protected:

    void GenGroup(
      JZRndArray& out,
      int grp,
      const JZBarInfo& BarInfo,
      JZRhythm* rhy[],
      int RhythmCount);

    void GenGroup(
      JZRndArray& out,
      int grp,
      const JZBarInfo& BarInfo,
      const std::vector<JZRhythm*>& Rhythms);

    int Clock2i(int Clock, const JZBarInfo& BarInfo) const;

    int GetClocksPerStep(const JZBarInfo& BarInfo) const;

  private:

    std::string mLabel;

    JZRndArray mRhythmArray;
    JZRndArray mLengthArray;
    JZRndArray mVelocityArray;

    int mStepsPerCount;
    int mCountPerBar;
    int mBarCount;
    int mKeyCount;
    int mKeys[MAX_KEYS];
    int mMode;
    int mParameter;

    bool mRandomizeFlag;
    JZRhythmGroups mRhythmGroups;
    JZRndArray mHistoryArray;

    // Set by GenInit()
    int mStartClock;
    int mNextClock;
};

//*****************************************************************************
//*****************************************************************************
class JZRhythmWindow : public wxFrame
{
  public:

    JZRhythmWindow(JZEventWindow* pEventWindow, JZSong* pSong);

    virtual ~JZRhythmWindow();

    virtual void OnMenuCommand(int id);

    virtual void OnSize(int w, int h);

    void OnPaint();

    void GenRhythm();

    bool OnClose();

  private:

    // callbacks
#ifdef OBSOLETE
    static void ItemCallback(wxItem& item, wxCommandEvent& event);
#endif
    static void SelectInstr(wxListBox& list, wxCommandEvent& event);
    static void SelectGroup(wxListBox& list, wxCommandEvent& event);
    static void Add(wxButton &but, wxCommandEvent& event);
    static void Del(wxButton &but, wxCommandEvent& event);
    static void Generate(wxButton &but, wxCommandEvent& event);

    void Instrument2Win();
    void Win2Instrument();
    void AddInstrumentDlg();
    void AddInstrument(JZRhythm *r);
    void DelInstrument();

    void RndEnable();

    void UpInstrument();
    void DownInstrument();
    void InitInstrumentList();

  private:

    wxPanel* mpInstrumentPanel;

    wxSlider* mpStepsPerCountSlider;
    wxSlider* mpCountsPerBarSlider;
    wxSlider* mpBarCountSlider;
    wxListBox* mpInstrumentListBox;
    int mActiveInstrumentIndex;  // -1 if none

    wxPanel* mpGroupPanel;
    wxSlider* mpGroupContribSlider;
    wxSlider* mpGroupListenSlider;
    wxListBox* mpGroupListBox;
    int mActiveGroup;
    wxCheckBox* mpRandomCheckBox;

    JZArrayEdit* mpLengthEdit;
    JZArrayEdit* mpVelocityEdit;
    JZRhyArrayEdit* mpRhythmEdit;

    enum
    {
      MAX_INSTRUMENTS = 20
    };

    JZRhythm* mpInstruments[MAX_INSTRUMENTS];
    int mInstrumentCount;

    // This one is edited and copied from and to mpInstruments[i].
    JZRhythm mRhythm;

    JZEventWindow* mpEventWindow;
    JZSong* mpSong;

    wxString mDefaultFileName;
    bool mHasChanged;
    wxToolBar* mpToolBar;
};

//*****************************************************************************
//*****************************************************************************
class JZRhythmGeneratorWindow : public wxPanel
{
  public:

    JZRhythmGeneratorWindow(
      JZEventWindow* pEventWindow,
      JZSong* pSong,
      wxFrame* pParent,
      const wxPoint& Position,
      const wxSize& Size);

    virtual ~JZRhythmGeneratorWindow();

    void Read(std::istream& Is);

    void Write(std::ostream& Os);

    void AddInstrument();

    void DeleteInstrument();

    void Generate();

  private:

    void ClearInstruments();

    void AddInstrument(JZRhythm* pRhythm);

    void Instrument2Win();

    void Win2Instrument();

    void RandomEnable();

    void GenerateRhythm();

    void OnSliderUpdate(wxCommandEvent& Event);

    void OnListBox(wxCommandEvent& Event);

  private:

    JZRhythm mRhythm;

    JZEventWindow* mpEventWindow;
    JZSong* mpSong;

    std::vector<JZRhythm*> mInstruments;

    wxSlider* mpStepsPerCountSlider;
    wxSlider* mpCountsPerBarSlider;
    wxSlider* mpBarCountSlider;
    wxListBox* mpInstrumentListBox;
    int mActiveInstrumentIndex;  // -1 if none
    wxSlider* mpGroupContribSlider;
    wxSlider* mpGroupListenSlider;
    wxListBox* mpGroupListBox;
    int mActiveGroup;
    wxCheckBox* mpRandomCheckBox;

    JZArrayControl* mpLengthEdit;
    JZArrayControl* mpVelocityEdit;
    JZRhythmArrayControl* mpRhythmEdit;

  DECLARE_EVENT_TABLE()
};

//*****************************************************************************
//*****************************************************************************
class JZRhythmGeneratorFrame : public wxFrame
{
  public:

    JZRhythmGeneratorFrame(JZEventWindow* pEventWindow, JZSong* pSong);

    ~JZRhythmGeneratorFrame();

  private:

    void CreateToolBar();

    void OnOpen(wxCommandEvent& Event);

    void OnSave(wxCommandEvent& Event);

    void OnAddInstrument(wxCommandEvent& Event);

    void OnDeleteInstrument(wxCommandEvent& Event);

    void OnGenerate(wxCommandEvent& Event);

    void OnHelp(wxCommandEvent& Event);

    void OnHelpContents(wxCommandEvent& Event);

  private:

    static const wxString mDefaultFileName;

    JZToolBar* mpToolBar;

    JZRhythmGeneratorWindow* mpRhythmGeneratorWindow;

  DECLARE_EVENT_TABLE()
};

//*****************************************************************************
//*****************************************************************************
extern void CreateRhythmGenerator(JZEventWindow* pEventWindow, JZSong* pSong);
