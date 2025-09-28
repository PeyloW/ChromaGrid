include ../toybox/common.mk

CG_OBJECTS=build/main.o
CG_OBJECTS+=build/resources.o build/level.o build/button.o build/scroller.o
CG_OBJECTS+=build/game.o build/game_menu.o build/game_scores.o build/game_credits.o build/game_help.o build/game_level.o build/game_level_edit.o

all: cgrid

toybox:
	$(MAKE) -C $(TOYBOX) libtoybox.a

cgrid: toybox $(CG_OBJECTS)
	$(CC) $(LIBCMINILIB)/startup.o $(CG_OBJECTS) $(LDFLAGS) -o build/cgrid.tos

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@

install: cgrid
	mkdir -p install
	mkdir -p install/auto
	cp build/cgrid.tos install/cgrid.tos
	cp build/cgrid.tos install/auto/cgrid.prg
	cp -r data install/data
	cp other/readme.txt install/readme.txt
	cp -r other/bonus install/bonus

cracked: install
	rm install/auto/cgrid.prg
	cp other/d-bug install/d-bug
	cp other/loader.prg install/auto/loader.prg

clean:
	$(MAKE) -C $(TOYBOX) clean
	rm -rf build
	rm -rf install
