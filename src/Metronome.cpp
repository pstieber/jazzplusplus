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

#include "Metronome.h"

#include "Configuration.h"
#include "Globals.h"
#include "Events.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMetronomeInfo::JZMetronomeInfo()
  : mKeyNormal(37),
    mKeyAccented(36),
    mVelocity(127),
    mIsOn(false),
    mIsAccented(true)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZMetronomeInfo& JZMetronomeInfo::operator = (const JZMetronomeInfo& Rhs)
{
  if (this != &Rhs)
  {
    mKeyNormal = Rhs.mKeyNormal;
    mKeyAccented = Rhs.mKeyAccented;
    mVelocity = Rhs.mVelocity;
    mIsOn = Rhs.mIsOn;
    mIsAccented = Rhs.mIsAccented;
  }
  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZMetronomeInfo::operator == (const JZMetronomeInfo& Rhs) const
{
  return
    mKeyNormal == Rhs.mKeyNormal &&
    mKeyAccented == Rhs.mKeyAccented &&
    mVelocity == Rhs.mVelocity &&
    mIsOn == Rhs.mIsOn &&
    mIsAccented == Rhs.mIsAccented;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool JZMetronomeInfo::operator != (const JZMetronomeInfo& Rhs) const
{
  return !operator == (Rhs);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeInfo::ReadFromConfiguration()
{
  mIsAccented = (gpConfig->GetValue(C_MetroIsAccented) != 0);
  mVelocity = gpConfig->GetValue(C_MetroVelocity);
  mKeyNormal = gpConfig->GetValue(C_MetroNormalClick);
  mKeyAccented = gpConfig->GetValue(C_MetroAccentedClick);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeInfo::ToggleIsOn()
{
  mIsOn = !mIsOn;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKeyOnEvent* JZMetronomeInfo::CreateNormalEvent(int Clock) const
{
  return new JZKeyOnEvent(
    Clock,
    gpConfig->GetValue(C_DrumChannel) - 1,
    mKeyNormal,
    mVelocity,
    15);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
JZKeyOnEvent* JZMetronomeInfo::CreateAccentedEvent(int Clock) const
{
  return new JZKeyOnEvent(
    Clock,
    gpConfig->GetValue(C_DrumChannel) - 1,
    mKeyAccented,
    mVelocity,
    15);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeInfo::SetKeyNormal(unsigned char KeyNormal)
{
  mKeyNormal = KeyNormal;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeInfo::SetKeyAccented(unsigned char KeyAccented)
{
  mKeyAccented = KeyAccented;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeInfo::SetVelocity(unsigned char Velocity)
{
  mVelocity = Velocity;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void JZMetronomeInfo::SetIsAccented(bool IsAccented)
{
  mIsAccented = IsAccented;
}
