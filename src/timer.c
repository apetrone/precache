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
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif


void timer_startup( timevalue_t * t )
{
#if PARANOID
	if ( !t )
	{
		printf( "timevalue_t * is invalid!\n" );
		return;
	}
#endif

#if LINUX || __APPLE__
	gettimeofday(&t->initialtime, 0);
#elif _WIN32
	QueryPerformanceFrequency( &t->frequency );
	QueryPerformanceCounter( &t->last );
#endif
}

double timer_ms( timevalue_t * t )
{
#if PARANOID
	if (!t)
	{
		printf( "timevalue_t * is invalid!\n" );
		return 0;
	}
#endif

#if LINUX || __APPLE__
    struct timeval now;
    gettimeofday(&now, 0);
    return ((now.tv_sec-t->initialtime.tv_sec)*1000.0f + (now.tv_usec-t->initialtime.tv_usec)/1000.0f);
#elif _WIN32
	LARGE_INTEGER now;
	QueryPerformanceCounter( &now );

	return ((now.QuadPart - t->last.QuadPart) / (double)t->frequency.QuadPart) * 1000.0;
#endif
}

#ifdef __cplusplus
}; // extern "C"
#endif
