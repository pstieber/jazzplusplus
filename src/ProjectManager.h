#pragma once

class JZTrackFrame;
class JZPianoFrame;
class JZGuitarFrame;

//*****************************************************************************
//*****************************************************************************
class JZProjectManager
{
  public:

    static JZProjectManager& Instance();

    JZTrackFrame* CreateTrackView();

    void CreatePianoView();

    void Detach(JZPianoFrame* pPianoFrame);

    void CreateGuitarView();

    void Detach(JZGuitarFrame* pGuitarFrame);

    void NewPlayPosition(int Clock);

    void ShowPitch(int Pitch);

    void UpdateAllViews();

  private:

    JZProjectManager();

    ~JZProjectManager();

  private:

    JZTrackFrame* mpTrackFrame;

    JZPianoFrame* mpPianoFrame;

    JZGuitarFrame* mpGuitarFrame;
};

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
JZProjectManager& JZProjectManager::Instance()
{
  static JZProjectManager ProjectManager;
  return ProjectManager;
}
