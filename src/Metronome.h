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

class JZKeyOnEvent;

//*****************************************************************************
//*****************************************************************************
class JZMetronomeInfo
{
  public:

    JZMetronomeInfo();

    JZMetronomeInfo& operator = (const JZMetronomeInfo& Rhs);

    bool operator == (const JZMetronomeInfo& Rhs) const;

    bool operator != (const JZMetronomeInfo& Rhs) const;

    void ReadFromConfiguration();

    unsigned char GetKeyNormal() const;

    void SetKeyNormal(unsigned char KeyNormal);

    unsigned char GetKeyAccented() const;

    void SetKeyAccented(unsigned char KeyAccented);

    unsigned char GetVelocity() const;

    void SetVelocity(unsigned char Velocity);

    bool IsOn() const;

    bool IsAccented() const;

    void SetIsAccented(bool IsAccented);

    void ToggleIsOn();

    JZKeyOnEvent* CreateNormalEvent(int Clock) const;

    JZKeyOnEvent* CreateAccentedEvent(int Clock) const;

  private:

    unsigned char mKeyNormal;

    unsigned char mKeyAccented;

    unsigned char mVelocity;

    bool mIsOn;

    bool mIsAccented;
};

inline
unsigned char JZMetronomeInfo::GetKeyNormal() const
{
  return mKeyNormal;
}

inline
unsigned char JZMetronomeInfo::GetKeyAccented() const
{
  return mKeyAccented;
}

inline
unsigned char JZMetronomeInfo::GetVelocity() const
{
  return mVelocity;
}

inline
bool JZMetronomeInfo::IsOn() const
{
  return mIsOn;
}

inline
bool JZMetronomeInfo::IsAccented() const
{
  return mIsAccented;
}
