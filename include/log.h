#pragma once


#ifdef __cplusplus
extern "C" {
#endif

// start logging to a file: returns 1 on success, 0 on failure
int log_init_file( const char * filename );
void log_msg( const char * msg, ... );
void log_shutdown();

#ifdef __cplusplus
}; // extern "C"
#endif
