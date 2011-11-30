#pragma once

#include "md5.h"
#include "JSON_parser.h"
#include "http.h"
#include "vars.h"

#define PRECACHE_USER_AGENT "precache-downloader"
#define PRECACHE_TIMEOUT_MS 20000
#define PRECACHE_DEFAULT_FILE_PERMISSIONS "755"
#define PRECACHE_TEMP_BUFFER_SIZE 512
#define PRECACHE_STATE_DOWNLOAD_REQUEST 1 // request to download a file
#define PRECACHE_STATE_DOWNLOAD 2 // downloading a file
#define PRECACHE_STATE_EXIT 3
#define PRECACHE_STATE_ERROR 4
#define KB_DIV 1024
#define MB_DIV 1048576

// define as > 0 to testing
// define as 0 for normal operations
#define PRECACHE_TEST 0

#define XWL_DEBUG 0
#define THREAD_DEBUG 0
#define PARSE_DEBUG 0

#if XWL_DEBUG
	#define xwlPrintf log_msg
#else
	#define xwlPrintf //
#endif

#if THREAD_DEBUG
	#define THREAD_MSG log_msg
#else
	#define THREAD_MSG //
#endif

#if PARSE_DEBUG
	#define parse_msg log_msg
#else
	#define parse_msg //
#endif


// parser constants
#define PRECACHE_LIST_REMOTE_PROJECT_PATH "remote_project_path"
#define PRECACHE_LIST_INSTALL_PATH "install_path"
#define PRECACHE_LIST_VERSION "version"
#define PRECACHE_LIST_MD5 "md5"
#define PRECACHE_LIST_TARGET "target"
#define PRECACHE_LIST_PATH "path"
#define PRECACHE_LIST_FLAGS "flags"
#define PRECACHE_LIST_FILELIST "filelist"
#define PRECACHE_LIST_UPDATELIST "updaters"
#define PRECACHE_LIST_CHMOD "mode"

/*
Example: precache.conf
/////////////////////////////////////////////

{
	"deploysource" : "test",
	"localpath" : "folder",
	"remotepath" : "test"
}

/////////////////////////////////////////////
*/



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

// NOTE: These constants should match the constants in precache.py! (get_platform_id, get_arch_id)
#if _WIN32
	#define PRECACHE_PLATFORM 1
#elif LINUX
	#define PRECACHE_PLATFORM 2
#elif __APPLE__
	#define PRECACHE_PLATFORM 3
#endif

#define PRECACHE_ARCH_X86 1
#define PRECACHE_ARCH_X64 2

// indices of file flags
enum
{
	// architecture type (see PRECACHE_ARCH_*)
	PRECACHE_FILE_ARCH_BIT = 0,

	// specific platform (see PRECACHE_PLATFORM)
	PRECACHE_FILE_PLATFORM_BIT = 4,

	// indicates this file will be executed after successful downloads
	PRECACHE_FILE_EXECUTE_BIT = 8
};

enum PrecacheFlags
{
	PF_DOWNLOADED = 1
};


enum json_parse_state
{
	JPS_NONE		= 0,
	JPS_FILELIST	= 1,
	JPS_FILE		= 2,
	JPS_UPDATERLIST	= 4,
};

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

	// operations completed successfully
	PS_COMPLETED,

	// precache needs to update itself
	PS_UPDATE_SELF,
};

typedef struct precache_file_s
{
    char path[ MAX_PATH_SIZE ];
	char targetpath[ MAX_PATH_SIZE ]; // if specified, the destination where to save this file locally. otherwise, same as 'path'
    char checksum[ 33 ];
    int flags;
	short extra_flags;
	float timestamp;
	int mode;

    struct precache_file_s * next;
} precache_file_t;

typedef struct precache_state_s
{
    int state;
    char relativepath[ 128 ]; // relative path to localpath where the files are stored
    char remotepath[ MAX_PATH_SIZE ]; // a full url to the base folder where this project resides
    char localpath[ MAX_PATH_SIZE ]; // an absolute path to the folder on the local machine where these files should be placed
    char precache_file[ MAX_PATH_SIZE ];
    char currentfilepath[ MAX_PATH_SIZE ];
    char err1[1024];
    char err2[1024];

    precache_file_t * files;
    precache_file_t * curfile;

	// files that are flagged in the updatelist
	precache_file_t * updatelist;
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


void md5_from_path( const char * filename, char * digest );
int precache_parse_list( precache_state_t * precache );
int precache_parse_listbuffer( precache_state_t * precache, char * buffer, int bufferSize );

precache_file_t * precache_locate_next_file( precache_file_t * start );
precache_file_t * precache_locate_executable_file( precache_file_t * start );

// make sure the path has no trailing slashes
void precache_sanitize_path( char * path );

// allocate an array to hold the size of the file at path; must be free'd
char * allocate_file_buffer( const char * path, long * fileSize );

// determine if precache needs to update itself
int precache_should_update_self( precache_state_t * precache );

#if LINUX || __APPLE__
// convert a string of characters (chmod) to integer flags
int precache_mode_string_to_integer( const char * mode );
#endif

void float_color_to_char( float * colors, unsigned char * out );