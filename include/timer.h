#pragma once

#if LINUX || __APPLE__
	#include <sys/time.h>
#elif _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct timer_s
{
#if LINUX || __APPLE__
	struct timeval initialtime;
#elif _WIN32
	LARGE_INTEGER frequency;
	LARGE_INTEGER last;
#endif

} timevalue_t;

void timer_startup( timevalue_t * t );
double timer_ms( timevalue_t * t );

#ifdef __cplusplus
}; // extern "C"
#endif
