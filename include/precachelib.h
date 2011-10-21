#pragma once

#include "md5.h"
#include "JSON_parser.h"
#include "http.h"

#define PRECACHE_TIMEOUT_MS 3000

#define PRECACHE_URL "http://192.168.0.100/precache/mp"
#define PRECACHE_WINDOW_TITLE "Precache Test"

// define as > 0 to test rendering
// define as 0 for normal operations
#define PRECACHE_TEST 0


#define KB_DIV 1024
#define MB_DIV 1048576

#define PRECACHE_TEMP_BUFFER_SIZE 512
#define PRECACHE_STATE_DOWNLOAD_REQUEST 1 // request to download a file
#define PRECACHE_STATE_DOWNLOAD 2 // downloading a file
#define PRECACHE_STATE_EXIT 3
#define PRECACHE_STATE_ERROR 4


#if _WIN32
    #define MAX_PATH_SIZE MAX_PATH
    #define PATH_SEPARATOR '\\'
	#define PATH_SEPARATOR_STRING "\\"
#elif LINUX || __APPLE__
    #include <limits.h>
    #define MAX_PATH_SIZE PATH_MAX
    #define PATH_SEPARATOR '/'
	#define PATH_SEPARATOR_STRING "/"

	#define strnicmp strncasecmp
	#define stricmp strcasecmp
#endif


enum PrecacheState
{
	// an invalid state (at startup)
	PS_INVALID = 0,

	// downloading the precache.list
	PS_DOWNLOAD_LIST,

	// parsing the locally downloaded precache.list
	PS_PARSING_LIST,

	// finished parsing list
	PS_FINISHED_PARSING_LIST,

	// calculating MD5 checksums for local files
	PS_CALCULATING_CHECKSUMS,

	// download list parse error
	PS_PARSE_ERROR,

	// download the next file
	PS_DOWNLOAD_NEXT,

	// error while attempting to download a file
	PS_DOWNLOAD_ERROR,

	// download timed out; file is incomplete
	PS_DOWNLOAD_TIMEDOUT,

	// currently downloading
	PS_DOWNLOADING,
};

typedef struct precache_file_s
{
    char path[ 256 ];
    char checksum[ 33 ];
    int flags;
	float timestamp;

    struct precache_file_s * next;
} precache_file_t;

typedef struct precache_state_s
{
    int state;
    char base[ 128 ];
    char remotepath[ 256 ]; // a full url to the base folder where this project resides
    char localpath[ MAX_PATH_SIZE ]; // an absolute path to the folder on the local machine where these files should be placed
    char precache_file[ MAX_PATH_SIZE ];
    char currentfilepath[ MAX_PATH_SIZE ];
    char err1[1024];
    char err2[1024];

    precache_file_t * files;
    precache_file_t * curfile;
} precache_state_t;

typedef struct precache_parse_state_s
{
    int flags;
    char lastkey[ 128 ];
    precache_state_t * state;
} precache_parse_state_t;


typedef struct precache_thread_data_s
{
    int state;
    http_download_state_t * download;
    precache_state_t * precache;
} precache_thread_data_t;

typedef char md5_digest_t[ 33 ];


int precache_parse_list( precache_state_t * precache );
