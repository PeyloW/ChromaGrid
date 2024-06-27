LIBCMINIINC=${LIBCMINI_INCLUDE_PATH}
MINTLIBINC=${MINTLIB_INCLUDE_PATH}
LIBCMINI=${LIBCMINI_LIBRARY_PATH}/mshort/mfastcall
TOYBOX=./toybox

#CC=m68k-atari-mint-g++
CC=/opt/cross-mint-OLD/bin/m68k-atari-mint-c++
AR=/opt/cross-mint-OLD/bin/m68k-atari-mint-ar

FLAGS=-m68000 -mshort -mfastcall
#FLAGS+=-g1 -DNDEBUG
FLAGS+=-g0 -DNDEBUG -DTOYBOX_TARGET_ATARI=2
FLAGS+=-s
FLAGS+=-DTOYBOX_DEBUG_CPU=0
#FLAGS+=-DTOYBOX_LOG_MALLOC=1
CFLAGS=-c -std=c++0x -Os -fomit-frame-pointer -fno-threadsafe-statics
#CFLAGS+=-S
CFLAGS+=-fno-exceptions -Wno-write-strings -Wno-pointer-arith -fno-rtti
CFLAGS+=-I $(LIBCMINIINC) -I $(MINTLIBINC) -I $(TOYBOX) $(FLAGS)
LDFLAGS=-nostdlib -L. -ltoybox -L$(LIBCMINI)/ -lcmini -lgcc $(FLAGS)
LDFLAGS+=-Wl,--traditional-format,--stack,16384,--mno-fastalloc

TOYBOX_OBJECTS=cpp_runtime.o
TOYBOX_OBJECTS+=timer.o input.o
TOYBOX_OBJECTS+=stream.o util_stream.o iffstream.o
TOYBOX_OBJECTS+=asset.o
TOYBOX_OBJECTS+=palette.o image.o tileset.o font.o
TOYBOX_OBJECTS+=canvas.o canvas_stencil.o dirtymap.o
TOYBOX_OBJECTS+=audio.o audio_atari.o audio_mixer_atari.o
TOYBOX_OBJECTS+=screen.o scene.o scene_transition.o
TOYBOX_OBJECTS_ATARI=machine_atari.o
TOYBOX_OBJECTS_ATARI+=timer_atari.o input_atari.o system_helpers_atari.o
TOYBOX_OBJECTS_ATARI+=canvas_atari.o
TOYBOX_OBJECTS_ATARI+=canvas_atari.o

CG_OBJECTS=main.o
CG_OBJECTS+=resources.o level.o button.o scroller.o
CG_OBJECTS+=game.o game_menu.o game_scores.o game_credits.o game_help.o game_level.o game_level_edit.o

cracked: install
	rm install/auto/cgrid.prg
	cp other/d-bug install/d-bug
	cp other/loader.prg install/auto/loader.prg

install: all
	mkdir install
	mkdir install/auto
	cp cgrid.tos install/cgrid.tos
	cp cgrid.tos install/auto/cgrid.prg
	cp -r data install/data
	cp other/readme.txt install/readme.txt
	cp -r other/bonus install/bonus

all: toybox cgrid

toybox: libtoybox.a

libtoybox.a: $(TOYBOX_OBJECTS) $(TOYBOX_OBJECTS_ATARI)
	$(AR) rcs libtoybox.a $(TOYBOX_OBJECTS) $(TOYBOX_OBJECTS_ATARI)

%.o: $(TOYBOX)/%.cpp
	$(CC) $(CFLAGS) $<
%.o: $(TOYBOX)/%.sx
	$(CC) $(CFLAGS) $<

cgrid: toybox $(CG_OBJECTS)
	$(CC) $(LIBCMINI)/startup.o $(CG_OBJECTS) $(LDFLAGS) -o cgrid.tos

%.o: %.cpp
	$(CC) $(CFLAGS) $<
%.o: %.s
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.tos
	rm -rf install
