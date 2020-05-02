######################################################################
# Example specific definitions and rules
OSTYPE  = $(shell uname -s)
CURRENT_DIR = $(shell pwd)

MAIN_OBJ = wifi_check.o

all: clean wifi_check
	@echo "Build complete"

wifi_check: $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -o $@ $(MAIN_OBJ)

######################################################################
clean:  
	-rm -rf *.o wifi_check 2>/dev/null || true
