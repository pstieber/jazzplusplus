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

#include <string>
#include <vector>

#include <wx/string.h>

//*****************************************************************************
//*****************************************************************************
class JZDoubleCommand
{
  public:
    int Command[2];
};

//*****************************************************************************
//*****************************************************************************
enum TEConfigurationNames
{
  C_Seq2Device = 0,
  C_MidiDriver,
  C_EnableAudio,
  C_WinInputDevice,
  C_WinOutputDevice,
  C_AlsaInputDevice,
  C_AlsaOutputDevice,
  C_AlsaAudioCard,
  C_AlsaAudioInputDevice,
  C_AlsaAudioOutputDevice,
  C_AlsaSyncInputDevice,
  C_AlsaSyncInputFormat,
  C_AlsaSyncOutput,
  C_AlsaSyncOutputFormat,
  C_AlsaSyncOutputDevice,
  C_SoftThru,
  C_HardThru,
  C_ClockSource,
  C_RealTimeOut,
  C_UseReverbMacro,
  C_UseChorusMacro,
  C_DrumChannel,
  C_BankControlNumber,
  C_BankControlNumber2,
  C_MaxBankTableEntries,
  C_PartsColumnsMax,
  C_PartsTracknamesRight,
  C_MaxVoiceNames,
  C_UseTwoCommandBankSelect,
  C_MetroIsAccented,
  C_MetroVelocity,
  C_MetroNormalClick,
  C_MetroAccentedClick,
  C_TrackWinXpos,
  C_TrackWinYpos,
  C_TrackWinWidth,
  C_TrackWinHeight,
  C_PianoWinXpos,
  C_PianoWinYpos,
  C_PianoWinWidth,
  C_PianoWinHeight,
  C_PartsDlgXpos,
  C_PartsDlgYpos,
  C_TrackDlgXpos,
  C_TrackDlgYpos,
  C_HarmonyXpos,
  C_HarmonyYpos,
  C_RhythmXpos,
  C_RhythmYpos,
  C_SynthConfig,
  C_SynthType,
  C_SendSynthReset,
  C_Include,
  C_BankTable,
  C_VoiceNames,
  C_DrumSets,
  C_CtrlNames,
  C_DrumNames,
  C_StartUpSong,
  C_OssBug1,
  C_OssBug2,
  C_SynthDialog,
  C_DuplexAudio,
  C_ThruInput,
  C_ThruOutput,
  C_EnableWelcome,
  NumConfigNames
};

//*****************************************************************************
//*****************************************************************************
enum TEConfigEntryType
{
  eConfigEntryTypeInt = 0,
  eConfigEntryTypeStr,
  eConfigEntryTypeEmpty
};

//*****************************************************************************
// Description:
//   These are the values associated with the C_MidiDriver entry.
//*****************************************************************************
enum TEMidiDriver
{
  eMidiDriverJazz = 0, // C_DRV_JAZZ  0
  eMidiDriverOss = 1,  // C_DRV_OSS   1
  eMidiDriverAlsa = 2  // C_DRV_ALSA  2
};

//*****************************************************************************
// Description:
//   This is the configuration entry class declaration.
//*****************************************************************************
class JZConfigurationEntry
{
  public:

    JZConfigurationEntry(const std::string& Name, int IntegerValue);

    JZConfigurationEntry(
      const std::string& Name,
      const std::string& StringValue);

    JZConfigurationEntry(const std::string& Name);

    TEConfigEntryType GetType() const;

    const std::string& GetName() const;

    int GetValue() const;

    void SetValue(const int& Value);

    const std::string& GetStrValue() const;

    void SetStringValue(const std::string& StringValue);

  private:

    TEConfigEntryType mType;
    std::string mName;
    int mValue;
    std::string mStringValue;
};

//*****************************************************************************
// Description:
//   These are the configuration entry class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
TEConfigEntryType JZConfigurationEntry::GetType() const
{
  return mType;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::string& JZConfigurationEntry::GetName() const
{
  return mName;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
int JZConfigurationEntry::GetValue() const
{
  return mValue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
void JZConfigurationEntry::SetValue(const int& Value)
{
  mValue = Value;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::string& JZConfigurationEntry::GetStrValue() const
{
  return mStringValue;
}

//*****************************************************************************
//*****************************************************************************
class JZConfiguration
{
  public:

    JZConfiguration();

    ~JZConfiguration();

    void LoadConfig(const wxString& FileName);

    int Check(const std::string& Name) const;

    int Load(const std::string& buf);

    const std::pair<std::string, int>& GetDrumName(unsigned Entry) const;
    const std::pair<std::string, int>& GetDrumSet(unsigned Entry) const;
    const std::pair<std::string, int>& GetVoiceName(unsigned Entry) const;
    const std::pair<std::string, int>& GetCtrlName(unsigned Entry) const;

    JZDoubleCommand& BankEntry(unsigned Entry);

    const std::string& GetName(int Entry) const;

    const std::string& GetStrValue(int Entry) const;

    int GetValue(const char* pName) const;
    int GetValue(int Index) const;

    bool Get(int Entry, std::string& Value);
    bool Get(int Entry, int& Value);

    bool Put(int Entry, const std::string& ValueString);
    bool Put(int Entry);
    bool Put(int Entry, int Value);

    const std::vector<std::pair<std::string, int> >& GetDrumSets() const;

    const std::vector<std::pair<std::string, int> >& GetDrumNames() const;

    const std::vector<std::pair<std::string, int> >& GetControlNames() const;

    const std::vector<std::pair<std::string, int> >& GetVoiceNames() const;

  private:

    // Description:
    //   Return the Jazz++ configuration file name, normally jazz.cfg.
    wxString GetFileName();

  private:

    wxString mFileName;

    JZConfigurationEntry* mNames[NumConfigNames];

    std::vector<std::pair<std::string, int> > mDrumNames;

    std::vector<std::pair<std::string, int> > mDrumSets;

    std::vector<std::pair<std::string, int> > mControlNames;

    std::vector<std::pair<std::string, int> > mVoiceNames;

    std::vector<JZDoubleCommand> mBankTable;
};

//*****************************************************************************
// Description:
//   These are the configuration class inline member functions.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::string& JZConfiguration::GetName(int Entry) const
{
  assert((Entry >= 0) && (Entry < NumConfigNames));
  return mNames[Entry]->GetName();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::string& JZConfiguration::GetStrValue(int Entry) const
{
  assert((Entry >= 0) && (Entry < NumConfigNames));
  return mNames[Entry]->GetStrValue();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::vector<std::pair<std::string, int> >&
JZConfiguration::GetDrumSets() const
{
  return mDrumSets;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::vector<std::pair<std::string, int> >&
JZConfiguration::GetDrumNames() const
{
  return mDrumNames;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::vector<std::pair<std::string, int> >&
JZConfiguration::GetControlNames() const
{
  return mControlNames;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline
const std::vector<std::pair<std::string, int> >&
JZConfiguration::GetVoiceNames() const
{
  return mVoiceNames;
}
