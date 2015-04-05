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

#include "Rhythm.h"

#include "ArrayControl.h"
#include "Command.h"
#include "EventWindow.h"
#include "FileSelector.h"
#include "Filter.h"
#include "Globals.h"
#include "Harmony.h"
#include "Help.h"
#include "KeyStringConverters.h"
#include "PianoWindow.h"
#include "Resources.h"
#include "RhythmArrayControl.h"
#include "SelectControllerDialog.h"
#include "Song.h"
#include "StringReadWrite.h"
#include "ToolBar.h"
#include "TrackFrame.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choicdlg.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/toolbar.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

#include "Bitmaps/open.xpm"
#include "Bitmaps/save.xpm"
#include "Bitmaps/rrgadd.xpm"
#include "Bitmaps/rrgdel.xpm"
#include "Bitmaps/rrgup.xpm"
#include "Bitmaps/rrgdown.xpm"
#include "Bitmaps/rrggen.xpm"
#include "Bitmaps/help.xpm"

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmGroup::JZRhythmGroup()
  : mListen(0),
    mContrib(0)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGroup::Write(ostream& Os) const
{
  Os << mListen << ' ';
  Os << mContrib << ' ';
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGroup::Read(istream& Is, int Version)
{
  Is >> mListen;
  Is >> mContrib;
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGroups::Write(ostream& Os) const
{
  for (int i = 0; i < MAX_GROUPS; i++)
  {
    mRhythmGroups[i].Write(Os);
  }
  Os << endl;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGroups::Read(istream& Is, int Version)
{
  for (int i = 0; i < MAX_GROUPS; i++)
  {
    mRhythmGroups[i].Read(Is, Version);
  }
}

// Pseudo key numbers for the harmony browser and sound effects.
static const int MODE_ALL_OF  = -1;
static const int MODE_ONE_OF  = -2;
static const int MODE_PIANO   = -3;
static const int MODE_CONTROL = -4;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythm::JZRhythm(int Key)
  : mLabel("random rhythm"),
    mRhythmArray(64, 0, 100),
    mLengthArray(8, 0, 100),
    mVelocityArray(32, 0, 100),
    mStepsPerCount(4),
    mCountPerBar(4),
    mBarCount(1),
    mKeyCount(1),
    mMode(MODE_ALL_OF),
    mParameter(0),
    mRandomizeFlag(true),
    mRhythmGroups(),
    mHistoryArray(64, 0, 100),
    mStartClock(0),
    mNextClock(0)
{
  mKeys[0] = Key;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythm::JZRhythm(const JZRhythm& Other)
  : mLabel(Other.mLabel),
    mRhythmArray(Other.mRhythmArray),
    mLengthArray(Other.mLengthArray),
    mVelocityArray(Other.mVelocityArray),
    mStepsPerCount(Other.mStepsPerCount),
    mCountPerBar(Other.mCountPerBar),
    mBarCount(Other.mBarCount),
    mKeyCount(Other.mKeyCount),
    mMode(Other.mMode),
    mParameter(Other.mParameter),
    mRandomizeFlag(Other.mRandomizeFlag),
    mRhythmGroups(Other.mRhythmGroups),
    mHistoryArray(Other.mHistoryArray),
    mStartClock(Other.mStartClock),
    mNextClock(Other.mNextClock)
{
  for (int i = 0; i < mKeyCount; i++)
  {
    mKeys[i] = Other.mKeys[i];
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythm& JZRhythm::operator = (const JZRhythm& Rhs)
{
  if (this != &Rhs)
  {
    mLabel = Rhs.mLabel;
    mRhythmArray = Rhs.mRhythmArray;
    mLengthArray = Rhs.mLengthArray;
    mVelocityArray = Rhs.mVelocityArray;
    mStepsPerCount = Rhs.mStepsPerCount;
    mCountPerBar = Rhs.mCountPerBar;
    mBarCount = Rhs.mBarCount;
    mKeyCount = Rhs.mKeyCount;
    for (int i = 0; i < mKeyCount; ++i)
    {
      mKeys[i] = Rhs.mKeys[i];
    }
    mMode = Rhs.mMode;
    mParameter = Rhs.mParameter;
    mRandomizeFlag = Rhs.mRandomizeFlag;
    mRhythmGroups = Rhs.mRhythmGroups;
    mHistoryArray = Rhs.mHistoryArray;
    mStartClock = Rhs.mStartClock;
    mNextClock = Rhs.mNextClock;
  }

  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythm::~JZRhythm()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::Write(ostream& Os) const
{
  Os << mRhythmArray;
  Os << mLengthArray;
  Os << mVelocityArray;

  Os << mStepsPerCount << ' ';
  Os << mCountPerBar << ' ';
  Os << mBarCount << ' ';
  Os << mMode << ' ';
  Os << mKeyCount << ' ';
  for (int i = 0; i < mKeyCount; ++i)
  {
    Os << mKeys[i] << ' ';
  }
  Os << mParameter << endl;
  WriteString(Os, mLabel) << endl;

  Os << mRandomizeFlag << ' ';
  mRhythmGroups.Write(Os);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::Read(istream& Is, int Version)
{
  Is >> mRhythmArray;
  Is >> mLengthArray;
  Is >> mVelocityArray;

  Is >> mStepsPerCount;
  Is >> mCountPerBar;
  Is >> mBarCount;
  Is >> mMode;
  if (mMode >= 0) // old format
  {
    mKeyCount = 1;
    mKeys[0] = mMode;
    mMode = MODE_ALL_OF;
  }
  else
  {
    Is >> mKeyCount;
    for (int i = 0; i < mKeyCount; i++)
    {
      Is >> mKeys[i];
    }
  }
  Is >> mParameter;

  string Label;
  ReadString(Is, Label);
  SetLabel(Label);

  if (Version > 1)
  {
    Is >> mRandomizeFlag;
    mRhythmGroups.Read(Is, Version);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::SetLabel(const string& Label)
{
  mLabel = Label;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZRhythm::Clock2i(int Clock, const JZBarInfo& BarInfo) const
{
  return
    (int)(((Clock - mStartClock) /
    GetClocksPerStep(BarInfo)) % mRhythmArray.Size());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZRhythm::GetClocksPerStep(const JZBarInfo& BarInfo) const
{
  return BarInfo.GetTicksPerBar() / (mStepsPerCount * mCountPerBar);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::GenInit(int StartClock)
{
  int i;
  mStartClock = StartClock;
  mNextClock  = StartClock;

  int Size = mRhythmArray.Size();
  mHistoryArray.Resize(Size);

  // Initialize history with random values.
  for (i = 0; i < Size; ++i)
  {
    mHistoryArray[i] = mHistoryArray.GetMin();
  }

  for (i = 0; i < Size; i++)
  {
    if (mRhythmArray.Random(i))
    {
      mHistoryArray[i] = mHistoryArray.GetMax();
      i += mLengthArray.Random();
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::GenerateEvent(JZTrack* pTrack, int Clock, short vel, short len)
{
  int chan = pTrack->mChannel - 1;

  // generate key events
  if (mMode == MODE_ALL_OF)
  {
    for (int ii = 0; ii < mKeyCount; ii++)
    {
      JZKeyOnEvent *k = new JZKeyOnEvent(Clock, chan, mKeys[ii], vel, len);
      pTrack->Put(k);
    }
  }
  else if (mMode == MODE_ONE_OF)
  {
    int ii = (int)(rnd.asDouble() * mKeyCount);
    if (ii < mKeyCount)
    {
      JZKeyOnEvent *k = new JZKeyOnEvent(Clock, chan, mKeys[ii], vel, len);
      pTrack->Put(k);
    }
  }
  else if (mMode == MODE_CONTROL)
  {
    // generate controller
    JZControlEvent* c = new JZControlEvent(Clock, chan, mParameter - 1, vel);
    pTrack->Put(c);
  }
  else
  {
    assert(0);
  }
}

#if 0
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::Generate(
  JZTrack* pTrack,
  int FromClock,
  int ToClock,
  int TicksPerBar)
{
  int chan   = pTrack->Channel - 1;
  int Clock = FromClock;

  int ClocksPerStep = TicksPerBar / (mStepsPerCount * mCountPerBar);
  int TotalSteps = (ToClock - FromClock) / ClocksPerStep;

  while (Clock < ToClock)
  {
    int i = ((Clock - FromClock) / ClocksPerStep) % mRhythmArray.Size();
    if (mRhythmArray.Random(i))
    {
      // put event here
      int rndval;
      if (mRandomizeFlag)
      {
        // keep seed < 1.0
        rndval = mVelocityArray.Random((double)mRhythmArray[i] / ((double)mRhythmArray.Max() + 0.001));
      }
      else
      {
        rndval = mVelocityArray.Random();
      }
      short vel = rndval * 127 / mVelocityArray.Size() + 1;
      short len = (mLengthArray.Random() + 1) * ClocksPerStep;

      // generate keys from harmony browser
      if (key == CHORD_KEY || key == BASS_KEY)
      {
        if (gpHarmonyBrowser)
        {
          int Step = (Clock - FromClock) * TotalSteps / (ToClock - FromClock);
          int Keys[12], KeyCount;
          if (key == CHORD_KEY)
          {
            KeyCount = gpHarmonyBrowser->GetChordKeys(Keys, Step, TotalSteps);
          }
          else
          {
            mKeyCount = gpHarmonyBrowser->GetBassKeys(Keys, Step, TotalSteps);
          }
          for (int j = 0; j < mKeyCount; j++)
          {
            JZKeyOnEvent* k = new JZKeyOnEvent(
              Clock,
              chan,
              Keys[j],
              vel,
              len - ClocksPerStep / 2);
            pTrack->Put(k);
          }
        }
      }

      // paste pianowin buffer
      else if (key == PASTE_KEY)
      {
        JZEventArray &src = gpTrackWindow->GetPianoWindow()->PasteBuffer;
        for (int ii = 0; ii < src.mEventCount; ii++)
        {
          JZKeyOnEvent* pKeyOn = src.Events[ii]->IsKeyOn();
          if (pKeyOn)
          {
            JZKeyOnEvent *k = new JZKeyOnEvent(
              Clock,
              chan,
              pKeyOn->Key,
              vel,
              len - ClocksPerStep / 2);
            pTrack->Put(k);
          }
        }
      }

      // generate controller
      else if (key == CONTROL_KEY)
      {
        JZControlEvent* c =
          new JZControlEvent(Clock, chan, mParameter - 1, vel);
        pTrack->Put(c);
      }
      // generate note on events
      else
      {
        JZKeyOnEvent* k =
          new JZKeyOnEvent(Clock, chan, key, vel, len - ClocksPerStep / 2);
        pTrack->Put(k);
      }

      Clock += len;
    }
    else
    {
      Clock += ClocksPerStep;
    }
  }
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::GenGroup(
  JZRndArray& out,
  int grp,
  const JZBarInfo& BarInfo,
  const vector<JZRhythm*>& Rhythms)
{
  out.Clear();

  int ClocksPerStep = GetClocksPerStep(BarInfo);

  for (const auto& pRhythm : Rhythms)
  {
    int fuzz = pRhythm->GetRhythmGroup(grp).GetContrib();
    if (fuzz && pRhythm != this)
    {
      JZRndArray tmp(mRhythmArray);
      tmp.Clear();
      int Clock = BarInfo.GetClock();
      while (Clock < BarInfo.GetClock() + BarInfo.GetTicksPerBar())
      {
        int i = Clock2i(Clock, BarInfo);
        int j = pRhythm->Clock2i(Clock, BarInfo);
        tmp[i] = pRhythm->mHistoryArray[j];
        Clock += ClocksPerStep;
      }
      out.SetUnion(tmp, fuzz);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::GenGroup(
  JZRndArray& out,
  int grp,
  const JZBarInfo& BarInfo,
  JZRhythm* rhy[],
  int RhythmCount)
{
  out.Clear();

  int ClocksPerStep = GetClocksPerStep(BarInfo);

  for (int RhythmIndex = 0; RhythmIndex < RhythmCount; ++RhythmIndex)
  {
    JZRhythm* pRhythm = rhy[RhythmIndex];
    int fuzz = pRhythm->mRhythmGroups[grp].mContrib;
    if (fuzz && pRhythm != this)
    {
      JZRndArray tmp(mRhythmArray);
      tmp.Clear();
      int Clock = BarInfo.GetClock();
      while (Clock < BarInfo.GetClock() + BarInfo.GetTicksPerBar())
      {
        int i = Clock2i(Clock, BarInfo);
        int j = pRhythm->Clock2i(Clock, BarInfo);
        tmp[i] = pRhythm->mHistoryArray[j];
        Clock += ClocksPerStep;
      }
      out.SetUnion(tmp, fuzz);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::Generate(
  JZTrack* pTrack,
  const JZBarInfo& BarInfo,
  JZRhythm* rhy[],
  int RhythmCount)
{
  JZRndArray rrg(mRhythmArray);

  // Add groups to the rhythm array.
  JZRndArray tmp(mRhythmArray);
  for (int gi = 0; gi < MAX_GROUPS; ++gi)
  {
    if (mRhythmGroups[gi].mListen)
    {
      GenGroup(tmp, gi, BarInfo, rhy, RhythmCount);
      if (mRhythmGroups[gi].mListen > 0)
      {
        rrg.SetIntersection(tmp, mRhythmGroups[gi].mListen);
      }
      else
      {
        rrg.SetDifference(tmp, -mRhythmGroups[gi].mListen);
      }
    }
  }

  // Clear part of the history.
  int Clock = BarInfo.GetClock();
  int ClocksPerStep = GetClocksPerStep(BarInfo);
  while (Clock < BarInfo.GetClock() + BarInfo.GetTicksPerBar())
  {
    int i = Clock2i(Clock, BarInfo);
    mHistoryArray[i] = 0;
    Clock += ClocksPerStep;
  }

  //  generate the events
  Clock = mNextClock;
  while (Clock < BarInfo.GetClock() + BarInfo.GetTicksPerBar())
  {
    int i = Clock2i(Clock, BarInfo);
    if ((!mRandomizeFlag && rrg[i] > 0) || rrg.Random(i))
    {
      // put event here
      mHistoryArray[i] = mRhythmArray.GetMax();

      short vel = 0;
      if (mRandomizeFlag)
      {
        vel = mVelocityArray.Random() * 127 / mVelocityArray.Size() + 1;
      }
      else
      {
        vel = rrg[i] * 126 / rrg.GetMax() + 1;
      }
      short len = (mLengthArray.Random() + 1) * ClocksPerStep;
      GenerateEvent(pTrack, Clock, vel, len - ClocksPerStep / 2);
      Clock += len;
    }
    else
    {
      Clock += ClocksPerStep;
    }
  }
  mNextClock = Clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythm::Generate(
  JZTrack* pTrack,
  const JZBarInfo& BarInfo,
  const std::vector<JZRhythm*>& Instruments)
{
  JZRndArray rrg(mRhythmArray);

  // Add groups to the rhythm array.
  JZRndArray tmp(mRhythmArray);
  for (int gi = 0; gi < MAX_GROUPS; ++gi)
  {
    if (mRhythmGroups[gi].mListen)
    {
      GenGroup(tmp, gi, BarInfo, Instruments);
      if (mRhythmGroups[gi].mListen > 0)
      {
        rrg.SetIntersection(tmp, mRhythmGroups[gi].mListen);
      }
      else
      {
        rrg.SetDifference(tmp, -mRhythmGroups[gi].mListen);
      }
    }
  }

  // Clear part of the history.
  int Clock = BarInfo.GetClock();
  int ClocksPerStep = GetClocksPerStep(BarInfo);
  while (Clock < BarInfo.GetClock() + BarInfo.GetTicksPerBar())
  {
    int i = Clock2i(Clock, BarInfo);
    mHistoryArray[i] = 0;
    Clock += ClocksPerStep;
  }

  //  generate the events
  Clock = mNextClock;
  while (Clock < BarInfo.GetClock() + BarInfo.GetTicksPerBar())
  {
    int i = Clock2i(Clock, BarInfo);
    if ((!mRandomizeFlag && rrg[i] > 0) || rrg.Random(i))
    {
      // put event here
      mHistoryArray[i] = mRhythmArray.GetMax();

      short vel = 0;
      if (mRandomizeFlag)
      {
        vel = mVelocityArray.Random() * 127 / mVelocityArray.Size() + 1;
      }
      else
      {
        vel = rrg[i] * 126 / rrg.GetMax() + 1;
      }
      short len = (mLengthArray.Random() + 1) * ClocksPerStep;
      GenerateEvent(pTrack, Clock, vel, len - ClocksPerStep / 2);
      Clock += len;
    }
    else
    {
      Clock += ClocksPerStep;
    }
  }
  mNextClock = Clock;
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmWindow::JZRhythmWindow(JZEventWindow* pEventWindow, JZSong* pSong)
  : wxFrame(
      0,
      wxID_ANY,
      "Random Rhythm Generator",
      wxPoint(
        gpConfig->GetValue(C_RhythmXpos),
        gpConfig->GetValue(C_RhythmYpos)),
      wxSize(640, 580)),
    mpStepsPerCountSlider(nullptr),
    mpCountsPerBarSlider(nullptr),
    mpBarCountSlider(nullptr),
    mpInstrumentListBox(nullptr),
    mActiveInstrumentIndex(-1),
    mpGroupPanel(nullptr),
    mpGroupContribSlider(nullptr),
    mpGroupListenSlider(nullptr),
    mpGroupListBox(nullptr),
    mActiveGroup(0),
    mpRandomCheckBox(nullptr),
    mpLengthEdit(nullptr),
    mpVelocityEdit(nullptr),
    mpRhythmEdit(nullptr),
    mInstrumentCount(0),
    mRhythm(0),
    mpEventWindow(pEventWindow),
    mpSong(pSong),
    mDefaultFileName("noname.rhy"),
    mHasChanged(false),
    mpToolBar(nullptr)
{
#ifdef OBSOLETE

  mStepsPerCount = 0;
  mpCountsPerBarSlider = nullptr;
  mpBarCountSlider = nullptr;
  mpInstrumentListBox = nullptr;

  int x = 0;
  int y = 0;
  int w, h;
  GetClientSize(&w, &h);
  wxPanel* pInstrumentPanel = new wxPanel(this, x, y, w/2, h/2, 0, "InstPanel");

  mpStepsPerCountSlider = new wxSlider(
    pInstrumentPanel,
    (wxFunction)ItemCallback,
    "",
    4,
    1,
    16,
    w / 6,
    10,
    1,
    wxFIXED_LENGTH);
  (void) new wxMessage(pInstrumentPanel, "steps/count");
  pInstrumentPanel->NewLine();

  mpCountsPerBarSlider = new wxSlider(
    pInstrumentPanel,
    (wxFunction)ItemCallback,
    "",
    4,
    1,
    16,
    w / 6,
    10,
    h / 12,
    wxFIXED_LENGTH);
  (void) new wxMessage(pInstrumentPanel, "count/bar");
  pInstrumentPanel->NewLine();

  mpBarCountSlider = new wxSlider(
    pInstrumentPanel,
    (wxFunction)ItemCallback,
    "",
    4,
    1,
    16,
    w / 6,
    10,
    2 * h / 12,
    wxFIXED_LENGTH);
  (void) new wxMessage(pInstrumentPanel, "# bars");
  pInstrumentPanel->NewLine();

  pInstrumentPanel->SetLabelPosition(wxVERTICAL);
  mpInstrumentListBox = new wxListBox(
    pInstrumentPanel,
    (wxFunction)SelectInstr,
    "Instrument",
    wxLB_SINGLE /* | wxLB_ALWAYS_SB */,
    -1,
    -1,
    220,
    80);

  pInstrumentPanel->NewLine();

  // Random array edits.

  mpLengthEdit = new JZArrayEdit(
    this,
    mRhythm.mLengthArray,
    x,
    y + h / 2,
    w/2,
    h / 4 - 4);
  mpLengthEdit->SetXMinMax(1, 8);
  mpLengthEdit->SetLabel("length/interval");

  mpVelocityEdit = new JZArrayEdit(
    this,
    mRhythm.mVelocityArray,
    x + w/2,
    y + h / 2,
    w / 2,
    h / 4 - 4);
  mpVelocityEdit->SetXMinMax(1, 127);
  mpVelocityEdit->SetLabel("velocity");

  mpRhythmEdit = new JZRhyArrayEdit(
    this,
    mRhythm.mRhythmArray,
    x,
    y + 3 * h / 4,
    w,
    h / 4 - 4);
  mpRhythmEdit->SetMeter(
    mRhythm.mpStepsPerCountSlider,
    mRhythm.mCountPerBar,
    mRhythm.mBarCount);
  mpRhythmEdit->SetLabel("rhythm");

  // group panel

  mpGroupPanel =
    new wxPanel(this, x + w / 2, y, w / 2, h / 2, 0, "GroupPanel");

  mpGroupPanel->SetLabelPosition(wxHORIZONTAL);

  mpGroupContribSlider = new wxSlider(
    mpGroupPanel,
    (wxFunction)ItemCallback,
    "",
    0,
    0,
    100,
    w / 6,
    10,
    1,
    wxFIXED_LENGTH);
  (void) new wxMessage(mpGroupPanel, "contrib");
  mpGroupPanel->NewLine();

  mpGroupListenSlider = new wxSlider(
    mpGroupPanel,
    (wxFunction)ItemCallback,
    "",
    0,
    -100,
    100,
    w / 6,
    10,
    h / 12,
    wxFIXED_LENGTH);
  (void) new wxMessage(mpGroupPanel, "listen");
  mpGroupPanel->NewLine();

  mpGroupPanel->SetLabelPosition(wxVERTICAL);
  mpGroupListBox = new wxListBox(
    mpGroupPanel,
    (wxFunction)SelectGroup,
    "Group",
    wxLB_SINGLE /* | wxLB_ALWAYS_SB */,
    -1,
    -1,
    220,
    80);
  mpGroupPanel->NewLine();

  {
    char buf[100];
    int i;
    for (i = 0; i < MAX_GROUPS; i++)
    {
      sprintf(buf, "group %d", i+1);
      mpGroupListBox->Append(buf);
    }
  }
  mActiveGroup = mpGroupListBox->GetSelection();

  mpRandomCheckBox = new wxCheckBox(mpGroupPanel, (wxFunction)ItemCallback, "Randomize");

  Show(TRUE);
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::OnSize(int w, int h)
{
  if (mpToolBar)
  {
    int cw, ch;
    GetClientSize(&cw, &ch);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::OnMenuCommand(int id)
{
  switch (id)
  {
    case wxID_CLOSE:
        // motif crashes, when Show(FALSE) is called before destructor!
      // Show(FALSE);
//        DELETE_THIS();
        Destroy();
        break;

    case ID_INSTRUMENT_ADD:
      AddInstrumentDlg();
      break;
    case ID_INSTRUMENT_DELETE:
      DelInstrument();
      break;
    case ID_INSTRUMENT_GENERATE:
      wxBeginBusyCursor();
      Win2Instrument();
      GenRhythm();
      wxEndBusyCursor();
      break;
    case ID_INSTRUMENT_UP:
      UpInstrument();
      break;
    case ID_INSTRUMENT_DOWN:
      DownInstrument();
      break;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::SelectInstr(wxListBox& ListBox, wxCommandEvent&)
{
  JZRhythmWindow* pRhythmWindow =
    (JZRhythmWindow *)ListBox.GetParent()->GetParent();
  pRhythmWindow->Win2Instrument();
  pRhythmWindow->mActiveInstrumentIndex =
    pRhythmWindow->mpInstrumentListBox->GetSelection();
  pRhythmWindow->Instrument2Win();
  pRhythmWindow->OnPaint();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::SelectGroup(wxListBox& ListBox, wxCommandEvent&)
{
  JZRhythmWindow* pRhythmWindow =
    (JZRhythmWindow *)ListBox.GetParent()->GetParent();
  pRhythmWindow->Win2Instrument();
  pRhythmWindow->mActiveGroup = ListBox.GetSelection();
  pRhythmWindow->Instrument2Win();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::Add(wxButton& Button, wxCommandEvent&)
{
  JZRhythmWindow* pRhythmWindow = (JZRhythmWindow *)Button.GetParent()->GetParent();
  pRhythmWindow->AddInstrumentDlg();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::AddInstrumentDlg()
{
  if (mInstrumentCount >= MAX_INSTRUMENTS)
  {
    return;
  }

  wxArrayString InstrumentNames;
  vector<int> Keys;

  InstrumentNames.Add("Controller");
  Keys.push_back(MODE_CONTROL);

#if 0
  if (gpHarmonyBrowser && gpHarmonyBrowser->SeqDefined())
  {
    InstrumentNames.Add("harmony: chords");
    Keys.push_back(CHORD_KEY);
    InstrumentNames.Add("harmony: bass");
    Keys.push_back(BASS_KEY);
  }
#endif

  InstrumentNames.Add("pianowin all");
  Keys.push_back(MODE_ALL_OF);
  InstrumentNames.Add("pianowin one");
  Keys.push_back(MODE_ONE_OF);

  for (const auto& StringIntPair : gpConfig->GetDrumNames())
  {
    if (!StringIntPair.first.empty())
    {
      Keys.push_back(StringIntPair.second - 1);
      InstrumentNames.Add(StringIntPair.first);
    }
  }

  int i = ::wxGetSingleChoiceIndex(
    "Instrument",
    "Select an instrument",
    InstrumentNames);

  if (i >= 0)
  {
    Win2Instrument(); // save actual values

    JZRhythm* pRhythm = 0;
    if (mActiveInstrumentIndex >= 0)
    {
      pRhythm = new JZRhythm(*mpInstruments[mActiveInstrumentIndex]);
    }
    else
    {
      pRhythm = new JZRhythm(Keys[i]);
    }

    // drum key?
    if (Keys[i] >= 0)
    {
      pRhythm->mKeyCount  = 1;
      pRhythm->mKeys[0] = Keys[i];
      pRhythm->mMode = MODE_ALL_OF;
      pRhythm->SetLabel(InstrumentNames[i]);
    }

    // choose controller?
    else if (Keys[i] == MODE_CONTROL)
    {
      pRhythm->mParameter = SelectControllerDlg();
      if (pRhythm->mParameter < 0)
      {
        return;
      }
      pRhythm->SetLabel(gpConfig->GetCtrlName(pRhythm->mParameter).first);
      pRhythm->mMode = MODE_CONTROL;
      pRhythm->mKeyCount = 0;
    }

    else if (Keys[i] == MODE_ONE_OF || Keys[i] == MODE_ALL_OF)
    {
      ostringstream Oss;
      if (Keys[i] == MODE_ONE_OF)
      {
        Oss << "one: ";
      }
      else
      {
        Oss << "all: ";
      }
      pRhythm->mKeyCount = 0;
      pRhythm->mMode = Keys[i];
      JZEventArray events;
      JZCommandCopyToBuffer cmd(gpTrackFrame->GetPianoWindow()->GetFilter(), &events);
      cmd.Execute(0);   // no UNDO

      for (int ii = 0; ii < events.mEventCount; ii++)
      {
        JZKeyOnEvent* pKeyOn = events.mppEvents[ii]->IsKeyOn();
        if (pKeyOn)
        {
          pRhythm->mKeys[pRhythm->mKeyCount++] = pKeyOn->GetKey();
          if (pRhythm->mKeyCount > 1)
          {
            Oss << ", ";
          }
          string KeyString;
          KeyToString(pKeyOn->GetKey(), KeyString);
          Oss << KeyString;
          if (pRhythm->mKeyCount >= MAX_KEYS)
          {
            break;
          }
        }
      }
      pRhythm->SetLabel(Oss.str());

      if (pRhythm->mKeyCount == 0)
      {
        wxMessageBox("select some notes in pianowin first", "Error", wxOK);
        delete pRhythm;
        pRhythm = 0;
      }
    }

    if (pRhythm != 0)
    {
      AddInstrument(pRhythm);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::AddInstrument(JZRhythm* pRhythm)
{
  mActiveInstrumentIndex = mInstrumentCount++;
  mpInstruments[mActiveInstrumentIndex] = pRhythm;
  mpInstrumentListBox->Append(pRhythm->GetLabel());

  mpInstrumentListBox->SetSelection(mActiveInstrumentIndex);
  Instrument2Win();
  OnPaint();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::UpInstrument()
{
  if (mActiveInstrumentIndex >= 1)
  {
    JZRhythm *tmp = mpInstruments[mActiveInstrumentIndex];
    mpInstruments[mActiveInstrumentIndex] = mpInstruments[mActiveInstrumentIndex-1];
    mpInstruments[mActiveInstrumentIndex - 1] = tmp;
    --mActiveInstrumentIndex;
    InitInstrumentList();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::DownInstrument()
{
  if (
    mActiveInstrumentIndex >= 0 &&
    mActiveInstrumentIndex < mInstrumentCount - 1)
  {
    JZRhythm *tmp = mpInstruments[mActiveInstrumentIndex];
    mpInstruments[mActiveInstrumentIndex] =
      mpInstruments[mActiveInstrumentIndex + 1];
    mpInstruments[mActiveInstrumentIndex + 1] = tmp;
    ++mActiveInstrumentIndex;
    InitInstrumentList();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::InitInstrumentList()
{
  mpInstrumentListBox->Clear();
  for (int i = 0; i < mInstrumentCount; ++i)
  {
    mpInstrumentListBox->Append(mpInstruments[i]->GetLabel());
  }
  if (mActiveInstrumentIndex >= 0)
  {
    mpInstrumentListBox->SetSelection(mActiveInstrumentIndex);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::Del(wxButton& Button, wxCommandEvent&)
{
  JZRhythmWindow* pRhythmWindow =
    (JZRhythmWindow *)Button.GetParent()->GetParent();
  pRhythmWindow->DelInstrument();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::DelInstrument()
{
  int i = mActiveInstrumentIndex;
  if (i >= 0)
  {
    int k;
    delete mpInstruments[i];
    for (k = i; k < mInstrumentCount - 1; ++k)
    {
      mpInstruments[k] = mpInstruments[k + 1];
    }
    mpInstruments[k] = 0;
    mInstrumentCount--;
    mpInstrumentListBox->Delete(i);
    mActiveInstrumentIndex = mpInstrumentListBox->GetSelection();
    Instrument2Win();
    OnPaint();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::Generate(wxButton& Button, wxCommandEvent&)
{
  wxBeginBusyCursor();
  JZRhythmWindow* pRhythmWindow = 
    (JZRhythmWindow *)Button.GetParent()->GetParent();
  pRhythmWindow->Win2Instrument();
  pRhythmWindow->GenRhythm();
  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::GenRhythm()
{
  if (
    !mpEventWindow->EventsSelected(
      "Please mark the destination track in the track window"))
  {
    return;
  }

  JZFilter* pFilter = mpEventWindow->mpFilter;

  if (pFilter->GetFromTrack() != pFilter->GetToTrack())
  {
    wxMessageBox("you must select exacty 1 track", "Error", wxOK);
    return;
  }

  int FromClock = pFilter->GetFromClock();
  int ToClock = pFilter->GetToClock();
  JZTrack* pTrack = mpSong->GetTrack(pFilter->GetFromTrack());
  mpSong->NewUndoBuffer();

  // remove selection
//  if (
//    wxMessageBox(
//      "Erase destination before generating?",
//      "Replace",
//      wxYES_NO) == wxYES)
  {
    JZCommandErase erase(pFilter, 1);
    erase.Execute(0);
  }

  for (int i = 0; i < mInstrumentCount; ++i)
  {
    mpInstruments[i]->GenInit(FromClock);
  }

  JZBarInfo BarInfo(*mpSong);
  BarInfo.SetClock(FromClock);

//  for (int i = 0; i < mInstrumentCount; ++i)
//  {
//    mpInstruments[i]->Generate(
//      pTrack,
//      FromClock,
//      ToClock,
//      BarInfo.GetTicksPerBar());
//  }

  while (BarInfo.GetClock() < ToClock)
  {
    for (int i = 0; i < mInstrumentCount; ++i)
    {
      mpInstruments[i]->Generate(
        pTrack,
        BarInfo,
        mpInstruments,
        mInstrumentCount);
    }
    BarInfo.Next();
  }

  pTrack->Cleanup();

  mpEventWindow->Refresh();
}

#ifdef OBSOLETE
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::ItemCallback(wxItem& item, wxCommandEvent&)
{
  JZRhythmWindow* pRhythmWindow =
    (JZRhythmWindow *)item.GetParent()->GetParent();
  pRhythmWindow->Win2Instrument();
  pRhythmWindow->RndEnable();
  pRhythmWindow->OnPaint();
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::Win2Instrument()
{
  if (mActiveInstrumentIndex < 0)
  {
    return;
  }

  mRhythm.mStepsPerCount = mpStepsPerCountSlider->GetValue();
  mRhythm.mCountPerBar   = mpCountsPerBarSlider->GetValue();
  mRhythm.mBarCount      = mpBarCountSlider->GetValue();
  mRhythm.mRandomizeFlag = mpRandomCheckBox->GetValue();

  if (mActiveGroup >= 0)
  {
    mRhythm.mRhythmGroups[mActiveGroup].mListen =
      mpGroupListenSlider->GetValue();
    mRhythm.mRhythmGroups[mActiveGroup].mContrib =
      mpGroupContribSlider->GetValue();
  }

  *mpInstruments[mActiveInstrumentIndex] = mRhythm;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::Instrument2Win()
{
  if (mActiveInstrumentIndex < 0)
  {
    return;
  }

  mRhythm = *mpInstruments[mActiveInstrumentIndex];
  mpStepsPerCountSlider->SetValue(mRhythm.mStepsPerCount);
  mpCountsPerBarSlider->SetValue(mRhythm.mCountPerBar);
  mpBarCountSlider->SetValue(mRhythm.mBarCount);
  mpRhythmEdit->SetMeter(
    mRhythm.mStepsPerCount,
    mRhythm.mCountPerBar,
    mRhythm.mBarCount);
  mpRandomCheckBox->SetValue(mRhythm.mRandomizeFlag);

  switch (mRhythm.mMode)
  {
    case MODE_CONTROL:
      mpVelocityEdit->SetLabel("ctrl value");
      break;
    default:
      mpVelocityEdit->SetLabel("velocity");
      break;
  }

  if (mActiveGroup >= 0)
  {
    mpGroupListenSlider->SetValue(
      mRhythm.mRhythmGroups[mActiveGroup].mListen);
    mpGroupContribSlider->SetValue(
      mRhythm.mRhythmGroups[mActiveGroup].mContrib);
  }

  RndEnable();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::RndEnable()
{
  mpLengthEdit->Enable(mRhythm.mRandomizeFlag);
  mpVelocityEdit->Enable(mRhythm.mRandomizeFlag);
  mpGroupListenSlider->Enable(mRhythm.mRandomizeFlag);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmWindow::~JZRhythmWindow()
{
  int XPixel, YPixel;
  GetPosition(&XPixel, &YPixel);
  gpConfig->Put(C_RhythmXpos, XPixel);
  gpConfig->Put(C_RhythmYpos, YPixel);

  for (int i = 0; i < mInstrumentCount; i++)
  {
    delete mpInstruments[i];
  }
  delete mpToolBar;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZRhythmWindow::OnClose()
{
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmWindow::OnPaint()
{
  mpRhythmEdit->SetMeter(
    mRhythm.mStepsPerCount,
    mRhythm.mCountPerBar,
    mRhythm.mBarCount);

  mpLengthEdit->Refresh();
  mpVelocityEdit->Refresh();
  mpRhythmEdit->Refresh();
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZRhythmGeneratorWindow, wxPanel)

  EVT_SLIDER(
    IDC_SL_RHYTHM_STEPS_PER_COUNT,
    JZRhythmGeneratorWindow::OnSliderUpdate)

  EVT_SLIDER(
    IDC_SL_RHYTHM_COUNTS_PER_BAR,
    JZRhythmGeneratorWindow::OnSliderUpdate)

  EVT_SLIDER(
    IDC_SL_RHYTHM_BAR_COUNT,
    JZRhythmGeneratorWindow::OnSliderUpdate)

  EVT_LISTBOX(IDC_LB_RHYTHM_INSTRUMENTS, JZRhythmGeneratorWindow::OnListBox)

  EVT_SLIDER(
    IDC_SL_RHYTHM_GROUP_CONTRIB,
    JZRhythmGeneratorWindow::OnSliderUpdate)

  EVT_SLIDER(
    IDC_SL_RHYTHM_GROUP_LISTEN,
    JZRhythmGeneratorWindow::OnSliderUpdate)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmGeneratorWindow::JZRhythmGeneratorWindow(
  JZEventWindow* pEventWindow,
  JZSong* pSong,
  wxFrame* pParent,
  const wxPoint& Position,
  const wxSize& Size)
  : wxPanel(pParent, wxID_ANY, Position, Size),
    mRhythm(0),
    mpEventWindow(pEventWindow),
    mpSong(pSong),
    mInstruments(),
    mpStepsPerCountSlider(nullptr),
    mpCountsPerBarSlider(nullptr),
    mpBarCountSlider(nullptr),
    mpInstrumentListBox(nullptr),
    mActiveInstrumentIndex(-1),
    mpGroupContribSlider(nullptr),
    mpGroupListenSlider(nullptr),
    mpGroupListBox(nullptr),
    mActiveGroup(0),
    mpRandomCheckBox(nullptr),
    mpLengthEdit(nullptr),
    mpVelocityEdit(nullptr),
    mpRhythmEdit(nullptr)
{
  int Width, Height;
  GetClientSize(&Width, &Height);

  wxPanel* pInstrumentPanel =
    new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(Width, Height / 2));

  mpStepsPerCountSlider = new wxSlider(
    pInstrumentPanel,
    IDC_SL_RHYTHM_STEPS_PER_COUNT,
    4,
    1,
    16,
    wxPoint(10, 1),
    wxSize(Width / 3, -1),
    wxSL_LABELS);

  mpCountsPerBarSlider = new wxSlider(
    pInstrumentPanel,
    IDC_SL_RHYTHM_COUNTS_PER_BAR,
    4,
    1,
    16,
    wxPoint(10, Height / 12),
    wxSize(Width / 3, -1),
    wxSL_LABELS);

  mpBarCountSlider = new wxSlider(
    pInstrumentPanel,
    IDC_SL_RHYTHM_BAR_COUNT,
    1,
    1,
    16,
    wxPoint(10, 2 * Height / 12),
    wxSize(Width / 3, -1),
    wxSL_LABELS);

  wxStaticText* pStaticText = new wxStaticText(
    pInstrumentPanel,
    wxID_ANY,
    "Instrument",
    wxPoint(10, 3 * Height / 12));

  mpInstrumentListBox = new wxListBox(
    pInstrumentPanel,
    IDC_LB_RHYTHM_INSTRUMENTS,
    wxPoint(10, 4 * Height / 12),
    wxSize(220, 80),
    wxArrayString(),
    wxLB_SINGLE);

  mpGroupContribSlider = new wxSlider(
    pInstrumentPanel,
    IDC_SL_RHYTHM_GROUP_CONTRIB,
    0,
    0,
    100,
    wxPoint(Width / 2 + 10, 1),
    wxSize(Width / 3, -1),
    wxSL_LABELS);

  mpGroupListenSlider = new wxSlider(
    pInstrumentPanel,
    IDC_SL_RHYTHM_GROUP_LISTEN,
    0,
    -100,
    100,
    wxPoint(Width / 2, Height / 12),
    wxSize(Width / 3, -1),
    wxSL_LABELS);

  mpRandomCheckBox  = new wxCheckBox(
    pInstrumentPanel,
    IDC_CB_RHYTHM_RANDOMIZE,
    "Randomize",
    wxPoint(Width / 2, 4 * Height / 12));

  wxPanel* pArrayControlPanel = new wxPanel(
    this,
    wxID_ANY,
    wxPoint(0, Height / 2),
    wxSize(Width, Height / 2));

  mpLengthEdit = new JZArrayControl(
    pArrayControlPanel,
    wxID_ANY,
    mRhythm.mLengthArray,
    wxPoint(0, 0),
    wxSize(Width / 2, Height / 4 - 4));
  mpLengthEdit->SetXMinMax(1, 8);
  mpLengthEdit->SetLabel("length/interval");

  mpVelocityEdit = new JZArrayControl(
    pArrayControlPanel,
    wxID_ANY,
    mRhythm.mVelocityArray,
    wxPoint(Width / 2, 0),
    wxSize(Width / 2, Height / 4 - 4));
  mpVelocityEdit->SetXMinMax(1, 127);
  mpVelocityEdit->SetLabel("velocity");

  mpRhythmEdit = new JZRhythmArrayControl(
    pArrayControlPanel,
    wxID_ANY,
    mRhythm.mRhythmArray,
    wxPoint(0, Height / 4),
    wxSize(Width, Height / 4 - 4));
  mpRhythmEdit->SetMeter(
    mRhythm.mStepsPerCount,
    mRhythm.mCountPerBar,
    mRhythm.mBarCount);
  mpRhythmEdit->SetLabel("rhythm");

  wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);

  pSizer->Add(pInstrumentPanel, wxSizerFlags().Border().Expand());
  pSizer->Add(pArrayControlPanel, wxSizerFlags(1).Border().Expand());

  SetSizer(pSizer);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmGeneratorWindow::~JZRhythmGeneratorWindow()
{
  ClearInstruments();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::AddInstrument()
{
  wxArrayString InstrumentNames;
  vector<int> Keys;

  InstrumentNames.Add("Controller");
  Keys.push_back(MODE_CONTROL);

#if 0
  if (gpHarmonyBrowser && gpHarmonyBrowser->SeqDefined())
  {
    InstrumentNames.Add("harmony: chords");
    Keys.push_back(CHORD_KEY);
    InstrumentNames.Add("harmony: bass");
    Keys.push_back(BASS_KEY);
  }
#endif

  InstrumentNames.Add("pianowin all");
  Keys.push_back(MODE_ALL_OF);
  InstrumentNames.Add("pianowin one");
  Keys.push_back(MODE_ONE_OF);

  for (const auto& StringIntPair : gpConfig->GetDrumNames())
  {
    if (!StringIntPair.first.empty())
    {
      Keys.push_back(StringIntPair.second - 1);
      InstrumentNames.Add(StringIntPair.first);
    }
  }

  int i = ::wxGetSingleChoiceIndex(
    "Instrument",
    "Select an instrument",
    InstrumentNames);

  if (i >= 0)
  {
    // Save actual values.
    Win2Instrument();

    JZRhythm* pRhythm = 0;
    if (mActiveInstrumentIndex >= 0)
    {
      pRhythm = new JZRhythm(*mInstruments[mActiveInstrumentIndex]);
    }
    else
    {
      pRhythm = new JZRhythm(Keys[i]);
    }

    // Is this a drum key?
    if (Keys[i] >= 0)
    {
      pRhythm->mKeyCount  = 1;
      pRhythm->mKeys[0] = Keys[i];
      pRhythm->mMode = MODE_ALL_OF;
      pRhythm->SetLabel(InstrumentNames[i]);
    }

    // Was a controller chosen?
    else if (Keys[i] == MODE_CONTROL)
    {
      pRhythm->mParameter = SelectControllerDlg();
      if (pRhythm->mParameter < 0)
      {
        return;
      }
      pRhythm->SetLabel(gpConfig->GetCtrlName(pRhythm->mParameter).first);
      pRhythm->mMode = MODE_CONTROL;
      pRhythm->mKeyCount = 0;
    }

    else if (Keys[i] == MODE_ONE_OF || Keys[i] == MODE_ALL_OF)
    {
      ostringstream Oss;
      if (Keys[i] == MODE_ONE_OF)
      {
        Oss << "one: ";
      }
      else
      {
        Oss << "all: ";
      }
      pRhythm->mKeyCount = 0;
      pRhythm->mMode = Keys[i];
      if (gpTrackFrame->GetPianoWindow())
      {
        JZEventArray Events;
        JZCommandCopyToBuffer cmd(gpTrackFrame->GetPianoWindow()->GetFilter(), &Events);
        cmd.Execute(0);   // no UNDO

        for (int ii = 0; ii < Events.mEventCount; ii++)
        {
          JZKeyOnEvent* pKeyOn = Events.mppEvents[ii]->IsKeyOn();
          if (pKeyOn)
          {
            pRhythm->mKeys[pRhythm->mKeyCount++] = pKeyOn->GetKey();
            if (pRhythm->mKeyCount > 1)
            {
              Oss << ", ";
            }
            string KeyString;
            KeyToString(pKeyOn->GetKey(), KeyString);
            Oss << KeyString;
            if (pRhythm->mKeyCount >= MAX_KEYS)
            {
              break;
            }
          }
        }
      }
      pRhythm->SetLabel(Oss.str());

      if (pRhythm->mKeyCount == 0)
      {
        wxMessageBox("select some notes in pianowin first", "Error", wxOK);
        delete pRhythm;
        pRhythm = 0;
      }
    }

    if (pRhythm != 0)
    {
      AddInstrument(pRhythm);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::Read(istream& Is)
{
  int Version;
  Is >> Version;
  if (Version > 2)
  {
    wxMessageBox("Wrong file format!", "Error", wxOK);
  }

  ClearInstruments();

  size_t InstrumentCount;
  Is >> InstrumentCount;
  for (size_t i = 0; i < InstrumentCount; ++i)
  {
    JZRhythm* pRhythm = new JZRhythm(0);
    pRhythm->Read(Is, Version);
    AddInstrument(pRhythm);
  }

  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::Write(ostream& Os)
{
  Win2Instrument();

  Os << 2 << endl;
  Os << mInstruments.size() << endl;
  for (const auto& pRhythm : mInstruments)
  {
    pRhythm->Write(Os);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::ClearInstruments()
{
  for (auto& pRhythm : mInstruments)
  {
    delete pRhythm;
  }
  mInstruments.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::AddInstrument(JZRhythm* pRhythm)
{
  mInstruments.push_back(pRhythm);
  mActiveInstrumentIndex = mInstruments.size() - 1;

  mpInstrumentListBox->Append(pRhythm->GetLabel());

  mpInstrumentListBox->SetSelection(mActiveInstrumentIndex);

  Instrument2Win();

  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::DeleteInstrument()
{
  if (mActiveInstrumentIndex >= 0)
  {
    vector<JZRhythm*> InstrumentsCopy(mInstruments);

    int i = mActiveInstrumentIndex;
    delete InstrumentsCopy[i];

    size_t k;
    for (k = i; k < InstrumentsCopy.size() - 1; ++k)
    {
      InstrumentsCopy[k] = InstrumentsCopy[k + 1];
    }

    mInstruments.clear();
    for (k = 0; k < InstrumentsCopy.size() - 1; ++k)
    {
      mInstruments.push_back(InstrumentsCopy[k]);
    }

    mpInstrumentListBox->Delete(i);
    mActiveInstrumentIndex = mpInstrumentListBox->GetSelection();
    Instrument2Win();
    Refresh();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::Generate()
{
  wxBeginBusyCursor();
  Win2Instrument();
  GenerateRhythm();
  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::Instrument2Win()
{
  if (
    mActiveInstrumentIndex < 0 ||
    mActiveInstrumentIndex >= mInstruments.size())
  {
    return;
  }

  mRhythm = *mInstruments[mActiveInstrumentIndex];
  mpStepsPerCountSlider->SetValue(mRhythm.mStepsPerCount);
  mpCountsPerBarSlider->SetValue(mRhythm.mCountPerBar);
  mpBarCountSlider->SetValue(mRhythm.mBarCount);
  mpRhythmEdit->SetMeter(
    mRhythm.mStepsPerCount,
    mRhythm.mCountPerBar,
    mRhythm.mBarCount);
  mpRandomCheckBox->SetValue(mRhythm.mRandomizeFlag);

  switch (mRhythm.mMode)
  {
    case MODE_CONTROL:
      mpVelocityEdit->SetLabel("ctrl value");
      break;
    default:
      mpVelocityEdit->SetLabel("velocity");
      break;
  }

  if (mActiveGroup >= 0)
  {
    mpGroupListenSlider->SetValue(
      mRhythm.mRhythmGroups[mActiveGroup].mListen);
    mpGroupContribSlider->SetValue(
      mRhythm.mRhythmGroups[mActiveGroup].mContrib);
  }

  RandomEnable();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::Win2Instrument()
{
  if (
    mActiveInstrumentIndex < 0 ||
    mActiveInstrumentIndex >= mInstruments.size())
  {
    return;
  }

  mRhythm.mStepsPerCount = mpStepsPerCountSlider->GetValue();
  mRhythm.mCountPerBar   = mpCountsPerBarSlider->GetValue();
  mRhythm.mBarCount      = mpBarCountSlider->GetValue();
  mpRhythmEdit->SetMeter(
    mRhythm.mStepsPerCount,
    mRhythm.mCountPerBar,
    mRhythm.mBarCount);
  mRhythm.mRandomizeFlag = mpRandomCheckBox->GetValue();

  if (mActiveGroup >= 0)
  {
    mRhythm.mRhythmGroups[mActiveGroup].mListen =
      mpGroupListenSlider->GetValue();
    mRhythm.mRhythmGroups[mActiveGroup].mContrib =
      mpGroupContribSlider->GetValue();
  }

  *mInstruments[mActiveInstrumentIndex] = mRhythm;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::RandomEnable()
{
  mpLengthEdit->Enable(mRhythm.mRandomizeFlag);
  mpVelocityEdit->Enable(mRhythm.mRandomizeFlag);
  mpGroupListenSlider->Enable(mRhythm.mRandomizeFlag);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::GenerateRhythm()
{
  if (
    !mpEventWindow->EventsSelected(
      "Please mark the destination track in the track window"))
  {
    return;
  }

  JZFilter* pFilter = mpEventWindow->mpFilter;

  if (pFilter->GetFromTrack() != pFilter->GetToTrack())
  {
    wxMessageBox("you must select exacty 1 track", "Error", wxOK);
    return;
  }

  int FromClock = pFilter->GetFromClock();
  int ToClock = pFilter->GetToClock();
  JZTrack* pTrack = mpSong->GetTrack(pFilter->GetFromTrack());
  mpSong->NewUndoBuffer();

  // Remove selection.
//  if (
//    wxMessageBox(
//      "Erase destination before generating?",
//      "Replace",
//      wxYES_NO) == wxYES)
  {
    JZCommandErase Erase(pFilter, 1);
    Erase.Execute(0);
  }

  for (auto& pRhythm : mInstruments)
  {
    pRhythm->GenInit(FromClock);
  }

  JZBarInfo BarInfo(*mpSong);
  BarInfo.SetClock(FromClock);

//  for (int i = 0; i < mInstrumentCount; ++i)
//  {
//    mpInstruments[i]->Generate(
//      pTrack,
//      FromClock,
//      ToClock,
//      BarInfo.GetTicksPerBar());
//  }

  while (BarInfo.GetClock() < ToClock)
  {
    for (auto& pRhythm : mInstruments)
    {
      pRhythm->Generate(pTrack, BarInfo, mInstruments);
    }
    BarInfo.Next();
  }

  pTrack->Cleanup();

  mpEventWindow->Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::OnSliderUpdate(wxCommandEvent&)
{
  Win2Instrument();
  RandomEnable();
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorWindow::OnListBox(wxCommandEvent&)
{
  Win2Instrument();
  mActiveInstrumentIndex = mpInstrumentListBox->GetSelection();
  Instrument2Win();
  Refresh();
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZRhythmGeneratorFrame, wxFrame)

  EVT_MENU(wxID_OPEN, JZRhythmGeneratorFrame::OnOpen)

  EVT_MENU(wxID_SAVE, JZRhythmGeneratorFrame::OnSave)

  EVT_MENU(ID_INSTRUMENT_ADD, JZRhythmGeneratorFrame::OnAddInstrument)

  EVT_MENU(ID_INSTRUMENT_DELETE, JZRhythmGeneratorFrame::OnDeleteInstrument)

  EVT_MENU(ID_INSTRUMENT_GENERATE, JZRhythmGeneratorFrame::OnGenerate)

  EVT_MENU(wxID_HELP, JZRhythmGeneratorFrame::OnHelp)

  EVT_MENU(wxID_HELP_CONTENTS, JZRhythmGeneratorFrame::OnHelpContents)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const wxString JZRhythmGeneratorFrame::mDefaultFileName = "noname.rhy";

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmGeneratorFrame::JZRhythmGeneratorFrame(
  JZEventWindow* pEventWindow,
  JZSong* pSong)
  : wxFrame(
      0,
      wxID_ANY,
      "Rhythm Generator",
      wxPoint(
        gpConfig->GetValue(C_RhythmXpos),
        gpConfig->GetValue(C_RhythmYpos)),
      wxSize(640, 580)),
    mpToolBar(nullptr),
    mpRhythmGeneratorWindow(nullptr)
{
  CreateToolBar();

  wxMenu* pFileMenu = new wxMenu;
  pFileMenu->Append(wxID_OPEN, "&Load...");
  pFileMenu->Append(wxID_SAVEAS, "Save &As...");
  pFileMenu->Append(wxID_CLOSE, "&Close");

  wxMenu* pInstrumentMenu = new wxMenu;
  pInstrumentMenu->Append(ID_INSTRUMENT_ADD, "&Add");
  pInstrumentMenu->Append(ID_INSTRUMENT_DELETE, "&Delete");
  pInstrumentMenu->Append(ID_INSTRUMENT_UP, "&Up");
  pInstrumentMenu->Append(ID_INSTRUMENT_DOWN, "&Down");
  pInstrumentMenu->Append(ID_INSTRUMENT_GENERATE, "&Generate");

  wxMenu* mpHelpMenu = new wxMenu;
  mpHelpMenu->Append(wxID_HELP_CONTENTS, "&Contents");
  mpHelpMenu->Append(wxID_HELP, "&Help");

  wxMenuBar* pMenuBar = new wxMenuBar;
  pMenuBar->Append(pFileMenu, "&File");
  pMenuBar->Append(pInstrumentMenu, "&Instrument");
  pMenuBar->Append(mpHelpMenu, "&Help");

  SetMenuBar(pMenuBar);

  int Width, Height;
  GetClientSize(&Width, &Height);
  mpRhythmGeneratorWindow = new JZRhythmGeneratorWindow(
    pEventWindow,
    pSong,
    this,
    wxPoint(0, 0),
    wxSize(Width, Height));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::CreateToolBar()
{
  JZToolDef ToolBarDefinitions[] =
  {
    { wxID_OPEN, false, open_xpm, "open rhythm file" },
    { wxID_SAVE, false, save_xpm, "save into rhythm file" },
    { JZToolBar::eToolBarSeparator },
    { ID_INSTRUMENT_ADD, false, rrgadd_xpm, "add instrument" },
    { ID_INSTRUMENT_DELETE, false, rrgdel_xpm, "remove instrument" },
    { ID_INSTRUMENT_UP, false, rrgup_xpm, "move instrument up" },
    { ID_INSTRUMENT_DOWN, false, rrgdown_xpm, "move instrument down" },
    { ID_INSTRUMENT_GENERATE, false, rrggen_xpm, "generate events into trackwin selection" },
    { JZToolBar::eToolBarSeparator },
    { wxID_HELP_CONTENTS, false, help_xpm, "help" },
    { JZToolBar::eToolBarEnd }
  };

  mpToolBar = new JZToolBar(this, ToolBarDefinitions);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZRhythmGeneratorFrame::~JZRhythmGeneratorFrame()
{
  int XPixel, YPixel;
  GetPosition(&XPixel, &YPixel);
  gpConfig->Put(C_RhythmXpos, XPixel);
  gpConfig->Put(C_RhythmYpos, YPixel);

  delete mpRhythmGeneratorWindow;

  gpRhythmGeneratorFrame = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::OnOpen(wxCommandEvent&)
{
  bool HasChanged = false;
  wxString FileName = file_selector(
    mDefaultFileName,
    "Load Rhythm",
    false,
    HasChanged,
    "*.rhy");
  if (!FileName.empty())
  {
    ifstream Is(FileName.mb_str());
    if (Is)
    {
      mpRhythmGeneratorWindow->Read(Is);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::OnSave(wxCommandEvent&)
{
  bool HasChanged = false;
  wxString FileName = file_selector(
    mDefaultFileName,
    "Save Rhythm",
    true,
    HasChanged,
    "*.rhy");
  if (!FileName.empty())
  {
    ofstream Os(FileName.mb_str());
    if (Os)
    {
      mpRhythmGeneratorWindow->Write(Os);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::OnAddInstrument(wxCommandEvent&)
{
  mpRhythmGeneratorWindow->AddInstrument();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::OnDeleteInstrument(wxCommandEvent&)
{
  mpRhythmGeneratorWindow->DeleteInstrument();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::OnGenerate(wxCommandEvent&)
{
  mpRhythmGeneratorWindow->Generate();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::OnHelp(wxCommandEvent&)
{
  JZHelp::Instance().ShowTopic("Random rhythm generator");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZRhythmGeneratorFrame::OnHelpContents(wxCommandEvent&)
{
  JZHelp::Instance().DisplayHelpContents();
}

//*****************************************************************************
//*****************************************************************************
void CreateRhythmGenerator(JZEventWindow* pEventWindow, JZSong* pSong)
{
  if (!gpRhythmGeneratorFrame)
  {
    gpRhythmGeneratorFrame = new JZRhythmGeneratorFrame(pEventWindow, pSong);
  }
  gpRhythmGeneratorFrame->Show(true);
}
