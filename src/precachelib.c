#include "precachelib.h"
#include "platform.h"
#include "log.h"

int parse_json( void *ctx, int type, const JSON_value * value )
{
    precache_file_t * file;
    precache_parse_state_t * ps = (precache_parse_state_t*)ctx;
    //printf( "JSON type: %i | %i\n", type, ps->flags );

    switch( type )
    {
        case JSON_T_INTEGER:
            //printf( "integer: %i\n", (int)value->vu.integer_value );
            if (ps->flags == 0 )
            {
                if (stricmp( ps->lastkey, "version" ) == 0 )
                {
                    //printf( "Version is: %i\n", (int)value->vu.integer_value );
                }
            }
            else if ( (ps->flags & 2) )
            {
				if ( ps->state->curfile )
				{
					if ( stricmp( ps->lastkey, "flags" ) == 0 )
					{
						ps->state->curfile->extra_flags = (int)value->vu.integer_value;
					}
				}
            }
            break;
        case JSON_T_ARRAY_BEGIN:
            ps->flags |= 1;
            // start reading files
            break;
        case JSON_T_ARRAY_END:
            ps->flags &= ~1;
            // stopped reading files
            break;
        case JSON_T_OBJECT_BEGIN:
            if ( (ps->flags & 1) )
            {
                ps->flags |= 2;
                // allocate a new file and link it in
                file = (precache_file_t*)malloc( sizeof(precache_file_t) );
                memset( file, 0, sizeof(precache_file_t) );
                file->next = ps->state->files;
                file->flags = 0;
				file->extra_flags = 0;
				memset( file->targetpath, 0, MAX_PATH_SIZE );
				memset( file->path, 0, MAX_PATH_SIZE );
                ps->state->files = file;
                ps->state->curfile = file;
            }

            break;
        case JSON_T_OBJECT_END:
            if ( (ps->flags & 1) )
                ps->flags &= ~2;
            break;
        case JSON_T_KEY:
            strncpy( ps->lastkey, value->vu.str.value, 127 );
            //printf( "key: '%s', value = ", ps->lastkey );

            break;

        case JSON_T_STRING:
            //printf( "string: '%s'\n", value->vu.str.value );
            if (ps->flags == 0 )
            {
                if ( stricmp( ps->lastkey, "base" ) == 0 )
                {
                    strncpy( ps->state->base, value->vu.str.value, 127 );
                    //printf( "Base is: %s\n", ps->state->base );
                }
            }
            else if ( ps->flags & 2 )
            {
				if ( ps->state->curfile )
				{
					if ( stricmp( ps->lastkey, "path" ) == 0 )
					{
						strncpy( ps->state->curfile->path, value->vu.str.value, MAX_PATH_SIZE-1 );
					}
					else if ( stricmp( ps->lastkey, "target" ) == 0 )
					{
						strncpy( ps->state->curfile->targetpath, value->vu.str.value, MAX_PATH_SIZE-1 );
					}
					else if ( stricmp( ps->lastkey, "md5" ) == 0 )
					{
						strncpy( ps->state->curfile->checksum, value->vu.str.value, 32 );
					}
				}
            }
            break;
    }

    return 1;
} // print



int precache_parse_list( precache_state_t * precache )
{
    char absolute_path[ PRECACHE_TEMP_BUFFER_SIZE ] = {0};
    JSON_config config;
    int i;
    int result = 1;
    struct JSON_parser_struct * jc = 0;
    FILE * fp;
    long fileSize;
    char * buffer;
    int linenumber;
    int colnumber;
	precache_file_t * file;
    precache_parse_state_t pstate;

    // construct absolute path to precache list...
    strcpy( absolute_path, precache->localpath );
    strcat( absolute_path, precache->curfile->path );

	platform_conform_slashes( absolute_path, PRECACHE_TEMP_BUFFER_SIZE );
    log_msg( "* precache.list file: %s\n", absolute_path );

    fp = fopen( absolute_path, "rb" );

    if ( !fp )
        return 0;

    fseek( fp, 0, SEEK_END );
    fileSize = ftell( fp );
    fseek( fp, 0, 0 );

    // don't allocate more than X bytes for the buffer. Warn the user?

    // allocate memory for the buffer
    buffer = (char*)malloc( fileSize+1 );
    memset( buffer, 0, fileSize+1 );

    fread( buffer, fileSize, 1, fp );
    fclose( fp );

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
    for( i = 0; i < fileSize; ++i )
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

    if ( buffer )
    {
        free( buffer );
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