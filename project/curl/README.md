# Simple libcurl GET Client (C)

A minimal example program that performs an HTTP **GET** request using **libcurl** and streams the response body directly to `stdout` via a callback.

---

## 1. Build & Run

### Linux / macOS (pkg-config available)

```bash
gcc curl.c $(pkg-config --cflags --libs libcurl) -o curl
./curl
```

If you do **not** use `pkg-config`, specify includes / libs manually (paths differ per system):

```bash
gcc curl.c -I/usr/include -lcurl -o curl
```

On macOS with Homebrew:

```bash
brew install curl
# If necessary add: -I"$(brew --prefix curl)/include" -L"$(brew --prefix curl)/lib"
```

### Windows (MSYS2 / MinGW)

Install curl:

```bash
pacman -S mingw-w64-x86_64-curl
```

Compile:

```bash
x86_64-w64-mingw32-gcc curl.c -lcurl -o curl.exe
```

Run inside the same shell so runtime DLLs are found.

---

## 2. Program Overview

| Step | Function / Call       | Purpose                                                    |
| ---- | --------------------- | ---------------------------------------------------------- |
| 1    | `curl_global_init`    | Initialize global libcurl state (thread-safety, SSL, etc.) |
| 2    | `curl_easy_init`      | Allocate and return an easy handle (per-transfer state)    |
| 3    | `curl_easy_setopt`    | Configure URL, callback, user-agent, error buffer          |
| 4    | `curl_easy_perform`   | Execute the transfer synchronously                         |
| 5    | `curl_easy_getinfo`   | Query result metadata (HTTP status code)                   |
| 6    | `curl_easy_cleanup`   | Free easy handle resources                                 |
| 7    | `curl_global_cleanup` | Free global resources (last libcurl call)                  |

---

## 3. Source Walkthrough

### 3.1 Write Callback

```c
static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;       // libcurl passes element size & count
    size_t written = fwrite(buffer, 1, total, stdout); // stream directly
    return written; // MUST return number of bytes actually taken
}
```

* libcurl calls this repeatedly for each received data chunk.
* If return value < `total`, libcurl treats it as an error (`CURLE_WRITE_ERROR`).
* `userp` can carry context if you set `CURLOPT_WRITEDATA` (unused here, so default `stdout`).

### 3.2 Main Setup

```c
if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) { /* error */ }
CURL *handle = curl_easy_init();
```

* `CURL_GLOBAL_DEFAULT` activates all typical subsystems (SSL, Win32 sockets, etc.). Only call once per process.

### 3.3 Error Buffer

```c
char error_buf[CURL_ERROR_SIZE] = {0};
curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buf);
```

* Provides more specific diagnostic text on certain failures (SSL handshake, resolve errors, protocol issues). Must outlive `curl_easy_perform`.

### 3.4 Core Options

```c
curl_easy_setopt(handle, CURLOPT_URL,
  "https://webhook.site/c5697147-1add-4476-a9c4-10d41359a688");

curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
curl_easy_setopt(handle, CURLOPT_USERAGENT, "my-curl-app/1.0");
```

* `CURLOPT_URL`: full absolute URL (supports redirects unless disabled).
* `CURLOPT_WRITEFUNCTION`: custom sink for body.
* `CURLOPT_USERAGENT`: sets `User-Agent` header; default is a libcurl version string if omitted.

### 3.5 Perform & Inspect

```c
CURLcode response = curl_easy_perform(handle);
if (response == CURLE_OK) {
    long http_code = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &http_code);
    printf("\nSuccess (HTTP %ld)\n", http_code);
} else {
    fprintf(stderr, "Failed request. Code %d (%s)\n", response,
            curl_easy_strerror(response));
    if (error_buf[0]) fprintf(stderr, "Detailed: %s\n", error_buf);
}
```

* `curl_easy_perform` is **blocking**; returns a `CURLcode` error enum.
* `CURLINFO_RESPONSE_CODE` yields the HTTP status (e.g. 200, 404).
* Always check the returned `CURLcode` before trusting output.

### 3.6 Cleanup

```c
curl_easy_cleanup(handle);
curl_global_cleanup();
```

* Clean up in reverse order of creation.
* Safe to call `curl_global_cleanup()` once all easy/multi handles are freed.

# 4. Output
```
Welcome to curl application
This URL has no default content configured. <a href="https://webhook.site/#!/edit/c5697147-1add-4476-a9c4-10d41359a688">Change response in Webhook.site</a>.
Success (HTTP 200)
```