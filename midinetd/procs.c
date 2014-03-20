/*
**  jazz - a midi sequencer for Linux
**
**  Copyright (C) 1994 Andreas Voss (andreas@avix.rhein-neckar.de)
**
**  Portions Copyright (C) 1995 Per Sigmond (Per.Sigmond@hia.no)
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* midinetd using RPC and TCP protocols - Written by Per Sigmond, HiA			*/
/* Version 2.2 */

#include <stdio.h>
#include <string.h>
#ifdef linux
#include <linux/time.h>
#endif
#ifdef sun386
#include <memory.h>
#endif
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#ifndef linux
#include <netinet/tcp.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <rpc/rpc.h>
#include "midi_p.h"
#include "midinetd.h"

extern int logfd;
extern int childpid;
extern int midi_fd;

#ifndef RPCGEN_NEW
#define mpu_write_ack_1_svc mpu_write_ack_1
#define mpu_write_noack_1_svc mpu_write_noack_1
#endif

#define MAXTRIES 100000

int *mpu_write_ack_1_svc( midi_buf *mbuf, struct svc_req * rqst ) {

int j, k, ii;
static int res;

if (mbuf->midi_buf_len) {
	/* Write command to mpu */
	j = 0;
	for (ii = 0; ii < MAXTRIES; ii++) {
		k = write(midi_fd, mbuf->midi_buf_val + j, mbuf->midi_buf_len - j);
		if (k > 0) j = j + k;
		if (j >= mbuf->midi_buf_len) break;
	}
	if (ii >= MAXTRIES) {
		printerror("mpu_write_ack_1_svc() write file");
		closedown(0);
	}
}
	
/* Return result to JAZZ */
res = mbuf->midi_buf_len;
return( &res );
}

void *mpu_write_noack_1_svc( midi_buf *mbuf, struct svc_req * rqst ) {

int j, k, ii;

if (mbuf->midi_buf_len) {
	/* Write command to mpu */
	j = 0;
	for (ii = 0; ii < MAXTRIES; ii++) {
		k = write(midi_fd, mbuf->midi_buf_val + j, mbuf->midi_buf_len - j);
		if (k > 0) j = j + k;
		if (j >= mbuf->midi_buf_len) break;
	}
	if (ii >= MAXTRIES) {
		printerror("mpu_write_noack_1_svc() write file");
		closedown(0);
	}
}

return( (void*) NULL );
}

