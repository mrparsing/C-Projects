#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    // Writes the block directly to stdout
    size_t written = fwrite(buffer, 1, total, stdout);
    // (Optional) force flush if you want to see streaming output
    // fflush(stdout);
    return written; // Must return total or cURL will complain
}

int main(void) {
    printf("Welcome to curl application\n");

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        fprintf(stderr, "curl_global_init failed\n");
        return 1;
    }

    CURL *handle = curl_easy_init();
    if (!handle) {
        fprintf(stderr, "curl_easy_init failed\n");
        curl_global_cleanup();
        return 1;
    }

    // Buffer for more detailed error messages
    char error_buf[CURL_ERROR_SIZE] = {0};
    curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buf);

    curl_easy_setopt(handle, CURLOPT_URL,
        "https://webhook.site/c5697147-1add-4476-a9c4-10d41359a688");

    // Set the write callback
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
    // If you wanted to pass data to the callback: use CURLOPT_WRITEDATA

    // (Optional) User-Agent
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "my-curl-app/1.0");

    CURLcode response = curl_easy_perform(handle);
    if (response == CURLE_OK) {
        long http_code = 0;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &http_code);
        printf("\nSuccess (HTTP %ld)\n", http_code);
    } else {
        fprintf(stderr, "Failed request. Code %d (%s)\n",
                response,
                curl_easy_strerror(response));
        if (error_buf[0] != '\0') {
            fprintf(stderr, "Detailed: %s\n", error_buf);
        }
    }

    curl_easy_cleanup(handle);
    curl_global_cleanup();
    return (response == CURLE_OK) ? 0 : 1;
}