#include <stdio.h>

#if _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	// windows.h must not be included before winsock2.h
	#include <windows.h>
	#include <gl/gl.h>
	#pragma comment( lib, "opengl32.lib" )
#elif __APPLE__
	#include <sys/stat.h> // for mkdir
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
#elif LINUX
	#include <errno.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <GL/gl.h>
	#include <GL/glx.h>
#endif


#include <xwl/xwl.h>
#include <thread.h>
#include "font.h"
#include "log.h"
#include "timer.h"
#include "platform.h"
#include "precachelib.h"
#include "precache.h"

#if _WIN32
	#include "resource.h"
#endif

typedef struct
{
	font_t font;
	timevalue_t ts;
	mutex_t dl;
	xwl_event_t event;
	short running;
	short width;
	short height;
	thread_t t0;
	precache_thread_data_t tdata;
	precache_state_t ps;
	http_download_state_t downloadState;

	// rendering related
	button bar;
	button closeButton;
	short progressBarWidth;
	float downloadPercent;
	i32 mx;
	i32 my;

	// messages
	char msg[ 256 ];
	char msg2[ 256 ];
	unsigned char msg_color[4];
	int textpos[ 4 ];
} application_state_t;
application_state_t state;

void set_msg_color( u8 r, u8 g, u8 b, u8 a )
{
	state.msg_color[0] = r;
	state.msg_color[1] = g;
	state.msg_color[2] = b;
	state.msg_color[3] = a;
}



void cmd_quit( button * src )
{
	state.running = 0;
}

i32 mouse_inside_button( button * b )
{
	if ( state.mx > (b->x+b->width) )
		return 0;
	if ( state.mx < b->x )
		return 0;

	if ( state.my > (b->y + b->height) )
		return 0;
	if ( state.my < b->y )
		return 0;
	return 1;
}


void callback( xwl_event_t * e )
{
    xwlPrintf( "event: %s\n", xwl_event_to_string(e->type) );

	if ( e->type == XWLE_GAINFOCUS )
	{
		//xwlPrintf( "Gain Focus\n" );
	}
	else if ( e->type == XWLE_LOSTFOCUS )
	{
		//xwlPrintf( "Lost Focus\n" );
	}
	else if ( e->type == XWLE_MOUSEWHEEL )
	{
		xwlPrintf( "\t-> wheel delta: %i\n", e->wheelDelta );
	}
	else if ( e->type == XWLE_KEYPRESSED || e->type == XWLE_KEYRELEASED )
	{
		xwlPrintf( "\t-> unicode: %i\n", e->unicode );
		xwlPrintf( "\t-> keymods: %i\n", e->keymods );
		xwlPrintf( "\t-> key: %i (%s)\n", e->key, xwl_key_to_string(e->key) );
	}
	else if ( e->type == XWLE_MOUSEBUTTON_PRESSED || e->type == XWLE_MOUSEBUTTON_RELEASED )
	{
		xwlPrintf( "\t-> button: %s\n", xwl_mouse_to_string(e->button) );
		if ( e->type == XWLE_MOUSEBUTTON_RELEASED && e->button == XWLMB_LEFT )
		{
			if ( mouse_inside_button( &state.closeButton ) )
			{
				if ( state.closeButton.event )
				{
					state.closeButton.event( &state.closeButton );
				}
			}
		}
	}
	else if ( e->type == XWLE_MOUSEMOVE )
	{
		xwlPrintf( "\t-> pos: (%i, %i)\n", e->mx, e->my );
		state.mx = e->mx;
		state.my = e->my;
	}
	else if ( e->type == XWLE_SIZE )
	{
		state.width = e->width;
		state.height = e->height;
		xwlPrintf( "\t-> width: %i\n", state.width );
		xwlPrintf( "\t-> height: %i\n", state.height );
	}
	else if ( e->type == XWLE_CLOSED )
	{
		xwlPrintf( "Closed the window\n" );
		state.running = 0;
	}


	if ( e->type == XWLE_KEYRELEASED && e->key == XWLK_ESCAPE )
	{
		state.running = 0;
	}
}

void render_button( button * b )
{
	glColor3ub( b->color[0], b->color[1], b->color[2] );
	glBegin( GL_TRIANGLES );
		glVertex2i( b->x, b->y );
		glVertex2i( b->x, b->y + b->height );
		glVertex2i( b->x + b->width, b->y + b->height );
		glVertex2i( b->x + b->width, b->y + b->height );
		glVertex2i( b->x + b->width, b->y );
		glVertex2i( b->x, b->y );
	glEnd();
}

