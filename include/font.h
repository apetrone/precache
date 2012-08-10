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
