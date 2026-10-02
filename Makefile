# GBDK-2020 Pfad anpassen
GBDK ?= /opt/gbdk
breakout.gb: main.c tiles.h
	$(GBDK)/bin/lcc -Wm-yn"BREAKOUT" -o $@ main.c
tiles.h: gen_tiles.py
	python3 gen_tiles.py
clean:
	rm -f *.gb *.o *.map *.noi
