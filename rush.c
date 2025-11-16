#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define VERSION "0.2.0"
#define MAX_URLS 1000

typedef struct {
    char *url;
    char *data;
    size_t size;
    long response_code;
    double time_taken;
    int success;
    char error[256];
} Response;

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if(!ptr) return 0;
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

int is_valid_url(const char *url) {
    if(!url || strlen(url) == 0) return 0;
    if(strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) return 0;
    return 1;
}

int fetch_urls_parallel(char **urls, int url_count, Response *responses) {
    CURLM *multi_handle = curl_multi_init();
    int still_running = 0;
    CURL *handles[MAX_URLS];
    MemoryStruct chunks[MAX_URLS];
    
    for(int i = 0; i < url_count; i++) {
        chunks[i].data = NULL;
        chunks[i].size = 0;
        handles[i] = NULL;
    }
    
    for(int i = 0; i < url_count; i++) {
        if(!is_valid_url(urls[i])) {
            responses[i].url = strdup(urls[i]);
            responses[i].success = 0;
            snprintf(responses[i].error, sizeof(responses[i].error), "Invalid URL");
            continue;
        }
        
        handles[i] = curl_easy_init();
        if(!handles[i]) continue;
        
        curl_easy_setopt(handles[i], CURLOPT_URL, urls[i]);
        curl_easy_setopt(handles[i], CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(handles[i], CURLOPT_WRITEDATA, &chunks[i]);
        curl_easy_setopt(handles[i], CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(handles[i], CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(handles[i], CURLOPT_PRIVATE, (void *)(long)i);
        curl_multi_add_handle(multi_handle, handles[i]);
    }
    
    curl_multi_perform(multi_handle, &still_running);
    while(still_running) {
        int numfds;
        curl_multi_wait(multi_handle, NULL, 0, 1000, &numfds);
        curl_multi_perform(multi_handle, &still_running);
    }
    
    int msgs_left;
    CURLMsg *msg;
    while((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
        if(msg->msg == CURLMSG_DONE) {
            long idx;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &idx);
            int i = (int)idx;
            responses[i].url = strdup(urls[i]);
            responses[i].data = chunks[i].data;
            responses[i].size = chunks[i].size;
            
            if(msg->data.result == CURLE_OK) {
                responses[i].success = 1;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &responses[i].response_code);
                curl_easy_getinfo(msg->easy_handle, CURLINFO_TOTAL_TIME, &responses[i].time_taken);
            } else {
                responses[i].success = 0;
                snprintf(responses[i].error, sizeof(responses[i].error), "%s", curl_easy_strerror(msg->data.result));
            }
        }
    }
    
    for(int i = 0; i < url_count; i++) {
        if(handles[i]) {
            curl_multi_remove_handle(multi_handle, handles[i]);
            curl_easy_cleanup(handles[i]);
        }
    }
    curl_multi_cleanup(multi_handle);
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s <url1> [url2] ...\n", argv[0]);
        return 1;
    }
    if(strcmp(argv[1], "--version") == 0) {
        printf("rush v%s\n", VERSION);
        return 0;
    }
    
    int url_count = argc - 1;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    Response *responses = calloc(url_count, sizeof(Response));
    
    printf("Fetching %d URLs...\n\n", url_count);
    fetch_urls_parallel(&argv[1], url_count, responses);
    
    for(int i = 0; i < url_count; i++) {
        printf("URL: %s\n", responses[i].url);
        if(responses[i].success) {
            printf("Status: %ld\n", responses[i].response_code);
            printf("Time: %.3fs\n", responses[i].time_taken);
            printf("Size: %zu bytes\n", responses[i].size);
        } else {
            printf("FAILED: %s\n", responses[i].error);
        }
        printf("\n");
        free(responses[i].url);
        free(responses[i].data);
    }
    
    free(responses);
    curl_global_cleanup();
    return 0;
}
