#pragma once


#define FONT_CHARINFO_ARRAY_SIZE 95

typedef struct font_charinfo_s
{
    unsigned int glyphIndex; // this character's glyph index
    int advanceX; // add advanceX and advanceY after drawing
    int advanceY;
    int left; // add left before drawing
    int top; // subtract top to offset origin at top, left
    int width;
    int height;
    int hBearingX;
    int hBearingY;
    int vBearingX;
    int vBearingY;
    float uv[4]; // uv rect
} font_charinfo_t;

typedef struct font_s
{
    unsigned int textureWidth;
    unsigned int textureHeight;
    unsigned int textureID;

    // glyph to character map
    struct font_charinfo_s char_info[ FONT_CHARINFO_ARRAY_SIZE ];
} font_t;


typedef unsigned char font_char_t;


void font_create( font_t * font );
void font_load_embedded( font_t * font, const unsigned char * data, int dataSize );
void font_draw( font_t * font, int x, int y, const char * str, unsigned char r, unsigned char g, unsigned char b, unsigned char a );
void font_destroy( font_t * font );
int font_string_width( font_t * font, const char * str );

font_charinfo_t * font_get_character( font_t * font, font_char_t c );


// embedded fonts
extern unsigned char font_liberation_mono8[];
extern int font_liberation_mono8_size;

extern unsigned char font_pf_tempesta_seven8[];
extern int font_pf_tempesta_seven8_size;
