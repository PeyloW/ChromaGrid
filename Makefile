include ../toybox/common.mk
GAME=.
GAMEINC=$(GAME)/include

CFLAGS+=-I $(GAMEINC)

# Source files for cgrid
SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(patsubst src/%.cpp,build/%.o,$(SOURCES))

all: cgrid

toybox:
	$(MAKE) -C $(TOYBOX) libtoybox.a $(if $(HOST),HOST=$(HOST))

cgrid: toybox $(OBJECTS)
ifeq ($(HOST),sdl2)
	$(CC) $(OBJECTS) $(LDFLAGS) -o build/cgrid
else
	$(CC) $(LIBCMINILIB)/startup.o $(OBJECTS) $(LDFLAGS) -o build/cgrid.tos
endif

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@

install: cgrid
	mkdir -p install
ifeq ($(HOST),sdl2)
	cp build/cgrid install/cgrid
else
	mkdir -p install/auto
	cp build/cgrid.tos install/cgrid.tos
	cp build/cgrid.tos install/auto/cgrid.prg
	cp -r other/bonus install/bonus
	cp other/readme.txt install/readme.txt
endif
	cp -r data install/data

cracked: install
ifeq ($(HOST),sdl2)
	@echo "cracked target not supported for SDL2 build"
else
	rm install/auto/cgrid.prg
	cp other/d-bug install/d-bug
	cp other/loader.prg install/auto/loader.prg
endif

clean:
	$(MAKE) -C $(TOYBOX) clean
	rm -rf build
	rm -rf install

.PHONY: all toybox install cracked clean
