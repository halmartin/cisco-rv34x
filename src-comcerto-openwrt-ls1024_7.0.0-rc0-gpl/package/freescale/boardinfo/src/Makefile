all: get_boardinfo

get_boardinfo.o: get_boardinfo.c
	$(CC) $(CFLAGS) -c -g get_boardinfo.c

get_boardinfo: get_boardinfo.o
	$(CC) $(CFLAGS) -lboardinfo get_boardinfo.o -o get_boardinfo

clean:
	rm *.o get_boardinfo
