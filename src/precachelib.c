#include "precachelib.h"
#include "platform.h"
#include "log.h"


int parse_json( void *ctx, int type, const JSON_value * value )
{
    precache_file_t * file;
    precache_parse_state_t * ps = (precache_parse_state_t*)ctx;
    parse_msg( "JSON type: %i | %i\n", type, ps->flags );

    switch( type )
    {
        case JSON_T_INTEGER:
            parse_msg( "integer: %i\n", (int)value->vu.integer_value );
            if (ps->flags == JPS_NONE )
            {
                if (stricmp( ps->lastkey, PRECACHE_LIST_VERSION ) == 0 )
                {
                    parse_msg( "Version is: %i\n", (int)value->vu.integer_value );
                }
            }
            else if ( (ps->flags & JPS_FILE) )
            {
				if ( ps->state->curfile )
				{
					if ( stricmp( ps->lastkey, PRECACHE_LIST_FLAGS ) == 0 )
					{
						ps->state->curfile->extra_flags = (int)value->vu.integer_value;
					}
				}
            }
            break;
        case JSON_T_ARRAY_BEGIN:
			parse_msg( "JSON_T_ARRAY_BEGIN\n" );
			if ( ps->lastkey )
			{
				if ( stricmp( ps->lastkey, PRECACHE_LIST_FILELIST ) == 0 )
				{
					ps->flags |= JPS_FILELIST;
				}
				else if ( stricmp( ps->lastkey, PRECACHE_LIST_UPDATELIST ) == 0 )
				{
					ps->flags |= JPS_UPDATERLIST;
				}
			}

            // start reading files
            break;
        case JSON_T_ARRAY_END:
			parse_msg( "JSON_T_ARRAY_END\n" );
			if ( ps->flags & JPS_FILELIST )
			{
				ps->flags &= ~JPS_FILELIST;
				ps->flags &= ~JPS_FILE;
			}
			else if ( ps->flags & JPS_UPDATERLIST )
			{
				ps->flags &= ~JPS_UPDATERLIST;
				ps->flags &= ~JPS_FILE;
			}
            // stopped reading files
            break;
        case JSON_T_OBJECT_BEGIN:
			parse_msg( "JSON_T_OBJECT_BEGIN\n" );

            ps->flags |= JPS_FILE;
            // allocate a new file and link it in
            file = (precache_file_t*)malloc( sizeof(precache_file_t) );
            memset( file, 0, sizeof(precache_file_t) );
            
            file->flags = 0;
			file->extra_flags = 0;
			memset( file->targetpath, 0, MAX_PATH_SIZE );
			memset( file->path, 0, MAX_PATH_SIZE );

            if ( (ps->flags & JPS_FILELIST) )
            {
				file->next = ps->state->files;
                ps->state->files = file;
                ps->state->curfile = file;
            }
			else if ( ps->flags & JPS_UPDATERLIST )
			{
				file->next = ps->state->updatelist;
				ps->state->updatelist = file;
				ps->state->curfile = file;
			}

#if LINUX || __APPLE__
			file->mode = precache_mode_string_to_integer( PRECACHE_DEFAULT_FILE_PERMISSIONS );
#endif

            break;
        case JSON_T_OBJECT_END:
			parse_msg( "JSON_T_OBJECT_END\n" );
            if ( (ps->flags & JPS_FILELIST) )
                ps->flags &= ~JPS_FILE;
            break;
        case JSON_T_KEY:
            strncpy( ps->lastkey, value->vu.str.value, 127 );
            parse_msg( "key: '%s', value = '%s'\n", ps->lastkey );

            break;

        case JSON_T_STRING:
            parse_msg( "string: '%s'\n", value->vu.str.value );
            if (ps->flags == JPS_NONE )
            {
				if ( stricmp( ps->lastkey, PRECACHE_LIST_INSTALL_PATH ) == 0 )
                {
                    strncpy( ps->state->relativepath, value->vu.str.value, 127 );
                }
				else if ( stricmp( ps->lastkey, PRECACHE_LIST_REMOTE_PROJECT_PATH ) == 0 )
				{
					strcat( ps->state->remotepath, value->vu.str.value );
				}
            }
            else if ( ps->flags & JPS_FILE )
            {
				if ( ps->state->curfile )
				{
					if ( stricmp( ps->lastkey, PRECACHE_LIST_PATH ) == 0 )
					{
						strncpy( ps->state->curfile->path, value->vu.str.value, MAX_PATH_SIZE-1 );
					}
					else if ( stricmp( ps->lastkey, PRECACHE_LIST_TARGET ) == 0 )
					{
						strncpy( ps->state->curfile->targetpath, value->vu.str.value, MAX_PATH_SIZE-1 );
					}
					else if ( stricmp( ps->lastkey, PRECACHE_LIST_MD5 ) == 0 )
					{
						strncpy( ps->state->curfile->checksum, value->vu.str.value, 32 );
					}

					else if ( stricmp( ps->lastkey, PRECACHE_LIST_CHMOD ) == 0 )
					{
#if LINUX || __APPLE__
						ps->state->curfile->mode = precache_mode_string_to_integer( value->vu.str.value );
#endif
					}

				}
            }
            break;
    }

    return 1;
} // print


