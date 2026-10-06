GBDK ?= /opt/gbdk
SOURCES = main.c pong.inc bierkuehler.inc bierkuehler_graphics.inc bierkuehler_menu_umlaut.inc tiles.h bierkuehler.patch Makefile

.PHONY: all clean
all: thunder.gb

thunder.gb: $(SOURCES)
	mkdir -p build
	cp main.c build/main.c
	git apply --check --no-index --unidiff-zero --directory=build bierkuehler.patch
	git apply --no-index --unidiff-zero --directory=build bierkuehler.patch
	$(GBDK)/bin/lcc -I. -Wm-yn"THUNDER" -o $@ build/main.c

clean:
	rm -rf build
	rm -f thunder.gb *.o *.map *.noi *.ihx *.lst *.rel *.sym
