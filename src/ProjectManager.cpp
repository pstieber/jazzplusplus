#include "ProjectManager.h"

#include "PianoFrame.h"
#include "TrackFrame.h"
#include "GuitarFrame.h"
#include "Globals.h"

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZProjectManager::JZProjectManager()
  : mpTrackFrame(nullptr),
    mpPianoFrame(nullptr),
    mpGuitarFrame(nullptr)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZProjectManager::~JZProjectManager()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZTrackFrame* JZProjectManager::CreateTrackView()
{
  if (!mpTrackFrame)
  {
    int XPosition(10), YPosition(10), Width(600), Height(400);

    gpConfig->Get(C_TrackWinXpos, XPosition);
    gpConfig->Get(C_TrackWinYpos, YPosition);
    gpConfig->Get(C_TrackWinWidth, Width);
    gpConfig->Get(C_TrackWinHeight, Height);

    wxPoint Position(XPosition, YPosition);

    wxSize Size(Width, Height);

    // Create the main application window.
    mpTrackFrame = new JZTrackFrame(
      nullptr,
      "Jazz++",
      gpProject,
      Position,
      Size);
  }

  mpTrackFrame->Show(true);

  return mpTrackFrame;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZProjectManager::CreatePianoView()
{
  if (!mpPianoFrame)
  {
    int XPosition(10), YPosition(10), Width(640), Height(480);

    gpConfig->Get(C_PianoWinXpos, XPosition);
    gpConfig->Get(C_PianoWinYpos, YPosition);
    gpConfig->Get(C_PianoWinWidth, Width);
    gpConfig->Get(C_PianoWinHeight, Height);

    wxPoint Position(XPosition, YPosition);

    wxSize Size(Width, Height);

    mpPianoFrame = new JZPianoFrame(
      mpTrackFrame,
      "Piano",
      gpProject,
      Position,
      Size);
  }

  mpPianoFrame->Show(true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZProjectManager::Detach(JZPianoFrame* pPianoFrame)
{
  if (mpPianoFrame == pPianoFrame)
  {
    mpPianoFrame = nullptr;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZProjectManager::CreateGuitarView()
{
  if (!mpGuitarFrame)
  {
    if (mpPianoFrame)
    {
      mpGuitarFrame = new JZGuitarFrame(mpPianoFrame);
    }
  }
  mpGuitarFrame->Show(true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZProjectManager::Detach(JZGuitarFrame* pGuitarFrame)
{
  if (pGuitarFrame == mpGuitarFrame)
  {
    mpGuitarFrame = nullptr;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZProjectManager::NewPlayPosition(int Clock)
{
  if (mpTrackFrame)
  {
    mpTrackFrame->NewPlayPosition(Clock);
  }

  if (mpPianoFrame)
  {
    mpPianoFrame->NewPlayPosition(Clock);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZProjectManager::ShowPitch(int Pitch)
{
  if (mpPianoFrame)
  {
    mpPianoFrame->ShowPitch(Pitch);
    mpPianoFrame->Update();
  }

  if (mpGuitarFrame)
  {
    mpGuitarFrame->ShowPitch(Pitch);
    mpGuitarFrame->Update();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZProjectManager::UpdateAllViews()
{
  if (mpTrackFrame)
  {
    mpTrackFrame->Refresh();
  }

  if (mpPianoFrame)
  {
    mpPianoFrame->Refresh();
  }

  if (mpGuitarFrame)
  {
    mpGuitarFrame->Refresh();
  }
}
