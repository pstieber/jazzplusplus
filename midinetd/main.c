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

/* midinetd.c using RPC and TCP protocols - Written by Per Sigmond, HiA			*/
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
#include <rpc/pmap_clnt.h>

int logfd;
int childpid = -1;
int sd1 = 0, sd2 = 0, rpc_sd, tcp_sd;
int midi_fd;
int inetd = 0;

#define MPUDEVICE "/dev/mpu401"
#define MIDINETSERVICE "midinet"

int printerror( char *str ) {

	char timestr[25];
	time_t now;

	now = time( NULL );
	strncpy( timestr, ctime( &now ), 24 );
	timestr[24] = '\0';

	fprintf( stderr, "%s midinetd: ", timestr );
	fprintf( stderr, "errno=%d ", errno );
	perror( str );
	return(0);
}

void closedown( int sig ) {
	if (!inetd) {
		close(sd1);
	}
	close(tcp_sd);
	close(rpc_sd);
	close (midi_fd);
	if (childpid > 0) {
		kill( childpid, SIGHUP );
	}
	exit(0);
}

int main(int argc, char **argv)
{
struct sockaddr_in remote_tcp_addr, tcp_addr;
struct sockaddr_in *remote_tcp_addr_ptr = &remote_tcp_addr;
SVCXPRT *transport;

struct protoent *pent;
struct servent *sent;
int addlen;
int *addlen_ptr = &addlen;
int setval;


/* argv[0] == "in.midinetd" means started by inetd */
if (!strcmp( argv[0], "in.midinetd") ) inetd = 1;

if (argc > 1) {
	logfd = open( argv[1], O_APPEND | O_CREAT | O_WRONLY, 00666 );
	if (logfd == -1) {
		fprintf( stderr, "Could not open logfile %s\n", argv[1] );
		exit(1);
	}
	if (dup2( logfd, 2 ) == -1) {
		fprintf( stderr, "Could not dup2 stderr to logfile\n" );
		exit(1);
	}
}

if (!inetd) {

	/* Set adress-type and address */
	memset((char *)&tcp_addr, 0,  sizeof tcp_addr);
	tcp_addr.sin_family      = AF_INET;
	tcp_addr.sin_addr.s_addr = INADDR_ANY;

	/* Set TCP-port to listen on for TCP-connection */
	sent = getservbyname( MIDINETSERVICE, "tcp" );
	if (!sent) {
		printerror("getservbyname");
		exit(1);
	}
	tcp_addr.sin_port        = sent->s_port;

	/* Open/bind socket for TCP connection */
	sd1 = socket(PF_INET,SOCK_STREAM,0);
	if (bind(sd1,(struct sockaddr*) &tcp_addr,sizeof(tcp_addr)) < 0) {
		printerror("bind");
      		exit(1);
	}

	/* Issue listen-command */
	if (listen(sd1,5) < 0) {
		printerror("listen");
		exit(1);
	}

	/* Wait for incoming call by issuing accept-call */
	/* (hangs until call arrives) */
	fprintf(stderr,"Start accepting\n");
	tcp_sd = accept(sd1,(struct sockaddr*) remote_tcp_addr_ptr,addlen_ptr);

	if (tcp_sd < 0) {
		printerror("accept");
    		exit(1);
	}
	else {
		fprintf( stderr, "Connected...\n");
	}
}
else {
	/* Started by inetd; the socket is at stdin/stdout ! */
	tcp_sd = 0;
}

/* Create the RPC service */
rpc_sd = RPC_ANYSOCK;
transport = svctcp_create( rpc_sd, 0, 0 );
if (!transport) {
	printerror("RPC transport create failed");
	if (!inetd) {
		close(sd1);
	}
	close(tcp_sd);
	exit(1);
}

/* Unregister any old services with portmapper */
(void) pmap_unset( MIDINET_PROG, MIDINET_VERS );

/* Install dispatch routine and register with portmapper */
if ( !svc_register( transport, MIDINET_PROG, MIDINET_VERS, midinet_prog_1, IPPROTO_TCP ) ) {
	printerror("svc_register");
	closedown(0);
}

/* Open device for reading/writing */
if ((midi_fd = open( MPUDEVICE, O_RDWR )) < 0) {
	printerror("Open file failed");
	if (!inetd) {
		close(sd1);
	}
	close(tcp_sd);
	close(rpc_sd);
	exit(0);
}

/* Set TCP options (send bytes immediately) */
pent = getprotobyname( "tcp" );
setval = 1;
if (setsockopt( tcp_sd, pent->p_proto, TCP_NODELAY, &setval, sizeof(setval) )) {
	printerror("setsockopt");
	closedown(0);
}

/* Start device-reader process */
childpid = fork();
if (!childpid) {
	/* We are child process */
	/* Start reader process (runs forever) */
	mpu_reader_process( midi_fd, tcp_sd );
	printerror("mpu_reader_process() returned");
}
else {
	/* We are parent process */
	/* Install signal handlers */
	(void) signal( SIGCHLD, (void(*)()) closedown );
	(void) signal( SIGHUP, (void(*)()) closedown );

	/* Start RPC server (runs forever) */
	svc_run();

	printerror("svc_run() returned");
}

return(0);
} /* main */
