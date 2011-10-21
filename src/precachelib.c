#include "precachelib.h"


static int print( void *ctx, int type, const JSON_value * value )
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
                if ( stricmp( ps->lastkey, "path" ) == 0 )
                {
                    if( ps->state->curfile )
                    {
                        strncpy( ps->state->curfile->path, value->vu.str.value, 255 );
                    }
                }
                else if ( stricmp( ps->lastkey, "md5" ) == 0 )
                {
                    if( ps->state->curfile )
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
    config.callback = &print;
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

	log_msg( "finished parsing precache.list. returning with result: %i\n", result );
    return result;
}
