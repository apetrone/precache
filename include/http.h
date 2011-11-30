/*
Copyright (c) 2011, <Adam Petrone>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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
void http_download_file( const char * url, const char * temporaryFilePath, const char * user_agent, http_download_state_t * state );

// tick file download - check for new data from stream
void http_download_tick( http_download_state_t * state, int timeout_seconds );

#ifdef __cplusplus
}; // extern "C"
#endif
