#include "http.h"

#include "log.h"

#if LINUX || __APPLE__
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define dprintf //
//#define dprintf log_msg

void http_download_write( http_download_state_t * state, const char * data, int dataSize )
{
    if ( !state->handle )
    {
        printf( "File handle is invalid.\n" );
        state->error = 1;
        return;
    }

	fwrite( data, 1, dataSize, state->handle );
	state->bytes_read += dataSize;
    fflush( state->handle );
} // http_download_write

void http_process_header( const char * line, int len, http_download_state_t * state )
{
	int i;

	if ( len > HTTP_MAX_HEADER_STRING_SIZE )
        return;

	if ( strnicmp( line, "connection", 10 ) == 0 )
	{
		dprintf( "Connection header\n" );
	}
	else if ( strnicmp( line, "content-type", 12 ) == 0 )
	{
		dprintf( "content type header\n" );
	}
	else if ( strnicmp( line, "content-length", 14 ) == 0 )
	{
	    state->content_length = atoi( (line+15) );
	    dprintf( "Content_length: %i\n", state->content_length );
	}

	dprintf( " -->" );

	for( i = 0; i < len; ++i )
	{
		if ( line[i] == ':' )
		{

		}

		dprintf("%c", line[i] );
	}

	dprintf("\n");
} // http_process_header

void http_download_file( const char * url, const char * temporaryFilePath, http_download_state_t * state )
{
    int err;
    int bytesSent;
	char service[ 32 ] = {0};
    char filename[ 256 ] = {0};
    char hostname[ 1024 ] = {0};
    char host[1024] = {0};
    char GETrequest[ 1024 ] = {0};
	char ip[32] = {0};
    short port = 0;
    address_t hostaddress;

    state->content_length = 0;
    state->bytes_read = 0;
    state->status = -1;
    state->error = 0;
    state->completed = 0;
    state->socket = -1;
    state->handle = 0;
    state->flags = 0;
    state->totalBytesIn = 0;


    // init socket, TCP
    err = net_sock_open( &state->socket, 1 );
    if ( err == -1 )
    {
		log_msg( "* HTTP: Unable to open socket!\n" );
        printf( "Unable to open socket!\n" );
        state->error = 1;
        return;
    }

    //net_enableNonBlocking( state->socket );
    //net_enableReuse( state->socket );

    // decompose into hostname and file
    net_decompose_url( url, filename, hostname, service, &port );

    // decompose host name into host/ip and port
    net_SplitHost( hostname, host, &port );

	// try to determine the correct port...
	if ( port == 0 )
	{
        port = net_standard_port_for_service(service);
	}

	// convert hostname to ip
	net_ipfromhost( host, "http", 1, ip );

	dprintf( "* HTTP: Host: %s (%s)\n", host, ip );
    dprintf( "* HTTP: Port: %i\n", port );
    dprintf( "* HTTP: File: %s\n", filename );

	// establish connection with server
    address_set( &hostaddress, ip, port );
    err = net_sock_connect( state->socket, &hostaddress );

    if ( err < 0 )
    {
        dprintf( "* HTTP: Could not connect to server %i -> %s (%s:%i)\n", err, net_errno_string(), host, port );
        state->error = 1;
        return;
    }

    // send GET request to server
    sprintf( GETrequest, "GET %s HTTP/1.1\r\nUser-Agent: Auto-Patch\r\nAccept: */*\r\nHost: %s\r\nConnection: close\r\n\r\n", filename, host );

    dprintf( "* HTTP: <- %s\n", GETrequest );
    bytesSent = net_sock_send( state->socket, 0, GETrequest, strlen(GETrequest), 1 );

    dprintf( "* HTTP: <- Sent %i bytes to server...\n", bytesSent );
    state->flags = 1;

    // open temporary file
    state->handle = fopen( temporaryFilePath, "wb" );
    if ( state->handle == 0 )
    {
        dprintf( "* HTTP: Error opening \"%s\"!\n", temporaryFilePath );
        state->error = 1;
        return;
    }

} // http_download_file


