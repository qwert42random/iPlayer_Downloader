default: all

all: scraper

scraper: scraper.c
	gcc -I./include -I./include/libxml2 scraper.c lib/libcurl.so lib/libxml2.so -g -O0 -o scraper

clean:
	rm scraper
.PHONY: clean
