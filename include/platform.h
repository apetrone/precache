#pragma once

#ifdef __cplusplus
extern "C"
{
#endif


#if _WIN32
    #define MAX_PATH_SIZE MAX_PATH
    #define PATH_SEPARATOR '\\'
	#define PATH_SEPARATOR_STRING "\\"
#elif LINUX || __APPLE__
    #include <limits.h>
    #define MAX_PATH_SIZE PATH_MAX
    #define PATH_SEPARATOR '/'
	#define PATH_SEPARATOR_STRING "/"
#endif


#if _WIN32

#elif LINUX || __APPLE__
	void * native_gl_findsymbol( const char * name );
#endif

#if _MSC_VER
	typedef unsigned __int64 memsize_t;
#elif __GNUC__
	typedef unsigned long long int memsize_t;
#endif

	int platform_startup( int argc, char ** argv );
	void platform_shutdown();

	typedef struct
	{
		// values are in bytes
		memsize_t totalPhysical;
		memsize_t availPhysical;
	} memory_info_t; // memory_info_t

    void platform_memory_info( memory_info_t * mi );

    // returns the path of the binary on success
    // returns 0 on failure
    // path will not have a trailing slash
    int platform_operating_directory( char * path, int size );

	// normalize slashes in path to this platform's conventions
	void platform_path_normalize( char * path, int size );

	// make a single directory (the last in the path)
	int platform_mkdir( const char * normalized_path );

	// make all directories in the path tree
	void platform_makedirs( const char * normalized_path );

#ifdef __cplusplus
}; // extern "C"
#endif

