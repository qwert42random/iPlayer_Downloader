default: all

all: scraper

scraper: scraper.c scraper_util.o
	gcc -I./include -I./include/libxml2 scraper.c scraper_util.o lib/libcurl.so lib/libxml2.so -g -O0 -o scraper

scraper_util.o: scraper_util.c scraper_util.h
	gcc -I./include -I./include/libxml2 scraper_util.c -c

clean:
	rm scraper scraper_util.h.gch scraper_util.o
.PHONY: clean
