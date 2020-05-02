CC=$(CROSS_COMPILE)gcc

all:
	$(CC) nbvpn.c -o nbvpn -I ../

install:
	mkdir -p $(DESTDIR)/usr/sbin/
	cp nbvpn $(DESTDIR)/usr/sbin/
