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
#include "log.h"
#include "dnet.h"
#include <stdio.h>


#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_BUFFER_SIZE 8192
#define LOG_MAX_BUFFERS 8

typedef struct
{
	FILE * handle;
	sock_t socket;
	short initialized;
	short buffer_id;
	char buffer[LOG_MAX_BUFFERS][ LOG_BUFFER_SIZE ];
} log_state_t;


log_state_t logstate;

int log_init_file( const char * filename )
{
	logstate.initialized = 1;
	logstate.socket = -1;

	logstate.handle = fopen( filename, "wb" );
	if ( !logstate.handle )
		return 0;

	log_msg( "* Initialized log file.\n" );
	return 1;
} // log_init_file


int log_init_net()
{
	logstate.initialized = 1;
	logstate.handle = 0;

	return 1;
}

void log_msg( const char * format, ... )
{
//	int buffer_size = 0;
	va_list args;

	if ( !logstate.initialized )
	{
		printf( "* LOG: Not initialized. Please init with log_init_file() or log_init_net()\n" );
		return;
	}

	if ( !logstate.handle && logstate.socket == -1 )
	{
		printf( "* LOG: No valid logging provider!\n" );
		return;
	}

	// increment the current buffer
	++logstate.buffer_id;
	logstate.buffer_id = logstate.buffer_id % LOG_MAX_BUFFERS;

	if ( logstate.handle )
	{
		va_start(args, format);
		vfprintf( logstate.handle, format, args );
		va_end(args);
		//fwrite( logstate.buffer, buffer_size, 1, logstate.handle );
		fflush( logstate.handle );
	}
	else if ( logstate.socket )
	{
		va_start(args, format);
		vsprintf(logstate.buffer[logstate.buffer_id], format, args);
		va_end(args);

//		buffer_size = strlen(logstate.buffer[logstate.buffer_id]);
	}

} // log_msg

void log_shutdown()
{
	logstate.initialized = 0;

	if (logstate.handle)
	{
		fclose(logstate.handle);
		logstate.handle = 0;
	}
} // log_shutdown




#ifdef __cplusplus
}; // extern "C"
#endif
