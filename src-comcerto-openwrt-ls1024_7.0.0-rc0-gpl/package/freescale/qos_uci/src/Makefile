#
# Makefile for making a iptables.o file...
#

ifndef PREFIX
PREFIX:=/usr/local/
endif

LIBDIR:=$(PREFIX)/lib
INCDIR:=$(PREFIX)/include
SBINDIR:=$(PREFIX)/sbin

ifndef IPT_LIBDIR
IPT_LIBDIR:=$(LIBDIR)/iptables
endif

ifndef XTABLES_LIBDIR
XTABLES_LIBDIR:=/usr/lib/iptables
endif


ifndef LDFLAGS
LDFLAGS = -L$(LIBDIR)
endif
LDFLAGS+=-rdynamic

ifndef EXTRALIBS
#EXTRALIBS += -luci -lubox -lm -ldl
EXTRALIBS += -luci -lubox -lm -ldl -liptext -liptext4 -liptext6
endif
LDLIBS=-ldl $(EXTRALIBS)

COPT_FLAGS:=-O2
CFLAGS:=$(COPT_FLAGS) -Wunused -Wall -Waggregate-return -Wmissing-declarations    -Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes     -Winline -pipe -I$(KERNEL_DIR)/include -I$(INCDIR) -I./include/ -I/usr/local/include/uci -I/usr/local/include/libubox -fPIC -DXTABLES_INTERNAL -DNO_SHARED_LIBS=1 -g -rdynamic -DALL_INCLUSIVE -DENABLE_IPV4 -DENABLE_IPV6 -D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 -D_REENTRANT -DIPTABLES_MULTI -DIPTABLES_1_4_21 

DEFINES:=-DXTABLES_LIBDIR=\"$(XTABLES_LIBDIR)\"
default: all

all: iptables.o qos-iptables-uci xshared.o qos-ip6tables-uci

iptables.o: iptables.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

ip6tables.o: ip6tables.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

xshared.o: xshared.c
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

qos-iptables-uci: qos-iptables-uci.c iptables.o xshared.o
	$(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS) -o $@ $^ $(UCIMAPLIB) $(LDLIBS)

qos-ip6tables-uci: qos-ip6tables-uci.c ip6tables.o xshared.o
	$(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS) -o $@ $^ $(UCIMAPLIB6) $(LDLIBS)

clean:
	rm *.o
