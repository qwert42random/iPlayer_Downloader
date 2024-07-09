#ifndef SCRAPER_UTIL_H
#define SCRAPER_UTIL_H

#include <stdio.h>
#include <curl/curl.h>

struct CURLResponse
{
    char *html;
    size_t size;
};

struct SeasonData {
    char *name;
    char *link;
};

static size_t WriteHTMLCallback(void *contents, size_t size, size_t nmemb, void *userp);

struct CURLResponse GetRequest(CURL *curl_handle, const char *url);

int get_page_count(char *season_link);

int scrape_season(struct SeasonData, char *dirPath, size_t pageCount);

#endif