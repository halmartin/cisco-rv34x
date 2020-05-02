GET_TIMZONEDAY = get_timezoneday
CHECKDSTTIME = check_dsttime
CHECKDST = checkDst
D_SRC  = get_timezoneday.c 
D1_SRC  = check_dsttime.c
D2_SRC  = checkDst.c
#-----------------------------------------------------------------------
#all:    $(GET_TIMZONEDAY) $(CHECKDSTTIME) $(CHECKDST)
all: time_bin dhcp_release

dhcp_release:
	$(CC) dhcp_release.c -o dhcp_release

time_bin:    $(GET_TIMZONEDAY) $(CHECKDSTTIME) $(CHECKDST)

$(GET_TIMZONEDAY):
	$(CC) $(CFLAGS) $(D_SRC) -o $@

$(CHECKDSTTIME):
	$(CC) $(CFLAGS) $(D1_SRC) -o $@

$(CHECKDST):
	$(CC) $(CFLAGS) $(D2_SRC) -o $@

# Clean up build products.
clean:
	rm -f $(GET_TIMZONEDAY) $(CHECKDSTTIME) $(CHECKDST) dhcp_release
