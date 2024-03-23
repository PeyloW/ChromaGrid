LIBCMINIINC=${LIBCMINI_INCLUDE_PATH}
MINTLIBINC=${MINTLIB_INCLUDE_PATH}
LIBCMINI=${LIBCMINI_LIBRARY_PATH}/mshort/mfastcall
TOYBOX=./toybox

#CC=m68k-atari-mint-g++
CC=/opt/cross-mint-OLD/bin/m68k-atari-mint-c++
AR=/opt/cross-mint-OLD/bin/m68k-atari-mint-ar

FLAGS=-m68000 -mshort -mfastcall
#FLAGS+=-g1 -DNDEBUG
FLAGS+=-g0 -DNDEBUG
CFLAGS=-c -std=c++0x -Os -fomit-frame-pointer
#CFLAGS+=-S
CFLAGS+=-fno-exceptions -Wno-write-strings -Wno-pointer-arith -fno-rtti -fpermissive
CFLAGS+=-I $(LIBCMINIINC) -I $(MINTLIBINC) -I $(TOYBOX) $(FLAGS)
LDFLAGS=-nostdlib -L. -ltoybox -L$(LIBCMINI)/ -lcmini -lgcc $(FLAGS)
LDFLAGS+=-Wl,--traditional-format

TOYBOX_OBJECTS=system.o system_m68k.o cpp_runtime.o
TOYBOX_OBJECTS+=iff_file.o
TOYBOX_OBJECTS+=graphics.o graphics_ilbm.o graphics_stencil.o graphics_draw.o graphics_dirtymap.o graphics_draw_blitter.o
TOYBOX_OBJECTS+=audio.o
TOYBOX_OBJECTS+=scene.o

CG_OBJECTS=main.o
CG_OBJECTS+=resources.o level.o button.o
CG_OBJECTS+=game.o game_intro.o game_scores.o game_credits.o game_level.o game_level_edit.o

all: toybox cgrid

toybox: libtoybox.a

libtoybox.a: $(TOYBOX_OBJECTS)
	$(AR) rcs libtoybox.a $(TOYBOX_OBJECTS)

%.o: $(TOYBOX)/%.cpp
	$(CC) $(CFLAGS) $<
%.o: $(TOYBOX)/%.s
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
