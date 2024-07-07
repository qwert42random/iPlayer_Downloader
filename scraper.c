#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/typecheck-gcc.h>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/xpath.h>

struct CURLResponse
{
    char *html;
    size_t size;
};

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

int main()
{
    // initialize curl globally
    curl_global_init(CURL_GLOBAL_ALL);

    // initialize a CURL instance
    CURL *curl_handle = curl_easy_init();

    // retrieve the HTML document of the target page
    struct CURLResponse response = GetRequest(curl_handle, "https://www.bbc.co.uk/iplayer/episodes/p0ggwr8l/doctor-who-19631996?seriesId=b009x51p");

    // Get the links for every season.

    // Scraping logic...
    htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size, NULL, NULL, HTML_PARSE_NOERROR);
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        fprintf(stderr, "There was an error\n");
    }

    xmlXPathObjectPtr productHTMLElements = xmlXPathEvalExpression((xmlChar *) "//div[contains(@id, 'main')]", context);
    if (productHTMLElements == NULL) {
        fprintf(stderr, "There was an error\n");
    }

    // Set the context to restrict XPath selectors to the children of the current element.
    xmlXPathSetContextNode(productHTMLElements->nodesetval->nodeTab[0], context);
    xmlXPathObjectPtr seasonHTMLElements = xmlXPathEvalExpression((xmlChar *) "./div[2]/div[2]/div/div/nav/ul/li", context);
    printf("Number of nodes: %d\n", seasonHTMLElements->nodesetval->nodeNr);

    for (int i = 0; i < seasonHTMLElements->nodesetval->nodeNr; ++i) {
        // Get the current element of the loop.
        xmlNodePtr productHTMLElement = seasonHTMLElements->nodesetval->nodeTab[i];

        // Get and print the scraped data.
        char *name = strdup((char *) (xmlNodeGetContent(productHTMLElement)));
        printf("%d: %s\n", i + 1, name);
    }

    // cleanup the curl instance
    curl_easy_cleanup(curl_handle);
    // cleanup the curl resources
    curl_global_cleanup();

    return 0;
}
