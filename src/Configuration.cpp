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

#include "Configuration.h"

#include "FindFile.h"
#include "Globals.h"
#include "StringUtilities.h"
#include "Synth.h"
#include "SynthesizerTypeEnums.h"

#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

using namespace std;

//*****************************************************************************
// Description:
//   This is the configuration entry class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZConfigurationEntry::JZConfigurationEntry(
  const string& Name,
  int IntegerValue)
  : mType(eConfigEntryTypeInt),
    mName(Name),
    mValue(IntegerValue),
    mStringValue()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZConfigurationEntry::JZConfigurationEntry(
  const string& Name,
  const string& StringValue)
  : mType(eConfigEntryTypeStr),
    mName(Name),
    mValue(0),
    mStringValue(StringValue)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZConfigurationEntry::JZConfigurationEntry(const string& Name)
  : mType(eConfigEntryTypeEmpty),
    mName(Name),
    mValue(0),
    mStringValue()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZConfigurationEntry::SetStringValue(const string& StringValue)
{
  mStringValue = StringValue;
}

//*****************************************************************************
// Description:
//   This is the configuration class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZConfiguration::JZConfiguration()
  : mFileName(),
    mDrumNames(),
    mDrumSets(),
    mControlNames(),
    mVoiceNames(),
    mBankTable()
{
  for (int i = 0; i < NumConfigNames; ++i)
  {
    mNames[i] = 0;
  }

  // search for midi device
  mNames[C_Seq2Device] = new JZConfigurationEntry(".device", -1);

  // use /dev/music
  mNames[C_MidiDriver] = new JZConfigurationEntry(".driver", 1);

  // Enable audio at startup.
  mNames[C_EnableAudio] = new JZConfigurationEntry(".enable_audio", 1);

  // Windows midi devices.
  mNames[C_WinInputDevice] = new JZConfigurationEntry(
    ".win_input_device",
    -1);
  mNames[C_WinOutputDevice] = new JZConfigurationEntry(
    ".win_output_device",
    -1);

  // ALSA midi devices.
  mNames[C_AlsaInputDevice] = new JZConfigurationEntry(
    ".alsa_input_device",
    -1);
  mNames[C_AlsaOutputDevice] = new JZConfigurationEntry(
    ".alsa_output_device",
    -1);

  // ALSA audio devices.
  mNames[C_AlsaAudioInputDevice] = new JZConfigurationEntry(
    ".alsa_audio_input_device",
    "hw:0,0");
  mNames[C_AlsaAudioOutputDevice] = new JZConfigurationEntry(
    ".alsa_audio_output_device",
    "hw:0,0");

  // Emulate MIDI thru.
  mNames[C_SoftThru] = new JZConfigurationEntry(".softthru", 1);

  // mpu401 hardware MIDI thru.
  mNames[C_HardThru] = new JZConfigurationEntry(".hardthru", 1);

  // MIDI clock source (0 = internal).
  mNames[C_ClockSource] = new JZConfigurationEntry(".clocksource", 0);

  // Send realtime MIDI messages to MIDI out.
  mNames[C_RealTimeOut] = new JZConfigurationEntry(".realtime_out", 0);

  // Use the GS reverb macro.
  mNames[C_UseReverbMacro] = new JZConfigurationEntry(".use_reverb_macro", 1);

  // Use the GS chorus macro.
  mNames[C_UseChorusMacro] = new JZConfigurationEntry(".use_chorus_macro", 1);

  // Default drum channel is 10.
  mNames[C_DrumChannel] = new JZConfigurationEntry(".drumchannel", 10);

  // Controller for bank select.
  mNames[C_BankControlNumber] = new JZConfigurationEntry(
    ".bank_control_number",
    0);

  // Controller2 for bank select with two commands.
  mNames[C_BankControlNumber2] = new JZConfigurationEntry(
    ".bank_2nd_control_number",
    32);

  // Maximum number of entries in bank table (two commands).
  mNames[C_MaxBankTableEntries] = new JZConfigurationEntry(
    ".max_bank_table_entries",
    256);

  // Number of columns to draw in Parts dialogs.
  mNames[C_PartsColumnsMax] = new JZConfigurationEntry(
    ".parts_columns_max",
    4);

  // Draw tracknames on the right too?
  mNames[C_PartsTracknamesRight] = new JZConfigurationEntry(
    ".parts_tracknames_right",
    1);

  // Maximum number of voice names in .jazz.
  mNames[C_MaxVoiceNames] = new JZConfigurationEntry(".max_voice_names", 317);

  // Use two-command bank select?
  mNames[C_UseTwoCommandBankSelect] = new JZConfigurationEntry(
    ".use_two_command_bank_select",
    0);

  // Metronome settings.
  mNames[C_MetroIsAccented] = new JZConfigurationEntry(
    ".metronome_is_accented",
    1);
  mNames[C_MetroVelocity] = new JZConfigurationEntry(
    ".metronome_velocity",
    127);
  mNames[C_MetroNormalClick] = new JZConfigurationEntry(
    ".metronome_normal_click",
    37);
  mNames[C_MetroAccentedClick] = new JZConfigurationEntry(
    ".metronome_accented_click",
    36);

  //--------------------------
  // Window geometry settings.
  //--------------------------

  // Track window.
  mNames[C_TrackWinXpos] = new JZConfigurationEntry(
    ".trackwin_xpos",
    10);
  mNames[C_TrackWinYpos] = new JZConfigurationEntry(
    ".trackwin_ypos",
    10);
  mNames[C_TrackWinWidth] = new JZConfigurationEntry(
    ".trackwin_width",
    600);
  mNames[C_TrackWinHeight] = new JZConfigurationEntry(
    ".trackwin_height",
    400);

  // Piano window.
  mNames[C_PianoWinXpos] = new JZConfigurationEntry(
    ".pianowin_xpos",
    30);
  mNames[C_PianoWinYpos] = new JZConfigurationEntry(
    ".pianowin_ypos",
    30);
  mNames[C_PianoWinWidth] = new JZConfigurationEntry(
    ".pianowin_width",
    600);
  mNames[C_PianoWinHeight] = new JZConfigurationEntry(
    ".pianowin_height",
    400);

  // Parts dialog.
  mNames[C_PartsDlgXpos] = new JZConfigurationEntry(".partsdialog_xpos", 50);
  mNames[C_PartsDlgYpos] = new JZConfigurationEntry(".partsdialog_ypos", 50);

  // Track dialog.
  mNames[C_TrackDlgXpos] = new JZConfigurationEntry(".trackdialog_xpos", 50);
  mNames[C_TrackDlgYpos] = new JZConfigurationEntry(".trackdialog_ypos", 50);

  // Harmony browser.
  mNames[C_HarmonyXpos] = new JZConfigurationEntry(
    ".harmonybrowser_xpos",
    100);
  mNames[C_HarmonyYpos] = new JZConfigurationEntry(
    ".harmonybrowser_ypos",
    100);

  // Random rhythm.
  mNames[C_RhythmXpos] = new JZConfigurationEntry(".randomrhythm_xpos", 150);
  mNames[C_RhythmYpos] = new JZConfigurationEntry(".randomrhythm_ypos", 150);

  // Show dialog unless initialized.
  mNames[C_SynthDialog] = new JZConfigurationEntry(".synth_dialog", 1);

  // Default synthesizer type.
  mNames[C_SynthType] = new JZConfigurationEntry(
    ".synth_type",
    gSynthesizerTypes[SynthTypeGS].first);

  // Default synthesizer configuration file.
  mNames[C_SynthConfig] = new JZConfigurationEntry(
    ".synth_config",
    gSynthesierTypeFiles[SynthTypeGS].first);

  // When to send synthesizer reset (0 = never, 1 = song start,
  // 2 = start play).
  mNames[C_SendSynthReset] = new JZConfigurationEntry(".send_synth_reset", 1);

  // Current include file.
  mNames[C_Include] = new JZConfigurationEntry(".include", "");

  // Entries with empty values.
  mNames[C_BankTable] = new JZConfigurationEntry(".bank_table");
  mNames[C_VoiceNames] = new JZConfigurationEntry(".voicenames");
  mNames[C_DrumSets] = new JZConfigurationEntry(".drumsets");
  mNames[C_CtrlNames] = new JZConfigurationEntry(".ctrlnames");
  mNames[C_DrumNames] = new JZConfigurationEntry(".drumnames");

  // The startup song.
  mNames[C_StartUpSong] = new JZConfigurationEntry(
    ".startup_song",
    "jazz.mid");

  mNames[C_OssBug1] = new JZConfigurationEntry(".ossbug1", 0);
  mNames[C_OssBug2] = new JZConfigurationEntry(".ossbug2", 0);
  mNames[C_DuplexAudio] = new JZConfigurationEntry(".duplex_audio", 0);
  mNames[C_ThruInput] = new JZConfigurationEntry(".thru_input", 0);
  mNames[C_ThruOutput] = new JZConfigurationEntry(".thru_output", 0);

  // Enable/disable splash dialog.
  mNames[C_EnableWelcome] = new JZConfigurationEntry(".enable_welcome", 1);

  // Other initialization.

  for (int i = 0; i < 130; ++i)
  {
    mDrumNames.push_back(make_pair("", i));
  }

  const string NoneString("None");

  mDrumSets.push_back(make_pair(NoneString, 0));
  for (int i = 1; i < 130; ++i)
  {
    mDrumSets.push_back(make_pair("", i));
  }

  for (int i = 0; i < 130; ++i)
  {
    mControlNames.push_back(make_pair("", i));
  }

  mVoiceNames.push_back(make_pair(NoneString, 0));
  mVoiceNames.push_back(make_pair("", 0));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZConfiguration::~JZConfiguration()
{
  for (int i = 0; i < NumConfigNames; ++i)
  {
    if (mNames[i])
    {
      delete mNames[i];
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const pair<string, int>& JZConfiguration::GetDrumName(unsigned Entry) const
{
  assert((Entry >= 0) && (Entry < mDrumNames.size()));
  return mDrumNames[Entry];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const pair<string, int>& JZConfiguration::GetDrumSet(unsigned Entry) const
{
  assert((Entry >= 0) && (Entry < mDrumSets.size()));
  return mDrumSets[Entry];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const pair<string, int>& JZConfiguration::GetVoiceName(unsigned Entry) const
{
  assert((Entry >= 0) && (Entry < mVoiceNames.size()));
  return mVoiceNames[Entry];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const pair<string, int>& JZConfiguration::GetCtrlName(unsigned Entry) const
{
   assert((Entry >= 0) && (Entry < mControlNames.size()));
   return mControlNames[Entry];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZDoubleCommand& JZConfiguration::BankEntry(unsigned Entry)
{
   assert((Entry >= 0) && (Entry < mBankTable.size()));
   return mBankTable[Entry];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZConfiguration::Check(const string& InputLine) const
{
  string::const_iterator iString = InputLine.begin();

  if (iString == InputLine.end() || *iString != '.')
  {
    return -1;
  }

  const string Delimiters(" \t");

  vector<string> Tokens;

  Tokenize(InputLine, Delimiters, Tokens);

  for (int i = 0; i < NumConfigNames; i++)
  {
    if (!mNames[i])
    {
      continue;
    }
    if (Tokens[0] == mNames[i]->GetName())
    {
      // Found
      return i;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
// Description:
//   Return the Jazz++ configuration file name, normally jazz.cfg.
//-----------------------------------------------------------------------------
wxString JZConfiguration::GetFileName()
{
  if (!mFileName.empty())
  {
    return mFileName;
  }

  wxString ConfigDir = wxStandardPaths::Get().GetUserDataDir();

  wxString JazzCfgFile =
    ConfigDir +
    wxFileName::GetPathSeparator() +
    "jazz.cfg";

  if (::wxFileExists(JazzCfgFile))
  {
    mFileName = JazzCfgFile;
  }

  return mFileName;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZConfiguration::Load(const string& InputLine)
{
  int Entry = Check(InputLine);

  if (Entry < 0)
  {
    return Entry;
  }

  int Result = 1;

  switch (mNames[Entry]->GetType())
  {
    case eConfigEntryTypeInt:
    {
      string Name;
      int Value;
      istringstream Iss(InputLine);
      Iss >> Name >> Value;
      if (Iss.fail())
      {
        Result = -1;
      }
      mNames[Entry]->SetValue(Value);
      break;
    }
    case eConfigEntryTypeStr:
    {
      string::size_type FindIndex = InputLine.find(mNames[Entry]->GetName());

      if (FindIndex == string::npos)
      {
        // Give up if the configuration entry name is not found.
        Result = -1;
        break;
      }

      // Construct a string from the rest of the line and trim leading and
      // trailing white space.
      string StringValue = TNStringUtilities::TrimLeadingAndTrailingBlanks(
        string(InputLine, FindIndex + mNames[Entry]->GetName().size()));

      mNames[Entry]->SetStringValue(StringValue);

      break;
    }
    case eConfigEntryTypeEmpty:
      break;
  }

  if (Result <= 0)
  {
    return -1;
  }

  return Entry;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZConfiguration::GetValue(const char* pName) const
{
  int i = Check(pName);

  assert(i >= 0);

  return mNames[i]->GetValue();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZConfiguration::GetValue(int Index) const
{
  assert((Index >= 0) && (Index < NumConfigNames));
  return mNames[Index]->GetValue();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZConfiguration::Get(int Entry, string& Value)
{
  assert((Entry >= 0) && (Entry < NumConfigNames));

  wxString FileName = GetFileName();
  if (FileName.IsEmpty())
  {
    return false;
  }

  ifstream Ifs(FileName.mb_str());
  if (!Ifs)
  {
    return false;
  }

  const string& Name = GetName(Entry);

  string::size_type Length = Name.length();

  string Line;

  bool Found = false;
  while (!Found && !Ifs.eof() && getline(Ifs, Line))
  {
    // Search the beginning of the string for the name.
    string::size_type Start = Line.find(Name);
    if (Start == 0)
    {
      // Skip white space to find the value.
      Start += Length;
      while (isspace(Line[Start]))
      {
        ++Start;
      }

      // Find the end of the value.
      string::size_type End = Line.length() - 1;
      while (End > 0 && isspace(Line[End]))
      {
        --End;
      }

      Value = Line.substr(Start, End - Start + 1);

      // Indicate a value was found.
      Found = true;
    }
  }

  Ifs.close();

  return Found;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZConfiguration::Get(int Entry, int& Value)
{
  string String;
  if (Get(Entry, String))
  {
    istringstream Iss(String);
    Iss >> Value;
    if (!Iss.fail())
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
// Description:
//   Write a configuration entry by making a temp file, and copying all
// entries to there.  If the name/value pair is found, replace it, otherwise
// write it.  Finally copy the temp file over the old configuration file.
//-----------------------------------------------------------------------------
bool JZConfiguration::Put(int Index, const string& ValueString)
{
  assert((Index >= 0) && (Index < NumConfigNames));

  wxString FileName = GetFileName();
  if (FileName.IsEmpty())
  {
    return false;
  }

  // Create a temporary file name from the current file name.
//  wxFileName TempFileName = wxFileName::CreateTempFileName(wxEmptyString);

  string TempFileName(FileName);
  TempFileName.append(".tmp");
  ofstream Os(TempFileName.c_str());
  if (!Os)
  {
    return false;
  }

  ifstream Ifs(FileName.mb_str());
  if (!Ifs)
  {
    return false;
  }

  const string& ValueName = GetName(Index);

  string Line;
  bool Found = false;
  while (!Ifs.eof() && getline(Ifs, Line))
  {
    string::size_type Start = Line.find(ValueName);
    if (Start == 0)
    {
      Os << ValueName << ' ' << ValueString << endl;
      Found = true;
    }
    else
    {
      Os << Line << endl;
    }
  }
  if (!Found)
  {
    Os << ValueName << ' ' << ValueString << endl;
  }
  Ifs.close();
  Os.close();

  ::wxRemoveFile(FileName.c_str());
  ::wxRenameFile(TempFileName, FileName);

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZConfiguration::Put(int Index)
{
  assert((Index >= 0) && (Index < NumConfigNames));
  mNames[Index]->SetValue(Index);
  int Value = mNames[Index]->GetValue();
  return Put(Index, Value);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZConfiguration::Put(int Index, int Value)
{
  assert((Index >= 0) && (Index < NumConfigNames));
  mNames[Index]->SetValue(Value);
  ostringstream Oss;
  Oss << Value;
  return Put(Index, Oss.str());
}

//-----------------------------------------------------------------------------
// Description:
//   Load the configuration from a file.  This code supports file inclusion.
//-----------------------------------------------------------------------------
void JZConfiguration::LoadConfig(const wxString& FileName)
{
  if (!::wxFileExists(FileName))
  {
    return;
  }

  mFileName = FileName;

  // Get the current working directory so the code can return to this
  // directory when it is done reading the configuration file.
  wxString OriginalCurrentWorkingDirectory = ::wxGetCwd();

  wxFileName FileNameObject(mFileName);

  wxString Path = FileNameObject.GetPath();
  ::wxSetWorkingDirectory(Path);

  int i, j;
  unsigned BankIndex = 0, VoiceIndex = 0, DrumsetIndex = 0;

  vector<pair<string, int> >* pVector = 0;

  stack<ifstream*> InputFileStreams;
  string InputLine;

  cout
    << "JZConfiguration::LoadConfig:" << '\n'
    << "  \"" << mFileName << '"'
    << endl;

  ifstream* pIs = new ifstream(mFileName.mb_str());
  InputFileStreams.push(pIs);

  if (!*InputFileStreams.top())
  {
    wxString String;
    String
      << "Error reading config file..." << '\n'
      << '"' << mFileName << '"' << '\n'
      << "Please check permissions and set the environment variable" << '\n'
      << "JAZZ to the installation directory";
    ::wxMessageBox(String, "Warning", wxOK);

    // Return to the original working directory.
    ::wxSetWorkingDirectory(OriginalCurrentWorkingDirectory);

    return;
  }

  while (1)
  {
    // Read a line from the current file.

    getline(*InputFileStreams.top(), InputLine);

    // Check for an end-of-file condition.
    if (InputFileStreams.top()->eof())
    {
      InputFileStreams.top()->close();
      delete InputFileStreams.top();
      InputFileStreams.pop();

      // Are there any open streams?
      if (InputFileStreams.empty())
      {
        // The code has reached the last line of the Jazz++ configuration file
        // (jazz.cfg or .jazz).
        break;
      }
      else
      {
        // The code has reached the last line of current include-file.
        continue;
      }
    }

    int Entry;

    // Read keyword lines.
    if ((Entry = gpConfig->Load(InputLine)) >= 0)
    {
      switch (Entry)
      {
        case C_BankTable:

          pVector = 0;

          // If this is the first bank table entry, create the table.
          if (mBankTable.empty())
          {
            mBankTable.clear();
            JZDoubleCommand DoubleCommand;
            for (i = 0; i <= GetValue(C_MaxBankTableEntries); ++i)
            {
              DoubleCommand.Command[0] = -1;
              DoubleCommand.Command[1] = -1;
              mBankTable.push_back(DoubleCommand);
            }
          }
          break;
        case C_VoiceNames:
          if (mVoiceNames.size() == 2)
          {
            mVoiceNames.clear();

            mVoiceNames.push_back(make_pair("None", i));
            for (i = 0; i < GetValue(C_MaxVoiceNames); i++)
            {
              mVoiceNames.push_back(make_pair("", i));
            }
          }
          pVector = &mVoiceNames;
          break;
        case C_DrumSets:
          pVector = &mDrumSets;
          break;
        case C_CtrlNames:
          pVector = &mControlNames;
          break;
        case C_DrumNames:
          pVector = &mDrumNames;
          break;
        case C_SynthConfig:
        case C_Include:
          {
            if (Entry == C_SynthConfig)
            {
              cout << "Include synthesizer configuration file \"";
            }
            else
            {
              cout << "Include file \"";
            }
            cout << GetStrValue(Entry) << '"' << endl;

            // Get the name of the include file.
            wxString IncludeFileName = FindFile(GetStrValue(Entry));

            if (IncludeFileName.empty())
            {
              InputFileStreams.push(0);
            }
            else
            {
              pIs = new ifstream(IncludeFileName.mb_str());
              InputFileStreams.push(pIs);
            }

            if (!InputFileStreams.top())
            {
              wxString String;
              String
                << "Could not open configuration include file:" << '\n'
                << '"' << InputLine << '"';
              ::wxMessageBox(String, "Warning", wxOK);

              InputFileStreams.pop();
            }
          }
          break;

        default:
          break;
      }
    }
    else if (pVector && !InputLine.empty() && isdigit(InputLine[0]))
    {
      // Read named entries.

      // Voice names
      if (pVector == &mVoiceNames)
      {
        if (VoiceIndex >= 0 && VoiceIndex < mVoiceNames.size())
        {
          int Value;
          istringstream Iss(InputLine);
          Iss >> Value;

          if (gpConfig->GetValue(C_UseTwoCommandBankSelect))
          {
            assert(0 <= Value && Value <= 65536);
          }
          else
          {
            assert(0 <= Value && Value <= 32639);
          }

          mVoiceNames[VoiceIndex + 1].second = Value + 1;

          string VoiceName;
          getline(Iss, VoiceName);

          mVoiceNames[VoiceIndex + 1].first =
            TNStringUtilities::TrimLeadingAndTrailingBlanks(VoiceName);

          ++VoiceIndex;
        }
        else
        {
          cout
            << "Voice index \"" << VoiceIndex << "\" out of range."
            << endl;
        }
      }

      else if (pVector == &mDrumSets)
      {
        // Drumset names.

        if (DrumsetIndex >= 0 && DrumsetIndex < 129)
        {
          int Value;
          istringstream Iss(InputLine);
          Iss >> Value;
          if (Iss.fail())
          {
            cout << "Unable to read index." << endl;
          }

          if (gpConfig->GetValue(C_UseTwoCommandBankSelect))
          {
            assert(0 <= Value && Value <= 65536);
          }
          else
          {
            assert(0 <= Value && Value <= 32639);
          }
          mDrumSets[DrumsetIndex + 1].second = Value + 1;

          string SetName;
          getline(Iss, SetName);

          mDrumSets[DrumsetIndex + 1].first =
            TNStringUtilities::TrimLeadingAndTrailingBlanks(SetName);

          ++DrumsetIndex;
        }
        else
        {
          cout
            << "Drumset index \"" << DrumsetIndex << "\" out of range."
            << endl;
        }
      }
      else if (pVector == &mControlNames)
      {
        // Controller names.

        istringstream Iss(InputLine);
        Iss >> i;
        assert(0 <= i && i <= 127);

        string ControllerName;
        getline(Iss, ControllerName);

        mControlNames[i + 1].first =
          TNStringUtilities::TrimLeadingAndTrailingBlanks(ControllerName);
      }
      else if (pVector == &mDrumNames)
      {
        // Drum instrument names.

        istringstream Iss(InputLine);
        Iss >> i;
        assert(0 <= i && i <= 127);

        string DrumName;
        getline(Iss, DrumName);

        mDrumNames[i + 1].first =
          TNStringUtilities::TrimLeadingAndTrailingBlanks(DrumName);
      }
      else
      {
        wxString String;
        String
          << "LoadConfig: error reading line" << '\n'
          << '"' << InputLine << '"';
        ::wxMessageBox(String, "Warning", wxOK);
      }
    }
    else if (
      pVector == 0 &&
      !mBankTable.empty() &&
      !InputLine.empty() &&
      isdigit(InputLine[0]))
    {
      // Read bank table entries.
      assert(0 <= BankIndex && BankIndex < mBankTable.size());

      istringstream Iss(InputLine);
      Iss >> i >> j;

      assert(0 <= i && i <= 255);
      assert(0 <= j && j <= 255);

      mBankTable[BankIndex].Command[0] = i;
      mBankTable[BankIndex].Command[1] = j;

      ++BankIndex;
    }
  }

  // Return to the original working directory.
  ::wxSetWorkingDirectory(OriginalCurrentWorkingDirectory);
}
