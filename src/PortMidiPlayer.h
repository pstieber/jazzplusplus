#pragma once

#include "Player.h"

#include "../portmidi/pm_common/portmidi.h"
#include "../portmidi/porttime/porttime.h"

class JZSong;
class JZEvent;

class JZPortMidiPlayer : public JZPlayer
{
  public:

    JZPortMidiPlayer(JZSong* pSong);

    virtual ~JZPortMidiPlayer();

    virtual bool IsInstalled();
    int SupportsMultipleDevices();

    virtual JZDeviceList& GetInputDevices();
    wxString GetInputDeviceName();
    void SetInputDevice(const wxString& Name);

    virtual JZDeviceList& GetOutputDevices();
    wxString GetOutputDeviceName();
    void SetOutputDevice(const wxString& Name);

    int OutEvent(JZEvent* pEvent, int now);
    int OutEvent(JZEvent* pEvent);
    void OutNow(JZEvent* pEvent);
    void OutBreak();

    void StartPlay(int Clock, int LoopClock = 0, int Continue = 0);
    void StopPlay();

    int GetRealTimeClock();
    int Clock2Time(int clock);
    int Time2Clock(int time);
    void SetTempo(int bpm, int clock);

    void DeviceSelectionDialog();

  private:

    bool InitPM();
    bool TermPM();
    PmDeviceID FindDevice(const wxString & name, bool input);

  private:

    JZDeviceList mInputDevices;
    JZDeviceList mOutputDevices;

    PortMidiStream* mpStream;

    bool mInitialized;
    int mStartTime;
    int mStartClock;
    int mTicksPerMinute;
    wxString mInputDevice;
    wxString mOutputDevice;
    int mInDev;
    int mOutDev;
};
