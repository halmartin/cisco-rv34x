#
# Makefile for making the utility 
#

default: all

ifndef LDFLAGS
LDFLAGS = -L$(LIBDIR)
endif

CFLAGS:=-g -O2

LDLIBS=-lnetfilter_conntrack -lnfnetlink -lmnl

all: vpn_clear_connections

vpn_clear_connections: vpn_clear_connections.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

	$(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm vpn_clear_connections *.o
