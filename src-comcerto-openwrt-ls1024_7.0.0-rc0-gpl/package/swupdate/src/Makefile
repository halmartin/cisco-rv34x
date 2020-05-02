LIBFLAG = -lm -lcurl -lssl -lcrypto -lcJSON
#CFLAG  = -Wall -g -O1
#CFLAG  = -Wall -g -O0

#STRIP  = strip
ASD_CLIENT = asdclient
D_OBJS  = request.o \
	chksum.o \
	asdclient.o 
#-----------------------------------------------------------------------
all:    $(ASD_CLIENT)

$(ASD_CLIENT): $(D_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(D_OBJS) -o $@ $(LIBFLAG) $(BCCSDK_LIB)

.c.o:
	$(CC) $(CFLAGS) -c $? -o $@

# Clean up build products.
clean:
	rm -f $(D_OBJS) $(ASD_CLIENT) 
