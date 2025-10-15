include ../toybox/common.mk

PRODUCT=cgrid

cracked: install
ifeq ($(HOST),sdl2)
	@echo "cracked target not supported for SDL2 build"
else
	rm install/auto/cgrid.prg
	cp crack/d-bug install/d-bug
	cp crack/loader.prg install/auto/loader.prg
endif

include ../toybox/product.mk
