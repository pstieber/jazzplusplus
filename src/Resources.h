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

// The wxWidgets documentation says it's safe to roll your own identifiers
// below "wxID_LOWEST" or above "wxID_HIGHEST".  In recent versions, the
// identifiers were enumerated but we need macros for resources since the
// resource compilers cannot handle C++ enum values, so we set the value
// of wxID_HIGHEST here.
#define wxID_HIGHEST                         5999

#define ID_FILE_REVERT_TO_SAVED              (wxID_HIGHEST + 1)

#define ID_IMPORT_MIDI                       (wxID_HIGHEST + 2)
#define ID_IMPORT_ASCII_MIDI                 (wxID_HIGHEST + 3)
#define ID_EXPORT_MIDI                       (wxID_HIGHEST + 4)
#define ID_EXPORT_ASCII_MIDI                 (wxID_HIGHEST + 5)
#define ID_EXPORT_SELECTION_AS_MIDI          (wxID_HIGHEST + 6)

#define ID_SETTINGS_METRONOME                (wxID_HIGHEST + 10)
#define ID_SETTINGS_SYNTHESIZER              (wxID_HIGHEST + 11)
#define ID_SETTINGS_MIDI_DEVICE              (wxID_HIGHEST + 12)
#define ID_SETTINGS_PITCH_PAINTER            (wxID_HIGHEST + 13)
#define ID_SETTINGS_WAHWAH                   (wxID_HIGHEST + 14)

#define ID_EDIT_PASTE_MERGE                  (wxID_HIGHEST + 15)
#define ID_EDIT_MAXIMIZE_VOLUME              (wxID_HIGHEST + 16)

#define ID_AUDIO_GLOBAL_SETTINGS             (wxID_HIGHEST + 20)
#define ID_AUDIO_SAMPLE_SETTINGS             (wxID_HIGHEST + 21)
#define ID_AUDIO_LOAD_SAMPLE_SET             (wxID_HIGHEST + 22)
#define ID_AUDIO_SAVE_SAMPLE_SET             (wxID_HIGHEST + 23)
#define ID_AUDIO_SAVE_SAMPLE_SET_AS          (wxID_HIGHEST + 24)
#define ID_AUDIO_NEW_SAMPLE_SET              (wxID_HIGHEST + 25)

#define ID_EFFECTS_EQUALIZER                 (wxID_HIGHEST + 27)
#define ID_EFFECTS_FILTER                    (wxID_HIGHEST + 28)
#define ID_EFFECTS_DISTORTION                (wxID_HIGHEST + 29)
#define ID_EFFECTS_REVERB                    (wxID_HIGHEST + 30)
#define ID_EFFECTS_ECHO                      (wxID_HIGHEST + 31)
#define ID_EFFECTS_CHORUS                    (wxID_HIGHEST + 32)
#define ID_EFFECTS_PITCH_SHIFTER             (wxID_HIGHEST + 33)
#define ID_EFFECTS_STRETCHER                 (wxID_HIGHEST + 34)
#define ID_EFFECTS_REVERSE                   (wxID_HIGHEST + 35)
#define ID_EFFECTS_SYNTH                     (wxID_HIGHEST + 36)

#define ID_PAINTERS_VOLUME                   (wxID_HIGHEST + 37)
#define ID_PAINTER_WAHWAH                    (wxID_HIGHEST + 38)
#define ID_PAINTER_PAN                       (wxID_HIGHEST + 39)
#define ID_PAINTER_PITCH                     (wxID_HIGHEST + 40)
#define ID_PAINTER_NONE                      (wxID_HIGHEST + 41)

#define ID_TRIM                              (wxID_HIGHEST + 50)
#define ID_QUANTIZE                          (wxID_HIGHEST + 51)
#define ID_SET_CHANNEL                       (wxID_HIGHEST + 52)
#define ID_SHIFT                             (wxID_HIGHEST + 53)
#define ID_SHIFT_LEFT                        (wxID_HIGHEST + 54)
#define ID_SHIFT_RIGHT                       (wxID_HIGHEST + 55)
#define ID_SNAP                              (wxID_HIGHEST + 56)
#define ID_SNAP_8                            (wxID_HIGHEST + 57)
#define ID_SNAP_8D                           (wxID_HIGHEST + 58)
#define ID_SNAP_16                           (wxID_HIGHEST + 59)
#define ID_SNAP_16D                          (wxID_HIGHEST + 60)
#define ID_MIXER                             (wxID_HIGHEST + 61)
#define ID_PIANOWIN                          (wxID_HIGHEST + 62)
#define ID_METRONOME_TOGGLE                  (wxID_HIGHEST + 63)
#define ID_VELOCITY                          (wxID_HIGHEST + 64)
#define ID_LENGTH                            (wxID_HIGHEST + 65)
#define ID_MISC_TRACK_MERGE                  (wxID_HIGHEST + 66)
#define ID_MISC_SPLIT_TRACKS                 (wxID_HIGHEST + 67)
#define ID_MISC_METER_CHANGE                 (wxID_HIGHEST + 68)
#define ID_MISC_RESET_MIDI                   (wxID_HIGHEST + 69)
#define ID_MISC_SET_COPYRIGHT                (wxID_HIGHEST + 70)
#define ID_TRANSPOSE                         (wxID_HIGHEST + 71)
#define ID_CLEANUP                           (wxID_HIGHEST + 72)
#define ID_SEARCH_AND_REPLACE                (wxID_HIGHEST + 73)