int precache_parse_listbuffer( precache_state_t * precache, char * buffer, int bufferSize )
{
	int result = 1;
	JSON_config config;
	int i;
    int linenumber;
    int colnumber;
	precache_parse_state_t pstate;
    struct JSON_parser_struct * jc = 0;

    // setup precache internal parser state
    pstate.state = precache;
    pstate.flags = 0;
    memset( pstate.lastkey, 0, 128);

    // setup the JSON config for parsing
    init_JSON_config( &config );
    config.depth = 19;
	config.callback = &parse_json;
    config.allow_comments = 1;
    config.handle_floats_manually = 0;
    config.callback_ctx = &pstate;

    jc = new_JSON_parser(&config);

    // no one ever says "line zero"
    linenumber = 1;
    colnumber = 0;

    // throw the buffer in the parser
    for( i = 0; i < bufferSize; ++i )
    {
        if ( buffer[i] == '\n' )
        {
            ++linenumber;
            colnumber = 0;
        }

        if ( !JSON_parser_char(jc, buffer[i] ) )
        {
            log_msg( "JSON_parser_char: syntax error, col %d, line: %i\n", colnumber, linenumber );
            strcpy( precache->err1, "Error parsing precache.list" );
            sprintf( precache->err2, "syntax error: line %i, col %i", linenumber, colnumber );
            result = 0;
            break;
        }

        ++colnumber;
    }

    if ( !JSON_parser_done(jc) )
    {
        log_msg( "JSON_parser_end: syntax error\n" );
        result = 0;
    }

    // cleanup resources
    delete_JSON_parser(jc);

	return result;
}

int precache_parse_list( precache_state_t * precache )
{
    char absolute_path[ PRECACHE_TEMP_BUFFER_SIZE ] = {0};
    int result = 1;
    long fileSize;
    char * buffer;
	precache_file_t * file;
    

    // construct absolute path to precache list...
    strcpy( absolute_path, precache->localpath );
    strcat( absolute_path, precache->curfile->path );

	platform_conform_slashes( absolute_path, PRECACHE_TEMP_BUFFER_SIZE );
    log_msg( "* precache.list file: %s\n", absolute_path );

	buffer = allocate_file_buffer( absolute_path, &fileSize );

	if ( buffer )
	{
		result = precache_parse_listbuffer( precache, buffer, fileSize );
		free( buffer );
	}
	else
	{
		log_msg( "* error allocating file buffer for (%s)\n", absolute_path );
	}

	// go through each file and make sure targetpath has a valid path
	file = precache->files;
	while( file )
	{
		if ( file->targetpath[0] == 0 )
			strcpy( file->targetpath, file->path );

		file = file->next;
	}


	//log_msg( "finished parsing precache.list. returning with result: %i\n", result );
    return result;
}



