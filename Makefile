# GBDK-2020 Pfad anpassen
GBDK ?= /opt/gbdk
thunder.gb: main.c pong.inc tiles.h
	$(GBDK)/bin/lcc -Wm-yn"THUNDER" -o $@ main.c
tiles.h: gen_tiles.py
	python3 gen_tiles.py
clean:
	rm -f *.gb *.o *.map *.noi
