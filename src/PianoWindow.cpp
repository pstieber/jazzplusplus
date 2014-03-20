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

#include "PianoWindow.h"

#include "Command.h"
#include "ControlEdit.h"
#include "Dialogs.h"
#include "Dialogs/SnapDialog.h"
#include "Filter.h"
#include "GuitarFrame.h"
#include "Harmony.h"
#include "HarmonyBrowserAnalyzer.h"
#include "HarmonyP.h"
#include "Help.h"
#include "PianoFrame.h"
#include "Player.h"
#include "ProjectManager.h"
#include "Resources.h"
#include "SelectControllerDialog.h"
#include "Song.h"
#include "Synth.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/msgdlg.h>

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZListen* JZListen::mpInstance = 0;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZListen* JZListen::Instance()
{
  if (!mpInstance)
  {
    mpInstance = new JZListen;
  }
  return mpInstance;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZListen::Destroy()
{
  delete mpInstance;
  mpInstance = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZListen::JZListen()
  : wxTimer(),
    mActive(false),
    mPitch(-1),
    mChannel(-1),
    mpTrack(0)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZListen::KeyOn(
  JZTrack* pTrack,
  int Pitch,
  int Channel,
  int Velocity,
  int MilliSeconds)
{
  if (!mActive)
  {
    mPitch = Pitch;
    mChannel = Channel;
    JZKeyOnEvent KeyOn(0, mChannel, Pitch, Velocity);
    gpMidiPlayer->OutNow(pTrack, &KeyOn);
    mActive = true;
    Start(MilliSeconds);
    mpTrack = pTrack;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZListen::Notify()
{
  Stop();
  JZKeyOffEvent KeyOff(0, mChannel, mPitch);
  gpMidiPlayer->OutNow(mpTrack, &KeyOff);
  mActive = false;
}

//*****************************************************************************
// Description:
//   JZMousePlay - Click in pianoroll
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class JZMousePlay : public JZMouseAction
{
  public:

    JZMousePlay(JZPianoWindow* pPianoWindow);

    virtual int ProcessMouseEvent(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

  private:

    int mPitch, mVeloc, mChannel;
    JZPianoWindow* mpPianoWindow;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMousePlay::JZMousePlay(JZPianoWindow* pPianoWindow)
  : mPitch(0),
    mVeloc(-1),
    mChannel(-1),
    mpPianoWindow(pPianoWindow)
{
  mChannel = mpPianoWindow->GetTrack()->mChannel ?
    mpPianoWindow->GetTrack()->mChannel - 1 : 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZMousePlay::ProcessMouseEvent(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  int x, y;
  MouseEvent.GetPosition(&x, &y);

  int OldPitch = mPitch;

  if (MouseEvent.LeftDown())
  {
    mPitch = mpPianoWindow->y2Pitch(y);
    mVeloc = 64;
  }
  else if (MouseEvent.MiddleDown())
  {
    mPitch = mpPianoWindow->y2Pitch(y);
    mVeloc = 80;
  }
  else if (MouseEvent.RightDown())
  {
    mPitch = mpPianoWindow->y2Pitch(y);
    mVeloc = 110;
  }
  else if (MouseEvent.ButtonUp())
  {
    mPitch = 0;
  }
  else if (MouseEvent.Dragging())
  {
    mPitch = mpPianoWindow->y2Pitch(y);
  }
  else
  {
    return 0;
  }

  if (mpPianoWindow->GetTrack()->GetAudioMode())
  {
    if (mPitch && mPitch != OldPitch)
    {
      gpMidiPlayer->ListenAudio(mPitch, 0);
    }
  }
  else
  {
    if (OldPitch && OldPitch != mPitch)
    {
      JZKeyOffEvent KeyOff(0, mChannel, OldPitch);
      gpMidiPlayer->OutNow(mpPianoWindow->GetTrack(), &KeyOff);
      OldPitch = 0;
    }

    if (mPitch && mPitch != OldPitch)
    {
      JZKeyOnEvent KeyOn(0, mChannel, mPitch, mVeloc);
      gpMidiPlayer->OutNow(mpPianoWindow->GetTrack(), &KeyOn);
      OldPitch = 0;
    }
  }

  if (!mPitch)
  {
    // Done.
    return 1;
  }
  return 0;
}

//*****************************************************************************
// Description:
//   JZKeyLengthDragger
//*****************************************************************************
class JZKeyLengthDragger : public JZMouseAction
{
  public:

    JZKeyLengthDragger(JZKeyOnEvent* pKeyOn, JZPianoWindow* pPianoWindow);

    int Dragging(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    int ProcessMouseEvent(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    int ButtonUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

  private:

    JZKeyOnEvent* mpKeyOn;
    JZKeyOnEvent    *Copy;
    JZPianoWindow* Win;
    JZTrack* mpTrack;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKeyLengthDragger::JZKeyLengthDragger(
  JZKeyOnEvent* pKeyOn,
  JZPianoWindow* pPianoWindow)
{
  mpKeyOn = pKeyOn;
  Copy  = pKeyOn->Copy()->IsKeyOn();
  Win   = pPianoWindow;

  // BUG FIX: undo/redo
  Win->GetProject()->NewUndoBuffer();

  wxClientDC Dc(Win);

  // to translate scrolled coordinates
  Win->PrepareDC(Dc);

  Win->DrawEvent(Dc, Copy, wxWHITE_BRUSH, 0);
  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 1, 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZKeyLengthDragger::ProcessMouseEvent(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (MouseEvent.Dragging())
  {
    return Dragging(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.ButtonUp())
  {
    return ButtonUp(MouseEvent, ScrolledX, ScrolledY);
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZKeyLengthDragger::Dragging(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  wxClientDC Dc(Win);
  Win->PrepareDC(Dc); //to translate scrolled coordinates
  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 1, 1);
  int fx, fy;
  MouseEvent.GetPosition(&fx, &fy);
  int Clock = Win->x2Clock(fx);
  int  Length = Clock - Copy->GetClock();
  if (Length <= 0)
  {
    Length = 1;
  }
  Copy->SetLength(Length);

  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 1, 1);
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZKeyLengthDragger::ButtonUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  // Key_Aftertouch
  if (Copy->GetEventLength() < mpKeyOn->GetEventLength())
  {
    int key, channel;
    JZEventIterator iter(Win->GetTrack());
    key = Copy->GetKey();
    channel = Copy->GetChannel();

    JZKeyPressureEvent* pKeyPressure;

    JZEvent* pEvent = iter.Range(
      Copy->GetClock() + Copy->GetEventLength(),
      Copy->GetClock() + mpKeyOn->GetEventLength());

    while (pEvent)
    {
      pKeyPressure = pEvent->IsKeyPressure();
      if (pKeyPressure)
      {
        if (
          pKeyPressure->GetKey() == key &&
          pKeyPressure->GetChannel() == channel)
        {
          Win->KillTrackEvent(pEvent);
        }
      }
      pEvent = iter.Next();
    }
  }
  //

  Win->ApplyToTrack(mpKeyOn, Copy);

  Win->mpMouseAction = 0;

  // Velocity or aftertouch editor update.
  Win->UpdateControl();

  // always repaint
  Win->Refresh();

  delete this;
  return 0;
}

//*****************************************************************************
// Description:
//   JZPlayTrackLengthDragger JAVE this is just copied from JZKeyLengthDragger,
// the need to be inherited somehow
//*****************************************************************************
class JZPlayTrackLengthDragger : public JZMouseAction
{
  public:

    JZPlayTrackLengthDragger(JZPlayTrackEvent* k, JZPianoWindow* pPianoWindow);

    int Dragging(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    int ProcessMouseEvent(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

    int ButtonUp(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

  private:
    JZPlayTrackEvent* mpKeyOn;
    JZPlayTrackEvent* Copy;
    JZPianoWindow* Win;
    JZTrack* mpTrack;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPlayTrackLengthDragger::JZPlayTrackLengthDragger(
  JZPlayTrackEvent* k,
  JZPianoWindow* pPianoWindow)
{
  mpKeyOn = k;
  Copy  = k->Copy()->IsPlayTrack();
  Win   = pPianoWindow;

  // BUG FIX: undo/redo
  Win->GetProject()->NewUndoBuffer();
  //
  wxClientDC Dc(Win);
  Win->PrepareDC(Dc);
  Win->DrawEvent(Dc, Copy, wxWHITE_BRUSH, 0);
  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 1, 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPlayTrackLengthDragger::ProcessMouseEvent(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (MouseEvent.Dragging())
  {
    return Dragging(MouseEvent, ScrolledX, ScrolledY);
  }
  else if (MouseEvent.ButtonUp())
  {
    return ButtonUp(MouseEvent, ScrolledX, ScrolledY);
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPlayTrackLengthDragger::Dragging(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  wxClientDC Dc(Win);
  Win->PrepareDC(Dc);
  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 1, 1);
  int fx, fy;
  MouseEvent.GetPosition(&fx, &fy);
  int Clock = Win->x2Clock(fx);
  int  Length = Clock - Copy->GetClock();
  if (Length <= 0)
    Length = 1;
  Copy->eventlength = Length;

  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 1, 1);
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPlayTrackLengthDragger::ButtonUp(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  wxClientDC Dc(Win);
  Win->PrepareDC(Dc);
  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 1, 1);
  Win->DrawEvent(Dc, Copy, Copy->GetBrush(), 0, 1);

  Win->ApplyToTrack(mpKeyOn, Copy);

  Win->mpMouseAction = 0;

  // Velocity or aftertouch editor update.
  Win->UpdateControl();

  // always repaint
  Win->Refresh();

  delete this;
  return 0;
}

//*****************************************************************************
//*****************************************************************************
class JZVelocityCounter : public JZMouseCounter
{
  public:
    JZVelocityCounter(
      JZPianoWindow* pPianoWindow,
      JZRectangle* pRectangle,
      JZKeyOnEvent* pKeyOn)
      : JZMouseCounter(pPianoWindow, pRectangle, pKeyOn->GetVelocity(), 1, 127)
    {
      Win = pPianoWindow;
      mpKeyOn = pKeyOn;

      // BUG FIX: undo/redo
      Win->GetProject()->NewUndoBuffer();
      //
      wxClientDC Dc(Win);
      Dc.SetFont(*(Win->GetFixedFont()));
    }

    virtual int ProcessMouseEvent(
      wxMouseEvent& MouseEvent,
      int ScrolledX,
      int ScrolledY);

  private:

    JZPianoWindow* Win;

    JZKeyOnEvent* mpKeyOn;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZVelocityCounter::ProcessMouseEvent(
  wxMouseEvent& MouseEvent,
  int ScrolledX,
  int ScrolledY)
{
  if (JZMouseCounter::ProcessMouseEvent(MouseEvent, ScrolledX, ScrolledY))
  {
    JZKeyOnEvent* pKeyOnCopy = (JZKeyOnEvent *)mpKeyOn->Copy();
    pKeyOnCopy->SetVelocity(Value);

    Win->ApplyToTrack(mpKeyOn, pKeyOnCopy);

    wxClientDC Dc(Win);
    Win->PrepareDC(Dc);
    Win->DrawEvent(Dc, pKeyOnCopy, pKeyOnCopy->GetBrush(), 0, 1);

    Win->UpdateControl();

    Win->mpMouseAction = 0;

    Dc.SetFont(*(Win->GetFont()));
    delete this;
  }
  return 0;
}











//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static int mPianoFontSizes[] =
{
  6,  // Tiny
  7,  // Small
  8,  // Medium
  10, // Large
  12, // Huge
  -1, // End of list
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const int isBlack[12] =
{
  0,
  1,
  0,
  1,
  0,
  0,
  1,
  0,
  1,
  0,
  1,
  0
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define IsBlack(Key) isBlack[(Key) % 12]

//-----------------------------------------------------------------------------
// Mouse Actions Mapping
//-----------------------------------------------------------------------------
enum
{
  MA_PLAY = 1, // 0 represents no action.
  MA_CYCLE,
  MA_SELECT,
  MA_CONTSEL,
  MA_CUTPASTE,
  MA_LENGTH,
  MA_DIALOG,
  MA_LISTEN,
  MA_COPY,
  MA_VELOCITY
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const int PlayAreaActions[12] =
{
  // Depressed mouse button:
  // left        middle           right
  MA_PLAY,       MA_CYCLE,        0,            // plain
  MA_CYCLE,      0,               0,            // shift
  0,             0,               0,            // ctrl
  0,             0,               0             // shift + ctrl
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const int EventAreaActions[12] =
{
  // Depressed mouse button:
  // left        middle          right
  MA_SELECT,     MA_CUTPASTE,    MA_LENGTH,      // plain
  MA_CONTSEL,    MA_COPY,        MA_LISTEN,      // shift
  MA_VELOCITY,   MA_DIALOG,      MA_VELOCITY,    // ctrl
  MA_CUTPASTE,   0,              MA_COPY         // shift + ctrl
};


//*****************************************************************************
// Description:
//   This is the piano window definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(JZPianoWindow, JZEventWindow)

  EVT_SIZE(JZPianoWindow::OnSize)

  EVT_ERASE_BACKGROUND(JZPianoWindow::OnEraseBackground)

  EVT_PAINT(JZPianoWindow::OnPaint)

  EVT_CHAR(JZPianoWindow::OnChar)

  EVT_MOUSE_EVENTS(JZPianoWindow::OnMouseEvent)

  EVT_SCROLLWIN(JZPianoWindow::OnScroll)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPianoWindow::JZPianoWindow(
  JZPianoFrame* pPianoFrame,
  JZProject* pProject,
  const wxPoint& Position,
  const wxSize& Size)
  : JZEventWindow(pPianoFrame, pProject, Position, Size),
    mpPianoFrame(pPianoFrame),
    mPlayClock(-1),
    mPasteBuffer(),
    mpTrack(0),
    mTrackIndex(0),
    mpCtrlEdit(0),
    mMousePlay(PlayAreaActions),
    mMouseEvent(EventAreaActions),
    mUseColors(true),
    mMouseLine(-1),
    mFontSize(12),
    mpFont(0),
    mpFixedFont(0),
    mFixedFontHeight(0),
    mpDrumFont(0),
    mSnapDenomiator(16),
    mVisibleKeyOn(true),
    mVisiblePitch(true),
    mVisibleController(true),
    mVisibleProgram(true),
    mVisibleTempo(true),
    mVisibleSysex(true),
    mVisiblePlayTrack(true),
    mVisibleDrumNames(true),
    mVisibleAllTracks(true),
    mVisibleHBChord(true),
    mVisibleMono(true),
    mDrawing(false),
    mpFrameBuffer(0)
{
  // This is more appropriate than the value in the event window constructor.
  mClockTicsPerPixel = 4;

  InitColors();

  mpTrack = mpProject->GetTrack(mTrackIndex);

  // Must be an entry in the array.
  mFontSize = mPianoFontSizes[1];

  for (int i = 0; i < eMaxTrackCount; ++i)
  {
    mFromLines[i] = 64;
  }

  mMouseEvent.SetLeftAction(MA_SELECT);

  mpFrameBuffer = new wxBitmap;

  Setup();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPianoWindow::~JZPianoWindow()
{
  delete mpCtrlEdit;
  delete mpFont;
  delete mpFixedFont;
  delete mpDrumFont;
  delete mpFrameBuffer;
  JZListen::Destroy();
}

//-----------------------------------------------------------------------------
// Description:
//   Generate some colors to represent note velocity.  The current settings use
// dark blue for quiet and bright red for loud.
//-----------------------------------------------------------------------------
void JZPianoWindow::InitColors()
{
  int c;
  for (int i = 0; i < NUM_COLORS; ++i)
  {
    c = 256 * i / NUM_COLORS;
    mpColorBrush[i].SetColour(c, 0, 127 - c / 2);
    mpColorBrush[i].SetStyle(wxSOLID);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Setup()
{
  wxClientDC Dc(this);

  Dc.SetFont(wxNullFont);

  delete mpFixedFont;
  mpFixedFont = new wxFont(12, wxSWISS, wxNORMAL, wxNORMAL);
  Dc.SetFont(*mpFixedFont);

  int Width, Height;
  Dc.GetTextExtent("M", &Width, &mFixedFontHeight);

  delete mpFont;
  mpFont = new wxFont(mFontSize, wxSWISS, wxNORMAL, wxNORMAL);
  Dc.SetFont(*mpFont);

  Dc.GetTextExtent("M", &Width, &Height);
  mLittleBit = Width / 2;

  mTopInfoHeight = mFixedFontHeight + 2 * mLittleBit;

  Dc.GetTextExtent("HXWjgi", &Width, &Height);
  mTrackHeight = Height + 2 * mLittleBit;

  delete mpDrumFont;
  mpDrumFont = new wxFont(mFontSize + 3, wxSWISS, wxNORMAL, wxNORMAL);

  Dc.SetFont(*mpDrumFont);

  Dc.GetTextExtent("Low Conga mid 2 or so", &Width, &Height);
  mPianoWidth = Width + mLittleBit;

  mLeftInfoWidth = mPianoWidth;

  SetScrollRanges();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::OnDraw(wxDC& Dc)
{
  Draw(Dc);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Draw(wxDC& Dc)
{
  if (!mpFrameBuffer->Ok() || mDrawing)
  {
    return;
  }

  mDrawing = true;

  // Create a memory device context and select the frame bitmap into it.
  wxMemoryDC LocalDc;
  LocalDc.SelectObject(*mpFrameBuffer);

  LocalDc.SetFont(*mpFont);

  // Setup the brush that is used to clear the background.
  LocalDc.SetBackground(*wxWHITE_BRUSH);

  // Clear the background using the brush that was just setup,
  // in case the following drawing calls fail.
  LocalDc.Clear();

//  int OldFromClock = mFromClock;

  GetClientSize(&mCanvasWidth, &mCanvasHeight);

  mEventsX = mLeftInfoWidth;
  mEventsY = mTopInfoHeight;

  mEventsWidth = mCanvasWidth - mLeftInfoWidth;
  mEventsHeight = mCanvasHeight - mTopInfoHeight;

  mFromLine = mScrolledY / mTrackHeight;
  mToLine = 1 + (mScrolledY + mCanvasHeight - mTopInfoHeight) / mTrackHeight;

  mFromClock = mScrolledX * mClockTicsPerPixel;
  mToClock = x2Clock(mCanvasWidth);

  // Because jazz has a ReDo function.  Fixes simultaneously update a
  // small problem when multiple ZoomOut.  Active Ctrl draw new windows or
  // reinitialize.

//  if (mpCtrlEdit && OldFromClock != mFromClock)
//    mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);

  if (mpCtrlEdit)
  {
    mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  }

  mPianoX = 0;

  JZBarInfo BarInfo(*mpProject);

//DEBUG  cout
//DEBUG    << "mLeftInfoWidth:                " << mLeftInfoWidth << '\n'
//DEBUG    << "mCanvasWidth - mLeftInfoWidth: " << mCanvasWidth - mLeftInfoWidth << '\n'
//DEBUG    << "BarInfo.TicksPerBar            " << BarInfo.TicksPerBar << '\n'
//DEBUG    << "From Clock:                    " << mFromClock << '\n'
//DEBUG    << "To Clock:                      " << mToClock << '\n'
//DEBUG    << "Clocks/Pixel:                  " << mClockTicsPerPixel << '\n'
//DEBUG    << "From Measure:                  " << mFromClock / BarInfo.TicksPerBar << '\n'
//DEBUG    << "To Measure:                    " << mToClock / BarInfo.TicksPerBar << '\n'
//DEBUG    << "From X:                        " << Clock2x(mFromClock) << '\n'
//DEBUG    << "To X:                          " << Clock2x(mToClock) << '\n'
//DEBUG    << endl;

  ///////////////////////////////////////////////////////////////
  // horizontal lines(ripped from drawpianoroll code)

//  for (
//    y = TrackIndex2y(mFromLine);
//    y < mEventsY + mEventsHeight;
//    y += mTrackHeight)
//  {
//    // cheaper than clipping
//    if (y > mEventsY)
//    {
//      LocalDc.DrawLine(mEventsX+1, y, mEventsX + mEventsWidth, y);
//    }
//  }

  LocalDc.SetPen(*wxGREY_PEN);
  wxBrush blackKeysBrush = wxBrush(wxColor(250, 240, 240), wxSOLID);
  int Pitch = 127 - mFromLine;
  int y = TrackIndex2y(mFromLine);
  while (Pitch >= 0 && y < mEventsY + mEventsHeight)
  {
    if (IsBlack(Pitch))
    {
      LocalDc.SetBrush(blackKeysBrush);
      LocalDc.DrawRectangle(0, y, 2000, mTrackHeight);
    }
    else if ((Pitch % 12) == 0)
    {
      LocalDc.SetPen(*wxCYAN_PEN);
      LocalDc.DrawLine(0, y + mTrackHeight, 2000, y + mTrackHeight);
    }
    else if (!IsBlack(Pitch - 1))
    {
      LocalDc.SetPen(*wxGREEN_PEN);
      LocalDc.DrawLine(0, y + mTrackHeight, 2000, y + mTrackHeight);
    }

    y += mTrackHeight;
    --Pitch;
  }


  ///////////////////////////////////////////////////////////////

  mMouseLine = -1;

  LocalDc.SetPen(*wxBLACK_PEN);

  DrawVerticalLine(LocalDc, 0);
  DrawVerticalLine(LocalDc, mEventsX);
  DrawVerticalLine(LocalDc, mEventsX - 1);

  DrawHorizontalLine(LocalDc, mEventsY);
  DrawHorizontalLine(LocalDc, mEventsY - 1);
  DrawHorizontalLine(LocalDc, mEventsY + mEventsHeight);

  // draw vlines and bar numbers

  LocalDc.SetFont(*mpFixedFont);
  BarInfo.SetClock(mFromClock);
  int StopClk = x2Clock(mCanvasWidth);
  int clk = BarInfo.GetClock();
  int intro = mpProject->GetIntroLength();
  while (clk < StopClk)
  {
    clk = BarInfo.GetClock();
    int x = Clock2x(clk);
    // vertical lines and bar numbers
    int i;
    LocalDc.SetPen(*wxBLACK_PEN);
    ostringstream Oss;
    Oss << BarInfo.GetBarIndex() + 1 - intro;
    if (x > mEventsX)
    {
      LocalDc.DrawText(
        Oss.str().c_str(),
        x + mLittleBit,
        mEventsY - mFixedFontHeight - 2);
      LocalDc.SetPen(*wxGREY_PEN);
      LocalDc.DrawLine(
        x,
        mEventsY - mFixedFontHeight,
        x,
        mEventsY + mEventsHeight);
    }

    LocalDc.SetPen(*wxLIGHT_GREY_PEN);
    for (i = 0; i < BarInfo.GetCountsPerBar(); i++)
    {
      clk += BarInfo.GetTicksPerBar() / BarInfo.GetCountsPerBar();
      x = Clock2x(clk);
      if (x > mEventsX)
      {
        LocalDc.DrawLine(x, mEventsY + 1, x, mEventsY + mEventsHeight);
      }
    }
    BarInfo.Next();
  }

  LineText(LocalDc, 0, 0, mPianoWidth, mTopInfoHeight);

  LocalDc.SetPen(*wxBLACK_PEN);
  DrawPianoRoll(LocalDc);

  // Draw chords from harmony-browser.
  if (mVisibleHBChord && gpHarmonyBrowser && !mpTrack->IsDrumTrack())
  {
    JZHarmonyBrowserAnalyzer* pAnalyzer = gpHarmonyBrowser->GetAnalyzer();
    if (pAnalyzer != 0)
    {
      wxBrush cbrush = *wxBLUE_BRUSH;
      wxBrush sbrush = *wxBLUE_BRUSH;
#ifdef __WXMSW__
      cbrush.SetColour(191,191,255);
      sbrush.SetColour(191,255,191);
#else
      cbrush.SetColour(220,220,255);
      sbrush.SetColour(230,255,230);
#endif

//      LocalDc.SetClippingRegion(
//        mEventsX,
//        mEventsY,
//        mEventsWidth,
//        mEventsHeight);
      LocalDc.SetLogicalFunction(wxXOR);
      LocalDc.SetPen(*wxTRANSPARENT_PEN);

      int steps = pAnalyzer->Steps();
      for (int step = 0; step < steps; step ++)
      {
        int start = pAnalyzer->Step2Clock(step);
        int stop  = pAnalyzer->Step2Clock(step + 1);
        if (stop > mFromClock && start < mToClock)
        {
          // this chord is visible
          JZHarmonyBrowserContext *context = pAnalyzer->GetContext(step);
          JZHarmonyBrowserChord chord = context->Chord();
          JZHarmonyBrowserChord scale = context->Scale();

          int x = Clock2x(start);
          if (x < mEventsX)        // clip to left border
          {
            x = mEventsX;
          }
          int w = Clock2x(stop) - x;
          if (w <= 0)
          {
            continue;
          }

          int h = mTrackHeight;
          for (int i = 0; i < 12; i++)
          {
            int pitch = i;
            wxBrush *brush = 0;
            if (chord.Contains(i))
            {
              brush = &cbrush;
            }
            else if (scale.Contains(i))
            {
              brush = &sbrush;
            }
            if (brush)
            {
              LocalDc.SetBrush(*brush);
              while (pitch < 127)
              {
                int y = Pitch2y(pitch);

                // Perform y-clipping.
                if (y >= mEventsY && y <= mEventsY + mEventsHeight - h)
                {
                  LocalDc.DrawRectangle(x, y, w, h);
                }
                pitch += 12;
              }
            }
          }
        }
      }

//      LocalDc.DestroyClippingRegion();
      LocalDc.SetLogicalFunction(wxCOPY);
      LocalDc.SetPen(*wxBLACK_PEN);
      LocalDc.SetBrush(*wxBLACK_BRUSH);
    }
  }
  /////////end draw choords

  if (mVisibleAllTracks)
  {
    for (int i = 0; i < mpProject->GetTrackCount(); ++i)
    {
      JZTrack* pTrack = mpProject->GetTrack(i);
      if (pTrack != mpTrack && IsVisible(pTrack))
      {
        DrawEvents(LocalDc, pTrack, StatKeyOn, wxLIGHT_GREY_BRUSH, TRUE);
      }
    }
  }

  if (mVisibleKeyOn)
  {
    DrawEvents(LocalDc, mpTrack, StatKeyOn, wxRED_BRUSH, FALSE);
  }
  if (mVisiblePitch)
  {
    DrawEvents(LocalDc, mpTrack, StatPitch, wxBLUE_BRUSH, FALSE);
  }
  if (mVisibleController)
  {
    DrawEvents(LocalDc, mpTrack, StatControl, wxCYAN_BRUSH, FALSE);
  }
  if (mVisibleProgram)
  {
    DrawEvents(LocalDc, mpTrack, StatProgram, wxGREEN_BRUSH, FALSE);
  }
  if (mVisibleTempo)
  {
    DrawEvents(LocalDc, mpTrack, StatSetTempo, wxGREEN_BRUSH, FALSE);
  }
  if (mVisibleSysex)
  {
    DrawEvents(LocalDc, mpTrack, StatSysEx, wxGREEN_BRUSH, FALSE);
  }
  if (mVisiblePlayTrack)
  {
    DrawEvents(LocalDc, mpTrack, StatPlayTrack, wxLIGHT_GREY_BRUSH, FALSE);
  }

  DrawEvents(LocalDc, mpTrack, StatEndOfTrack, wxRED_BRUSH, FALSE);
  DrawEvents(LocalDc, mpTrack, StatText, wxBLACK_BRUSH, FALSE);

//  LocalDc.SetPen(*wxBLACK_PEN);
//  LocalDc.SetBrush(*wxBLACK_BRUSH);
//  LocalDc.SetBackground(*wxWHITE_BRUSH);        // xor-bug

  DrawPlayPosition(LocalDc);

  // Draw the selection box.
  mpSnapSel->Draw(
    LocalDc,
    mScrolledX,
    mScrolledY,
    mEventsX,
    mEventsY,
    mEventsWidth,
    mEventsHeight);

  Dc.Blit(
    0,
    0,
    mCanvasWidth,
    mCanvasHeight,
    &LocalDc,
    0,
    0,
    wxCOPY);

  LocalDc.SetFont(wxNullFont);
  LocalDc.SelectObject(wxNullBitmap);

  mDrawing = false;
}

//-----------------------------------------------------------------------------
// Decription:
//   Draw the "play position", by placing a vertical line where the
// "play clock" is.
//-----------------------------------------------------------------------------
void JZPianoWindow::DrawPlayPosition(wxDC& Dc)
{
  if (
    !mpSnapSel->IsActive() &&
    mPlayClock >= mFromClock &&
    mPlayClock < mToClock)
  {
    Dc.SetBrush(*wxBLACK_BRUSH);
    Dc.SetPen(*wxBLACK_PEN);

    int x = Clock2x(mPlayClock);

    // Draw a line, 2 pixels wide.
    Dc.DrawLine(x,     0, x,     mEventsY + mEventsHeight);
    Dc.DrawLine(x + 1, 0, x + 1, mEventsY + mEventsHeight);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::OnSize(wxSizeEvent& Event)
{
  GetClientSize(&mCanvasWidth, &mCanvasHeight);
  if (mCanvasWidth > 0 && mCanvasHeight > 0)
  {
    mpFrameBuffer->Create(mCanvasWidth, mCanvasHeight);
    SetScrollRanges();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::OnMenuCommand(int Id)
{
  switch (Id)
  {
    case ID_HELP_PIANO_WINDOW:
      JZHelp::Instance().ShowTopic("Piano Window");
      break;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::OnChar(wxKeyEvent& Event)
{
  if (Event.ControlDown())
  {
    switch (Event.GetKeyCode())
    {
      case 'Z':
        OnMenuCommand(wxID_UNDO);
        return;
      case 'Y':
        OnMenuCommand(wxID_REDO);
        return;
      case 'X':
        OnMenuCommand(wxID_CUT);
        return;
      case 'C':
      case WXK_INSERT:
        OnMenuCommand(wxID_COPY);
        return;
    }
  }
  else if (Event.ShiftDown())
  {
    switch (Event.GetKeyCode())
    {
      case WXK_UP:
        if (mTrackIndex > 0)
        {
          --mTrackIndex;
          NewPosition(mTrackIndex, -1);
        }
        return;
      case WXK_DOWN:
        if (mTrackIndex < mpProject->GetTrackCount() - 1)
        {
          ++mTrackIndex;
          NewPosition(mTrackIndex, -1);
        }
        return;
    }
  }
  else
  {
    switch (Event.GetKeyCode())
    {
      case WXK_DELETE:
        OnMenuCommand(wxID_DELETE);
        return;
    }
  }

  Event.Skip();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::NewPosition(int TrackIndex, int Clock)
{
  mFromLines[mTrackIndex] = mFromLine;

  // change track
  if (TrackIndex >= 0)
  {
    mTrackIndex = TrackIndex;
    mpTrack = mpProject->GetTrack(mTrackIndex);
    mpPianoFrame->SetTitle(mpTrack->GetName());

    SetYScrollPosition(TrackIndex2y(mFromLines[mTrackIndex]));
  }

  // change position
  if (Clock >= 0)
  {
    int x = Clock2x(Clock);
//OLD    SetScrollPosition(x, TrackIndex2y(mFromLines[mTrackIndex]));
    SetXScrollPosition(x);
  }

  // Is changed.  OnPaint always draws new -> Bug Fix for ZoomOut!
  // OnPaint() redraws only if clock has changed
//  if (mpCtrlEdit && TrackIndex >= 0)
//  {
//    mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
//  }

  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::SetScrollRanges()
{
  int EventWidth, EventHeight;
  GetVirtualEventSize(EventWidth, EventHeight);

  // Must add the thumb size to the passed range to reach the maximum
  // desired value.
  int ThumbSize;

  ThumbSize = EventWidth / 10;
  SetScrollbar(wxHORIZONTAL, mScrolledX, ThumbSize, EventWidth + ThumbSize);

  ThumbSize = EventHeight / 10;
  SetScrollbar(wxVERTICAL, mScrolledY, ThumbSize, EventHeight + ThumbSize);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::TrackIndex2y(int TrackIndex)
{
  return TrackIndex * mTrackHeight + mTopInfoHeight - mScrolledY;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::y2TrackIndex(int y)
{
  return (y + mScrolledY - mTopInfoHeight) / mTrackHeight;
}

//=============================================================================
// Painting
//=============================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::DrawPianoRoll(wxDC& Dc)
{
  // Draw the grey background for the keyboard.
  Dc.SetBrush(*wxLIGHT_GREY_BRUSH);
  Dc.DrawRectangle(
    mPianoX,
    mEventsY,
    mPianoWidth,
    mEventsHeight);

  Dc.SetBrush(*wxBLACK_BRUSH);

//  Dc.SetTextBackground(*wxLIGHT_GREY);

  int wBlack = mPianoWidth * 2 / 3;
  int Pitch = 127 - mFromLine;
  int y = TrackIndex2y(mFromLine);

  if (
    mVisibleKeyOn &&
    !mpTrack->GetAudioMode() &&
    (!mpTrack->IsDrumTrack() || !mVisibleDrumNames))
  {
    Dc.SetFont(*mpFixedFont);

    while (Pitch >= 0 && y < mEventsY + mEventsHeight)
    {
      if (IsBlack(Pitch))
      {
        Dc.DrawRectangle(0, y, wBlack, mTrackHeight);
        Dc.DrawLine(
          wBlack,
          y + mTrackHeight / 2,
          mPianoWidth,
          y + mTrackHeight / 2);
        Dc.SetPen(*wxWHITE_PEN);
        Dc.DrawLine(
          wBlack + 1,
          y + mTrackHeight / 2 + 1,
          mPianoWidth,
          y + mTrackHeight / 2 + 1);
        Dc.DrawLine(0, y, wBlack, y);
        Dc.SetPen(*wxBLACK_PEN);
      }
      else if ((Pitch % 12) == 0)
      {
        Dc.DrawLine(0, y + mTrackHeight, mPianoWidth, y + mTrackHeight);
        Dc.SetPen(*wxWHITE_PEN);
        Dc.DrawLine(
          0,
          y + mTrackHeight + 1,
          mPianoWidth,
          y + mTrackHeight + 1);
        Dc.SetPen(*wxBLACK_PEN);
        ostringstream Oss;
        Oss << Pitch / 12;
        Dc.DrawText(
          Oss.str().c_str(),
          wBlack + mLittleBit,
          y + mTrackHeight / 2);
      }
      else if (!IsBlack(Pitch - 1))
      {
        Dc.DrawLine(0, y + mTrackHeight, mPianoWidth, y + mTrackHeight);
        Dc.SetPen(*wxWHITE_PEN);
        Dc.DrawLine(
          0,
          y + mTrackHeight + 1,
          mPianoWidth,
          y + mTrackHeight + 1);
        Dc.SetPen(*wxBLACK_PEN);
      }

      y += mTrackHeight;
      --Pitch;
    }
  }
  else if (mpTrack->GetAudioMode())
  {
    Dc.SetFont(*mpDrumFont);
    while (Pitch >= 0 && y < mEventsY + mEventsHeight)
    {
      Dc.DrawText(gpMidiPlayer->GetSampleLabel(Pitch), mLittleBit, y);
      y += mTrackHeight;
      --Pitch;
    }
  }
  else
  {
    // Draw text?
    if (mVisibleKeyOn && mVisibleDrumNames)
    {
      Dc.SetFont(*mpDrumFont);
      while (Pitch >= 0 && y < mEventsY + mEventsHeight)
      {
        Dc.DrawText(
          gpConfig->GetDrumName(Pitch + 1).first.c_str(),
          mLittleBit,
          y);

        y += mTrackHeight;

        --Pitch;
      }
    }
    else if (mVisibleController)
    {
      Dc.SetFont(*mpDrumFont);
      while (Pitch >= 0 && y < mEventsY + mEventsHeight)
      {
        Dc.DrawText(
          gpConfig->GetCtrlName(Pitch + 1).first.c_str(),
          mLittleBit,
          y);

        y += mTrackHeight;
        --Pitch;
      }
    }
    else if (mVisibleProgram)
    {
      Dc.SetFont(*mpDrumFont);
      while (Pitch >= 0 && y < mEventsY + mEventsHeight)
      {
        Dc.DrawText(
          gpConfig->GetVoiceName(Pitch + 1).first.c_str(),
          mLittleBit,
          y);

        y += mTrackHeight;
        --Pitch;
      }
    }
    else if (mVisibleSysex)
    {
      Dc.SetFont(*mpDrumFont);
      while (Pitch >= 0 && y < mEventsY + mEventsHeight)
      {
        Dc.DrawText(
          JZSynthesizerSysex::GetSysexGroupName(Pitch + 1),
          mLittleBit,
          y);
        y += mTrackHeight;
        --Pitch;
      }
    }
    else if (mVisiblePitch)
    {
    }
  }

  //Dc.DestroyClippingRegion();
  //Dc.SetTextBackground(*wxWHITE);
  Dc.SetFont(*mpFont);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::DrawEvent(
  wxDC& Dc,
  JZEvent* pEvent,
  const wxBrush* Brush,
  int xoor,
  int force_color)
{
  if (pEvent->IsKeyPressure() || pEvent->IsChnPressure())
  {
    return;
  }

  int length = pEvent->GetLength() / mClockTicsPerPixel;
  // Always draw at least two pixels to avoid invisible (behind a
  // vertical line) or zero-length events:
  if (length < 3)
  {
    length = 3;
  }

  if (xoor)
  {
    Dc.SetLogicalFunction(wxXOR);
  }
  int x = Clock2x(pEvent->GetClock());
  int y = Pitch2y(pEvent->GetPitch());
  if (!xoor)
  {
    Dc.SetBrush(*wxWHITE_BRUSH);
    Dc.DrawRectangle(x, y + mLittleBit, length, mTrackHeight - 2 * mLittleBit);
  }

  // show velocity as colors
  if (force_color != 0 && mUseColors && pEvent->IsKeyOn())
  {
    int vel = pEvent->IsKeyOn()->GetVelocity();

    // Next line is "Patrick Approved."
    Dc.SetBrush(mpColorBrush[ vel * NUM_COLORS / 128 ]);
  }
  else
  {
    Dc.SetBrush(*Brush);
  }
  // end velocity colors

  Dc.DrawRectangle(x, y + mLittleBit, length, mTrackHeight - 2 * mLittleBit);

  if (xoor)
  {
    Dc.SetLogicalFunction(wxCOPY);
  }
  Dc.SetBrush(*wxBLACK_BRUSH);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::DrawEvents(
  wxDC& Dc,
  JZTrack* pTrack,
  int Stat,
  const wxBrush* Brush,
  int force_color)
{
//  Dc.SetClippingRegion(mEventsX, mEventsY, mEventsWidth, mEventsHeight);
  Dc.SetBrush(*Brush);

  JZEventIterator Iterator(pTrack);
  JZEvent* pEvent = Iterator.First();
  int FromPitch = 127 - mToLine;
  int ToPitch   = 127 - mFromLine;

  // Coordinate lines.

//  int x0 = Clock2x(0);
//  int y0 = TrackIndex2y(64);

  while (pEvent)
  {
    if (pEvent->GetStat() == Stat)
    {
      int Pitch   = pEvent->GetPitch();
      int Length = pEvent->GetLength();
      int Clock  = pEvent->GetClock();

      int x1 = Clock2x(Clock);
      int y1 = TrackIndex2y(127 - Pitch);

//      if (pEvent->IsPlayTrack())
//      {
//        // JAVE so the y position of playtrack events tell which
//        // track they play (the drawing should rather be polymorpic
//        // in my opinion)
//        y1 = TrackIndex2y(127 - pEvent->IsPlayTrack()->track);
//        // use pitch instead
//      }

      // Test to determine if the event is partially visible.
      if (
        Clock + Length >= mFromClock &&
        FromPitch < Pitch && Pitch <= ToPitch)
      {
        int DrawLength = Length / mClockTicsPerPixel;

        // Perform manual clipping.
        if (x1 < mEventsX)
        {
          DrawLength -= mEventsX - x1;
          x1 = mEventsX;
        }

        // Always draw at least two pixels to avoid invisible (behind a
        // vertical line) or zero-length events:
        if (DrawLength < 3)
        {
          DrawLength = 3;
        }

        // Show velocity as colors.
        if (!force_color && mUseColors && pEvent->IsKeyOn())
        {
          int vel = pEvent->IsKeyOn()->GetVelocity();
          Dc.SetBrush(mpColorBrush[ vel * NUM_COLORS / 128 ]);
        }
        else
        {
          Dc.SetBrush(*Brush);
        }

        Dc.DrawRectangle(
          x1,
          y1 + mLittleBit,
          DrawLength,
          mTrackHeight - 2 * mLittleBit);
        //shouldnt it be in drawevent? odd.

        if (pEvent->IsPlayTrack())
        {
          Dc.SetPen(*wxBLACK_PEN);
          ostringstream Oss;
          Oss << "Track:" << pEvent->IsPlayTrack()->track;
          Dc.DrawText(Oss.str().c_str(), x1, y1 + mLittleBit);
        }
      }

      if (Clock + Length >= mFromClock)
      {
        // These events are always visible in vertical.
        if (pEvent->IsEndOfTrack())
        {
          Dc.SetPen(*wxRED_PEN);

          // Draw a vertical bar.
          DrawVerticalLine(Dc, x1);

          Dc.SetPen(*wxBLACK_PEN);
          Dc.DrawText("EOT", x1, y1 + mLittleBit);
        }

        if (pEvent->IsText())
        {
          Dc.SetPen(*wxGREEN_PEN);

          // Draw a vertical bar.
          DrawVerticalLine(Dc, x1);

          Dc.SetPen(*wxBLACK_PEN);
          int textX;
          int textY;

          Dc.GetTextExtent(pEvent->IsText()->GetText(), &textX, &textY);
          Dc.SetBrush(*wxWHITE_BRUSH);

          // Draw text labels drawn at top.
          int textlabely = mTopInfoHeight;
          Dc.DrawRectangle(
            x1 - textX,
            textlabely + mLittleBit,
            textX,
            textY); //mTrackHeight - 2 * mLittleBit);
          Dc.DrawText(
            pEvent->IsText()->GetText(),
            x1 - textX,
            textlabely + mLittleBit);
        }
      }

//      x0 = x1;
//      y0 = y1;

      if (Clock > mToClock)
      {
        break;
      }
    }
    pEvent = Iterator.Next();
  }
  Dc.SetBrush(*wxBLACK_BRUSH);
//  Dc.DestroyClippingRegion();
}

//-----------------------------------------------------------------------------
// Description:
//   Draws the a 3D button with text in it.  Used to draw the little area in
// the top left of the window.
//-----------------------------------------------------------------------------
void JZPianoWindow::LineText(
  wxDC& Dc,
  int x,
  int y,
  int w,
  int h,
  wxString str,
  bool down)
{
  Dc.SetBrush(*wxLIGHT_GREY_BRUSH); // Fill
  Dc.SetPen(*wxLIGHT_GREY_PEN);     // Outline
  Dc.DrawRectangle(x, y, w, h);

  x += 1;
  y += 1;
  w -= 2;
  h -= 2;

  // Draw the top and left lines of the 3D button.
  if (down)
  {
    Dc.SetPen(*wxBLACK_PEN);
  }
  else
  {
    Dc.SetPen(*wxWHITE_PEN);
  }

  Dc.DrawLine(x, y, x+w, y);
  Dc.DrawLine(x, y, x, y+h);

  // Draw the bottom and right lines of the 3D button.
  if (down)
  {
    Dc.SetPen(*wxWHITE_PEN);
  }
  else
  {
    Dc.SetPen(*wxBLACK_PEN);
  }

  Dc.DrawLine(x+w, y, x+w, y+h);
  Dc.DrawLine(x, y+h, x+w, y+h);

  // Print the message in the button.
  Dc.DrawText(str, x + mLittleBit, y + mLittleBit);
}

//-----------------------------------------------------------------------------
// Description:
//   Do nothing, to avoid flickering.
//-----------------------------------------------------------------------------
void JZPianoWindow::OnEraseBackground(wxEraseEvent& Event)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::OnPaint(wxPaintEvent& Event)
{
  // One must always create a wxPaintDC object, even if it is not used.
  // Otherwise, under MS Windows, refreshing for this and other windows will
  // fail.
  wxPaintDC Dc(this);
  PrepareDC(Dc);

  OnDraw(Dc);
}

//-----------------------------------------------------------------------------
// Descriptions:
//   This mouse handler delegates to the subclassed event window.
//-----------------------------------------------------------------------------
void JZPianoWindow::OnMouseEvent(wxMouseEvent& MouseEvent)
{
  if (MouseEvent.Moving() && !MouseEvent.Dragging() && !mpMouseAction)
  {
    int fx, fy;
    MouseEvent.GetPosition(&fx, &fy);
    int Pitch = y2Pitch(fy);
    JZProjectManager::Instance()->ShowPitch(Pitch);
  }

  // dispatch

  if (!mpMouseAction)
  {
    int x, y;
    MouseEvent.GetPosition(&x, &y);

    // Was the mouse event below the top line that indicates the measure?
    if (y > mEventsY)
    {
      if (mPianoX < x && x < mPianoX + mPianoWidth)
      {
        // The mouse event was in the piano keys area.
        MousePiano(MouseEvent);
      }
      else if (mEventsX < x && x < mEventsX + mEventsWidth)
      {
        // The mouse event was in the MIDI event area.
        MouseEvents(MouseEvent);
      }
      else
      {
        OnEventWinMouseEvent(MouseEvent);
      }
    }
    else if (x > mEventsX)
    {
      // click in top line
      int action = mMousePlay.GetAction(MouseEvent);

      if (action)
      {
        if (!gpMidiPlayer->IsPlaying())
        {
          int Clock, LoopClock;
          if (action == MA_CYCLE)
          {
            if (mpSnapSel->IsSelected())
            {
              Clock = mpFilter->GetFromClock();
              LoopClock = mpFilter->GetToClock();
            }
            else
            {
              Clock = x2BarClock(x, 0);
              LoopClock = x2BarClock(x, 4);
            }
          }
          else
          {
            Clock = SnapClock(x2Clock(x));
            LoopClock = 0;
          }
          gpMidiPlayer->SetRecordInfo(0);
          gpMidiPlayer->StartPlay(Clock, LoopClock);
        }
        else
        {
          // Stop Record/Play
          gpMidiPlayer->StopPlay();
        }
      }
    }
  }
  else
  {
    MouseEvent.Skip();
//NEW    OnEventWinMouseEvent(MouseEvent);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::OnScroll(wxScrollWinEvent& Event)
{
  if (Event.GetOrientation() == wxHORIZONTAL)
  {
    HorizontalScroll(Event);
  }
  else if (Event.GetOrientation() == wxVERTICAL)
  {
    VerticalScroll(Event);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::HorizontalScroll(wxScrollWinEvent& Event)
{
  int EventWidth, EventHeight;
  GetVirtualEventSize(EventWidth, EventHeight);

  int NewScrolledX = mScrolledX;

  if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
  {
    --NewScrolledX;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
  {
    ++NewScrolledX;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP)
  {
    NewScrolledX -= 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN)
  {
    NewScrolledX += 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_TOP)
  {
    NewScrolledX = 0;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_BOTTOM)
  {
    NewScrolledX = EventWidth - 1;
  }
  else if (
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK ||
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE)
  {
    NewScrolledX = Event.GetPosition();
  }

  if (NewScrolledX < 0)
  {
    NewScrolledX = 0;
  }
  if (NewScrolledX > EventWidth - 1)
  {
    NewScrolledX = EventWidth - 1;
  }

  if (NewScrolledX != mScrolledX)
  {
    mScrolledX = NewScrolledX;
    SetScrollPos(wxHORIZONTAL, mScrolledX, true);
    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::VerticalScroll(wxScrollWinEvent& Event)
{
  int EventWidth, EventHeight;
  GetVirtualEventSize(EventWidth, EventHeight);

  int NewScrolledY = mScrolledY;

  if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
  {
    --NewScrolledY;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
  {
    ++NewScrolledY;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP)
  {
    NewScrolledY -= 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN)
  {
    NewScrolledY += 10;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_TOP)
  {
    NewScrolledY = 0;
  }
  else if (Event.GetEventType() == wxEVT_SCROLLWIN_BOTTOM)
  {
    NewScrolledY = EventHeight - 1;
  }
  else if (
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK ||
    Event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE)
  {
    NewScrolledY = Event.GetPosition();
  }

  if (NewScrolledY < 0)
  {
    NewScrolledY = 0;
  }
  if (NewScrolledY > EventHeight - 1)
  {
    NewScrolledY = EventHeight - 1;
  }

  if (NewScrolledY != mScrolledY)
  {
    mScrolledY = NewScrolledY;
    SetScrollPos(wxVERTICAL, mScrolledY, true);
    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Snapper
//-----------------------------------------------------------------------------
void JZPianoWindow::SnapSelectionStop(wxMouseEvent& MouseEvent)
{
  if (mpSnapSel->IsSelected())
  {
    int fr = y2Pitch(
      mpSnapSel->GetRectangle().y + mpSnapSel->GetRectangle().height - 1);
    int to = y2Pitch(mpSnapSel->GetRectangle().y + 1);

    mpFilter->SetFilterEvent(eFilterKeyOn, mVisibleKeyOn, fr, to);

    mpFilter->SetFilterEvent(
      eFilterPitch,
      mVisiblePitch,
      (fr << 7) - 8192,
      ((to + 1) << 7) - 8192);

    mpFilter->SetFilterEvent(eFilterControl, mVisibleController, fr, to);

    mpFilter->SetFilterEvent(eFilterProgram, mVisibleProgram, fr, to);

    mpFilter->SetFilterEvent(eFilterKeyPressure, mVisibleKeyOn, fr, to);

    mpFilter->SetFilterMeter(mVisibleTempo);

    mpFilter->SetFilterChannelAftertouch(mVisibleMono);

    mpFilter->SetFilterSysEx(mVisibleSysex);

    mpFilter->SetFromTrack(mTrackIndex);
    mpFilter->SetToTrack(mTrackIndex);
    mpFilter->SetFromClock(
      SnapClock(x2Clock(mpSnapSel->GetRectangle().x + 1)));
    mpFilter->SetToClock(SnapClock(x2Clock(
      mpSnapSel->GetRectangle().x + mpSnapSel->GetRectangle().width + 1)));
  }

  // Velocity or aftertouch editor update.
  if (mpCtrlEdit)
  {
    mpCtrlEdit->UpDate();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::SnapClock(int Clock, bool Up)
{
  int qnt = SnapClocks();
  Clock -= (Clock % qnt);
  if (Up)
  {
    Clock += qnt;
  }
  return Clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::SnapSelectionStart(wxMouseEvent& MouseEvent)
{
  int clk = SnapClock(mFromClock, false);
  int qnt = SnapClocks();
  vector<int> XSnaps;
  while (clk <= mToClock)
  {
    XSnaps.push_back(Clock2x(clk));
    clk += qnt;
  }
  mpSnapSel->SetXSnap(XSnaps, 0);

  mpSnapSel->SetYSnap(
    mFromLine * mTrackHeight + mTopInfoHeight,
    mEventsY + mEventsHeight,
    mTrackHeight);
}

//-----------------------------------------------------------------------------
// Description:
//   Update the play position to the clock argument, and trigger a redraw so
// the play bar will be drawn.
//-----------------------------------------------------------------------------
void JZPianoWindow::NewPlayPosition(int Clock)
{
  int scroll_clock = (mFromClock + 5 * mToClock) / 6;

  if (
    !mpSnapSel->IsActive() &&
    (Clock > scroll_clock || Clock < mFromClock) && Clock >= 0)
  {
    // Avoid permenent redraws when end of scroll range is reached.
    if (
      Clock > mFromClock &&
      mToClock >= mpProject->GetMaxQuarters() * mpProject->GetTicksPerQuarter())
    {
      return;
    }

    int x = Clock2x(Clock);
    SetXScrollPosition(x);
  }

  if (!mpSnapSel->IsActive())  // sets clipping
  {
    if (mPlayClock != Clock)
    {
//      int OldPlayClock = mPlayClock;
      mPlayClock = Clock;
//      wxRect InvalidateRect;
//      InvalidateRect.x = Clock2x(OldPlayClock) - 1;
//      InvalidateRect.y = 0;
//      InvalidateRect.width = 3;
//      InvalidateRect.height= 100000000;

//      Refresh(true, &InvalidateRect);

//      InvalidateRect.x = Clock2x(mPlayClock) - 1;

//      Refresh(true, &InvalidateRect);

      Refresh(false);
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::EventsSelected(const char *msg)
{
  if (!mpSnapSel->IsSelected())
  {
    if (msg == 0)
    {
      msg = "please select some events first";
    }
    wxMessageBox((char *)msg, "Error", wxOK);
    return 0;
  }
  return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::ZoomIn()
{
  if (mClockTicsPerPixel >= 2)
  {
    mClockTicsPerPixel /= 2;
    mScrolledX *= 2;

    SetScrollRanges();

//    NewPosition(mTrackIndex, mFromClock);

    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::ZoomOut()
{
  if (mClockTicsPerPixel <= 120)
  {
    mClockTicsPerPixel *= 2;
    mScrolledX /= 2;

    SetScrollRanges();

//    NewPosition(mTrackIndex, mFromClock);

    Refresh(false);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::OnEventWinMouseEvent(wxMouseEvent& MouseEvent)
{
  if (!mpMouseAction)
  {
    int x, y;
    MouseEvent.GetPosition(&x, &y);
    if (
      mEventsX < x && x < mEventsX + mEventsWidth &&
      mEventsY < y && y < mEventsY + mEventsHeight)
    {
      if (MouseEvent.LeftDown())
      {
        {
          SnapSelectionStart(MouseEvent);

//          if (mpSnapSel->IsSelected())
//          {
            // Redraw the whole window instead (inefficient, we should rather
            // invalidate a rect).
            Refresh();
//          }
          mpSnapSel->ProcessMouseEvent(MouseEvent, mScrolledX, mScrolledY);
          mpMouseAction = mpSnapSel;
        }
      }
    }
  }
  else
  {
    // mpMouseAction active

    if (mpMouseAction->ProcessMouseEvent(MouseEvent, mScrolledX, mScrolledY))
    {
      // mpMouseAction finished

      if (mpMouseAction == mpSnapSel)
      {
        SnapSelectionStop(MouseEvent);

        // inefficient, invalidate rect first instead.
        Refresh();

        mpMouseAction = 0;
        return 1;
      }

      mpMouseAction = 0;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
// Description:
//   Indicate which key on the pianoroll that the mouse is hovering over by
// highlighting it.  This function is bad because it draws directly in the dc,
// rather it should invalidate and let OnDraw do the actual painting.
//-----------------------------------------------------------------------------
void JZPianoWindow::ShowPitch(int Pitch)
{
  // This is the current position of the mouse.  mMouseLine is the last
  // position.
  int Line = y2TrackIndex(Pitch2y(Pitch));
  if (Line >= mFromLine && Line != mMouseLine)
  {
    wxClientDC Dc(this);

    // Translate scrolled coordinates.
    PrepareDC(Dc);

    Dc.SetLogicalFunction(wxXOR);

    Dc.SetBrush(*wxBLUE_BRUSH);
    if (mMouseLine >= 0)
    {
      // Erase the previous highlight.
      Dc.DrawRectangle(
        mPianoX,
        TrackIndex2y(mMouseLine) + mLittleBit,
        mPianoWidth,
        mTrackHeight - 2 * mLittleBit);
    }

    mMouseLine = Line;

    // Draw the new position.
    Dc.DrawRectangle(
      mPianoX,
      TrackIndex2y(mMouseLine) + mLittleBit,
      mPianoWidth,
      mTrackHeight - 2 * mLittleBit);

    Dc.SetLogicalFunction(wxCOPY);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::MouseCutPaste(wxMouseEvent& MouseEvent, bool Cut)
{
  wxClientDC Dc(this);

  PrepareDC(Dc);

  // Convert physical coordinates to logical (scrolled) coordinates.
  wxPoint Point = MouseEvent.GetLogicalPosition(Dc);

  int x = Point.x;
  int y = Point.y;

  int Clock = x2Clock(x);
  int Pitch = y2Pitch(y);
  JZEvent* pEvent = FindEvent(mpTrack, Clock, Pitch);
  if (pEvent)
  {
    Copy(mpTrack, pEvent, Cut);
  }
  else
  {
    Paste(mpTrack, SnapClock(Clock), Pitch);
  }

  // always redraw
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::MouseEvents(wxMouseEvent& MouseEvent)
{
  int action = mMouseEvent.GetAction(MouseEvent);

  if (action)
  {
    int x, y;
    MouseEvent.GetPosition(&x, &y);

    int Clock = x2Clock(x);
    int Pitch = y2Pitch(y);
    JZEvent* pEvent = FindEvent(mpTrack, Clock, Pitch);
    JZKeyOnEvent* pKeyOn = 0;
    JZPlayTrackEvent* pPlayTrack = 0;
    if (pEvent)
    {
      // both these events are drag length
      pKeyOn = pEvent->IsKeyOn();
      pPlayTrack = pEvent->IsPlayTrack();
    }
    switch (action)
    {
      case MA_CUTPASTE:
        MouseCutPaste(MouseEvent, 1);
        break;

      case MA_COPY:
        MouseCutPaste(MouseEvent, 0);
        break;

      case MA_LENGTH:
        if (pKeyOn)
        {
          if (!mpTrack->GetAudioMode())
          {
            mpMouseAction = new JZKeyLengthDragger(pKeyOn, this);
          }
        }
        else
        {
          if (pPlayTrack)
          {
            mpMouseAction = new JZPlayTrackLengthDragger(pPlayTrack, this);
          }
          else if (mVisibleAllTracks)
          {
            // event not found, maybe change to another Track
            for (int i = 0; i < mpProject->GetTrackCount(); ++i)
            {
              JZTrack* pTrack = mpProject->GetTrack(i);
              if (IsVisible(pTrack) && FindEvent(pTrack, Clock, Pitch))
              {
                NewPosition(i, -1);
                break;
              }
            }
          }
        }
        break;


      case MA_DIALOG:
        EventDialog(pEvent, this, mpTrack, Clock, mpTrack->mChannel - 1, Pitch);
        Refresh();
        break;


      case MA_LISTEN:
        MousePiano(MouseEvent);
        break;

      case MA_SELECT:
      case MA_CONTSEL:
        OnEventWinMouseEvent(MouseEvent);
        break;

      case MA_VELOCITY:
        if (pKeyOn)
        {
          JZRectangle r;
          r.x = mLittleBit;
          r.y = 0;
          r.SetWidth(mPianoWidth - 2 * mLittleBit);
          r.SetHeight(mTopInfoHeight);

          JZVelocityCounter* pVelocCounter =
            new JZVelocityCounter(this, &r, pKeyOn);
          pVelocCounter->ProcessMouseEvent(MouseEvent, mScrolledX, mScrolledY);
          mpMouseAction = pVelocCounter;
        }
        break;
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::MousePiano(wxMouseEvent& MouseEvent)
{
  if (MouseEvent.ButtonDown())
  {
    mpMouseAction = new JZMousePlay(this);
    int Status =
      mpMouseAction->ProcessMouseEvent(MouseEvent, mScrolledX, mScrolledY);
    if (Status == 1)
    {
      delete mpMouseAction;
      mpMouseAction = 0;
    }
  }
}

//-----------------------------------------------------------------------------
// Description:
//   This is an event handler for JZMouseCounter.
//-----------------------------------------------------------------------------
void JZPianoWindow::ButtonLabelDisplay(const wxString& Text, bool IsButtonDown)
{
  wxClientDC Dc(this);
  LineText(Dc, 0, 0, mPianoWidth, mTopInfoHeight, Text, IsButtonDown);
}

//=============================================================================
// Visible
//=============================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::IsVisible(JZEvent* pEvent)
{
  switch (pEvent->GetStat())
  {
    case StatKeyOn:
      return mVisibleKeyOn;
    case StatPitch:
      return mVisiblePitch;
    case StatControl:
      return mVisibleController;
    case StatProgram:
      return mVisibleProgram;
    case StatSetTempo:
      return mVisibleTempo;
    case StatSysEx:
      return mVisibleSysex;
    case StatPlayTrack:
      return mVisiblePlayTrack;
    case StatEndOfTrack:
      return true;
    case StatText:
      return true;
    case StatChnPressure:
      return mVisibleMono;
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::IsVisible(JZTrack* pTrack)
{
  if (!mVisibleAllTracks)
  {
    return pTrack == mpTrack;
  }

  return
    (mpTrack->mChannel == gpConfig->GetValue(C_DrumChannel)) ==
    (pTrack->mChannel == gpConfig->GetValue(C_DrumChannel));
}

//=============================================================================
// Utilities
//=============================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::SnapClocks()
{
  int Clock = mpProject->GetTicksPerQuarter() * 4 / mSnapDenomiator;
  if (Clock < 1)
  {
    return 1;
  }
  return Clock;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::SetSnapDenom(int Value)
{
  const int Size = 4;
  const struct
  {
    int mId;
    int mValue;
  } Table[Size] =
  {
    { ID_SNAP_8,    8 },
    { ID_SNAP_8D,  12 },
    { ID_SNAP_16,  16 },
    { ID_SNAP_16D, 24 },
  };

  int Id = 0;

  // find the button
  for (int i = 0; i < Size; ++i)
  {
    if (Table[i].mValue == Value)
    {
      Id = Table[i].mId;
    }
  }

  mpPianoFrame->SetToolbarButtonState(Id);

  mSnapDenomiator = Value;
//  mMouseEvent.SetLeftAction(MA_CUTPASTE);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::y2Pitch(int y)
{
  int Pitch = 127 - y2TrackIndex(y);
  if (Pitch < 0)
  {
    return 0;
  }
  if (Pitch > 127)
  {
    return 127;
  }
  return Pitch;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::Pitch2y(int Pitch)
{
  return TrackIndex2y(127 - Pitch);
}

//-----------------------------------------------------------------------------
// Description:
//   If Pitch == -1: search for any pitches.
//-----------------------------------------------------------------------------
JZEvent *JZPianoWindow::FindEvent(JZTrack* pTrack, int Clock, int Pitch)
{
  JZEventIterator Iterator(pTrack);
  JZEvent* pEvent = Iterator.First();
  while (pEvent)
  {
    if (pEvent->GetClock() <= Clock)
    {
      if (
        (pEvent->GetClock() + pEvent->GetLength() >= Clock) &&
        (pEvent->GetPitch() == Pitch || Pitch == -1) &&
        IsVisible(pEvent))
      {
        return pEvent;
      }
    }
    else
    {
      return 0;
    }
    pEvent = Iterator.Next();
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::kill_keys_aftertouch(JZTrack* pTrack, JZEvent* pEvent)
{
  int key,channel;
  JZEventIterator iter(pTrack);
  JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
  if (!pKeyOn)
  {
    return;
  }
  if (pKeyOn->GetEventLength() < 2)
  {
    return;
  }
  key = pKeyOn->GetKey();
  channel = pKeyOn->GetChannel();

  JZKeyPressureEvent* pKeyPressure;
  pEvent = iter.Range(
    pKeyOn->GetClock() + 1,
    pKeyOn->GetClock() + pKeyOn->GetEventLength());
  while (pEvent)
  {
    pKeyPressure = pEvent->IsKeyPressure();
    if (pKeyPressure)
    {
      if (
        pKeyPressure->GetKey() == key &&
        pKeyPressure->GetChannel() == channel)
      {
        pTrack->Kill(pEvent);
      }
    }
    pEvent = iter.Next();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::paste_keys_aftertouch(JZTrack* pTrack, JZEvent* pEvent)
{
  int key,channel;
  JZEventIterator iter(pTrack);
  JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
  if (!pKeyOn)
  {
    return;
  }
  channel = pKeyOn->GetChannel();
  if (pKeyOn->GetEventLength() < 2)
  {
    return;
  }
  key = pKeyOn->GetKey();

  JZKeyPressureEvent* pKeyPressure;

  pEvent = iter.Range(
    pKeyOn->GetClock() + 1,
    pKeyOn->GetClock() + pKeyOn->GetEventLength());

  while (pEvent)
  {
    pKeyPressure = pEvent->IsKeyPressure();
    if (pKeyPressure)
    {
      if (
        pKeyPressure->GetKey() == key &&
        pKeyPressure->GetChannel() == channel)
      {
        mPasteBuffer.Put(pEvent->Copy());
      }
    }
    pEvent = iter.Next();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::UpdateControl()
{
  if (mpCtrlEdit)
  {
    mpCtrlEdit->UpDate();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::ApplyToTrack(JZEvent* pEvent1, JZEvent* pEvent2)
{
  mpTrack->Kill(pEvent1);
  mpTrack->Put(pEvent2);
  mpTrack->Cleanup();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::KillTrackEvent(JZEvent* pEvent)
{
  mpTrack->Kill(pEvent);
}

//-----------------------------------------------------------------------------
// Description:
//   Positions for controller editor.
//-----------------------------------------------------------------------------
#define CtrlH(h)        ((h)/4)
#define CtrlY(h)        (h - CtrlH(h))

//-----------------------------------------------------------------------------
// Description:
//   Activate velocity edit.
//-----------------------------------------------------------------------------
void JZPianoWindow::CtrlVelocity()
{
  int Width, Height;
  GetClientSize(&Width, &Height);

  delete mpCtrlEdit;

  mpCtrlEdit = new JZVelocityEdit(
    this,
    "Velocity",
    mPianoWidth,
    0,
    CtrlY(Height),
    mCanvasWidth,
    CtrlH(Height));

  mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);

  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::CtrlChannelAftertouchEdit()
{
  int Width, Height;
  GetClientSize(&Width, &Height);

  delete mpCtrlEdit;

  mpCtrlEdit = new JZChannelAftertouchEdit(
    this,
    "Channel Aftertouch",
    mPianoWidth,
    0,
    CtrlY(Height),
    mCanvasWidth,
    CtrlH(Height));

  mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::CtrlPolyAftertouchEdit()
{
  int Width, Height;
  GetClientSize(&Width, &Height);

  delete mpCtrlEdit;

  mpCtrlEdit = new JZPolyAfterEdit(
    this,
    "Key Aftertouch",
    mPianoWidth,
    0,
    CtrlY(Height),
    mCanvasWidth,
    CtrlH(Height));

  mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::CtrlNone()
{
  delete mpCtrlEdit;
  mpCtrlEdit = 0;
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::CtrlTempo()
{
  JZEventIterator Iterator(mpTrack);

  mpTrack->Sort();

  JZEvent* pEvent = Iterator.Range(0, (unsigned) mpTrack->GetLastClock() + 1);
  JZSetTempoEvent* pSetTempoEvent;
  int Min = 240;
  int Max = 20;
  while (pEvent)
  {
    if ((pSetTempoEvent = pEvent->IsSetTempo()) != 0)
    {
      if (pSetTempoEvent->GetBPM() < Min)
      {
        Min = pSetTempoEvent->GetBPM();
      }
      if (pSetTempoEvent->GetBPM() > Max)
      {
        Max = pSetTempoEvent->GetBPM();
      }
    }
    pEvent = Iterator.Next();
  }
  if (Min - 50 > 20)
  {
    Min -= 50;
  }
  else
  {
    Min = 20;
  }
  if (Max + 50 < 240)
  {
    Max += 50;
  }
  else
  {
    Max = 240;
  }

  int Width, Height;
  GetClientSize(&Width, &Height);

  delete mpCtrlEdit;

  mpCtrlEdit = new JZTempoEdit(
    Min,
    Max,
    this,
    "Tempo",
    mPianoWidth,
    0,
    CtrlY(Height),
    mCanvasWidth,
    CtrlH(Height));

  mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::EditFilter()
{
  mpFilter->Dialog(0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::SelectController()
{
  int i = SelectControllerDlg();
  if (i > 0)
  {
    int Width, Height;
    GetClientSize(&Width, &Height);

    delete mpCtrlEdit;

    mpCtrlEdit = new JZControlEdit(
      i - 1,
      this,
      gpConfig->GetCtrlName(i).first.c_str(),
      mPianoWidth,
      0,
      CtrlY(Height),
      mCanvasWidth,
      CtrlH(Height));

    mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
    Refresh();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::CtrlModulation()
{
  int Width, Height;
  GetClientSize(&Width, &Height);

  delete mpCtrlEdit;

  mpCtrlEdit = new JZControlEdit(
    1,
    this,
    "Modulation",
    mPianoWidth,
    0,
    CtrlY(Height),
    mCanvasWidth,
    CtrlH(Height));

  mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::CtrlPitch()
{
  int Width, Height;
  GetClientSize(&Width, &Height);

  delete mpCtrlEdit;

  mpCtrlEdit = new JZPitchEdit(
    this,
    "Pitch",
    mPianoWidth,
    0,
    CtrlY(Height),
    mCanvasWidth,
    CtrlH(Height));

  mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Redo()
{
  mpProject->Redo();
  Refresh();
  if (mpCtrlEdit && mpTrack >= 0)
  {
    mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Undo actions
//-----------------------------------------------------------------------------
void JZPianoWindow::Undo()
{
  mpProject->Undo();
  Refresh();
  if (mpCtrlEdit && mpTrack >= 0)
  {
    mpCtrlEdit->ReInit(mpTrack, mFromClock, mClockTicsPerPixel);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Quantize selected events.
//-----------------------------------------------------------------------------
void JZPianoWindow::Quantize()
{
  if (EventsSelected())
  {
    JZCommandQuantize QuantizeCommand(
      mpFilter,
      SnapClocks(),
      true,
      false,
      0,
      0);
    QuantizeCommand.Execute(1);
    Refresh();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Flip events up and down.
//-----------------------------------------------------------------------------
void JZPianoWindow::ExchangeUpDown()
{
  if (EventsSelected())
  {
    JZCommandExchangeUpDown cmd(mpFilter);
    cmd.Execute(1);
    Refresh();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Flip events left to right.
//-----------------------------------------------------------------------------
void JZPianoWindow::ExchangeLeftRight()
{
  if (EventsSelected())
  {
    JZCommandExchangeLeftRight cmd(mpFilter);
    cmd.Execute(1);
    Refresh();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Shift events snapclock clocks to left.
//-----------------------------------------------------------------------------
void JZPianoWindow::ShiftLeft()
{
  if (EventsSelected())
  {
    int steps = -SnapClocks();
    JZCommandShift cmd(mpFilter, steps);
    cmd.Execute();
    Refresh();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Shift events snapclock clocks to right.
//-----------------------------------------------------------------------------
void JZPianoWindow::ShiftRight()
{
  if (EventsSelected())
  {
    int steps = SnapClocks();
    JZCommandShift cmd(mpFilter, steps);
    cmd.Execute();
    Refresh();
  }
}

//-----------------------------------------------------------------------------
// Description:
//   This is a helper function for cut and copy events.
//-----------------------------------------------------------------------------
void JZPianoWindow::CutOrCopy(int Id)
{
  if (EventsSelected())
  {
    mPasteBuffer.Clear();
    JZCommandCopyToBuffer cmd(mpFilter, &mPasteBuffer);
    mpFilter->SetOtherSelected(mVisibleTempo);
    cmd.Execute(0);        // no UNDO
    if (Id == wxID_CUT)
    {
      JZCommandErase cmd(mpFilter);
      cmd.Execute(1);        // with UNDO
      Refresh();
    }
    mpFilter->SetOtherSelected(false);
//OLD    if (mpGuitarFrame)
//OLD    {
//OLD      mpGuitarFrame->Update();
//OLD//      mpGuitarFrame->Redraw();
//OLD    }
    // Need a guitar window hint here.
    JZProjectManager::Instance()->UpdateAllViews();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Erase()
{
  if (EventsSelected())
  {
    JZCommandErase cmd(mpFilter);
    cmd.Execute(1);        // with UNDO
    Refresh();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::ToggleVisibleAllTracks()
{
  mVisibleAllTracks = !mVisibleAllTracks;

  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::MSelect()
{
  mpPianoFrame->PressRadio(ID_SELECT);
  mMouseEvent.SetLeftAction(MA_SELECT);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::MLength()
{
  mpPianoFrame->PressRadio(ID_CHANGE_LENGTH);
  mMouseEvent.SetLeftAction(MA_LENGTH);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::MDialog()
{
  mpPianoFrame->PressRadio(ID_EVENT_DIALOG);
  mMouseEvent.SetLeftAction(MA_DIALOG);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::MCutPaste()
{
  mpPianoFrame->PressRadio(ID_CUT_PASTE_EVENTS);
  mMouseEvent.SetLeftAction(MA_CUTPASTE);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Snap8()
{
  mPasteBuffer.Clear();
  SetSnapDenom(8);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Snap8D()
{
  mPasteBuffer.Clear();
  SetSnapDenom(12);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Snap16()
{
  mPasteBuffer.Clear();
  SetSnapDenom(16);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Snap16D()
{
  mPasteBuffer.Clear();
  SetSnapDenom(24);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Copy(JZTrack* pTrack, JZEvent* pEvent, int Kill)
{
  if (!pEvent)
  {
    return;
  }

  mpProject->NewUndoBuffer();
  mPasteBuffer.Clear();
  mPasteBuffer.Put(pEvent->Copy());

  if (pEvent->IsKeyOn())
  {
    paste_keys_aftertouch(pTrack, pEvent);
  }

  if (Kill)
  {
    JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
    if (pKeyOn)
    {
      kill_keys_aftertouch(pTrack, pEvent);
      if (pTrack->GetAudioMode())
      {
        gpMidiPlayer->ListenAudio(pKeyOn->GetKey(), 0);
      }
      else
      {
        JZListen::Instance()->KeyOn(
          pTrack,
          pKeyOn->GetKey(),
          pKeyOn->GetChannel(),
          pKeyOn->GetVelocity(),
          pKeyOn->GetEventLength());
      }
    }

    wxClientDC Dc(this);
    PrepareDC(Dc);
    DrawEvent(Dc, pEvent, wxWHITE_BRUSH, 0);
    pTrack->Kill(pEvent);
    pTrack->Cleanup();
  }

//OLD  if (mpGuitarFrame)
//OLD  {
//OLD    mpGuitarFrame->Update();
//OLD//    mpGuitarFrame->Redraw();
//OLD  }
  // Need a guitar window hint here.
  JZProjectManager::Instance()->UpdateAllViews();

  // Velocity or aftertouch editor update.
  if (mpCtrlEdit)
  {
    mpCtrlEdit->UpDate();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::Paste(JZTrack* pTrack, int Clock, int Pitch)
{
  if (mPasteBuffer.mEventCount == 0)
  {
    int len = SnapClocks() - 4;
    if (len < 2)
    {
      len = 2;
    }
    JZKeyOnEvent* pEvent = new JZKeyOnEvent(0, 0, 64, 64, len);
    mPasteBuffer.Put(pEvent);
  }

  if (GetKeyOnEventCount() > 1)
  {
    // don't change Pitch
    Pitch = -1;
  }

  mpProject->NewUndoBuffer();
  JZEventIterator Iterator(&mPasteBuffer);
  JZEvent* pEvent = Iterator.First();
  if (pEvent)
  {
    JZEvent *a = pEvent;
    while (a)
    {
      if (pEvent->IsChnPressure())
      {
        a = Iterator.Next();
        pEvent = a;
      }
      else
      {
        a = 0;
      }
    }

    int DeltaClock = Clock - pEvent->GetClock();
    int DeltaPitch = 0;
    if (Pitch >= 0)
    {
      DeltaPitch = Pitch - pEvent->GetPitch();
    }
    while (pEvent)
    {
      JZEvent *c = pEvent->Copy();
      c->SetPitch(c->GetPitch() + DeltaPitch);
      c->SetClock(c->GetClock() + DeltaClock);
      if (pTrack->mForceChannel && c->IsChannelEvent())
      {
        c->IsChannelEvent()->SetChannel(pTrack->mChannel - 1);
      }
      JZKeyOnEvent* pKeyOn = c->IsKeyOn();
      if (pKeyOn)
      {
        if (pTrack->GetAudioMode())
        {
          gpMidiPlayer->ListenAudio(pKeyOn->GetKey(), 0);
        }
        else
        {
          JZListen::Instance()->KeyOn(
            pTrack,
            pKeyOn->GetKey(),
            pKeyOn->GetChannel(),
            pKeyOn->GetVelocity(),
            pKeyOn->GetEventLength());
        }
      }
      wxClientDC Dc(this);
      PrepareDC(Dc);
      DrawEvent(Dc, c, c->GetBrush(), 0, 1);
      pTrack->Put(c);
      pEvent = Iterator.Next();
    }
    pTrack->Cleanup();

    // Velocity or aftertouch editor update.
    if (mpCtrlEdit)
    {
      mpCtrlEdit->UpDate();
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::GetKeyOnEventCount()
{
  int Count = 0;

  JZEventIterator Iterator(&mPasteBuffer);
  JZEvent* pEvent = Iterator.First();
  while (pEvent)
  {
    if (pEvent->IsKeyOn())
    {
      ++Count;
    }
    pEvent = Iterator.Next();
  }
  return Count;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPianoWindow::Channel()
{
  return mpTrack->mChannel ? mpTrack->mChannel - 1 : 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::SnapDialog()
{
  JZSnapDialog SnapDialog(mSnapDenomiator, this);
  SnapDialog.ShowModal();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::SetVisibleAllTracks(bool Value)
{
  mVisibleAllTracks = Value;
  Refresh();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::ActivateSettingsDialog()
{
//  jppResourceDialog Dialog(this, "windowSettings");
//
//  Dialog.Attach("use_colours", &mUseColors);
//  Dialog.Attach("font_size", &mFontSize, mPianoFontSizes);
//
//  if (Dialog.ShowModal() == wxID_OK)
//  {
//    Setup();
//    SetScrollRanges();
//    Refresh();
//  }
}

//-----------------------------------------------------------------------------
// Description:
//   This is a test to see how to implement a dialog with Patrick's system.
// It replaces JZMidiDelayDlg, which isnt necesarily a good idea.
//-----------------------------------------------------------------------------
void JZPianoWindow::ActivateMidiDelayDialog()
{
//  if (!EventsSelected())
//  {
//    return;
//  }
//
//  int scale = 50; //in percent
//  int clockDelay = 10;
//  int repeat = 6;
//
//  jppResourceDialog dialog(this, "midiDelay");
//
//  dialog.Attach("scale", &scale);
//  dialog.Attach("clockDelay", &clockDelay);
//  dialog.Attach("repeat", &repeat);
//
//  if (dialog.ShowModal() == wxID_OK)
//  {
//    //execute the command
//    JZCommandMidiDelay cmd(mpFilter, scale / 100.0, clockDelay, repeat);
//    cmd.Execute();
//    SetScrollRanges();
//    Refresh();
//  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPianoWindow::ActivateSequenceLengthDialog()
{
//  if (!EventsSelected())
//  {
//    return;
//  }
//
//  int scale = 100; //in percent
//
//  jppResourceDialog dialog(this, "sequenceLength");
//
//  dialog.Attach("scale", &scale);
//
//  if (dialog.ShowModal() == wxID_OK)
//  {
//    //execute the command
//    JZCommandSequenceLength cmd(mpFilter, scale / 100.0);
//    cmd.Execute();
//    SetScrollRanges();
//    Refresh();
//  }
}
