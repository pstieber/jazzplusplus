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

#include "Audio.h"

#include "Dialogs/AudioSettingsDialog.h"
#include "Dialogs/SamplesDialog.h"
#include "Events.h"
#include "FileSelector.h"
#include "FindFile.h"
#include "Globals.h"
#include "Help.h"
#include "Player.h"
#include "Random.h"
#include "RecordingInfo.h"
#include "Resources.h"
#include "Sample.h"
#include "SampleFrame.h"
#include "StringReadWrite.h"
#include "Track.h"
#include "TrackFrame.h"
#include "TrackWindow.h"

#include <wx/filename.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/slider.h>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/stat.h>
#include <string.h>

using namespace std;

//*****************************************************************************
// Description:
//   This is the sample voice class declaration.  This class is activated via a
// MIDI note on signal.  The class copies data from a JZSample object to the
// output buffer as needed by the driver.
//*****************************************************************************
class JZSampleVoice
{
  public:

    JZSampleVoice(JZSampleSet& s)
      : set(s)
    {
    }

    void Start(JZSample* s, long c)
    {
      spl    = s;
      clock  = c;
      first  = 1;
      length = spl->length;
      mpData = spl->mpData;
      prev   = 0;
    }

    void AddBuffer(short* b, long buffer_clock, unsigned int bufsize)
    {

      // everything done?
      if (length <= 0)
        return;

      // not this buffer
      if (buffer_clock + set.GetClocksPerBuffer() <= clock)
        return;

      // compute offset in buffer
      long offset = 0;
      if (first)
      {
        if (clock >= buffer_clock)
        {
          offset = set.Ticks2Samples(clock - buffer_clock);
        }
        else
        {
          // output starts somewhere in the middle of a sample
          long data_offs = set.Ticks2Samples(buffer_clock - clock);
          mpData += data_offs;
          length -= data_offs;
          if (length <= 0)
            return;
        }
        first = 0;
      }

      // compute number of samples to put into this buffer
      int count = bufsize - offset;
      if (count > length)
        count = length;

      // update length and copy data
      length -= count;
      b += offset;
      while (count--)
      {
        *b++ += *mpData++;
      }
    }

    void AddListen(short* b, long fr_smpl, long to_smpl, unsigned int bufsize)
    {
      // Is everything done?
      if (length <= 0)
      {
        return;
      }

      if (first)
      {

        if (to_smpl > 0 && to_smpl < length)
        {
          length = to_smpl;
        }

        if (fr_smpl > 0 && fr_smpl < length)
        {
          mpData += fr_smpl;
          length -= fr_smpl;
        }
        first = false;
        if (length <= 0)
        {
          return;
        }
      }

      int count = bufsize;
      if (count > length)
      {
        count = length;
      }

      // update length and copy data
      length -= count;
      while (count--)
      {
        *b++ += *mpData++;
      }
    }

    int Finished()
    {
      return length <= 0;
    }