void render_button_frame( button * b, int width, int height )
{
	glColor3ub( 0, 0, 0 );
	glBegin( GL_LINES );

		glVertex2i( b->x, b->y );
		glVertex2i( b->x, b->y + height );

		glVertex2i( b->x, b->y + height );
		glVertex2i( b->x + width, b->y + height );

		glVertex2i( b->x + width, b->y + height );
		glVertex2i( b->x + width, b->y );

		glVertex2i( b->x + width, b->y );
		glVertex2i( b->x, b->y );

	glEnd();
}


static g_thread_id = 0;

THREAD_ENTRY precache_download_thread( void * data )
{
    precache_thread_data_t * td = (precache_thread_data_t*)data;
    http_download_state_t * download = td->download;
    precache_state_t * precache = td->precache;
    char remotepath[ PRECACHE_TEMP_BUFFER_SIZE ] = {0};
    char localpath[ PRECACHE_TEMP_BUFFER_SIZE ] = {0};
	float dt;
	int thread_id;
	int last_bytes;
	float last_read;

    if ( !td || !download || !precache )
        return 0;

	last_bytes = 0;
	last_read = timer_ms(&state.ts);
	memset( download, 0, sizeof(http_download_state_t) );

	thread_id = g_thread_id;
	THREAD_MSG( "-> T: Entering Thread (%i)...\n", g_thread_id );
	g_thread_id++;

	while( td->state != PRECACHE_STATE_EXIT && td->state != PRECACHE_STATE_ERROR )
	{
	    mutex_lock( &state.dl );
		//log_msg( "* T: Think\n" );

	    if ( td->state == PRECACHE_STATE_DOWNLOAD )
	    {
            http_download_tick( download, 1 );

			if ( download->bytes_read != last_bytes )
			{
				last_bytes = download->bytes_read;
				last_read = timer_ms(&state.ts);
				//log_msg( "-> T: %i - total=%i bytes at last_read=%g\n", thread_id, last_bytes, last_read );
			}
			else
			{
				// if we haven't read any more bytes, see how long it's been since we last read bytes
				dt = (timer_ms(&state.ts) - last_read);
				if ( dt > PRECACHE_TIMEOUT_MS )
				{
					THREAD_MSG( "-> T: %i - Download of file \"%s\" timed out (%g)!\n", thread_id, remotepath, timer_ms(&state.ts) );
					//td->precache->state = PS_DOWNLOAD_TIMEDOUT;
					td->state = PRECACHE_STATE_EXIT;
					td->download->error = 1;
					strcpy( precache->err1, "Timed out while downloading:" );
					strcpy( precache->err2, remotepath );
				}
			}
	    }
        else if ( td->state == PRECACHE_STATE_DOWNLOAD_REQUEST )
        {
			if ( precache->curfile )
			{
				precache->curfile->timestamp = timer_ms(&state.ts);

				// construct remote path
				memset( remotepath, 0, PRECACHE_TEMP_BUFFER_SIZE );
				strcpy( remotepath, precache->remotepath );
				strcat( remotepath, precache->curfile->path );

				// construct local path
				memset( localpath, 0, PRECACHE_TEMP_BUFFER_SIZE );
				strcpy( localpath, precache->localpath );
				strcat( localpath, precache->curfile->targetpath );
				platform_conform_slashes( localpath, PRECACHE_TEMP_BUFFER_SIZE );

				THREAD_MSG( "-> T: %i - Requesting a new file [remotepath=%s, localpath=%s]\n", thread_id, remotepath, localpath );

				http_download_file( remotepath, localpath, PRECACHE_USER_AGENT, download );
				td->state = PRECACHE_STATE_DOWNLOAD;
			}
        }

		if ( td->state != PRECACHE_STATE_EXIT )
		{
			if( download->error || download->completed )
			{
				if ( download->completed )
				{
					THREAD_MSG( "-> T: %i - download complete!\n", thread_id );
				}
				else
				{
					THREAD_MSG( "-> T: %i - Unable to download file: %s\n", thread_id, remotepath );
					strcpy( precache->err1, "Update failed! File not found:" );
					strcpy( precache->err2, remotepath );
				}
				td->state = PRECACHE_STATE_EXIT;
			}
		}

        mutex_unlock( &state.dl );
	}

	THREAD_MSG( "-> T: %i - Leaving Thread.\n", thread_id );

    return 0;
}


