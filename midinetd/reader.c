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
#ifdef sun386
#include <memory.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <rpc/rpc.h>
#include "midi_p.h"
#include "midinetd.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

#if 0
/* WARNING: This function was made in a hurry for testing the performance
   of software midi through, it doesn't work properly (but good enough for
   the test...)

   Per.
*/
void mpu_midi_thru( int c, int fd ) {

enum { timing_byte = 0, status_or_data_1, data_1, data_2, done };
static unsigned char c1, c2;
static unsigned char running_status = 0;
static int state = 0;
static const int three_bytes[8] = { 1, 1, 1, 1, 0, 0, 1 };
char buf[20];
int len;

#define DAT (1<<6)
#define CMD (2<<6)
	
switch (state) {

	case timing_byte:
		if (c < 0xf0) {
			/* timing byte */
			state = status_or_data_1;
		}
		break;
		
	case status_or_data_1:
		if (c < 0xf0) {
			/* midi voice message */
			if (c & 0x80) {
				/* status */
				running_status = c;
				state = data_1;
			}
			else {
				/* data 1 */
				c1 = c;
				if (three_bytes[((unsigned char) running_status >> 4) - 8]) {
					state = data_2;
				}
				else {
					state = done;
					len = 1;
				}
			}
		}
		else {
			state = timing_byte;
		}
		break;

	case data_1:
		c1 = c;
		if (three_bytes[((unsigned char) running_status >> 4) - 8]) {
			state = data_2;
		}
		else {
			state = done;
			len = 1;
		}
		break;

	case data_2:
		c2 = c;
		state = done;
		len = 2;
		break;

	default:
		state = timing_byte;
}

if (state == done) {
	buf[0] = CMD+1;	
	buf[1] = 0xd7;
	buf[2] = DAT+len;
	buf[3] = c1;
	if (len == 2) buf[4] = c2;

	write( fd, buf, 3 + len );

	state = timing_byte;
}

}
#endif