  private:
    JZSampleSet& set;
    long         clock;
    JZSample*    spl;
    short*       mpData;
    int          first;
    long         length;
    short        prev;
};

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSampleSet::JZSampleSet(long TicksPerMinute)
  : mSamplingRate(22050),
    mChannelCount(1),

    // Dont change!!!
    mBitsPerSample(16),

    mSoftwareSynchonization(true),
    mTicksPerMinute(TicksPerMinute),
    mpSamplesDialog(nullptr),
    mDefaultFileName("noname.spl"),
    mRecordFileName("noname.wav")
{
  int i;
  for (i = 0; i < BUFCOUNT; ++i)
  {
    mpBuffers[i] = new JZAudioBuffer(0);
  }

  adjust_audio_length = 1;
  has_changed  = false;
  is_playing   = 0;
  dirty        = 0;

  for (i = 0; i < eSampleCount; i++)
  {
    mSamples[i] = new JZSample(*this);
    mSampleFrames[i] = 0;
  }

  for (i = 0; i < MAXPOLY; i++)
  {
    voices[i] = new JZSampleVoice(*this);
  }
  num_voices = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSampleSet::~JZSampleSet()
{
  int i;
  for (i = 0; i < eSampleCount; i++)
  {
    delete mSamples[i];
    delete mSampleFrames[i];
  }
  for (i = 0; i < MAXPOLY; i++)
  {
    delete voices[i];
  }
  for (i = 0; i < BUFCOUNT; i++)
  {
    delete mpBuffers[i];
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::Edit(int key)
{
  if (mSampleFrames[key] == 0)
  {
    JZSample* spl = mSamples[key];

    mSampleFrames[key] = new JZSampleFrame(
      gpTrackWindow,
      &mSampleFrames[key],
      *spl);
  }
  mSampleFrames[key]->Show(true);
  mSampleFrames[key]->Redraw();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::LoadDefaultSettings()
{
  wxString FileName = FindFile("jazz.spl");
  if (!FileName.empty())
  {
    Load(FileName);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleSet::Load(const wxString& FileName)
{
  // Enable audio when loading a sample set.
  gpMidiPlayer->SetAudioEnabled(true);

  wxBeginBusyCursor();
  for (int i = 0; i < eSampleCount; i++)
  {
    mSamples[i]->Clear();
  }

  // Get the path of the sample file.
  wxString SplFilePath = ::wxPathOnly(FileName);

  ifstream Is(FileName.mb_str());
  int Version;
  Is >> Version >> mSamplingRate >> mChannelCount >> mSoftwareSynchonization;
  while (Is && !Is.eof())
  {
    int key, pan, vol, pitch;
    wxFileName SampleFileName;
    Is >> key;
    if (Is.fail())
    {
      break;
    }
    string FileNameString;
    ReadString(Is, FileNameString);
    SampleFileName.Assign(FileNameString);
    string Label;
    ReadString(Is, Label);
    Is >> pan >> vol >> pitch;
    if (!SampleFileName.FileExists())
    {
      // Try to prepend the sample file path.
      wxString PathAndFileName;
      PathAndFileName =
        SplFilePath +
        wxFileName::GetPathSeparator() +
        SampleFileName.GetFullPath();
      SampleFileName = PathAndFileName;
    }
    if (!SampleFileName.FileExists())
    {
      wxString String;
      String = "File not found: \"" + SampleFileName.GetFullPath() + '"';
      ::wxMessageBox(String, "Error", wxOK);
      continue;
    }
    assert(0 <= key && key < eSampleCount);
    mSamples[key]->SetFileName(SampleFileName.GetFullPath());
    mSamples[key]->SetLabel(Label.c_str());
    mSamples[key]->SetVolume(vol);
    mSamples[key]->SetPan(pan);
    mSamples[key]->SetPitch(pitch);
    if (mSamples[key]->Load())
    {
      wxString String;
      String
        << "Could not load: \""
        << mSamples[key]->GetFileName()
        << '"';
      ::wxMessageBox(String, "Error", wxOK);
    }
    if (mSampleFrames[key])
    {
      mSampleFrames[key]->Redraw();
    }
  }
  wxEndBusyCursor();
  dirty = 0;
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::ReloadSamples()
{
  for (int i = 0; i < eSampleCount; i++)
  {
    mSamples[i]->Load(dirty);
  }
  dirty = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleSet::Save(const wxString& FileName)
{
  ofstream Ofs(FileName.mb_str());
  Ofs
    << 1
    << ' ' << mSamplingRate
    << ' ' << mChannelCount
    << ' ' << mSoftwareSynchonization
    << endl;
  for (int i = 0; i < eSampleCount; i++)
  {
    JZSample* pSample = mSamples[i];
    const string& FileName = pSample->GetFileName();
    int vol = pSample->GetVolume();
    int pan = pSample->GetPan();
    int pitch = pSample->GetPitch();
    if (!FileName.empty())
    {
      Ofs << i << ' ';
      WriteString(Ofs, FileName);
      Ofs << ' ';
      WriteString(Ofs, pSample->GetLabel());
      Ofs << ' ' << pan << ' ' << vol << ' ' << pitch << endl;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const string& JZSampleSet::GetSampleLabel(int Index)
{
  if (Index >= 0 && Index < eSampleCount)
  {
    return mSamples[Index]->GetLabel();
  }
  static string EmptyString;
  return EmptyString;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleSet::ResetBuffers(
  JZEventArray* evnt_arr,
  long clock,
  long TicksPerMinute)
{
  int i;
  mFreeBuffers.Clear();
  mFullBuffers.Clear();
  mDriverBuffers.Clear();
  for (i = 0; i < BUFCOUNT; i++)
  {
    mFreeBuffers.Put(mpBuffers[i]);
  }
  buffers_written   = 0;

  events            = evnt_arr;
  start_clock       = clock;
  mTicksPerMinute  = TicksPerMinute;
  event_index       = 0;
  bufshorts = BUFSHORTS;
  mClocksPerBuffer = Samples2Ticks(bufshorts);
  num_voices        = 0;
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleSet::ResetBufferSize(unsigned int bufsize)
{
  if (bufsize == 0 || bufsize > BUFBYTES || (bufsize & 1))
  {
    cerr << "invalid buffer size " << bufsize << '\n';
    return 1;
  }
  bufshorts = bufsize / 2;
  mClocksPerBuffer = Samples2Ticks(bufshorts);
  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleSet::FillBuffers(long last_clock)
{
  // check if last_clock is bigger than free buffer space
  // and compute the count of buffers that can be filled
  int i;

  int nfree = mFreeBuffers.Count();
  if (nfree <= 0)
  {
    return 0;
  }

  long max_buffer_clock = BufferClock(buffers_written + nfree);

  if (max_buffer_clock <= last_clock)
  {
    last_clock = max_buffer_clock;
  }
  else
  {
    nfree = (int)(
      (last_clock - start_clock) / mClocksPerBuffer) - buffers_written;
  }

  if (nfree <= 0)
  {
    return 0;
  }

  // iterate the events and add sounding voices
  while (event_index < events->mEventCount)
  {
    JZEvent* pEvent = events->mppEvents[event_index];
    if (pEvent->GetClock() >= last_clock)
    {
      break;
    }
    event_index++;

    JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
    if (pKeyOn && num_voices < MAXPOLY)
    {
      voices[num_voices++]->Start(
        mSamples[pKeyOn->GetKey()],
        pKeyOn->GetClock());
    }
  }

  // add remaining sample data to the buffers
  for (i = 0; i < nfree; i++)
  {
    JZAudioBuffer* buf = mFreeBuffers.Get();
    buf->Clear();
    long buffer_clock = BufferClock(buffers_written + i);

//    cout
//      << "write " << (buffers_written + i) % buffer_count
//      << ", clock " << buffer_clock
//      << endl;

    for (int k = 0; k < num_voices; k++)
    {
      voices[k]->AddBuffer(buf->data, buffer_clock, bufshorts);
    }
    mFullBuffers.Put(buf);
  }

  // delete finished voices
  for (i = 0; i < num_voices; i++)
  {
    if (voices[i]->Finished())
    {
      JZSampleVoice* v = voices[i];
      voices[i] = voices[num_voices-1];
      voices[num_voices-1] = v;
      num_voices--;
    }
  }

  // all buffer filled up?
  buffers_written += nfree;

  return nfree;
}

//-----------------------------------------------------------------------------
//   Returns the number of buffers containing sound.  Fills as many buffers as
// possible, the last buffers may contain silence only.
//-----------------------------------------------------------------------------
int JZSampleSet::PrepareListen(JZSample* spl, long fr_smpl, long to_smpl)
{
  listen_sample = spl;

  assert(mTicksPerMinute);
  ResetBuffers(0, 0, mTicksPerMinute);
  voices[0]->Start(spl, 0);
  int nfree = mFreeBuffers.Count();
  int sound_buffers = 0;

  for (int i = 0; i < nfree; i++)
  {
    JZAudioBuffer* buf = mFreeBuffers.Get();
    buf->Clear();
    if (!voices[0]->Finished())
    {
      voices[0]->AddListen(buf->Data(), fr_smpl, to_smpl, bufshorts);
      sound_buffers++;
    }
    mFullBuffers.Put(buf);
  }
  buffers_written = nfree;
  return sound_buffers;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleSet::PrepareListen(int key, long fr_smpl, long to_smpl)
{
  JZSample* spl = mSamples[key];
  return PrepareListen(spl, fr_smpl, to_smpl);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSampleSet::ContinueListen()
{
  int nfree = mFreeBuffers.Count();
  int sound_buffers = 0;

  for (int i = 0; i < nfree; i++)
  {
    JZAudioBuffer* buf = mFreeBuffers.Get();
    buf->Clear();
    if (!voices[0]->Finished())
    {
      voices[0]->AddListen(buf->Data(), -1, -1, bufshorts);
      sound_buffers++;
    }
    mFullBuffers.Put(buf);
  }
  buffers_written += nfree;
  return sound_buffers;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::AdjustAudioLength(JZTrack* pTrack, long TicksPerMinute)
{
  if (!pTrack->GetAudioMode() || !adjust_audio_length)
  {
    return;
  }

  mTicksPerMinute = TicksPerMinute;

  JZEventIterator it(pTrack);
  JZEvent* pEvent = it.First();
  while (pEvent)
  {
    JZKeyOnEvent* pKeyOn = pEvent->IsKeyOn();
    if (pKeyOn)
    {
      pKeyOn->SetLength(
        (int)Samples2Ticks(mSamples[pKeyOn->GetKey()]->GetLength()));

      // Is the event visble?
      if (pKeyOn->GetEventLength() < 15)
      {
        pKeyOn->SetLength(15);
      }
    }
    pEvent = it.Next();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::StartPlay(int clock)
{
  ReloadSamples();

  // touch all playback sample data, so they may get swapped into memory
  for (int i = 0; i < eSampleCount; i++)
  {
    JZSample* spl = mSamples[i];
    spl->GotoRAM();
  }

  is_playing = 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::StopPlay()
{
  is_playing = 0;
}

#if 0
//*****************************************************************************
// Description:
//   This is the sample dialog.
//*****************************************************************************
class JZSamplesDlg : public wxDialog
{
  friend class JZSampleSet;

  public:

    JZSamplesDlg(wxWindow* pParent, JZSampleSet& SampleSet);

    ~JZSamplesDlg();

#ifdef OBSOLETE
    static void CloseButton(wxItem& item, wxCommandEvent& event);
    static void PlayButton(wxItem& item, wxCommandEvent& event);
    static void EditButton(wxItem& item, wxCommandEvent& event);
    static void AddButton(wxItem& item, wxCommandEvent& event);
    static void ClrButton(wxItem& item, wxCommandEvent& event);
    static void HelpButton(wxItem& item, wxCommandEvent& event);
    static void ListClick(wxItem& item, wxCommandEvent& event);
#endif // OBSOLETE

    void OnCloseButton();
    void OnPlayButton();
    void OnEditButton();
    void OnAddButton();
    void OnClrButton();
    void OnHelpButton();
    void OnListClick();

  private:

    JZSampleSet& set;

    wxListBox* mpListBox;
    wxSlider* mpPanSlider;
    wxSlider* mpVolumeSlider;
    wxSlider* mpPitchSlider;
#ifdef OBSOLETE
    wxText* pLabel;
    wxText* pFile;
#endif // OBSOLETE

    static wxString mSamplePath;
    static int  current;

    std::string ListEntry(int i);
    void Sample2Win(int index);
    void Win2Sample(int index);
    void SetCurrentListEntry(int i);
};
#endif

#ifdef OBSOLETE

class JZAudioGloblForm : public wxForm
{
  public:
    JZAudioGloblForm(JZSampleSet& s)
    : wxForm( USED_WXFORM_BUTTONS ),
      mSampleSet(s)
    {

      ossbug1      = gpConfig->GetValue(C_OssBug1);
      ossbug2      = gpConfig->GetValue(C_OssBug2);
      duplex_audio = gpConfig->GetValue(C_DuplexAudio);

      static const char* speedtxt[] =
      {
        "8000",
        "11025",
        "22050",
        "44100",
        0
      };
      speed = mSampleSet.GetSamplingRate();
      for (int i = 0; speedtxt[i]; i++)
      {
        strlist.Append((wxObject*)speedtxt[i]);  // ???
        if (atol(speedtxt[i]) == speed)
        {
          mSpeedString = speedtxt[i];
        }
      }
      if (mSpeedString.empty())
      {
        mSpeedString = speedtxt[0];
      }

      enable = gpMidiPlayer->GetAudioEnabled();
      stereo = (mSampleSet.GetChannelCount() == 2);
      mSoftwareSynchonization = mSampleSet.GetSoftSync();

      Add(wxMakeFormBool("Enable Audio", &enable));
      Add(wxMakeFormNewLine());
      //Add(wxMakeFormString("Sample Freq", (char**)&mSpeedString, wxFORM_CHOICE,
      Add(wxMakeFormString("Sample Freq", (char**)&mSpeedString.c_str(), wxFORM_DEFAULT,
          new wxList(wxMakeConstraintStrings(&strlist), 0), NULL, wxHORIZONTAL));
      Add(wxMakeFormNewLine());
      Add(wxMakeFormBool("Stereo", &stereo));
      Add(wxMakeFormNewLine());
      Add(wxMakeFormBool("Software Midi/Audio Sync", &mSoftwareSynchonization));

      #ifdef wx_x
      Add(wxMakeFormNewLine());
      Add(wxMakeFormBool("OSS 'start audio' bug workaround", &ossbug1));
      //Add(wxMakeFormNewLine());
      //Add(wxMakeFormBool("OSS Bug workaround (2)", &ossbug2));
      #endif

      Add(wxMakeFormNewLine());
      Add(wxMakeFormBool("Duplex Play/Record", &duplex_audio));
    }

    void OnHelp()
    {
      JZHelp::Instance().ShowTopic("Global Settings");
    }

    void OnOk()
    {
      if (mSampleSet.is_playing)
        return;
      wxBeginBusyCursor();
      mSampleSet.mpGlobalSettingsDialog = 0;
      istringstream Iss(mSpeedString);
      Iss >> speed;
      mSampleSet.SetSamplingRate(speed);
      mSampleSet.SetChannelCount(stereo ? 2 : 1);
      mSampleSet.SetSoftSync(mSoftwareSynchonization);
      gpMidiPlayer->SetAudioEnabled(enable);

      if (gpConfig->GetValue(C_EnableAudio) != enable)
      {
//        Config(C_EnableAudio) = enable;
        Config.Put(C_EnableAudio, enable);
      }

      if (gpConfig->GetValue(C_OssBug1) != ossbug1)
      {
//        Config(C_OssBug1) = ossbug1;
        Config.Put(C_OssBug1, ossbug1);
      }

      if (gpConfig->GetValue(C_OssBug2) != ossbug2)
      {
//        Config(C_OssBug2) = ossbug2;
        Config.Put(C_OssBug2, ossbug2);
      }

      if (gpConfig->GetValue(C_DuplexAudio) != duplex_audio)
      {
        Config(C_DuplexAudio) = duplex_audio;
        Config.Put(C_DuplexAudio, duplex_audio);
      }

      if (enable)
        mSampleSet.ReloadSamples();
      wxEndBusyCursor();
      wxForm::OnOk();
    }
    void OnCancel()
    {
      mSampleSet.mpGlobalSettingsDialog = 0;
      wxForm::OnCancel();
    }
  private:
    JZSampleSet& mSampleSet;
    wxList  strlist;

    long speed;
    const std::string mSpeedString;
    bool enable;
    bool stereo;
    bool mSoftwareSynchonization;
    bool ossbug1;
    bool ossbug2;
    bool duplex_audio;
};

#endif // OBSOLETE

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::EditAudioGlobalSettings(wxWindow* pParent)
{
  if (mpSamplesDialog->IsShown())
  {
    mpSamplesDialog->SetFocus();
    return;
  }

  JZAudioSettingsDialog AudioSettingsDialog(pParent, *this);
  AudioSettingsDialog.ShowModal();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::EditAudioSamples(wxWindow* pParent)
{
  SamplesDlg();
}

//-----------------------------------------------------------------------------
// case ID_AUDIO_LOAD_SAMPLE_SET:
//-----------------------------------------------------------------------------
void JZSampleSet::LoadSampleSet(wxWindow* pParent)
{
  wxString FileName = file_selector(
    mDefaultFileName,
    "Load Sample Set",
    false,
    has_changed,
    "*.spl");
  if (!FileName.empty())
  {
    Load(FileName);
  }
}

//-----------------------------------------------------------------------------
// case ID_AUDIO_SAVE_SAMPLE_SET_AS:
//-----------------------------------------------------------------------------
void JZSampleSet::SaveSampleSetAs(wxWindow* pParent)
{
  wxString FileName = file_selector(
    mDefaultFileName,
    "Save Sample Set",
    true,
    has_changed,
    "*.spl");
  if (!FileName.empty())
  {
    Save(FileName);
  }
}

//-----------------------------------------------------------------------------
// case ID_AUDIO_SAVE_SAMPLE_SET:
//-----------------------------------------------------------------------------
void JZSampleSet::SaveSampleSet(wxWindow* pParent)
{
  if (mDefaultFileName == "noname.spl")
  {
    return SaveSampleSetAs(pParent);
  }
  Save(mDefaultFileName);
}

//-----------------------------------------------------------------------------
// case ID_AUDIO_NEW_SAMPLE_SET:
//-----------------------------------------------------------------------------
void JZSampleSet::ClearSampleSet(wxWindow* pParent)
{
  if (mpSamplesDialog == 0)
  {
    if (wxMessageBox("Clear Sample Set?", "Confirm", wxYES_NO) == wxNO)
    {
      return;
    }
    for (int i = 0; i < eSampleCount; ++i)
    {
      mSamples[i]->Clear();
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::SaveRecordingDlg(
  long frc,
  long toc,
  JZAudioRecordBuffer& buf)
{
  if (frc >= toc)
  {
    return;
  }

  if (buf.num_buffers == 0)
  {
    return;
  }

  int Choice = ::wxMessageBox(
    "Save audio recording ?",
    "Save?",
    wxOK | wxCANCEL);

  if (Choice == wxOK)
  {
    wxString FileName = file_selector(
      mRecordFileName,
      "Save Audio Recording",
      true,
      false,
      "*.wav");
    if (!FileName.empty())
    {
      wxBeginBusyCursor();
      SaveWave(FileName, frc, toc, buf);
      AddNote(FileName, frc, toc);
      wxEndBusyCursor();
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::AddNote(const string& FileName, long frc, long toc)
{
  int i;
  JZSample* spl;

  // See if the file name is already present in sample list.
  for (i = 0; i < eSampleCount; i++)
  {
    spl = mSamples[i];
    if (spl->GetFileName() == FileName)
    {
      break;
    }
  }

  // if no entry is there, add an entry
  if (i >= eSampleCount)
  {
    // start somewhere near the top of the list
    for (i = 15; i < eSampleCount; i++)
    {
      spl = mSamples[i];
      if (spl->GetFileName()[0] == 0)
      {
        break;
      }
    }
  }

  if (i >= eSampleCount)
    return;

  int key = i;
  spl->Clear();   // reset everything to defaults
  spl->SetFileName(FileName);
  spl->SetLabel(wxFileNameFromPath(FileName));
  spl->Load();   // reload data

  // delete selection
#ifdef OBSOLETE
  JZSong* pSong = gpProject->mpSong;
#endif
  const JZRecordingInfo* info = gpProject->GetRecInfo();
  JZTrack* track = info->mpTrack;
#ifdef OBSOLETE
  pSong->NewUndoBuffer();
#endif
  JZEventIterator iter(info->mpTrack);
  JZEvent* pEvent = iter.Range(frc, toc);
  while (pEvent != nullptr)
  {
    track->Kill(pEvent);
    pEvent = iter.Next();
  }
  // add a noteon
  JZKeyOnEvent* pKeyOn = new JZKeyOnEvent(
    frc,
    track->mChannel - 1,
    key,
    64,
    (unsigned short)(toc - frc));
  track->Put(pKeyOn);
  track->Cleanup();

  // repaint trackwin
//  gpTrackWindow->Redraw();
  gpTrackWindow->Update();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::SaveWave(
  const wxString& FileName,
  long frc,
  long toc,
  JZAudioRecordBuffer& buf)
{
  WaveHeader wh;
  wh.main_chunk = RIFF;
  wh.chunk_type = WAVE;
  wh.sub_chunk  = FMT;
  wh.data_chunk = DATA;
  wh.format     = PCM_CODE;
  wh.modus      = mChannelCount;
  wh.sc_len     = 16;
  wh.sample_fq  = mSamplingRate;
  wh.bit_p_spl  = mBitsPerSample;
  wh.byte_p_spl = mChannelCount * (mBitsPerSample > 8 ? 2 : 1);
  wh.byte_p_sec = wh.byte_p_spl * wh.sample_fq;


  long start_index = Ticks2Samples(frc - start_clock);
  long end_index   = Ticks2Samples(toc - start_clock);

  int bufsize = buf.bufbytes / 2;

  // recording aborted?
  if (end_index > buf.num_buffers * bufsize)
  {
    end_index = buf.num_buffers * bufsize;
  }

  wh.data_length   = (end_index - start_index) * sizeof(short);
  wh.length        = wh.data_length + sizeof(WaveHeader);

  ofstream os(FileName.mb_str(), ios::out | ios::binary | ios::trunc);

  os.write((char*)&wh, sizeof(wh));

  int start_buffer = start_index / bufsize;
  int start_offs   = start_index % bufsize;
  int start_length = bufsize - start_offs;
  int end_buffer   = end_index / bufsize;
  int end_length   = end_index % bufsize;

  // Save part of first buffer.
  os.write(
    (char*)&buf.mBuffers[start_buffer]->data[start_offs],
    2 * start_length);

  // write some complete buffers
  for (int i = start_buffer + 1; i < end_buffer; i++)
    os.write((char*)buf.mBuffers[i]->data, bufsize * 2);
  // save part of last buffer
  if (end_length > 0)
    os.write((char*)buf.mBuffers[end_buffer]->data, 2 * end_length);
#if 0
  // very slow, but works!
  ofstream slow("t2.wav", ios::out | ios::bin | ios::trunc);
  slow.write((char*)&wh, sizeof(wh));
  for (long i = start_index; i < end_index; i++)
  {
    int bi = i / bufsize;
    int di = i % bufsize;
    slow.write((char*)&buf.mBuffers[bi]->mpData[di], sizeof(short));
  }
#endif
}

// -----------------------------------------------------------------
// ------------------------------- record  ------------------------
// -----------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAudioRecordBuffer::Clear()
{
  int n = mBuffers.GetSize();
  for (int i = 0; i < n; i++)
  {
    delete mBuffers[i];
    mBuffers[i] = 0;
  }
  num_buffers = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZAudioBuffer* JZAudioRecordBuffer::RequestBuffer()
{
  if (mBuffers[num_buffers] == 0)
    mBuffers[num_buffers] = new JZAudioBuffer(0);
  if (mBuffers[num_buffers] == 0)
  {
    Clear();
    fprintf(stderr, "memory exhausted!\n");
  }
  return mBuffers[num_buffers++];
}

// -----------------------------------------------------------------
// ------------------------ sample settings ------------------------
// -----------------------------------------------------------------

#if 0
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
wxString JZSamplesDlg::mSamplePath;
int   JZSamplesDlg::current = 0;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSamplesDlg::JZSamplesDlg(wxWindow* pParent, JZSampleSet& s)
  : wxDialog(pParent, wxID_ANY, wxString("Sample Settings")),
    set(s)
{
  if (mSamplePath.empty())
  {
    mSamplePath = "*.wav";
  }

  wxArrayString SampleNames;
  for (int i = 0; i < JZSampleSet::eSampleCount; ++i)
  {
    SampleNames.Add(ListEntry(i));
  }

  // buttons
#ifdef OBSOLETE
  new wxButton(this, (wxFunction)CloseButton, "Close") ;
  new wxButton(this, (wxFunction)PlayButton,  "Play") ;
  new wxButton(this, (wxFunction)EditButton,  "Edit") ;
  new wxButton(this, (wxFunction)AddButton,   "Add") ;
  new wxButton(this, (wxFunction)ClrButton,   "Clear") ;
  new wxButton(this, (wxFunction)HelpButton,  "Help") ;
  NewLine();
#endif

#ifdef OBSOLETE
  // list box
  int y = 80;
  SetLabelPosition(wxVERTICAL);
#endif // OBSOLETE

  mpListBox = new wxListBox(
    this,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize,
    SampleNames,
    wxLB_SINGLE);

#ifdef OBSOLETE
  SetLabelPosition(wxHORIZONTAL);
#endif // OBSOLETE

  // Sliders
  mpVolumeSlider = new wxSlider(this, wxID_ANY, 64,  0, 127);
  mpPanSlider = new wxSlider(this, wxID_ANY, 0, -63, 63);
  mpPitchSlider = new wxSlider(this, wxID_ANY, 0, -12, 12);
#ifdef OBSOLETE
  pLabel = new wxText(this, (wxFunction)0, "Label", "", -1, -1, 300);
  file = new wxText(this, (wxFunction)0, "File", "", -1, -1, 300);
#endif // OBSOLETE
  Fit();
  Sample2Win(current);
  mpListBox->SetSelection(current);
  Show(true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
string JZSamplesDlg::ListEntry(int i)
{
  ostringstream Oss;
  Oss << i + 1 << ' ' << set.mSamples[i]->GetLabel();
//  KeyToString(i, buf + strlen(buf));
  return Oss.str();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::Sample2Win(int i)
{
  JZSample* spl = set.mSamples[i];
  mpVolumeSlider->SetValue(spl->GetVolume());
  mpPitchSlider->SetValue(spl->GetPitch());
  mpPanSlider->SetValue(spl->GetPan());
#ifdef OBSOLETE
  pLabel->SetValue((char*)spl->GetLabel());
  file->SetValue((char*)spl->GetFileName());
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::Win2Sample(int i)
{
  JZSample* spl = set.mSamples[i];
  spl->SetPitch(mpPitchSlider->GetValue());
  spl->SetVolume(mpVolumeSlider->GetValue());
  spl->SetPan(mpPanSlider->GetValue());
#ifdef OBSOLETE
  spl->SetLabel(pLabel->GetValue());
  spl->SetFileName(file->GetValue());
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::SetCurrentListEntry(int i)
{
  if (i >= 0)
  {
    mpListBox->SetString(current, ListEntry(i));
    mpListBox->SetSelection(i, true);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZSamplesDlg::~JZSamplesDlg()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::OnCloseButton()
{
  if (set.is_playing)
  {
    return;
  }
  Win2Sample(current);
  wxBeginBusyCursor();
  set.ReloadSamples();
  wxEndBusyCursor();
  set.mpSamplesDialog = 0;
//  DELETE_THIS();
  Destroy();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::OnAddButton()
{
  wxString FileName = file_selector(
    mSamplePath,
    "Load Sample",
    false,
    false,
    "*.wav");

  if (FileName)
  {
#ifdef OBSOLETE
    file->SetValue(FileName);
    pLabel->SetValue(wxFileNameFromPath(FileName));
#endif
    Win2Sample(current);
    SetCurrentListEntry(current);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::OnEditButton()
{
  wxBeginBusyCursor();
  Win2Sample(current);
  SetCurrentListEntry(current);
  JZSample* spl = set.mSamples[current];
  spl->Load();
  wxEndBusyCursor();

  set.Edit(current);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::OnPlayButton()
{
  if (set.is_playing)
  {
    return;
  }
  if (gpMidiPlayer->IsListening())
  {
    gpMidiPlayer->ListenAudio(-1);
    return;
  }
  Win2Sample(current);
  SetCurrentListEntry(current);
  JZSample* spl = set.mSamples[current];
  wxBeginBusyCursor();
  spl->Load();
  gpMidiPlayer->ListenAudio(current);
  wxEndBusyCursor();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::OnClrButton()
{
  JZSample* spl = set.mSamples[current];
  spl->Clear();
  SetCurrentListEntry(current);
  Sample2Win(current);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::OnHelpButton()
{
  JZHelp::Instance().ShowTopic("Sample Settings");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSamplesDlg::OnListClick()
{
  Win2Sample(current);
  int i = mpListBox->GetSelection();
  if (i >= 0)
  {
    current = i;
    SetCurrentListEntry(i);
    Sample2Win(current);
  }
}

#ifdef OBSOLETE

void JZSamplesDlg::CloseButton(wxItem& itm, wxCommandEvent& event)
{
  ((JZSamplesDlg*)itm.GetParent())->OnCloseButton();
}
void JZSamplesDlg::PlayButton(wxItem& itm, wxCommandEvent& event)
{
  ((JZSamplesDlg*)itm.GetParent())->OnPlayButton();
}
void JZSamplesDlg::EditButton(wxItem& itm, wxCommandEvent& event)
{
  ((JZSamplesDlg*)itm.GetParent())->OnEditButton();
}
void JZSamplesDlg::AddButton(wxItem& itm, wxCommandEvent& event)
{
  ((JZSamplesDlg*)itm.GetParent())->OnAddButton();
}
void JZSamplesDlg::ClrButton(wxItem& itm, wxCommandEvent& event)
{
  ((JZSamplesDlg*)itm.GetParent())->OnClrButton();
}
void JZSamplesDlg::HelpButton(wxItem& itm, wxCommandEvent& event)
{
  ((JZSamplesDlg*)itm.GetParent())->OnHelpButton();
}
void JZSamplesDlg::ListClick(wxItem& itm, wxCommandEvent& event)
{
  ((JZSamplesDlg*)itm.GetParent())->OnListClick();
}
#endif // OBSOLETE
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::SamplesDlg()
{
  if (mpSamplesDialog == 0)
  {
    mpSamplesDialog = new JZSamplesDialog(gpTrackWindow, *this);
  }
  mpSamplesDialog->Show(true);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZSampleSet::RefreshDialogs()
{
  if (mpSamplesDialog)
  {
//    mpSamplesDialog->Sample2Win(mpSamplesDialog->current);
  }
}