THREAD_ENTRY precache_calculate_checksums( void * data )
{
    precache_thread_data_t * td = (precache_thread_data_t*)data;
    http_download_state_t * download = td->download;
    precache_state_t * precache = td->precache;
	precache_file_t * file;
	md5_digest_t digest;
    char localpath[ PRECACHE_TEMP_BUFFER_SIZE ] = {0};

    if ( !td || !download || !precache )
        return 0;

	file = precache->files;

	//log_msg( "* MD5 Checksum calculation - started\n" );

	while( file )
	{
		// if this file has NOT already been downloaded...
		if ( !(file->flags & 1) )
		{
			// file has a valid checksum read from the precache list
			if ( file->checksum[0] != '\0' )
			{
				memset( localpath, 0, 256 );
				strcpy( localpath, precache->localpath );
				strcat( localpath, file->targetpath );

				//log_msg( "* MD5 Calculate checksum for file \"%s\"\n", file->path );

				md5_from_path( localpath, digest );

				// if the local checksum matches the one specified in the precache list, set the skip flag on this file
				if ( stricmp( digest, file->checksum ) == 0 )
				{
					//log_msg( "* MD5 Checksums match for file \"%s\" (%s)\n", file->path, digest );
					file->flags |= 1;
				}
			}

			if ( !(file->flags & 1) )
			{
				//log_msg( "* MD5 Checksums DO NOT MATCH for file \"%s\" (%s)\n", file->path, digest );
			}
		}

		file = file->next;
	}

	//log_msg( "* MD5 Finished calculating checksums.\n" );
	precache->state = PS_FINISHED_PARSING_LIST;
	return 0;
} // precache_calculate_checksums


