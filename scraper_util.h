#ifndef SCRAPER_UTIL_H
#define SCRAPER_UTIL_H

#include <stdio.h>

struct CURLResponse
{
    char *html;
    size_t size;
};

static size_t WriteHTMLCallback(void *contents, size_t size, size_t nmemb, void *userp);

struct CURLResponse GetRequest(CURL *curl_handle, const char *url);

#endif