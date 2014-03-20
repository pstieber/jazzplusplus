CC=gcc
OPT=-O2
# 
# If you don't have the new rpcgen take away this define:
DEFINES=-DRPCGEN_NEW
#
# Non-linux users may want to alter this:
CFLAGS=-m486 -Wall -DLINUX
#
all : midinetd

OBJ = main.o reader.o procs.o midi_p_disp.o midi_p_xdr.o

midinetd: $(OBJ)
	gcc -o midinetd $(OBJ)

$(OBJ) : %.o : %.c
	gcc -c $(OPT) $(CFLAGS) $(DEFINES) $< -o $@

clean:
	rm -f midinetd *.o midi_p_disp.c midi_p_xdr.c midi_p.h midi_p_svc.c \
	midi_p_clnt.c

main.o: main.c midinetd.h midi_p.h
reader.o: reader.c midinetd.h midi_p.h
procs.o: procs.c midinetd.h midi_p.h
midi_p_disp.o: midi_p_disp.c midi_p.h
midi_p_xdr.o: midi_p_xdr.c midi_p.h

midi_p.h: midi_p.x
	rpcgen -h midi_p.x > midi_p.h

midi_p_xdr.c: midi_p.x
	rpcgen -c midi_p.x > midi_p_xdr.c

midi_p_disp.c: midi_p.x
	rpcgen -m midi_p.x > midi_p_disp.c