void process_downloads()
{
	mutex_lock( &state.dl );
	if ( state.ps.state == PS_DOWNLOAD_LIST && state.downloadState.completed )
	{
		if ( state.downloadState.completed )
		{
			state.bar.color[0] = 16;
			state.bar.color[1] = 128;
			state.bar.color[2] = 16;

			state.downloadState.completed = 0;
			state.downloadPercent = 1;
			log_msg( "* Finished downloading precache.list\n" );
			state.ps.state = PS_PARSING_LIST;
			state.ps.curfile->flags = 1; // downloaded

            // analyze the precache list and update the file list
            if ( precache_parse_list( &state.ps ) )
            {
				// cat the base path onto the binary directory...
				strcat( state.ps.localpath, state.ps.base );
				platform_conform_slashes( state.ps.localpath, MAX_PATH_SIZE );

                // ensure this directory exists..
#if 0
#if LINUX || __APPLE__
                // http://pubs.opengroup.org/onlinepubs/009695399/functions/mkdir.html
                mkdir( state.ps.localpath, (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) );
#else
				platform_conform_slashes( state.ps.localpath, MAX_PATH_SIZE );
				created_directory = CreateDirectoryA( state.ps.localpath, 0 );

				if ( !created_directory )
				{
					log_msg( "* CreateDirectory FAILED: [%s]\n", state.ps.localpath );
				}

				
#endif
#endif
				
				platform_makedirs( state.ps.localpath );

				strcpy( state.msg, "Calculating MD5 checksums..." );
				strcpy( state.msg2, "(this may take a while)" );

				THREAD_MSG( "* THREAD: Initiating thread calculate checksums.\n" );

				// fire up a task thread to determine what needs to be downloaded...
				state.ps.state = PS_CALCULATING_CHECKSUMS;
				thread_stop( &state.t0 );
				thread_start( &state.t0, precache_calculate_checksums, &state.tdata );
				
            }
            else
            {
                log_msg( "Error parsing the precache.list file.\n" );
                // precache list error
				state.ps.state = PS_PARSE_ERROR;

                strcpy( state.msg, state.ps.err1 );
                strcpy( state.msg2, state.ps.err2 );
				set_msg_color( 255, 0, 0, 255 );
            }
		}
		else
		{
			if (state.downloadState.bytes_read > 0)
			{
				state.downloadPercent = (state.downloadState.bytes_read/(float)state.downloadState.content_length);
			}
			else if ( state.downloadState.bytes_read == 0 )
			{
				state.downloadPercent = 0;
			}
			else
			{
				log_msg( "Error occurred while downloading. bytes_read is < 0!\n" );
			}
			sprintf( state.msg, "Downloading \"%s\"", state.ps.curfile->path );
			sprintf( state.msg2, "Progress: (%iKB/%iKB)", state.downloadState.bytes_read/KB_DIV, state.downloadState.content_length/KB_DIV );
			set_msg_color( 255, 255, 255, 255 );
		}
	}
	else if ( state.ps.state == PS_FINISHED_PARSING_LIST )
	{
		log_msg( "* finished parsing precache.list, beginning downloads\n" );

		state.ps.curfile = state.ps.files;
		// finished analysis, resume normal downloading state
		state.ps.state = PS_DOWNLOAD_NEXT;
	}
	else if ( state.ps.state == PS_DOWNLOAD_NEXT )
	{
		state.ps.curfile = precache_locate_next_file( state.ps.curfile );

		if ( !state.ps.curfile )
		{
			strcpy( state.msg, "All files up to date." );
			strcpy( state.msg2, "" );
			set_msg_color( 0, 255, 0, 255 );
			state.running = 0;
			state.ps.state = PS_COMPLETED;
		}
		else
		{
			state.ps.state = PS_DOWNLOADING;
			// initiate download
			thread_stop( &state.t0 );
			state.tdata.state = PRECACHE_STATE_DOWNLOAD_REQUEST;

			THREAD_MSG( "* THREAD: Initiating download thread to fetch (%s)\n", state.ps.curfile->path );
			thread_start( &state.t0, precache_download_thread, &state.tdata );
		}
	}
	else if ( state.ps.state == PS_DOWNLOADING )
	{
		if (state.downloadState.bytes_read > 0)
		{
			state.downloadPercent = (state.downloadState.bytes_read/(float)state.downloadState.content_length);
		}
		else if ( state.downloadState.bytes_read == 0 )
		{
			state.downloadPercent = 0;
		}
		else
		{
			log_msg( "Error occurred while downloading. bytes_read is < 0!\n" );
		}
		sprintf( state.msg, "Downloading \"%s\"", state.ps.curfile->path );
		sprintf( state.msg2, "Progress: (%iKB/%iKB)", state.downloadState.bytes_read/KB_DIV, state.downloadState.content_length/KB_DIV );
		set_msg_color( 255, 255, 255, 255 );

		if ( state.downloadState.completed )
		{
			state.bar.color[0] = 16;
			state.bar.color[1] = 128;
			state.bar.color[2] = 16;

			state.downloadState.completed = 0;
			log_msg( "\tDownloading \"%s\" -> success! (%i/%i)\n", state.ps.curfile->path, state.downloadState.bytes_read, state.downloadState.content_length );
			state.ps.curfile->flags = 1;
			state.ps.state = PS_DOWNLOAD_NEXT;
		}
		else
		{
			state.bar.color[0] = 0;
			state.bar.color[1] = 255;
			state.bar.color[2] = 0;
		}
	}

	if ( state.ps.state == PS_DOWNLOAD_LIST || state.ps.state == PS_DOWNLOADING )
	{
		if ( state.downloadState.error )
		{
			if ( state.ps.state == PS_DOWNLOAD_LIST )
			{
				// unable to download precache.list
				strcpy( state.msg, "Error downloading precache.list from:" );
				strcpy( state.msg2, state.ps.remotepath );
				strcat( state.msg2, "/precache.list" );
				set_msg_color( 255, 0, 0, 255 );
			}
			else if ( state.ps.state == PS_DOWNLOADING )
			{
				strcpy( state.msg, state.ps.err1 );
				strcpy( state.msg2, state.ps.err2 );
				set_msg_color( 255, 0, 0, 255 );

				if ( state.ps.curfile )
				{
					strcpy( state.ps.currentfilepath, state.ps.localpath );
					strcat( state.ps.currentfilepath, state.ps.curfile->path );
					unlink( state.ps.currentfilepath );
				}
			}

			log_msg( "An error occurred! Messages: [%s] [%s] | state: %i\n", state.msg, state.msg2, state.ps.state );
			state.ps.state = PS_DOWNLOAD_ERROR;
		}

	}

	mutex_unlock( &state.dl );
}