precache_file_t * precache_locate_next_file( precache_file_t * start )
{
	precache_file_t * cur = start;
	precache_file_t * next = 0;
	int platform_id;

	log_msg( "* LOC: start is [%s]\n", start->path );
	do
	{
		// is this file platform agnostic?
		platform_id = ((cur->extra_flags >> PRECACHE_FILE_PLATFORM_BIT) & 0x0F);
		log_msg( "* LOC: platform_id=%i [PRECACHE_PLATFORM=%i]\n", platform_id, PRECACHE_PLATFORM );
		if ( platform_id == 0 || platform_id == PRECACHE_PLATFORM )
		{
			// has this file been downloaded yet?
			if ( !(cur->flags & 1) )
			{
				log_msg( "* LOC: file not downloaded: [%s]\n", cur->path );
				next = cur;
				break;
			}
			else
			{
				log_msg( "* LOC: file downloaded, skipping: [%s]\n", cur->path );
			}
		}
		else
		{
			log_msg( "* LOC: file not required for this platform: [%s]\n", cur->path );
		}
		
		cur = cur->next;
	} while( cur );

	return next;
} // precache_locate_next_file

precache_file_t * precache_locate_executable_file( precache_file_t * start )
{
	precache_file_t * cur = start;
	precache_file_t * next = 0;
	int platform_id;
	int executable_bit;

	log_msg( "* precache_locate_executable_file: start is [%s]\n", start->path );
	do
	{
		// is this file platform agnostic?
		platform_id = ((cur->extra_flags >> PRECACHE_FILE_PLATFORM_BIT) & 0x0F);
		if ( platform_id == 0 || platform_id == PRECACHE_PLATFORM )
		{
			executable_bit = ((cur->extra_flags >> PRECACHE_FILE_EXECUTE_BIT) & 0x0F);
			if ( executable_bit > 0 )
			{
				log_msg( "* precache_locate_executable_file: found first executable file [%s]\n", cur->path );
				return cur;
			}
		}

		cur = cur->next;
	} while( cur );

	return next;
} // precache_locate_executable_file

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
    md5_append( &md5state, (unsigned char*)data, (int)fileSize );

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


void precache_sanitize_path( char * path )
{
	size_t len = strlen(path);

	if ( path[ len-1 ] == '/' || path[ len-1 ] == '\\' )
		path[ len-1 ] = '\0';
} // precache_sanitize_path


char * allocate_file_buffer( const char * path, long * fileSize )
{
	char * buffer;
	FILE * fp;

	// open the file
    fp = fopen( path, "rb" );

    if ( !fp )
        return 0;

	// seek to the end of the file and get the file size
    fseek( fp, 0, SEEK_END );
    *fileSize = ftell( fp );
    fseek( fp, 0, 0 );

    // allocate memory for the buffer
    buffer = (char*)malloc( (*fileSize)+1 );
    memset( buffer, 0, (*fileSize)+1 );

	// read file into buffer
    fread( buffer, (*fileSize), 1, fp );

	// close file
    fclose( fp );

	return buffer;
}

int precache_should_update_self( precache_state_t * precache )
{
	int result = 0;

	if ( precache->updatelist )
	{
		log_msg( "* Updatelist items are present in the file...\n" );
	}




	return result;
} // precache_should_update_self

#if LINUX || __APPLE__
int precache_mode_string_to_integer( const char * mode )
{
	int value = 0;
	char tmp;

	// permissions for owner
	tmp = mode[0] - '0';
	if ( tmp & 4 )
		value |= S_IRUSR;

	if ( tmp & 2 )
		value |= S_IWUSR;

	if ( tmp & 1 )
		value |= S_IXUSR;

	// permissions for group
	tmp = mode[1] - '0';
	if ( tmp & 4 )
		value |= S_IRGRP;

	if ( tmp & 2 )
		value |= S_IWGRP;

	if ( tmp & 1 )
		value |= S_IXGRP;

	// permissions for others
	tmp = mode[2] - '0';
	if ( tmp & 4 )
		value |= S_IROTH;

	if ( tmp & 2 )
		value |= S_IWOTH;

	if ( tmp & 1 )
		value |= S_IXOTH;

	return value;
} // precache_mode_string_to_integer
#endif


void float_color_to_char( float * colors, unsigned char * out )
{
	if ( !out || !colors )
	{
		log_msg( "* invalid pointer to float_color_to_char!\n" );
		return;
	}

	out[0] = (unsigned char)(colors[0] * 255.0);
	out[1] = (unsigned char)(colors[1] * 255.0);
	out[2] = (unsigned char)(colors[2] * 255.0);
	out[3] = (unsigned char)(colors[3] * 255.0);
} // float color to char