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
	int buffer_size = 0;
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

		buffer_size = strlen(logstate.buffer[logstate.buffer_id]);
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
