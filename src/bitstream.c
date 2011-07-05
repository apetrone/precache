#include <bitstream.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C"
{
#endif

bitstream_t * bitstream_create()
{
	return (bitstream_t*)malloc( sizeof(bitstream_t) );
} // bitstream_create

void bitstream_destroy( bitstream_t * stream )
{
	free( stream );
} // bitstream_destroy

void bitstream_set( bitstream_t * stream, char * buffer, long dataSize )
{
	if ( stream )
	{
		stream->data = buffer;
		stream->dataSize = dataSize;
		stream->offset = 0;
	}
} // bitstream-set

void bitstream_reset( bitstream_t * stream )
{
	if ( stream )
	{
		stream->offset = 0;
	}
} // bitstream_reset

int bitstream_read( bitstream_t * stream, void * dst, int numBytes )
{
	if ( !stream )
	{
		return 0;
	}

	if ( stream->offset + numBytes > stream->dataSize )
	{
		printf( "bitstream_read: Read would cause access violation.\n" );
		return 0;
	}

	memcpy( dst, &stream->data[stream->offset], numBytes );
	stream->offset += numBytes;
	return numBytes;
} // bitstream_read

int bitstream_write( bitstream_t * stream, const void * src, int numBytes )
{
	if ( !stream )
	{
		return 0;
	}

	if ( stream->offset + numBytes >= stream->dataSize )
	{
		printf( "Write would cause access violation!\n" );
		return 0;
	}

	memcpy( &stream->data[stream->offset], src, numBytes );

	stream->offset += numBytes;
	return numBytes;
} // bitstream_write

void bitstream_seek( bitstream_t * stream, long offset, int isAbsolute )
{
	if ( !stream )
	{
		return;
	}

	if ( isAbsolute )
	{
		stream->offset = offset;
	}
	else
	{
		stream->offset += offset;
	}
} // bitstream_seek

#ifdef __cplusplus
}; // extern "C"
#endif
