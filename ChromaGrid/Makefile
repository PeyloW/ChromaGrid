LIBCMINIINC=${LIBCMINI_INCLUDE_PATH}
MINTLIBINC=${MINTLIB_INCLUDE_PATH}
LIBCMINI=${LIBCMINI_LIBRARY_PATH}/mshort/mfastcall

#CC=m68k-atari-mint-g++
CC=/opt/cross-mint-OLD/bin/m68k-atari-mint-c++

FLAGS=-m68000 -mshort -mfastcall
FLAGS+=-g1 -DNDEBUG
CFLAGS=-c -std=c++0x -Os -fomit-frame-pointer
#CFLAGS+=-S
CFLAGS+=-fno-exceptions -Wno-write-strings -fno-rtti -fpermissive
CFLAGS+=-I $(LIBCMINIINC) -I $(MINTLIBINC) $(FLAGS)
LDFLAGS=-nostdlib -L$(LIBCMINI)/ -lcmini -lgcc $(FLAGS)
LDFLAGS+=-Wl,--traditional-format
OBJECTS=main.o cpp_runtime.o system.o system_m68k.o iff_file.o graphics.o graphics_m68k.o game.o

all: cgrid

cgrid: $(OBJECTS)
	$(CC) $(LIBCMINI)/startup.o $(OBJECTS) $(LDFLAGS) -o cgrid.tos

%.o: %.cpp
	$(CC) $(CFLAGS) $<
%.o: %.s
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o
	rm -rf *.tos
