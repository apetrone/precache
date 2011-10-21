#include <stdio.h>
#include <xwl/xwl.h>

#include <thread.h>
#include "font.h"
#include "log.h"
#include "timer.h"
#include "platform.h"
#include "precachelib.h"
#include "precache.h"

#if _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	// windows.h must not be included before winsock2.h
	#include <windows.h>
#endif

#if __APPLE__
	#include <sys/stat.h> // for mkdir
#endif

#if LINUX
	#include <errno.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif











int running = 1;

i32 mx;
i32 my;
xwl_windowparams_t p;
timevalue_t timestate;

typedef struct
{
	font_t font;
} application_state_t;
application_state_t state;

// temporary OpenGL tests
#if _WIN32
#include <windows.h>
#include <gl/gl.h>
#pragma comment( lib, "opengl32.lib" )
#elif LINUX
#include <GL/gl.h>
#include <GL/glx.h>
#elif __APPLE__
#include <OpenGL/OpenGL.h>
#endif


#define XWL_DEBUG 0

#if XWL_DEBUG
#define xwlPrintf printf
#else
#define xwlPrintf //
#endif



button b;
button bar;

void cmd_quit( button * src )
{
	running = 0;
}

i32 mouse_inside_button( button * b )
{
	if ( mx > (b->x+b->width) )
		return 0;
	if ( mx < b->x )
		return 0;

	if ( my > (b->y + b->height) )
		return 0;
	if ( my < b->y )
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
			if ( mouse_inside_button( &b ) )
			{
				if ( b.event )
				{
					b.event( &b );
				}
			}
		}
	}
	else if ( e->type == XWLE_MOUSEMOVE )
	{
		xwlPrintf( "\t-> pos: (%i, %i)\n", e->mx, e->my );
		mx = e->mx;
		my = e->my;
	}
	else if ( e->type == XWLE_SIZE )
	{
		p.width = e->width;
		p.height = e->height;
		xwlPrintf( "\t-> width: %i\n", p.width );
		xwlPrintf( "\t-> height: %i\n", p.height );
	}
	else if ( e->type == XWLE_CLOSED )
	{
		xwlPrintf( "Closed the window\n" );
		running = 0;
	}


	if ( e->type == XWLE_KEYRELEASED && e->key == XWLK_ESCAPE )
		running = 0;
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

mutex_t dl;




