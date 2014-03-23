#include "PortMidiPlayer.h"

#include "JazzPlusPlusApplication.h"
#include "TrackFrame.h"
#include "TrackWindow.h"
#include "Song.h"
#include "Globals.h"
#include "MidiDeviceDialog.h"

#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPortMidiPlayer::JZPortMidiPlayer(JZSong* pSong)
  : JZPlayer(pSong),
    mInputDevices(),
    mOutputDevices(),
    mpStream(0),
    mInitialized(false),
    mStartTime(0),
    mStartClock(0),
    mTicksPerMinute(100),
    mInputDevice(),
    mOutputDevice(),
    mInDev(-1),
    mOutDev(-1)
{
  InitPM();
  mInDev = Pm_GetDefaultOutputDeviceID();
  mOutDev = Pm_GetDefaultOutputDeviceID();
  TermPM();

  Pt_Start(1, 0, 0);

//  mOutputQueue = Pm_QueueCreate(1024, sizeof(PmEvent));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZPortMidiPlayer::~JZPortMidiPlayer()
{
//   Pm_QueueDestroy(mOutputQueue);
  TermPM();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZPortMidiPlayer::IsInstalled()
{
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
wxString JZPortMidiPlayer::GetInputDeviceName()
{
  return mInputDevice;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
wxString JZPortMidiPlayer::GetOutputDeviceName()
{
  return mOutputDevice;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::SetInputDevice(const wxString& Name)
{
  bool NeedToTerminate = InitPM();
  PmDeviceID id = FindDevice(Name, true);

  if (id != pmNoDevice)
  {
    mInputDevice = Name;
  }

  if (NeedToTerminate)
  {
    TermPM();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::SetOutputDevice(const wxString& Name)
{
  bool NeedToTerminate = InitPM();
  PmDeviceID id = FindDevice(Name, false);

  if (id != pmNoDevice)
  {
    mOutputDevice = Name;
  }

  if (NeedToTerminate)
  {
    TermPM();
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPortMidiPlayer::SupportsMultipleDevices()
{
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZDeviceList& JZPortMidiPlayer::GetOutputDevices()
{
  bool NeedToTerminate = InitPM();
  int Count = Pm_CountDevices();

  mOutputDevices.Clear();

  for (int i = 0; i < Count; ++i)
  {
    const PmDeviceInfo* pPmDeviceInfo = Pm_GetDeviceInfo(i);

    if (pPmDeviceInfo && pPmDeviceInfo->output)
    {
      wxString Name =
        wxString(pPmDeviceInfo->interf, wxConvISO8859_1) +
        wxT(", ") +
        wxString(pPmDeviceInfo->name, wxConvISO8859_1);
      mOutputDevices.Add(Name);
    }
  }

  if (NeedToTerminate)
  {
    TermPM();
  }

  return mOutputDevices;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZDeviceList& JZPortMidiPlayer::GetInputDevices()
{
  bool NeedToTerminate = InitPM();
  int Count = Pm_CountDevices();

  mInputDevices.Clear();

  for (int i = 0; i < Count; ++i)
  {
    const PmDeviceInfo* pPmDeviceInfo = Pm_GetDeviceInfo(i);

    if (pPmDeviceInfo && pPmDeviceInfo->input)
    {
      wxString Name =
        wxString(pPmDeviceInfo->interf, wxConvISO8859_1) +
        wxT(", ") +
        wxString(pPmDeviceInfo->name, wxConvISO8859_1);
      mInputDevices.Add(Name);
    }
  }

  if (NeedToTerminate)
  {
    TermPM();
  }

  return mInputDevices;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
PmDeviceID JZPortMidiPlayer::FindDevice(const wxString& Name, bool input)
{
  int Count = Pm_CountDevices();

  for (int i = 0; i < Count; i++)
  {
    const PmDeviceInfo* pPmDeviceInfo = Pm_GetDeviceInfo(i);

    if (pPmDeviceInfo && (input ? pPmDeviceInfo->input : pPmDeviceInfo->output))
    {
      wxString n =
        wxString(pPmDeviceInfo->interf, wxConvISO8859_1) +
        wxT(", ") +
        wxString(pPmDeviceInfo->name, wxConvISO8859_1);

      if (Name == n)
      {
        return i;
      }
    }
  }

  return pmNoDevice;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPortMidiPlayer::Clock2Time(int clock)
{
  if (clock < mStartClock)
  {
    return mStartTime;
  }

  return (int)((double)(clock - mStartClock) * 60000.0 / (double) mTicksPerMinute + mStartTime);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPortMidiPlayer::Time2Clock(int time)
{
  if (time < mStartTime)
  {
    return mStartClock;
  }

  return (int)((double)(time - mStartTime) * (double) mTicksPerMinute / 60000.0 + mStartClock);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::SetTempo(int bpm, int clock)
{
  int t1 = Clock2Time(clock);
  mTicksPerMinute = bpm * mpSong->GetTicksPerQuarter();
  int t2 = Clock2Time(clock);
  mStartTime += (t1 - t2);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::OutBreak()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPortMidiPlayer::OutEvent(JZEvent* pEvent, int now)
{
  PmError rc = pmNoError;
  PmTimestamp t = (now ? 0 : pEvent->GetClock());

  t = Clock2Time(t);

#define WSHORT(a, b) \
  rc = Pm_WriteShort(mpStream, t, Pm_Message(pEvent->GetStat() | k->GetChannel(), a, b))

  switch (pEvent->GetStat())
  {
    case StatKeyOn:
    {
      JZKeyOnEvent *k = pEvent->IsKeyOn();

      WSHORT(k->GetKey(), k->GetVelocity());
    }
    break;

    case StatKeyOff:
    {
      JZKeyOffEvent *k = pEvent->IsKeyOff();

      WSHORT(k->GetKey(), k->GetOffVelocity());
    }
    break;

    case StatProgram:
    {
      JZProgramEvent* k = pEvent->IsProgram();

      WSHORT(k->GetProgram(), 0);
    }
    break;

    case StatKeyPressure:
    {
      JZKeyPressureEvent *k = pEvent->IsKeyPressure();

      WSHORT(k->GetKey(), k->GetValue());
    }
    break;

    case StatChnPressure:
    {
      JZChnPressureEvent *k = pEvent->IsChnPressure();

      WSHORT(k->GetValue(), 0);
    }
    break;

    case StatControl:
    {
      JZControlEvent* k = pEvent->IsControl();

      WSHORT(k->GetControl(), k->GetValue());
    }
    break;

    case StatPitch:
    {
      JZPitchEvent *k = pEvent->IsPitch();

      WSHORT(k->GetValue(), 0);
    }
    break;

    case StatSetTempo:
    {
      JZSetTempoEvent* k = pEvent->IsSetTempo();
      if (k->GetClock() > 0)
      {
        SetTempo(k->GetBPM(), k->GetClock());
      }
    }
    break;

    case StatSysEx:
    {
      JZSysExEvent* s = pEvent->IsSysEx();

      unsigned char *buf = new unsigned char[s->GetLength() + 2];

      buf[0] = 0xF0;
      memcpy(&buf[1], s->GetData(), s->GetLength());
      buf[s->GetLength() + 2] = 0xf7;
      rc = Pm_WriteSysEx(mpStream, t, buf);

      delete[] buf;
    }
    break;

    default:
      break;
  }

  return rc != pmNoError;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPortMidiPlayer::OutEvent(JZEvent* pEvent)
{
  return OutEvent(pEvent, 0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::OutNow(JZEvent*pEvent)
{
  OutEvent(pEvent, 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::StartPlay(int clock, int loopClock, int cont)
{
  bool NeedToTerminate = InitPM();
  PmDeviceID id = FindDevice(mOutputDevice, false);

  if (id == pmNoDevice)
  {
    if (NeedToTerminate)
    {
      TermPM();
    }

    return;
  }

  cout
    << "rc = " << Pm_OpenOutput(&mpStream, id, NULL, 0, NULL, NULL, 100)
    << ' ' << id
    << endl;

  mStartTime = Pt_Time() + 500;
  mStartClock = clock;
  mTicksPerMinute  = mpSong->GetTicksPerQuarter() * mpSong->Speed();

  JZPlayer::StartPlay(clock, loopClock, cont);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::StopPlay()
{
  JZPlayer::StopPlay();

  if (mpStream)
  {
    Pm_Abort(mpStream);
    Pm_Close(mpStream);
    mpStream = NULL;
  }

  TermPM();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZPortMidiPlayer::GetRealTimeClock()
{
  long t = Pt_Time();

  gpTrackWindow->NewPlayPosition(
    mpPlayLoop->Ext2IntClock(Time2Clock(t) / 48 * 48));

  return Time2Clock(t);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZPortMidiPlayer::InitPM()
{
  if (mInitialized)
  {
    return false;
  }

  Pm_Initialize();

  mInitialized = true;

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZPortMidiPlayer::TermPM()
{
  if (!mInitialized)
  {
    return false;
  }

  Pm_Terminate();

  mInitialized = false;

  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZPortMidiPlayer::DeviceSelectionDialog()
{
  // Create a list of devices.
  bool NeedToTerminate = InitPM();
  int Count = Pm_CountDevices();

  vector<pair<wxString, int> > MidiDevices;

  // Create a container of input devices.
  for (int i = 0; i < Count; ++i)
  {
    const PmDeviceInfo* pPmDeviceInfo = Pm_GetDeviceInfo(i);

    if (pPmDeviceInfo && pPmDeviceInfo->input)
    {
      wxString Name =
        wxString(pPmDeviceInfo->interf, wxConvISO8859_1) +
        wxT(", ") +
        wxString(pPmDeviceInfo->name, wxConvISO8859_1);

      MidiDevices.push_back(make_pair(Name, i));
    }
  }

  // Select the input device.
  int InputDevice = -1;
  if (!MidiDevices.empty())
  {
    JZMidiDeviceDialog MidiInputDeviceDialog(
      MidiDevices,
      InputDevice,
      ::wxGetApp().GetMainFrame(),
      "Input MIDI device");
    MidiInputDeviceDialog.ShowModal();

    // Set the input device based on the selected integer.
    for (const auto& StringIntPair : MidiDevices)
    {
      if (StringIntPair.second == InputDevice)
      {
        SetOutputDevice(StringIntPair.first);
        break;
      }
    }
  }

  MidiDevices.clear();

  // Create a container of output devices.
  for (int i = 0; i < Count; ++i)
  {
    const PmDeviceInfo* pPmDeviceInfo = Pm_GetDeviceInfo(i);

    if (pPmDeviceInfo && pPmDeviceInfo->output)
    {
      wxString Name =
        wxString(pPmDeviceInfo->interf, wxConvISO8859_1) +
        wxT(", ") +
        wxString(pPmDeviceInfo->name, wxConvISO8859_1);

      MidiDevices.push_back(make_pair(Name, i));
    }
  }

  // Select the output device.
  int OutputDevice = -1;
  if (!MidiDevices.empty())
  {
    JZMidiDeviceDialog MidiOutputDeviceDialog(
      MidiDevices,
      OutputDevice,
      gpTrackWindow,
      "Output MIDI device");
    MidiOutputDeviceDialog.ShowModal();

    // Set the output device based on the selected integer.
    for (const auto& StringIntPair : MidiDevices)
    {
      if (StringIntPair.second == OutputDevice)
      {
        SetOutputDevice(StringIntPair.first);
        break;
      }
    }
  }

//  gpConfig->Put(C_WinInputDevice, InputDevice);

//  gpConfig->Put(C_WinOutputDevice, OutputDevice);

  if (NeedToTerminate)
  {
    TermPM();
  }
}
