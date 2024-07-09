#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <curl/typecheck-gcc.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/xpath.h>
#include <sys/stat.h>
#include "scraper_util.h"

int main(void)
{
    mkdir("output", 0755);
    mkdir("output/DoctorWho_1963-1996", 0755);

    // initialize curl globally
    curl_global_init(CURL_GLOBAL_ALL);

    // initialize a CURL instance
    CURL *curl_handle = curl_easy_init();

    // retrieve the HTML document of the target page
    struct CURLResponse response = GetRequest(curl_handle, "https://www.bbc.co.uk/iplayer/episodes/p0ggwr8l/doctor-who-19631996?seriesId=more-like-this");

    htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size, NULL, NULL, HTML_PARSE_NOERROR);
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        fprintf(stderr, "There was an error\n");
    }

    // Get the links for every season.
    // Scraping logic...
    xmlXPathObjectPtr productHTMLElements = xmlXPathEvalExpression((xmlChar *) "//div[contains(@id, 'main')]", context);
    if (productHTMLElements == NULL) {
        fprintf(stderr, "There was an error\n");
    }

    // Set the context to restrict XPath selectors to the children of the current element.
    xmlXPathSetContextNode(productHTMLElements->nodesetval->nodeTab[0], context);
    xmlXPathObjectPtr seasonHTMLElements = xmlXPathEvalExpression((xmlChar *) "./div[2]/div[2]/div/div/nav/ul/li", context);
    printf("Number of nodes: %d\n", seasonHTMLElements->nodesetval->nodeNr);

    struct SeasonData *season_list;
    season_list = malloc(sizeof(struct SeasonData) * (seasonHTMLElements->nodesetval->nodeNr - 1));

    for (int i = 0; i < seasonHTMLElements->nodesetval->nodeNr - 1; ++i) {
        // Get the current element of the loop.
        xmlNodePtr seasonNodePtr = seasonHTMLElements->nodesetval->nodeTab[i];

        // Get and print the scraped data.
        season_list[i].name = strdup((char *) (xmlNodeGetContent(seasonNodePtr)));
        season_list[i].link = strdup((char *) (xmlGetProp(seasonNodePtr->children, (xmlChar *) "href")));
    }

    for (int i = 0; i < seasonHTMLElements->nodesetval->nodeNr - 1; i++) {
        struct SeasonData *seasonPtr = &season_list[i];
        char dirPath[256];

        printf("\n--- %d: %s---\n\n", i + 1, seasonPtr->name);
        snprintf(dirPath, sizeof(dirPath), "output/DoctorWho_1963-1996/%d_%s", i + 1, seasonPtr->name);
        mkdir(dirPath, 0755);

        int pageCount = get_page_count(seasonPtr->link);
        if (pageCount < 0) {
            fprintf(stderr, "Error getting number of pages");
        }

        if (scrape_season(*seasonPtr, dirPath, pageCount))
            return -1;
    }

    // cleanup the curl instance
    curl_easy_cleanup(curl_handle);
    // cleanup the curl resources
    curl_global_cleanup();

    return 0;
}
