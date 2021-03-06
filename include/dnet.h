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

#ifdef WIN32
	#pragma warning(disable: 4996)
	#pragma warning(disable: 4018)
	#include <winsock2.h>
	#include <windows.h>
	#include <Ws2tcpip.h>
	#pragma comment( lib, "ws2_32.lib" )
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <sys/time.h>
	#include <arpa/inet.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <unistd.h> // warning: implicit declaration of function 'close'
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
	typedef SOCKET sock_t;
#else
	typedef int sock_t;
#endif

typedef struct sockaddr_in address_t;



void address_init( address_t * address );
void address_set( address_t * address, const char * host, unsigned short port );
void address_host( address_t * address, char * buffer, unsigned int bufferLength );
unsigned short address_port( address_t * address );
int address_size();

//
// socket functions
// returns -1 on failure
int net_sock_open( sock_t * sock, int type );
void net_sock_close( sock_t * sock );
void net_sock_bind( sock_t sock, unsigned short port, int type );

// returns bytes sent
int net_sock_send( sock_t sock, address_t * to, const char * packet, int packetSize, int type );

// returns bytes received
int net_sock_recv( sock_t sock, address_t * from, char * buffer, int packetSize, int type );

// returns 0 on success, -1 on error
int net_sock_connect( sock_t sock, address_t * to );



// returns > 0 on success
int net_enableReuse( sock_t sock );
int net_enableNonBlocking( sock_t sock );

// split a hostname up into ip and port (if specified), otherwise returns hostname
void net_SplitHost( const char * ip, char * host, short * port );

// decompose a full url, "http://www.gesturebox.net/update/update.xml" into a hostname and file string.
// ex: "www.gesturebox.net", and "update.xml"
int net_decompose_url( const char * fullURL, char * file, char * hostname, char * service, short * port );

// retrieve the standard port for a service name; http -> 80, https -> 443, ftp -> 21, ssh -> 22, smtp -> 25
short net_standard_port_for_service( const char * service );

int net_startup();
void net_shutdown();

// convert errno to string
const char * net_errno_string();

int net_ipfromhost( const char * host, const char * service, int protocol, char * ip );

#ifdef __cplusplus
}; // extern "C"
#endif