void tick()
{
    xwl_pollevent( &state.event );

	glClearColor(0.25, 0.25, 0.25, 1.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport( 0, 0, state.width, state.height );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0, state.width, state.height, 0, -.1f, 256.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	render_button( &state.closeButton );
	render_button( &state.bar );
	render_button_frame( &state.bar, state.progressBarWidth, state.bar.height );

	if ( mouse_inside_button( &state.closeButton ) )
	{
		state.closeButton.color[0] = 0;
		state.closeButton.color[1] = 192;
		state.closeButton.color[2] = 192;
	}
	else
	{
		state.closeButton.color[0] = 0;
		state.closeButton.color[1] = 128;
		state.closeButton.color[2] = 128;
	}


	state.bar.width = (i16)(state.downloadPercent * state.progressBarWidth);

	// draw black text to act as a drop-shadow
	font_draw( &state.font, state.textpos[0]+2, state.textpos[1]+2, state.msg, 0, 0, 0, 255 );
	if ( state.msg2[0] != 0 )
	{
		font_draw( &state.font, state.textpos[2]+2, state.textpos[3]+2, state.msg2, 0, 0, 0, 255 );
	}

	// draw foreground text
    font_draw( &state.font, state.textpos[0], state.textpos[1], state.msg, state.msg_color[0], state.msg_color[1], state.msg_color[2], state.msg_color[3] );
	if ( state.msg2[0] != 0 )
	{
		font_draw( &state.font, state.textpos[2], state.textpos[3], state.msg2, state.msg_color[0], state.msg_color[1], state.msg_color[2], state.msg_color[3] );
	}

	// render button text
	font_draw( &state.font, 388, 114, "Close", 255, 255, 255, 255 );

    // do a swap of buffers
	xwl_finish();

#if !PRECACHE_TEST
	// runs logic of processing downloads
	process_downloads();
#endif
}





#ifndef _WIN32
int main()
#else
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
#endif
{
    xwl_window_t * window = 0;
	xwl_windowparams_t p;
	precache_file_t * file;
	char log_path[ MAX_PATH_SIZE ];
	char temp_path[ MAX_PATH_SIZE ];


	timer_startup(&state.ts);

	state.running = 1;

    /* setup button */
	state.closeButton.event = cmd_quit;
	state.closeButton.width = 70;
	state.closeButton.height = 25;
	state.closeButton.x = 365;
	state.closeButton.y = 95;
	state.closeButton.color[0] = 0;
	state.closeButton.color[1] = 128;
	state.closeButton.color[2] = 128;

	// setup progress bar
	state.bar.event = 0;
	state.bar.x = 25;
	state.bar.y = 60;
	state.bar.height = 15;
	state.bar.width = 1;
	state.bar.color[0] = 255;
	state.bar.color[1] = 0;
	state.bar.color[2] = 255;

	// setup text position
	state.textpos[0] = 30;
	state.textpos[1] = 25;
	state.textpos[2] = 30;
	state.textpos[3] = 45;

	state.progressBarWidth = 400;
	state.downloadPercent = 0;

	memset(&state.downloadState, 0, sizeof(http_download_state_t));
    state.tdata.download = &state.downloadState;
    state.tdata.precache = &state.ps;
    state.tdata.state = PRECACHE_STATE_DOWNLOAD_REQUEST;

    memset( &state.ps, 0, sizeof(precache_state_t) );

    // add the first file - which is the precache.list
    state.ps.curfile = (precache_file_t*)malloc( sizeof(precache_file_t) );
    state.ps.curfile->next = 0;
    memset( state.ps.curfile->checksum, 0, 33 );
    state.ps.curfile->flags = 0;
    memset( state.ps.curfile->path, 0, MAX_PATH_SIZE );
    strcpy( state.ps.curfile->path, "/precache.list" );
	strcpy( state.ps.curfile->targetpath, state.ps.curfile->path );
    state.ps.files = state.ps.curfile;
    state.ps.state = 0; // download precache.list mode.

    // setup configure the paths
    memset( state.ps.remotepath, 0, MAX_PATH_SIZE );
	memset( state.ps.localpath, 0, MAX_PATH_SIZE );

    // the directory which contains the precache.list
	strcpy( state.ps.remotepath, PRECACHE_URL );

	platform_operating_directory( state.ps.localpath, MAX_PATH_SIZE );

	memset( log_path, 0, MAX_PATH_SIZE );
	strcpy( log_path, state.ps.localpath );
	strcat( log_path, "/" );
	strcat( log_path, "precache.log" );

	if ( !log_init_file( log_path ) )
	{
		printf( "Unable to open log file: %s\n", log_path );
	}

// Binaries for Apple bundles run three directories deeper from their perceived location.
// So we are going to strip three directories from that path to get the perceived location...
#if __APPLE__
	{
		char * p = strrchr( state.ps.localpath, PATH_SEPARATOR );

		if ( p )
		{
			*p = '\0';

			p = strrchr(state.ps.localpath, PATH_SEPARATOR);
			if ( p ) // found it again...
			{
				*p = '\0';

				p = strrchr(state.ps.localpath, PATH_SEPARATOR);
				if ( p )
				{
					*p = '\0';
				}
			}
		}
	}
#endif

	printf( "LocalPath: %s\n", state.ps.localpath );

    strcpy( state.ps.precache_file, state.ps.localpath );
	strcat( state.ps.precache_file, "/precache.list" );

    if ( state.ps.files )
    {
        printf( "Dumping files on startup----------------------\n" );
        file = state.ps.files;

        while( file )
        {
            printf( "File path: '%s', checksum: '%s'\n", file->path, file->checksum );
            file = file->next;
        }
        printf( "----------------------\n" );
    }

    p.width = 450;
    p.height = 130;
    p.flags = XWL_OPENGL | XWL_WINDOWED | XWL_NORESIZE;

#if _WIN32
	p.flags |= XWL_WIN32_ICON;
	p.hIcon = LoadIcon( GetModuleHandle(0), MAKEINTRESOURCE( IDI_PRECACHE_ICON ) );
#endif

    net_startup();
    xwl_startup();

    window = xwl_create_window( &p, PRECACHE_WINDOW_TITLE, 0 );

    if ( !window )
    {
        fprintf( stderr, "Unable to create window!\n" );
        exit(1);
    }

	state.width = p.width;
	state.height = p.height;
    xwl_set_callback( callback );

    // init font
    //font_load_embedded( &state.font, font_liberation_mono8, font_liberation_mono8_size );
    font_load_embedded( &state.font, font_pf_tempesta_seven8, font_pf_tempesta_seven8_size );

	memset( state.msg, 0, 256 );
	memset( state.msg2, 0, 256 );
	set_msg_color( 255, 255, 255, 255 );

#if PRECACHE_TEST
	strcpy( state.msg, "PRECACHE_TEST is enabled..." );

	if ( platform_is64bit() )
	{
		strcpy( state.msg2, "x86_64 OS detected." );
	}
	else
	{
		strcpy( state.msg2, "x86 OS detected." );
	}
	//strcpy( state.msg2, "Testing message two string..." );
#else
	strcpy( state.msg, "Downloading precache.list..." );
	mutex_create( &state.dl );
	THREAD_MSG( "* THREAD: Initiating download thread to fetch precache.list\n" );
	state.ps.state = PS_DOWNLOAD_LIST;
    // start the download thread - this will attempt to grab the precache.list file
    thread_start( &state.t0, precache_download_thread, &state.tdata );
#endif

    memset( &state.event, 0, sizeof(xwl_event_t) );
    while( state.running )
    {
		tick();
    }

#if !PRECACHE_TEST
	// kill t0, if running
	if ( state.t0.state == THREAD_STATE_ACTIVE )
	{
		thread_stop( &state.t0 );
	}

	// try to find a file (for this platform) with the execute bit set
	if ( state.ps.state == PS_COMPLETED )
	{
		file = precache_locate_executable_file( state.ps.files );
		if ( file )
		{
			log_msg( "* Located executable file [%s]\n", file->targetpath );

			// construct local path
			memset( temp_path, 0, MAX_PATH_SIZE );
			strcpy( temp_path, state.ps.localpath );
			strcat( temp_path, file->targetpath );
			platform_conform_slashes( temp_path, MAX_PATH_SIZE );
			
			log_msg( "* Executable file is [%s]\n", temp_path );

			log_msg( "* Set permissions on file...\n" );


			log_msg( "* Spawn process.\n" );
			platform_spawn_process( temp_path );
		}
	}

#endif

	mutex_destroy( &state.dl );

    // cleanup files
    if ( state.ps.files )
    {
        precache_file_t * file = state.ps.files;
        precache_file_t * temp;

        while( file )
        {
            temp = file;
            file = file->next;
            free( temp );
        }
    }

	thread_stop( &state.t0 );

	// cleanup the precache list from the filesystem...
	unlink( state.ps.precache_file );

	log_msg( "* Shutting down gracefully.\n" );

	// destroy font stuff
	font_destroy( &state.font );

    // shutdown xwl and network
	xwl_shutdown();
	net_shutdown();
	log_shutdown();
	return 0;
}