#define ID_PLAY                              (wxID_HIGHEST + 80)
#define ID_PLAY_LOOP                         (wxID_HIGHEST + 81)
#define ID_RECORD                            (wxID_HIGHEST + 82)

#define ID_SELECT                            (wxID_HIGHEST + 85)

#define ID_CHANGE_LENGTH                     (wxID_HIGHEST + 90)

#define ID_EVENT_DIALOG                      (wxID_HIGHEST + 91)
#define ID_CUT_PASTE_EVENTS                  (wxID_HIGHEST + 92)
#define ID_SHOW_ALL_EVENTS_FROM_ALL_TRACKS   (wxID_HIGHEST + 93)

#define ID_TOOLS_HARMONY_BROWSER             (wxID_HIGHEST + 95)
#define ID_TOOLS_RHYTHM_GENERATOR            (wxID_HIGHEST + 96)

#define ID_INSTRUMENT_ADD                    (wxID_HIGHEST + 100)
#define ID_INSTRUMENT_DELETE                 (wxID_HIGHEST + 101)
#define ID_INSTRUMENT_UP                     (wxID_HIGHEST + 102)
#define ID_INSTRUMENT_DOWN                   (wxID_HIGHEST + 103)
#define ID_INSTRUMENT_GENERATE               (wxID_HIGHEST + 104)

#define ID_HELP_PIANO_WINDOW                 (wxID_HIGHEST + 108)

#define MEN_CLEAR                            (wxID_HIGHEST + 110)
#define ID_VIEW_SETTINGS                     (wxID_HIGHEST + 120)

#define IDC_BN_VISIT_WEB_SITE                (wxID_HIGHEST + 1000)

#define IDC_KB_VOLUME                        (wxID_HIGHEST + 1050)

#define IDC_KB_VELOCITY                      (wxID_HIGHEST + 1100)
#define IDC_KB_OFF_VELOCITY                  (wxID_HIGHEST + 1101)
#define IDC_KB_CHANNEL                       (wxID_HIGHEST + 1102)

// JZVelocityDialog resource IDs.
#define IDC_KB_VELOCITY_START                (wxID_HIGHEST + 1200)
#define IDC_KB_VELOCITY_STOP                 (wxID_HIGHEST + 1201)

// JZLengthDialog resource IDs.
#define IDC_KB_LENGTH_START                  (wxID_HIGHEST + 1210)
#define IDC_KB_LENGTH_STOP                   (wxID_HIGHEST + 1211)

// JZMidiChannelDialog resource IDs.
#define IDC_KB_MIDI_CHANNEL                  (wxID_HIGHEST + 1220)

// JZQuantizeDialog resource IDs.
#define IDC_KB_GROOVE                        (wxID_HIGHEST + 1230)
#define IDC_KB_DELAY                         (wxID_HIGHEST + 1231)

// JZTransposeDialog resource IDs.
#define IDC_KB_AMOUNT                        (wxID_HIGHEST + 1232)

// JZSamplesDialog resource IDs.
#define IDC_BN_SD_FILE_SELECT_BROWSE         (wxID_HIGHEST + 1240)

// Rhythm window resource IDs.
#define IDC_SL_RHYTHM_STEPS_PER_COUNT        (wxID_HIGHEST + 1250)
#define IDC_SL_RHYTHM_COUNTS_PER_BAR         (wxID_HIGHEST + 1251)
#define IDC_SL_RHYTHM_BAR_COUNT              (wxID_HIGHEST + 1252)
#define IDC_LB_RHYTHM_INSTRUMENTS            (wxID_HIGHEST + 1253)
#define IDC_SL_RHYTHM_GROUP_CONTRIB          (wxID_HIGHEST + 1254)
#define IDC_SL_RHYTHM_GROUP_LISTEN           (wxID_HIGHEST + 1255)
#define IDC_CB_RHYTHM_RANDOMIZE              (wxID_HIGHEST + 1258)
