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

#include "Events.h"

#include "ErrorMessage.h"
#include "Globals.h"
#include "JazzPlusPlusApplication.h"
#include "Synth.h"

#include <sstream>

using namespace std;

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZReadBase::JZReadBase()
  : mTicksPerQuarter(0),
    mTrackCount(0),
    mIfs()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZReadBase::~JZReadBase()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZReadBase::Open(const string& FileName)
{
  mIfs.open(FileName.c_str(), ios::binary);
  if (!mIfs)
  {
    ostringstream Oss;
    Oss << "Error opening file " << FileName;
    Error(Oss.str());
    return 0;
  }
  return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZReadBase::Close()
{
  mIfs.close();
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZWriteBase::JZWriteBase()
  : mOfs()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZWriteBase::~JZWriteBase()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZWriteBase::Open(
  const string& FileName,
  int TrackCount,
  int TicksPerQuarter)
{
  mOfs.open(FileName.c_str(), ios::binary);
  if (!mOfs)
  {
    ostringstream Oss;
    Oss << "Error opening file " << FileName;
    Error(Oss.str());
    return 0;
  }
  return TrackCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZWriteBase::Close()
{
  mOfs.close();
}

//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZGetMidiBytes::Write(JZEvent* pEvent, unsigned char* pData, int Length)
{
  int Stat = pEvent->GetStat();

  switch (Stat)
  {
    case StatKeyOff:
// SN-- want key off veloc      Stat = StatKeyOn;  // better RunningStatus
    case StatKeyOn:
    case StatKeyPressure:
    case StatControl:
    case StatProgram:
    case StatChnPressure:
    case StatPitch:
      mByteCount = 0;

      mBuffer[mByteCount++] = Stat | ((JZChannelEvent *)pEvent)->GetChannel();
      while (Length--)
      {
        mBuffer[mByteCount++] = *pData++;
      }
      return 0;

    default:
      return 1;
  }
}

//*****************************************************************************
// Description:
//   This is the meta event class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
// Description:
//   This function fixed the checksum bytes at the end of the data stream.
//-----------------------------------------------------------------------------
void JZMetaEvent::FixCheckSum()
{
  unsigned char Sum = 0x00;
  for (unsigned short i = 4; i < (mLength - 2); ++i)
  {
    Sum += mpData[i];
  }
  mpData[mLength - 2] = (0x80 - (Sum & 0x7f)) & 0x7f;
  mpData[mLength - 1] = 0xf7;
}

//*****************************************************************************
// Description:
//   This is the System Exclusive (SysEx) event class definition.
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int JZSysExEvent::GetPitch() const
{
  int Id = gpSynth->GetSysexId(this);

  if (Id >= SX_GM_ON && Id < SX_GS_ON)
  {
    return SX_GROUP_GM;
  }
  else if (Id >= SX_GS_ON && Id < SX_XG_ON)
  {
    return SX_GROUP_GS;
  }
  else if (Id >= SX_XG_ON && Id < NumSysexIds)
  {
    return SX_GROUP_XG;
  }
  return SX_GROUP_UNKNOWN;
}
