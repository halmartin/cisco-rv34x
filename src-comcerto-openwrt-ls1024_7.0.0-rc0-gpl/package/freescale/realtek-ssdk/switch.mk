OBJS=$(patsubst %.c,%.o,$(shell ls *.c | grep -v '^main.c$$'))
INCLUDES=-I.

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -DFORCE_PROBE_RTL8367C -std=gnu99 -fstrict-aliasing $(FPIC) -c -o $@ $<

librtkssdk.so: $(OBJS) 
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS)

clean:
	rm -f *.o *.so 
