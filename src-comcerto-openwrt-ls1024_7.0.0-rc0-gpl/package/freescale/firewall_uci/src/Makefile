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
#EXTRALIBS += -lnsl -luci -lubox -lm -ldl
EXTRALIBS += -luci -lubox -lm -ldl -liptext -liptext4 -liptext6
endif
LDLIBS=-ldl $(EXTRALIBS)

COPT_FLAGS:=-O2
CFLAGS:=$(COPT_FLAGS) -Wunused -Wall -Waggregate-return -Wmissing-declarations    -Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes     -Winline -pipe -I$(KERNEL_DIR)/include -I$(INCDIR) -I./include/ -I/usr/local/include/uci -I/usr/local/include/libubox -fPIC -DXTABLES_INTERNAL -DNO_SHARED_LIBS=1 -g -rdynamic -DALL_INCLUSIVE -DENABLE_IPV4 -DENABLE_IPV6 -D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 -D_REENTRANT -DIPTABLES_MULTI -DIPTABLES_1_4_21 

DEFINES:=-DXTABLES_LIBDIR=\"$(XTABLES_LIBDIR)\"
default: all

all: iptables.o iptables-uci xshared.o ip6tables-uci

iptables.o: iptables.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

ip6tables.o: ip6tables.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

xshared.o: xshared.c
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

iptables-uci: iptables-uci.c iptables-uci_staticnat.c iptables-uci_basicsettings.c iptables-uci_acl.c iptables-uci_ifaceNAT.c iptables-uci_allWebCntntFltr.c iptables-uci_portForward.c iptables-uci_portTrigger.c iptables-uci_dmzhost.c iptables-uci_policyNAT.c iptables.o xshared.o
	$(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS) -o $@ $^ $(UCIMAPLIB) $(LDLIBS)

ip6tables-uci: ip6tables-uci.c ip6tables-uci_basicsettings.c ip6tables.o xshared.o
	$(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS) -o $@ $^ $(UCIMAPLIB6) $(LDLIBS)

clean:
	rm iptables-standalone iptables-blocking *.o
