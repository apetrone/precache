#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	char * data;
	long dataSize;
	long offset;
} bitstream_t;

bitstream_t * bitstream_create();
void bitstream_destroy( bitstream_t * stream );
void bitstream_set( bitstream_t * stream, char * buffer, long dataSize );
void bitstream_reset( bitstream_t * stream );
int bitstream_read( bitstream_t * stream, void * dst, int numBytes );
int bitstream_write( bitstream_t * stream, const void * src, int numBytes );
void bitstream_seek( bitstream_t * stream, long offset, int isAbsolute );

#ifdef __cplusplus
}; // extern "C"
#endif
