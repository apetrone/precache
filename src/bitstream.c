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
