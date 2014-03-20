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
//
// Description:
//   midinet.h using RPC and TCP protocol - Written by Per Sigmond, HiA
//*****************************************************************************

#ifndef midinet_h
#define midinet_h

#ifdef __cplusplus
extern "C"
{
#endif
void non_block_io( int s, int on );
int midinetconnect(char *hostname, char *service);
int write_ack_mpu( const void *buf, int len );
int write_noack_mpu( const void *buf, int len );
int get_recbuf_mpu( unsigned char **bufptr );
#ifdef __cplusplus
}
#endif

#endif
