ifdef OS
# Crosscompile for rPI
CPP=arm-linux-gnueabihf-g++
LD=arm-linux-gnueabihf-ld
else
# Native
LD=ld
CPP=c++
endif

CFLAGS= -Wpedantic -Werror -O1 -std=gnu99
CPPFLAGS= -Wpedantic -Werror -O1
LDFLAGS=

.PHONY: all upload clean
all:  build/seag_test


# Upload to rPI
upload: all
	@scp build/seag_test root@hallonpaj:/mnt/usb/seag_test

# Main app
RASPOBJS=\
	build/main.o \
	build/serial.o \
	build/seagate_lsi.o \
	build/seagate_st.o

build/main.o: main.cpp
	@$(CPP) $(CPPFLAGS) $< -c -o $@
build/serial.o: serial.cpp
	@$(CPP) $(CPPFLAGS) $< -c -o $@

build/seagate_lsi.o: seagate_lsi.cpp
	@$(CPP) $(CPPFLAGS) $< -c -o $@
build/seagate_st.o: seagate_st.cpp
	@$(CPP) $(CPPFLAGS) $< -c -o $@


build/seag_test: $(RASPOBJS)
	@$(CPP) $(LDFLAGS) -o $@ $(RASPOBJS)

clean:
	@rm -f build/*.o
	@rm -f build/seag_test
