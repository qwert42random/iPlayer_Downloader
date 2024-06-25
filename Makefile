default: all

all: scraper

scraper: scraper.c
	gcc -I./include -I./include/libxml2 scraper.c -o scraper

clean:
	rm scraper
.PHONY: clean
