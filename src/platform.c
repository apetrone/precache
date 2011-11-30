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
#include "platform.h"
#include <string.h>

#if _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <direct.h> // for _mkdir
#elif LINUX
    #include <sys/sysinfo.h>
	//#include <errno.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <stdio.h> // for snprintf
    #include <stdlib.h> // for abort
	#include <unistd.h>
#elif __APPLE__
	#include <sys/types.h>
	#include <stdio.h>
	#include <sys/stat.h>
	#include <inttypes.h>
	#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif


void platform_conform_slashes( char * path, int path_len )
{
#if _WIN32
	int i;

	for( i = 0; i < path_len; ++i )
	{
		if ( path[i] == '/' )
		{
			path[i] = '\\';
		}
	}
#endif
}

// ------------------------------------------------------------------
int platform_operating_directory( char * path, int size )
{
    int result;
    char * sep;

#if _WIN32
	result = GetModuleFileNameA( GetModuleHandleA(0), path, size);
#elif LINUX
    {
        // http://www.flipcode.com/archives/Path_To_Executable_On_Linux.shtml
        char linkname[ 64 ] = {0};
        pid_t pid;


        pid = getpid();

        if ( snprintf(linkname, sizeof(linkname), "/proc/%i/exe", pid ) < 0 )
        {
            abort();
        }

        result = readlink( linkname, path, size );

        if ( result == -1 )
        {
            result = 0;
        }
        else
        {
            path[result] = 0;
        }
    }
#elif __APPLE__
	extern int _NSGetExecutablePath(char* buf, uint32_t* bufsize);
	char basepath[1024];
	uint32_t bpsize = sizeof(basepath);

	if ( _NSGetExecutablePath(basepath, &bpsize) == 0 )
	{
		if ( realpath(basepath, path) != 0 )
		{
		    result = strlen( path );
		}
	}
	else
	{
	    // basepath too small
	    result = 0;
	}

#endif

    if ( result != 0 )
    {
        sep = strrchr( path, PATH_SEPARATOR );

        if ( sep )
        {
            *sep = '\0';
        }
    }


    return result;
}


// ------------------------------------------------------------------
void platform_path_normalize( char * path, int size )
{
	while( *path )
	{
		if ( *path == '/' || *path == '\\' )
		{
			// conform to this platform's path separator
			*path = PATH_SEPARATOR;
		}

		++path;
	}
}


// ------------------------------------------------------------------
int platform_mkdir( const char * path )
{
	int result = 0;

#if _WIN32
	result = _mkdir( path );
#elif LINUX || __APPLE__
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/mkdir.html
	result = mkdir( path, (S_IRUSR | S_IWUSR | S_IXUSR ) | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH );
#endif

	return result;
}

// ------------------------------------------------------------------
void platform_makedirs( const char * normalized_path )
{
	const char * path = normalized_path;
	char directory[ MAX_PATH_SIZE ];

	// don't accept paths that are too short
	if ( strlen( normalized_path ) < 2 )
	{
		return;
	}

	memset( directory, 0, MAX_PATH_SIZE );

	// loop through and call mkdir on each separate directory progressively
	while( *path )
	{
		if ( *path == PATH_SEPARATOR )
		{
			strncpy( directory, normalized_path, (path+1)-normalized_path );
			platform_mkdir( directory );
		}

		++path;
	}
}

// ------------------------------------------------------------------
int platform_spawn_process( const char * path )
{
	int value;
#if _WIN32
	STARTUPINFOA startupInfo;
	PROCESS_INFORMATION processInfo;

	memset( &startupInfo, 0, sizeof(STARTUPINFO) );
	startupInfo.cb = sizeof(STARTUPINFO);

	memset( &processInfo, 0, sizeof(PROCESS_INFORMATION) );

	// returns nonzero on success, 0 on failure
	value = CreateProcessA( path, 0, 0, 0, 0, NORMAL_PRIORITY_CLASS, 0, 0, &startupInfo, &processInfo );
#elif LINUX || __APPLE__
	pid_t pid;

#if LINUX
	const char * args[ 1024 ];
	memset( args, 0, sizeof(char*)*1024 );
#else
	char * const args[] = { "/bin/echo", "success", 0 };
#endif

	pid = fork();
	value = 1;
	if ( pid == 0 )
	{
		execvp( path, (char**)args );
		return 1;
	}
#endif

	return value;
}

// ------------------------------------------------------------------
int platform_is64bit()
{
#if _WIN32
	// straight out of MSDN; this function isn't supported on all versions of Windows.
	// We must check and see if kernel32.dll implements it first.
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	BOOL b64 = 0;

	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    if(NULL != fnIsWow64Process)
    {
        fnIsWow64Process( GetCurrentProcess(), &b64 );
    }

	return b64;
#elif LINUX || __APPLE__

	char data[32] = {0};
	FILE * fp;
	fp = popen( "uname -m", "r" );
	if ( !fp )
	{
		return 0;
	}

	if ( !fgets( data, 7, fp ) )
	{
		return 0;
	}

	if ( !strncmp( data, "x86_64", 6 ) )
	{
		return 1;
	}
#endif

	return 0;
}

#ifdef __cplusplus
}; // extern "C"
#endif