void md5_from_path( const char * filename, char * digest )
{
	md5_state_t md5state;
	FILE * fp;
	size_t fileSize;
	int loop;
	char * data;
	md5_byte_t md5digest[16];

	fp = fopen( filename, "rb" );
	if ( !fp )
	{
        // clear digest
	    memset( digest, 0, 32 );
	    return;
	}

    fseek( fp, 0, SEEK_END );
    fileSize = ftell( fp );
    fseek( fp, 0, SEEK_SET );

    // init the state
    md5_init( &md5state );

    memset( md5digest, 0, 16 );

    data = (char*)malloc( fileSize );
    fread( data, 1, fileSize, fp );

    // add all bytes from the file
    md5_append( &md5state, (unsigned char*)data, fileSize );

    // close the file
    fclose( fp );

    free( data );

    // finish up and get the digest
    md5_finish( &md5state, md5digest );

    // generate a hex digest; code lifted from the md5main.c source
    for( loop = 0; loop < 16; ++loop )
    {
        sprintf( digest + (loop * 2), "%02x", md5digest[loop] );
    }
} // md5_from_path




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
	last_read = timer_ms(&timestate);
	memset( download, 0, sizeof(http_download_state_t) );

	thread_id = g_thread_id;
	log_msg( "-> T: Entering Thread (%i)...\n", g_thread_id );
	g_thread_id++;

	while( td->state != PRECACHE_STATE_EXIT && td->state != PRECACHE_STATE_ERROR )
	{
	    mutex_lock( &dl );
		//log_msg( "* T: Think\n" );

	    if ( td->state == PRECACHE_STATE_DOWNLOAD )
	    {
            http_download_tick( download, 1 );

			if ( download->bytes_read != last_bytes )
			{
				last_bytes = download->bytes_read;
				last_read = timer_ms(&timestate);
			}
			else
			{
				// if we haven't read any more bytes, see how long it's been since we last read bytes
				dt = (timer_ms(&timestate) - last_read);
				if ( dt > PRECACHE_TIMEOUT_MS )
				{
					log_msg( "-> T: %i - Download of file \"%s\" timed out!\n", thread_id, remotepath );
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
				precache->curfile->timestamp = timer_ms(&timestate);

				// construct remote path
				memset( remotepath, 0, PRECACHE_TEMP_BUFFER_SIZE );
				strcpy( remotepath, precache->remotepath );
				strcat( remotepath, precache->curfile->path );

				// construct local path
				memset( localpath, 0, PRECACHE_TEMP_BUFFER_SIZE );
				strcpy( localpath, precache->localpath );
				strcat( localpath, precache->curfile->path );
				platform_conform_slashes( localpath, PRECACHE_TEMP_BUFFER_SIZE );

				log_msg( "-> T: %i - Requesting a new file [remotepath=%s, localpath=%s]\n", thread_id, remotepath, localpath );

				http_download_file( remotepath, localpath, download );
				td->state = PRECACHE_STATE_DOWNLOAD;
			}
        }

		if ( td->state != PRECACHE_STATE_EXIT )
		{
			if( download->error || download->completed )
			{
				if ( download->completed )
				{
					log_msg( "-> T: %i - download complete!\n", thread_id );
				}
				else
				{
					log_msg( "-> T: %i - Unable to download file: %s\n", thread_id, remotepath );
					strcpy( precache->err1, "Update failed! File not found:" );
					strcpy( precache->err2, remotepath );
				}
				td->state = PRECACHE_STATE_EXIT;
			}
		}

        mutex_unlock( &dl );

	}

	log_msg( "-> T: %i - Leaving Thread.\n", thread_id );

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
				strcat( localpath, file->path );

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





precache_file_t * precache_locate_next_file( precache_file_t * start )
{
	precache_file_t * cur = start;
	precache_file_t * next = 0;

	log_msg( "* LOC: start is [%s]\n", start->path );
	do
	{
		// this file hasn't been downloaded...
		if ( !(cur->flags & 1) )
		{
			log_msg( "* LOC: file not downloaded: [%s]\n", cur->path );
			next = cur;
			break;
		}

		log_msg( "* LOC: file downloaded, skipping: [%s]\n", cur->path );
		cur = cur->next;
	} while( cur );

	return next;
} // precache_locate_next_file





#ifndef _WIN32
int main()
#else
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
#endif
{
    xwl_window_t * window = 0;
    xwl_event_t event;
    http_download_state_t downloadState;
	u32 progressWidth = 400;
	float pct = 0;
	thread_t t0;
    precache_state_t ps;
    precache_thread_data_t tdata;
	precache_file_t * file;
	char log_path[ 255 ];
	char msg[ 256 ];
	char msg2[ 256 ];
	unsigned char msg_color[4];
	int created_directory;


	int textpos[ 4 ];

	timer_startup(&timestate);

    /* setup button */
	b.event = cmd_quit;
	b.width = 70;
	b.height = 25;
	b.x = 365;
	b.y = 95;
	b.color[0] = 0;
	b.color[1] = 128;
	b.color[2] = 128;

	// setup progress bar
	bar.event = 0;
	bar.x = 25;
	bar.y = 60;
	bar.height = 15;
	bar.width = 1;
	bar.color[0] = 255;
	bar.color[1] = 0;
	bar.color[2] = 255;

	// setup text position
	textpos[0] = 30;
	textpos[1] = 25;
	textpos[2] = 30;
	textpos[3] = 45;

	memset(&downloadState, 0, sizeof(http_download_state_t));
    tdata.download = &downloadState;
    tdata.precache = &ps;
    tdata.state = PRECACHE_STATE_DOWNLOAD_REQUEST;

    memset( &ps, 0, sizeof(precache_state_t) );

    // add the first file - which is the precache.list
    ps.curfile = (precache_file_t*)malloc( sizeof(precache_file_t) );
    ps.curfile->next = 0;
    memset( ps.curfile->checksum, 0, 33 );
    ps.curfile->flags = 0;
    memset( ps.curfile->path, 0, 256 );
    strcpy( ps.curfile->path, "/precache.list" );
    ps.files = ps.curfile;
    ps.state = 0; // download precache.list mode.

    // setup configure the paths
    memset( ps.remotepath, 0, 256 );
	memset( ps.localpath, 0, MAX_PATH_SIZE );

    // the directory which contains the precache.list
	strcpy( ps.remotepath, PRECACHE_URL );

	platform_operating_directory( ps.localpath, MAX_PATH_SIZE );

	memset( log_path, 0, 255 );
	strcpy( log_path, ps.localpath );
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
		char * p = strrchr( ps.localpath, PATH_SEPARATOR );

		if ( p )
		{
			*p = '\0';

			p = strrchr(ps.localpath, PATH_SEPARATOR);
			if ( p ) // found it again...
			{
				*p = '\0';

				p = strrchr(ps.localpath, PATH_SEPARATOR);
				if ( p )
				{
					*p = '\0';
				}
			}
		}
	}
#endif

	printf( "LocalPath: %s\n", ps.localpath );

    strcpy( ps.precache_file, ps.localpath );
	strcat( ps.precache_file, "/precache.list" );

    if ( ps.files )
    {
        printf( "Dumping files on startup----------------------\n" );
        file = ps.files;

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

    net_startup();
    xwl_startup();


    window = xwl_create_window( &p, PRECACHE_WINDOW_TITLE, 0 );

    if ( !window )
    {
        fprintf( stderr, "Unable to create window!\n" );
        exit(1);
    }

    xwl_set_callback( callback );

    // init font
    //font_load_embedded( &state.font, font_liberation_mono8, font_liberation_mono8_size );
    font_load_embedded( &state.font, font_pf_tempesta_seven8, font_pf_tempesta_seven8_size );

	memset( msg, 0, 256 );
	memset( msg2, 0, 256 );
	msg_color[0] = msg_color[1] = msg_color[2] = msg_color[3] = 255;


#if PRECACHE_TEST


#else

	strcpy( msg, "Downloading precache.list..." );
	log_msg( "* MSG: %s\n", msg );

    // start thread here
	mutex_create( &dl );

	log_msg( "* THREAD: Initiating download thread to fetch precache.list\n" );

	ps.state = PS_DOWNLOAD_LIST;

    // start the download thread - this will attempt to grab the precache.list file
    thread_start( &t0, precache_download_thread, &tdata );

    memset( &event, 0, sizeof(xwl_event_t) );
    while( running )
    {
        xwl_pollevent( &event );

		glClearColor(0.25, 0.25, 0.25, 1.0);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glViewport( 0, 0, p.width, p.height );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glOrtho( 0, p.width, p.height, 0, -.1f, 256.0f );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		render_button( &b );
		render_button( &bar );

		render_button_frame( &bar, progressWidth, bar.height );

		if ( mouse_inside_button( &b ) )
		{
			b.color[0] = 0;
			b.color[1] = 192;
			b.color[2] = 192;
		}
		else
		{
			b.color[0] = 0;
			b.color[1] = 128;
			b.color[2] = 128;
		}

		bar.width = (i16)(pct * progressWidth);

        // draw test font string

		// draw black text to act as a drop-shadow
		font_draw( &state.font, textpos[0]+2, textpos[1]+2, msg, 0, 0, 0, 255 );
		if ( msg2[0] != 0 )
		{
			font_draw( &state.font, textpos[2]+2, textpos[3]+2, msg2, 0, 0, 0, 255 );
		}

		// draw foreground text
        font_draw( &state.font, textpos[0], textpos[1], msg, msg_color[0], msg_color[1], msg_color[2], msg_color[3] );
		if ( msg2[0] != 0 )
		{
			font_draw( &state.font, textpos[2], textpos[3], msg2, msg_color[0], msg_color[1], msg_color[2], msg_color[3] );
		}

		// render button text
		font_draw( &state.font, 380, 114, "Close", 255, 255, 255, 255 );


        // do a swap of buffers
		xwl_finish();


		mutex_lock(&dl);
		if ( ps.state == PS_DOWNLOAD_LIST && downloadState.completed )
		{
			if ( downloadState.completed )
			{
				bar.color[0] = 16;
				bar.color[1] = 128;
				bar.color[2] = 16;

				downloadState.completed = 0;
				pct = 1;
				log_msg( "Finished downloading precache.list\n" );
				ps.state = PS_PARSING_LIST;
				ps.curfile->flags = 1; // downloaded

                // analyze the precache list and update the file list
                if ( precache_parse_list( &ps ) )
                {
					// cat the base path onto the binary directory...
					strcat( ps.localpath, ps.base );

                    // ensure this directory exists..
#if LINUX || __APPLE__
                    // http://pubs.opengroup.org/onlinepubs/009695399/functions/mkdir.html
                    mkdir( ps.localpath, (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) );
#else
					created_directory = CreateDirectoryA( ps.localpath, 0 );

					if ( !created_directory )
					{
						log_msg( "* CreateDirectory FAILED: [%s]\n", ps.localpath );
					}
#endif

					strcpy( msg, "Calculating MD5 checksums..." );
					strcpy( msg2, "(this may take a while)" );

					log_msg( "* THREAD: Initiating thread calculate checksums.\n" );

					ps.state = PS_CALCULATING_CHECKSUMS;

					// fire up a task thread to determine what needs to be downloaded...
					thread_stop( &t0 );
					thread_start( &t0, precache_calculate_checksums, &tdata );
                }
                else
                {
                    log_msg( "Error parsing the precache.list file.\n" );
                    // precache list error
					ps.state = PS_PARSE_ERROR;

                    strcpy( msg, ps.err1 );
                    strcpy( msg2, ps.err2 );
                    msg_color[0] = 255;
                    msg_color[1] = 0;
                    msg_color[2] = 0;
                    msg_color[3] = 255;
                }
			}
			else
			{
				if (downloadState.bytes_read > 0)
				{
					pct = (downloadState.bytes_read/(float)downloadState.content_length);
				}
				else if ( downloadState.bytes_read == 0 )
				{
					pct = 0;
				}
				else
				{
					log_msg( "Error occurred while downloading. bytes_read is < 0!\n" );
				}
				sprintf( msg, "Downloading \"%s\"", ps.curfile->path );
				sprintf( msg2, "Progress: (%iKB/%iKB)", downloadState.bytes_read/KB_DIV, downloadState.content_length/KB_DIV );
				msg_color[0] = msg_color[1] = msg_color[2] = 255;
				msg_color[3] = 255;
			}
		}
		else if ( ps.state == PS_FINISHED_PARSING_LIST )
		{
			log_msg( "* finished parsing precache.list, beginning downloads\n" );

			ps.curfile = ps.files;
			// finished analysis, resume normal downloading state
			ps.state = PS_DOWNLOAD_NEXT;
		}
		else if ( ps.state == PS_DOWNLOAD_NEXT )
		{
			ps.curfile = precache_locate_next_file( ps.curfile );

			if ( !ps.curfile )
			{
				strcpy( msg, "All files up to date." );
				strcpy( msg2, "" );
				msg_color[0] = 0;
				msg_color[1] = 255;
				msg_color[2] = 0;
				msg_color[3] = 255;
				running = 0;
			}
			else
			{
				ps.state = PS_DOWNLOADING;
				// initiate download
				thread_stop( &t0 );
				tdata.state = PRECACHE_STATE_DOWNLOAD_REQUEST;

				log_msg( "* THREAD: Initiating download thread to fetch (%s)\n", ps.curfile->path );
				thread_start( &t0, precache_download_thread, &tdata );
			}
		}
		else if ( ps.state == PS_DOWNLOADING )
		{
			if (downloadState.bytes_read > 0)
			{
				pct = (downloadState.bytes_read/(float)downloadState.content_length);
			}
			else if ( downloadState.bytes_read == 0 )
			{
				pct = 0;
			}
			else
			{
				log_msg( "Error occurred while downloading. bytes_read is < 0!\n" );
			}
			sprintf( msg, "Downloading \"%s\"", ps.curfile->path );
			sprintf( msg2, "Progress: (%iKB/%iKB)", downloadState.bytes_read/KB_DIV, downloadState.content_length/KB_DIV );
			msg_color[0] = msg_color[1] = msg_color[2] = 255;
			msg_color[3] = 255;

			if ( downloadState.completed )
			{
				bar.color[0] = 16;
				bar.color[1] = 128;
				bar.color[2] = 16;

				downloadState.completed = 0;
				log_msg( "\t-> File (%s) downloaded successfully (%i/%i)\n", ps.curfile->path, downloadState.bytes_read, downloadState.content_length );
				ps.curfile->flags = 1;
				ps.state = PS_DOWNLOAD_NEXT;
			}
			else
			{
				bar.color[0] = 0;
				bar.color[1] = 255;
				bar.color[2] = 0;
			}
		}

		if ( ps.state == PS_DOWNLOAD_LIST || ps.state == PS_DOWNLOADING )
		{
			if ( downloadState.error )
			{
				if ( ps.state == PS_DOWNLOAD_LIST )
				{
					// unable to download precache.list
					strcpy( msg, "Error downloading precache.list from:" );
					strcpy( msg2, ps.remotepath );
					strcat( msg2, "/precache.list" );
					msg_color[0] = 255;
					msg_color[1] = 0;
					msg_color[2] = 0;
					msg_color[3] = 255;
				}
				else if ( ps.state == PS_DOWNLOADING )
				{
					strcpy( msg, ps.err1 );
					strcpy( msg2, ps.err2 );
					msg_color[0] = 255;
					msg_color[1] = 0;
					msg_color[2] = 0;
					msg_color[3] = 255;

					if ( ps.curfile )
					{
						strcpy( ps.currentfilepath, ps.localpath );
						strcat( ps.currentfilepath, ps.curfile->path );
						unlink( ps.currentfilepath );
					}
				}

				log_msg( "An error occurred! Messages: [%s] [%s] | state: %i\n", msg, msg2, ps.state );
				ps.state = PS_DOWNLOAD_ERROR;
			}

		}

		mutex_unlock( &dl );
    }

	// kill t0, if running
	if ( t0.state == THREAD_STATE_ACTIVE )
	{
		thread_stop( &t0 );
	}

	mutex_destroy( &dl );

    // cleanup files
    if ( ps.files )
    {
        precache_file_t * file = ps.files;
        precache_file_t * temp;

        while( file )
        {
            temp = file;
            file = file->next;
            free( temp );
        }
    }

	thread_stop( &t0 );


	// cleanup the precache list from the filesystem...
	unlink( ps.precache_file );

#endif

	log_msg( "precache: shutting down gracefully.\n" );

    // shutdown xwl and network
	xwl_shutdown();
	net_shutdown();
	log_shutdown();
	return 0;
}