void http_download_tick( http_download_state_t * state, int timeout_seconds )
{
    int socksready;
    char buffer[ HTTP_BUFFER_SIZE ];
    state->timeout.tv_sec = timeout_seconds;
    state->timeout.tv_usec = 0;

    FD_ZERO( &state->readset );
    FD_ZERO( &state->exceptset );

    FD_SET( state->socket, &state->readset );
    FD_SET( state->socket, &state->exceptset );

    socksready = select( (state->socket+1), &state->readset, &state->exceptset, 0, &state->timeout );

    if ( socksready > 0 )
    {
        // is our socket ready for read?
        if ( FD_ISSET( state->socket, &state->readset ) )
        {
            memset( buffer, 0, HTTP_BUFFER_SIZE );

            if ( state->flags & 2 ) // are we reading content?
            {
                int bytesIn = net_sock_recv(state->socket, 0, buffer, HTTP_BUFFER_SIZE, 1);
                http_download_write( state, buffer, bytesIn );

                if ( bytesIn == 0 )
                {
                    state->completed = 1;
					if ( state->handle )
					{
						fclose( state->handle );
						state->handle = 0;
					}

					// close the socket
					net_sock_close( &state->socket );

                    dprintf( "* File Completed. Size: %i bytes\n", state->bytes_read );
                }
            }
            else if ( state->flags & 1 ) // we are reading headers, the GET request was already sent...
            {
                int bytesIn;
                int headerSize;
                char * contentStart;

                // for header processing
                char * cur;
                char * lineStart;
                int protocol_major;
                int protocol_minor;
                int status = -1;
                char msg[32];

                //do
                //{
                    bytesIn = net_sock_recv(state->socket, 0, buffer, HTTP_BUFFER_SIZE, 1);
                    state->totalBytesIn += bytesIn;

                //} while( bytesIn > 0 );


                contentStart = strstr( buffer, "\r\n\r\n" );
                // if we detect "\r\n\r\n" in the buffer, then we can pause reading the socket.
                state->flags |= 2;
                contentStart += 4;
                headerSize = (contentStart-buffer);
                dprintf( "* Detected content start!\n" );


                // process headers here

                // start reading headers from buffer to p
                dprintf( "* Begin reading Response Headers. Header Size is %i bytes.\n", headerSize );

                cur = buffer;
                lineStart = buffer;

                for( ; cur <= contentStart; )
                {
                    if ( cur[0] == '\r' && cur[1] == '\n' )
                    {
                        if ( status > 0 )
                        {
                            if ( (cur-lineStart) > 0 )
                            {
								dprintf( "* Parse Header, %i bytes\n", (cur-lineStart) );
                                http_process_header( lineStart, cur-lineStart, state );
                            }
                        }
                        else
                        {
                            memset( msg, 0, 32 );
                            dprintf( "* Parse Status Line, %i bytes\n", (cur-lineStart) );
                            sscanf( lineStart, "HTTP/%d.%d %d %s", &protocol_major, &protocol_minor, &status, (char*)&msg );
                            dprintf( "* HTTP/%d.%d %d %s\n", protocol_major, protocol_minor, status, msg );
                            if ( status != 200 )
                            {
                                // the data we read was not content for the file...
                                state->bytes_read = 0;
                                dprintf( "Status: %i\n", status );
                                state->error = 1;
                                if ( status == 404 )
                                {
                                    printf( "File not Found!\n" );
                                }
                                else if ( status == 500 )
                                {
                                    printf( "Internal Server Error!\n" );
                                }
                            }
                        }

                        lineStart = cur+2;
                        cur+= 2;
                        continue;
                    }

                    ++cur;
                } // for loops

                if ( status == 200 )
                {
                    // TCP can arrive in bursts, so we may actually read past the headers and into the file data...
                    if ( state->totalBytesIn > headerSize )
                    {
                        dprintf( "* Buffer read %i bytes past headers - writing this to file...\n", state->totalBytesIn-headerSize);

                        // these bytes belong to the file data
                        http_download_write( state, &buffer[(contentStart-buffer)], state->totalBytesIn - (contentStart-buffer) );
                    }
                }
                else
                {
                    // not going to read into this file, cleanup.
                    fclose( state->handle );
                    state->handle = 0;
                }
            }
        }
        else if ( FD_ISSET( state->socket, &state->exceptset ) )
        {

        }
    }
    else if ( socksready == -1 )
    {
        printf( "select() returned error!\n" );
        state->error = 1;
    }
    else
    {
        printf( "no socks ready.\n" );
    }
} // http_download_tick

#ifdef __cplusplus
}; // extern "C"
#endif
