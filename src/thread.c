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
#include <thread.h>

#define PARANOID 1

#if LINUX || __APPLE__
	#include <signal.h> // for pthread_kill
#endif

void thread_start( thread_t * t, thread_entry entry, void * data )
{
	if ( !t || !entry )
		return;

#if _WIN32
	t->handle = CreateThread( 0, 0, entry, data, 0, 0 );
	//t->handle = (HANDLE)_beginthreadex( 0, 0, entry, data, 0, &t->id );
	t->state = THREAD_STATE_ACTIVE;
#elif LINUX || __APPLE__
	{
		int err;

		// create a joinable thread
		pthread_attr_init( &t->attribs );
		pthread_attr_setdetachstate( &t->attribs, PTHREAD_CREATE_JOINABLE );

		err = pthread_create( &t->handle, &t->attribs, entry, data );

		if ( err == 0 )
		{
			t->state = THREAD_STATE_ACTIVE; // success
		}
		else
		{
			t->state = THREAD_STATE_INVALID;
		}
	}
#endif
} // thread_start

void thread_stop( thread_t * t )
{
	if ( !t )
		return;

#if _WIN32
	if ( WaitForSingleObject( t->handle, 1000 ) != WAIT_OBJECT_0 )
	{
		// Thread timed out!
	}

	CloseHandle( (HANDLE)t->handle );
	t->handle = 0;
	t->state = THREAD_STATE_STOPPED;
#elif LINUX || __APPLE__
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, 0 );
	pthread_cancel( (pthread_t)&t->handle );
	t->state = THREAD_STATE_STOPPED;
#endif
} // thread_stop

unsigned int thread_status( thread_t * t )
{
	if ( !t )
	{
		return THREAD_STATE_INVALID;
	}
#if _WIN32
	{
		unsigned int code;
		if ( GetExitCodeThread( t->handle, (LPDWORD)&code ) )
		{
			if ( code == STILL_ACTIVE )
			{
				t->state = THREAD_STATE_ACTIVE;
			}
			else
			{
				t->state = THREAD_STATE_STOPPED;
			}
		}
	}

#elif LINUX || __APPLE__
	if ( pthread_kill( t->handle, 0 ) == 0 )
	{
		// thread is still active
		t->state = THREAD_STATE_ACTIVE;
	}
	else
	{
		t->state = THREAD_STATE_STOPPED;
	}
#endif

	return t->state;
} // thread_status

#if 0
unsigned int thread_id( thread_t * t )
{
	if ( !t )
		return 0;

	return t->id;
} // thread_id

int thread_selfid()
{
#if _WIN32
	return GetCurrentThreadId();
#elif LINUX || __APPLE__
	return pthread_self();
#endif
} // thread_selfid
#endif

int thread_join( thread_t * t, int timeout_milliseconds )
{
	int ret;

#if _WIN32

	if ( timeout_milliseconds == -1 )
		timeout_milliseconds = INFINITE;

	// NOTE:
	// * MsgWait continues the MessagePump in case this thread created any Windows.
	// * MsgWaitForMultipleObjectsEx( 1, &hThread, milliseconds, QS_ALLEVENTS, 0 );
	ret = WaitForSingleObjectEx( (HANDLE)t->handle, timeout_milliseconds, 0 );

	if ( ret != WAIT_OBJECT_0 )
	{
		// thread did not terminate
		TerminateThread( (HANDLE)t->handle, 0 );
	}

	// close the OS handle
	CloseHandle( (HANDLE)t->handle );
	t->handle = 0;
	t->state = THREAD_STATE_STOPPED;

	if ( ret == WAIT_OBJECT_0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
#elif LINUX || __APPLE__
	ret = pthread_join( t->handle, 0 );

	if ( ret == 0 )
	{
		t->state = THREAD_STATE_STOPPED;
		return 1;
	}
	else
	{
		t->state = THREAD_STATE_INVALID;
		return 0;
	}
#endif
} // thread_join

void thread_sleep( int milliseconds )
{
#if _WIN32
	Sleep( milliseconds );
#elif LINUX || __APPLE__
	// convert milliseconds to microseconds
	usleep( milliseconds * 1000 );
#endif
} // thread_sleep

//--------------------------------------------------

// NOTE:
// * pthread_mutex_trylock( &m->handle );
// * if mutex is unlocked, this will lock it and return zero.
// * otherwise it will return with EBUSY



void mutex_create( mutex_t * m )
{
#if PARANOID
	if ( !m )
		return;
#endif

#if _WIN32
	InitializeCriticalSection( &m->cs );
#elif LINUX || __APPLE__
	pthread_mutexattr_init( &m->attribs );
	pthread_mutex_init( &m->handle, &m->attribs );
#endif
} // mutex_create

void mutex_destroy( mutex_t * m )
{
#if _WIN32
	DeleteCriticalSection( &m->cs );
#elif LINUX || __APPLE__
	pthread_mutex_destroy( &m->handle );
	pthread_mutexattr_destroy( &m->attribs );
#endif
} // mutex_destroy

void mutex_lock( mutex_t * m )
{
#if PARANOID
	if ( !m )
		return;
#endif

#if _WIN32
	if ( m->cs.DebugInfo != 0 )
	{
		EnterCriticalSection( &m->cs );
	}
#elif LINUX || __APPLE__
	pthread_mutex_lock( &m->handle );
#endif
} // mutex_lock

void mutex_unlock( mutex_t * m )
{
#if PARANOID
	if ( !m )
		return;
#endif

#if _WIN32
	LeaveCriticalSection( &m->cs );
#elif LINUX || __APPLE__
	pthread_mutex_unlock( &m->handle );
#endif
} // mutex_unlock
