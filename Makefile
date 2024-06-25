default: all

all: scraper

scraper: scraper.c
	gcc scraper.c -o scraper

clean:
	rm scraper
.PHONY: clean
