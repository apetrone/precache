#pragma once

#include "dnet.h"

#ifdef __cplusplus
extern "C" {
#endif

// don't process a header string longer than x bytes.
#define HTTP_MAX_HEADER_STRING_SIZE 256
#define HTTP_BUFFER_SIZE 65535

// Flags:
// 1 -> sent GET request, to read: headers
// 2 -> read headers, to read: content
typedef struct http_download_state_s
{
    int content_length;
    int bytes_read;
    int status;
    int error;
    int completed;
    int totalBytesIn;
    short flags;
    sock_t socket;
    FILE * handle; // temporary file handle
    fd_set readset;
    fd_set exceptset;
    struct timeval timeout;
} http_download_state_t;

// process an HTTP header line
void http_process_header( const char * line, int len, http_download_state_t * state );

// write stream data to temp file
void http_download_write( http_download_state_t * state, const char * data, int dataSize );

// initiate file download
void http_download_file( const char * url, const char * temporaryFilePath, http_download_state_t * state );

// tick file download - check for new data from stream
void http_download_tick( http_download_state_t * state, int timeout_seconds );

#ifdef __cplusplus
}; // extern "C"
#endif