/* Following are a family of functions, one for each midi device type. The
   following general description can act as an introduction to their behaviour.
   "xxx" is one of "mpu", "vox", "yyy"...:

   void xxx_reader_process( int fd, int tcp_sd );

   This function is started as a child process. Most of the time it reads
   the midi interface device (fd) and sends realtime info to JAZZ through
   a TCP connection (tcp_sd). It also stores incoming midi events in a memory
   buffer. If software midi through is required the events will also be
   written to the midi device.
   On play/record stop JAZZ will send one byte through the TCP-connection
   as a command and will wait for the function to send the recorded midi data
   through the TCP-connection. This will be caught by the select-statement.
   After sending the record buffer the function will clear the buffer and
   allocate a new one.

   Per Sigmond
*/
void mpu_reader_process( int fd, int tcp_sd ) {

#define CLOCK_TO_HOST 0xfd
#define START_OF_RECORD_BUFFER 0xfb

unsigned char filebuf[BUFSIZ];
int filebytes;
int i, j, k;
unsigned char ch;
unsigned char clock_to_host_byte = CLOCK_TO_HOST;

unsigned char *recbuf = NULL;
int bytes_in_recbuf = 0;
int size_of_recbuf = 0;
unsigned char start_marker = START_OF_RECORD_BUFFER;
int recbuf_bytes_sent, bytes_sent_now;
int writing_song_ptr = 0;

int nfds;
fd_set rfds;

/* Initialize record buffer */
recbuf = (unsigned char*) malloc( BUFSIZ );
if ( !recbuf ) {
	printerror("malloc");
	closedown(0);
}
size_of_recbuf = BUFSIZ;
bytes_in_recbuf = 0;

/* Prepare for select */
nfds = MAX( fd, tcp_sd ) + 1;
FD_ZERO( &rfds );

/* Read-loop */
for (;;) {

    FD_SET( fd, &rfds );
    FD_SET( tcp_sd, &rfds );

    /* Hang here until data is ready for reading */
    if (select( nfds, &rfds, (fd_set*) 0, (fd_set*) 0, (struct timeval*) 0 ) < 0) {
	printerror("select");
	closedown(0);
    }

    /* File ready ? */
    if (FD_ISSET( fd, &rfds )) {

	/* Read from file (blocking) */
	filebytes = read( fd, filebuf, BUFSIZ );
	if ( filebytes == -1 ) {
		printerror("read file");
		closedown(0);
	}

	/* Write to recbuffer and send clock_to_host_byte to socket */
	if (filebytes > 0) {
		for (k = 0; k < filebytes; k++) {
			ch = filebuf[k];
			if (ch == 0xff) continue;
			/* I sometimes get these 0xff bytes,
			   don't know why but they are of no use */

			/* clock-to-host received ? */
			if (ch == 0xfd) {
				/* Send clock-to-host to jazz */
				j = write(tcp_sd, &clock_to_host_byte, 1);
				if (j <= 0) {
					printerror("write socket (1)");
					closedown(0);
				}
			}
			else if ( (ch == 0xfa) || (ch == 0xfb) || (ch == 0xfc) ) {
				/* Real time message */
				j = write(tcp_sd, &ch, 1);
				if (j <= 0) {
					printerror("write socket (2)");
					closedown(0);
				}
			}
			else if ( (ch == 0xf2) || (writing_song_ptr) ) {
				/* Song pointer received */
				writing_song_ptr++;
				j = write(tcp_sd, &ch, 1);
				if (j <= 0) {
					printerror("write socket (3)");
					closedown(0);
				}
				if (writing_song_ptr >= 3) {
					writing_song_ptr = 0;
				}
			}
			else {
				/* record byte received; write to recbuffer */
#if 0
/* Don't do this with mpu-401 */
				/* (any midi-thru would also be done here...) */
				midi_thru( ch, fd );
#endif

				if (bytes_in_recbuf >= size_of_recbuf) {
					recbuf = (unsigned char*) realloc( recbuf, size_of_recbuf + BUFSIZ );
					size_of_recbuf = size_of_recbuf + BUFSIZ;
					if ( !recbuf ) {
						printerror("realloc");
						closedown(0);
					}
				}				
				recbuf[bytes_in_recbuf++] = ch;

			} /* else */
		} /* for k */
	} /* if filebytes > 0 */
    } /* file ready */

    /* Socket ready ? */
    if (FD_ISSET( tcp_sd, &rfds )) {
	/* Read the socket byte */
	if ( (i = read( tcp_sd, &ch, 1 )) != 1 ) {
		if (i != 0) {
			printerror("read socket");
		}
		closedown(0);
	}

	/* Send start marker */
	if (write( tcp_sd, &start_marker, 1 ) < 1 ) {
		printerror("write socket (2)");
		closedown(0);
	}

	/* Send number of bytes in record buffer */
	j = htonl( bytes_in_recbuf );
	if ( write( tcp_sd, &j, sizeof(int)) < sizeof(int) ) {
		printerror("write socket (3)");
		closedown(0);
	}

	/* Send contents of record buffer */
	recbuf_bytes_sent = 0;
	while (recbuf_bytes_sent < bytes_in_recbuf) {
		if ( (bytes_in_recbuf - recbuf_bytes_sent) >= BUFSIZ)
			k = BUFSIZ;
		else
			k = bytes_in_recbuf - recbuf_bytes_sent;
		bytes_sent_now = write(tcp_sd, recbuf + recbuf_bytes_sent, k );

		if (bytes_sent_now < 0) {
			printerror("write socket (4)");
			closedown(0);
		}
		recbuf_bytes_sent = recbuf_bytes_sent + bytes_sent_now;
	}

	/* Free memory of record buffer and allocate some new */
	free( recbuf );
	recbuf = (unsigned char*) malloc( BUFSIZ );
	if ( !recbuf ) {
		printerror("malloc");
		closedown(0);
	}
	size_of_recbuf = BUFSIZ;
	bytes_in_recbuf = 0;
    } /* socket ready */

} /* for (;;) */

} /* reader_process() */
