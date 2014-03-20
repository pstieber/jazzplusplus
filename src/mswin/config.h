//*****************************************************************************
// The JAZZ++ Midi Sequencer
//
// Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
// Modifications Copyright (C) 2004 Patrick Earl
// Modifications Copyright (C) 2008-2010 Peter J. Stieber
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

// This is the Windows version of the config.h file that is automatically
// generated for the Linux build.  Unfortunately, if anything changes are made
// to the AC_INIT macro in configure.ac or any new autoconf/automake defines
// are introduced, they will have to be entered here manually.

// Do not use alsa drivers on a Windows box.
#undef DEV_ALSA

// Support jazz's own driver over tcp/ip.
// #undef DEV_MPU401

// Do not support /dev/sequencer2 on Windows.
#undef DEV_SEQUENCER2
