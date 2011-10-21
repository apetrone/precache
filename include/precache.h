#pragma once

struct button_s;

typedef void (*button_event)( struct button_s * source );

typedef struct button_s
{
	i16 width;
	i16 height;
	i16 x;
	i16 y;
	unsigned char color[4];

	button_event event;
} button;





i32 mouse_inside_button( button * b );
