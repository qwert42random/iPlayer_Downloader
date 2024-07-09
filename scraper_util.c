#include "scraper_util.h"

#include <curl/curl.h>
#include <curl/typecheck-gcc.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/xpath.h>
#include <stdlib.h>
#include <string.h>

static size_t WriteHTMLCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct CURLResponse *mem = (struct CURLResponse *)userp;
    char *ptr = realloc(mem->html, mem->size + realsize + 1);

    if (!ptr)
    {
        printf("Not enough memory available (realloc returned NULL)\n");
        return 0;
    }

    mem->html = ptr;
    memcpy(&(mem->html[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->html[mem->size] = 0;

    return realsize;
}

struct CURLResponse GetRequest(CURL *curl_handle, const char *url)
{
    CURLcode res;
    struct CURLResponse response;

    // initialize the response
    response.html = malloc(1);
    response.size = 0;

    // specify URL to GET
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    // send all data returned by the server to WriteHTMLCallback
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteHTMLCallback);
    // pass "response" to the callback function
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response);
    // set a User-Agent header
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36");
    // Allow redirecting
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // perform the GET request
    res = curl_easy_perform(curl_handle);

    // check for HTTP errors
    if (res != CURLE_OK)
    {
        fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));
    }

    return response;
}

int get_page_count(char *season_link) {
    // Check number of pages.
    CURL *curl_handle = curl_easy_init();
    char pageUrl[256];
    snprintf(pageUrl, sizeof(pageUrl), "https://www.bbc.co.uk%s", season_link);

    struct CURLResponse response = GetRequest(curl_handle, pageUrl);
    htmlDocPtr seasonDoc = htmlReadMemory(response.html, (unsigned long)response.size, NULL, NULL, HTML_PARSE_NOERROR);
    xmlXPathContextPtr context = xmlXPathNewContext(seasonDoc);
    if (context == NULL) {
        return -1;
    }

    xmlXPathObjectPtr pageNavNodePtr = xmlXPathEvalExpression((xmlChar *) "//div[contains(@id, 'main')]/div[2]/div[3]/div[2]/div/nav/ol/li", context);
    if (pageNavNodePtr == NULL) {
        return -1;
    }

    if (pageNavNodePtr == NULL || pageNavNodePtr->nodesetval == NULL)
        return 1;
    else
        return pageNavNodePtr->nodesetval->nodeNr;
}

// int scrape_season(char *season_link, char *outputPath, size_t pageCount) {
int scrape_season(struct SeasonData season, char *outputPath, size_t pageCount) {
    CURL *curl_handle = curl_easy_init();
    char url[256];
    int episodeIndex = 0;

    for (int page = 1; page <= pageCount; page++) {
        snprintf(url, sizeof(url), "https://www.bbc.co.uk%s&page=%d", season.link, (int) page);
        struct CURLResponse response = GetRequest(curl_handle, url);
        printf("URL: %s\n", url);

        htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size, NULL, NULL, HTML_PARSE_NOERROR);
        xmlXPathContextPtr context = xmlXPathNewContext(doc);
        if (context == NULL) {
            fprintf(stderr, "There was an error\n");
            return -1;
        }

        xmlXPathObjectPtr episodeNodePtr = xmlXPathEvalExpression((xmlChar *) "//div[contains(@id, 'main')]/div[2]/div[3]/div/div/ul/li", context);
        if (episodeNodePtr == NULL) {
            fprintf(stderr, "There was an error\n");
            return -1;
        }

        for (int i = 0; i < episodeNodePtr->nodesetval->nodeNr; i++) {
            xmlNodePtr episodeNode = episodeNodePtr->nodesetval->nodeTab[i];
            xmlXPathSetContextNode(episodeNode, context);
            xmlNodePtr xmlNodeEpisodeTitle = xmlXPathEvalExpression((xmlChar *) "./a/div[2]", context)->nodesetval->nodeTab[0];

            char *episodeTitle = strdup((char *) (xmlNodeGetContent(xmlNodeEpisodeTitle)));
            char *episodeLink = strdup((char *) (xmlGetProp(xmlNodeEpisodeTitle->parent, (xmlChar *) "href")));

            char command[512];
            snprintf(command, sizeof(command), "youtube-dl 'https://www.bbc.co.uk%s' --write-sub --output '%s/%d %s'", episodeLink, outputPath, ++episodeIndex, episodeTitle);
            if (system(command) != 0) {
                return -1;
            }
        }
    }

    return 0;
}