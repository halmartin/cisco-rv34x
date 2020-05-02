CURRENT_DIR = $(shell pwd)

all: boardinfo.so boardutil

boardinfo.so: boardinfo.o
	$(CC) -shared boardinfo.o -o libboardinfo.so

boardinfo.o: boardinfo.c
	$(CC) -Wall -fPIC  -c $< -o $@

boardutil: boardutil.o
	$(CC) $(LDFLAGS) -L$(CURRENT_DIR) -lboardinfo boardutil.o -o boardutil

boardutil.o: boardutil.c
	$(CC) $(CFLAGS) -c boardutil.c

clean:
	rm *.o boardutil
