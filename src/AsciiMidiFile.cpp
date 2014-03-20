//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
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

#include "AsciiMidiFile.h"

#include "ErrorMessage.h"

#include <iomanip>
#include <sstream>

using namespace std;

//*****************************************************************************
// Description:
//   This is the ASCII reader class.  This is used to debug MIDI events.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAsciiRead::Open(const string& FileName)
{
  mIfs.open(FileName.c_str());
  if (!mIfs)
  {
    ostringstream Oss;
    Oss << "Error opening file " << FileName;
    Error(Oss.str());
    return 0;
  }

  int TrackCount, TicksPerQuarter;
  string Junk;
  mIfs >> Junk >> TrackCount >> Junk >> Junk >> TicksPerQuarter;
  if (mIfs.fail())
  {
    return 0;
  }
  return TrackCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZEvent* JZAsciiRead::Read()
{
  JZEvent* pEvent = 0;

  int Clock;
  int StatusByte, Channel, Length;
  mIfs >> Clock >> hex >> StatusByte >> dec >> Channel >> Length;
  if (mIfs.fail())
  {
    return pEvent;
  }

  unsigned char* pBuffer = new unsigned char[Length];
  for (int i = 0; i < Length; ++i)
  {
    int d;
    mIfs >> hex >> d >> dec;
    if (mIfs.fail())
    {
      delete [] pBuffer;
      return pEvent;
    }
    pBuffer[i] = (unsigned char)d;
  }

  switch (StatusByte)
  {
    case StatUnknown:
      break;

    case StatKeyOff:
      pEvent = new JZKeyOffEvent(Clock, Channel, pBuffer[0]);
      break;

    case StatKeyOn:
      pEvent = new JZKeyOnEvent(Clock, Channel, pBuffer[0], pBuffer[1]);
      break;

    case StatKeyPressure:
      pEvent = new JZKeyPressureEvent(Clock, Channel, pBuffer[0], pBuffer[1]);
      break;

    case StatControl:
      pEvent = new JZControlEvent(Clock, Channel, pBuffer[0], pBuffer[1]);
      break;

    case StatProgram:
      pEvent = new JZProgramEvent(Clock, Channel, pBuffer[0]);
      break;

    case StatChnPressure:
      pEvent = new JZChnPressureEvent(Clock, Channel, pBuffer[0]);
      break;

    case StatPitch:
      pEvent = new JZPitchEvent(Clock, Channel, pBuffer[0], pBuffer[1]);
      break;

    case StatSysEx:
      pEvent = new JZSysExEvent(Clock, pBuffer, Length);
      break;

    case StatSongPtr:
      pEvent = new JZSongPtrEvent(Clock,  pBuffer, Length);
      break;

    case StatMidiClock:
      pEvent = new JZMidiClockEvent(Clock,  pBuffer, Length);
      break;

    case StatStartPlay:
      pEvent = new JZStartPlayEvent(Clock,  pBuffer, Length);
      break;

    case StatContPlay:
      pEvent = new JZContPlayEvent(Clock,  pBuffer, Length);
      break;

    case StatStopPlay:
      pEvent = new JZStopPlayEvent(Clock,  pBuffer, Length);
      break;

    case StatText:
      pEvent = new JZTextEvent(Clock, pBuffer, Length);
      break;

    case StatCopyright:
      pEvent = new JZCopyrightEvent(Clock, pBuffer, Length);
      break;

    case StatTrackName:
      pEvent = new JZTrackNameEvent(Clock, pBuffer, Length);
      break;

    case StatMarker:
      pEvent = new JZMarkerEvent(Clock, pBuffer, Length);
      break;

    case StatEndOfTrack:
      pEvent = new JZEndOfTrackEvent(Clock);
      break;

    case StatSetTempo:
      pEvent = new JZSetTempoEvent(Clock, pBuffer[0], pBuffer[1], pBuffer[2]);
      break;

    case StatMtcOffset:
      pEvent = new JZMtcOffsetEvent(Clock, pBuffer, Length);
      break;

    case StatTimeSignat:
      pEvent = new JZTimeSignatEvent(
        Clock,
        pBuffer[0],
        pBuffer[1],
        pBuffer[2],
        pBuffer[3]);
      break;

    case StatKeySignat:
      pEvent = new JZKeySignatEvent(Clock, pBuffer[0], pBuffer[1]);
      break;

    case StatJazzMeta:
      if (memcmp(pBuffer, "JAZ2", 4) == 0)
      {
        pEvent = new JZJazzMetaEvent(Clock, pBuffer, Length);
      }
      else
      {
        pEvent = new JZMetaEvent(Clock, StatusByte, pBuffer, Length);
      }
      break;

    case StatPlayTrack:
      pEvent = new JZPlayTrackEvent(Clock, pBuffer, Length);
      break;

    case 33:
      pEvent = new JZMetaEvent(Clock, StatusByte, pBuffer, Length);
      break;
  }

  delete [] pBuffer;

  return pEvent;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAsciiRead::NextTrack()
{
  string String;
  mIfs >> String;
  return String == "NextTrack";
}

//*****************************************************************************
// Description:
//   This is the ASCII writer class.  This is used to debug MIDI events.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAsciiWrite::Open(
  const string& FileName,
  int TrackCount,
  int TicksPerQuarter)
{
  mOfs.open(FileName.c_str());
  if (!mOfs)
  {
    ostringstream Oss;
    Oss << "Error opening file " << FileName;
    Error(Oss.str());
    return 0;
  }

  mOfs
    << "Tracks " << TrackCount << ", TicksPerQuarter " << TicksPerQuarter
    << endl;

  return TrackCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZAsciiWrite::Write(JZEvent* pEvent, unsigned char* pData, int Length)
{
  JZChannelEvent* pChannelEvent;

  mOfs
    << setw(6) << pEvent->GetClock()
    << ' ' << setw(2) << hex << static_cast<unsigned>(pEvent->GetStat())
    << dec;
  if ((pChannelEvent = pEvent->IsChannelEvent()) != 0)
  {
    mOfs
      << ' ' << setw(2) << static_cast<unsigned>(pChannelEvent->GetChannel());
  }
  else
  {
    mOfs << " -1";
  }

  mOfs << ' ' << Length;
  for (int i = 0; i < Length; ++i)
  {
    mOfs << ' ' << setw(2) << hex << static_cast<unsigned>(pData[i]) << dec;
  }
  mOfs << endl;

  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZAsciiWrite::NextTrack()
{
  mOfs << "NextTrack" << endl;
}
