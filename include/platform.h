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

    void platform_conform_slashes( char * path, int path_len );

	// normalize slashes in path to this platform's conventions
	void platform_path_normalize( char * path, int size );

	// make a single directory (the last in the path)
	int platform_mkdir( const char * normalized_path );

	// make all directories in the path tree
	void platform_makedirs( const char * normalized_path );

	// spawn a process; returns nonzero on success, zero on failure
	int platform_spawn_process( const char * path );

	// returns nonzero if this platform is 64bit; otherwise returns 0
	int platform_is64bit();
#ifdef __cplusplus
}; // extern "C"
#endif

