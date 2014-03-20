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

#include "Song.h"
#include "Metronome.h"

class JZFilter;
class JZPlayer;
class JZRecordingInfo;
class JZSynthesizer;
class wxWindow;

//*****************************************************************************
// Description:
//   This is the Jazz++ project class declaration.  This class is derived from
// JZSong solely as a shortcut to having a track interface in the project
// object.  As the backend storage stuff gets rewritten, we'll be overloading
// the JZSong members accordingly, until JZSong itself will be gone.  JZSong is
// deprecated right now, and anything that still uses it needs to switch to
// using JZProject instead through its global instance.
//*****************************************************************************
class JZProject : public JZSong
{
  public:

    JZProject();

    ~JZProject();

    // restart play here if space bar hit
    int mStartTime;

    // Not yet sure what this does
    int mStopTime;

    // Loop flag, loops play if true
    bool mLoop;

    // If true, mutes output
    bool mMuted;

    // If true, records from midi in
    bool mRecord;

    // Not yet sure what this does
    JZFilter* Filter;

    // Description:
    //   Returns a constant reference to the metronome.
    const JZMetronomeInfo& GetMetronomeInfo() const;

    // Description:
    //   Sets the metronome data.
    void SetMetronomeInfo(const JZMetronomeInfo& MetronomeInfo);

    // Description:
    //   Toggles the "is on" state of the metronome.
    void ToggleMetronome();

    // Description:
    //   Returns the "is on" state of the metronome.
    bool IsMetronomeOn() const;

    // Number of bars
    int mNumBars;

    // Returns whether or not the project has changed since last save
    bool HasChanged();

    // Name of the song file currently loaded
    wxString mSongFileName;

    // Name of the pattern file currently loaded
    wxString mPatternFileName;

    // These provide access to the Project

    // Description:
    //   Set the song file name.
    void SetSong(const wxString& SongFileName);

    // Description:
    //   Set the pattern file name.
    void SetPattern(const wxString& PatternFileName);

    // Description:
    //   Open a MIDI file.
    //
    // Inputs:
    //   const wxString& SongFileName:
    //     Song path and file name.
    void OpenSong(const wxString& SongFileName);

    void OpenAndReadAsciiMidiFile(const wxString& AsciiMidiFileName);

    // Description:
    //   Save a MIDI file.  This function will overwrite the file if it
    // already exists!
    //
    // Inputs:
    //   const wxString& MidiFileName:
    //     MIDI file path and file name.
    void ExportMidiFile(const wxString& MidiFileName);

    void ExportAsciiMidiFile(const wxString& AsciiMidiFileName);

    // Here is the new play interface.  For now it just acts as a layer
    // between the Project and the GUI.
    // Returns true during playback
    bool IsPlaying();

    // Starts playback
    void Play();

    // Stops playback
    void Stop();

    // Sets the playback cursor to a specific position
    void SetPlayPosition(int newposition);

    // Mutes playback
    void Mute(bool newmute);

    // Set loop, true to loop, false not to loop
    void SetLoop(bool newloop);

    // Set record, true to record, false not to record
    void SetRecord(bool newrecord);

    // Beats me what this does.
    void SetLoopClock(int newclock);

    // Sets selection
//    void SetSelection();

    JZRecordingInfo* GetRecInfo();

    // Sets RecInfo, JZProject takes ownership of this object
    void SetRecInfo(JZRecordingInfo* pRecInfo);

    void EditAudioGlobalSettings(wxWindow* pParent);

    void EditAudioSamples(wxWindow* pParent);

    void LoadSampleSet(wxWindow* pParent);

    void SaveSampleSetAs(wxWindow* pParent);

    void SaveSampleSet(wxWindow* pParent);

    void ClearSampleSet(wxWindow* pParent);

    JZPlayer* GetPlayer()
    {
      return mpMidiPlayer;
    }

  private:

    void ReadConfiguration();

    bool FindAndRegisterConfFilePath(wxString& ConfFilePath) const;

  private:

    static wxString mConfFileName;

    JZConfiguration* mpConfig;

    JZPlayer* mpMidiPlayer;

    JZSynthesizer* mpSynth;

    JZRecordingInfo* mpRecInfo;

    // Stores metronome information.
    JZMetronomeInfo mMetronomeInfo;

    bool mChanged;

    bool mIsPlaying;
};
