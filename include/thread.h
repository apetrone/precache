#pragma once

// os includes
#if _WIN32
	#include <windows.h>
	#include <process.h> // for _beginthreadex
#elif LINUX || __APPLE__
	#include <pthread.h>
	#include <unistd.h>
	#include <sys/types.h>
#endif


// thread entry function
#if _WIN32
	typedef unsigned int (__stdcall *thread_entry)( void * );
	#define THREAD_ENTRY unsigned int __stdcall
#elif LINUX || __APPLE__
	typedef void * (*thread_entry)( void * );
	#define THREAD_ENTRY void *
#endif

// thread states
#define THREAD_STATE_INVALID		0
#define THREAD_STATE_ACTIVE			1
#define THREAD_STATE_SUSPENDED		2
#define THREAD_STATE_STOPPED		3

typedef struct thread_s
{
	unsigned int state;
#if _WIN32
	unsigned int id;
	HANDLE handle;
#elif LINUX || __APPLE__
	pthread_t handle;
	pthread_attr_t attribs;
#endif
} thread_t;


// start a thread
void thread_start( thread_t * t, thread_entry entry, void * data );

// kill a thread
void thread_stop( thread_t * t );

// retrieve thread status (one of the states above)
unsigned int thread_status( thread_t * t );

#if 0
// get a thread's id
unsigned int thread_id( thread_t * t );

// returns this thread's ID (according to the OS)
int thread_selfid();
#endif


// wait for a thread to complete. WINDOWS ONLY: Waits milliseconds before killing thread. If timeout_milliseconds is -1, wait indefinitely
// returns 0 if the thread timed out
// returns 1 if the thread exited before timeout
int thread_join( thread_t * t, int timeout_milliseconds );

// let the thread sleep for at least <milliseconds>
void thread_sleep( int milliseconds );

//--------------------------------------------------

typedef struct mutex_s
{
#if _WIN32
	CRITICAL_SECTION cs;
#elif LINUX || __APPLE__
	pthread_mutex_t handle;
	pthread_mutexattr_t attribs;
#endif
} mutex_t;

// create mutex
void mutex_create( mutex_t * m );

// destroy, cleanup
void mutex_destroy( mutex_t * m );

// locks a mutex
void mutex_lock( mutex_t * m );

// unlock mutex
void mutex_unlock( mutex_t * m );


