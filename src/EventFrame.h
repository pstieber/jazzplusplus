//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 2009-2013 Peter J. Stieber, all rights reserved.
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

#include <wx/frame.h>

class JZEventWindow;
class JZSong;
class JZToolBar;
class wxDialog;

//*****************************************************************************
// Description:
//   A frame window that containes a scrolled event window.  This class acts
// as the common base class for the JZTrackFrame class and the JZPianoFrame
// class.
// Functionality:
//   - Settings dialog
//*****************************************************************************
class JZEventFrame : public wxFrame
{
  public:

    // 2-step initialization: 1) constructor
    JZEventFrame(
      wxWindow* pParent,
      const wxString& Title,
      JZSong* pSong,
      const wxPoint& Position = wxDefaultPosition,
      const wxSize& Size = wxDefaultSize,
      long WindowStyle = wxDEFAULT_FRAME_STYLE);

    virtual ~JZEventFrame();

    virtual void SetEventWindow(JZEventWindow* pEventWindow);

    virtual bool OnClose();

  private:

    void OnUpdateEventsSelected(wxUpdateUIEvent& Event);

    void OnShift(wxCommandEvent& Event);

    void OnQuantize(wxCommandEvent& Event);

    void OnSetChannel(wxCommandEvent& Event);

    void OnTranspose(wxCommandEvent& Event);

    void OnDelete(wxCommandEvent& Event);

    void OnVelocity(wxCommandEvent& Event);

    void OnLength(wxCommandEvent& Event);

//    void OnSeqLength(wxCommandEvent& Event);

//    void OnMidiDelay(wxCommandEvent& Event);

    void OnConvertToModulation(wxCommandEvent& Event);

    void OnCleanup(wxCommandEvent& Event);

    void OnSearchReplace(wxCommandEvent& Event);

    void OnMeterChange(wxCommandEvent& Event);

  protected:

    JZToolBar* mpToolBar;

    JZEventWindow* mpEventWindow;

  DECLARE_EVENT_TABLE()
};
