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
