#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// platform includes
#if LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#elif _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC 0
#endif

#elif __APPLE__
#endif



#ifndef Z_TYPES
typedef signed char i8;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;
typedef short i16;
typedef int i32;
#endif



// keys
enum
{
	XWLK_INVALID,
	XWLK_A,
	XWLK_B,
	XWLK_C,
	XWLK_D,
	XWLK_E,
	XWLK_F,
	XWLK_G,
	XWLK_H,
	XWLK_I,
	XWLK_J,
	XWLK_K,
	XWLK_L,
	XWLK_M,
	XWLK_N,
	XWLK_O,
	XWLK_P,
	XWLK_Q,
	XWLK_R,
	XWLK_S,
	XWLK_T,
	XWLK_U,
	XWLK_V,
	XWLK_W,
	XWLK_Y,
	XWLK_X,
	XWLK_Z,
	XWLK_LSYSTEM,
	XWLK_RSYSTEM,
	XWLK_MENU,
	XWLK_SEMICOLON,
	XWLK_SLASH,
	XWLK_BACKSLASH,
	XWLK_EQUALS,
	XWLK_MINUS,
	XWLK_LBRACKET,
	XWLK_RBRACKET,
	XWLK_COMMA,
	XWLK_PERIOD,
	XWLK_QUOTE,
	XWLK_TILDE,
	XWLK_ESCAPE,
	XWLK_SPACE,
	XWLK_RETURN,
	XWLK_BACKSPACE,
	XWLK_TAB,
	XWLK_PAGEUP,
	XWLK_PAGEDN,
	XWLK_END,
	XWLK_HOME,
	XWLK_INSERT,
	XWLK_DELETE,
	XWLK_ADD,
	XWLK_SUBTRACT,
	XWLK_MULTIPLY,
	XWLK_DIVIDE,
	XWLK_PAUSE,
	XWLK_F1,
	XWLK_F2,
	XWLK_F3,
	XWLK_F4,
	XWLK_F5,
	XWLK_F6,
	XWLK_F7,
	XWLK_F8,
	XWLK_F9,
	XWLK_F10,
	XWLK_F11,
	XWLK_F12,
	XWLK_F13,
	XWLK_F14,
	XWLK_F15,
	XWLK_LEFT,
	XWLK_RIGHT,
	XWLK_UP,
	XWLK_DOWN,
	XWLK_NUMPAD0,
	XWLK_NUMPAD1,
	XWLK_NUMPAD2,
	XWLK_NUMPAD3,
	XWLK_NUMPAD4,
	XWLK_NUMPAD5,
	XWLK_NUMPAD6,
	XWLK_NUMPAD7,
	XWLK_NUMPAD8,
	XWLK_NUMPAD9,
	XWLK_0,
	XWLK_1,
	XWLK_2,
	XWLK_3,
	XWLK_4,
	XWLK_5,
	XWLK_6,
	XWLK_7,
	XWLK_8,
	XWLK_9,
	XWLK_LSHIFT,
	XWLK_RSHIFT,
	XWLK_LCONTROL,
	XWLK_RCONTROL,
	XWLK_LALT,
	XWLK_RALT
};

// key mods
enum
{
	XWLKM_INVALID,

	XWLKM_ALT = 1,
	XWLKM_SHIFT = 2,
	XWLKM_CONTROL = 4
};

// mouse buttons
enum
{
	XWLMB_INVALID,
	XWLMB_LEFT,
	XWLMB_RIGHT,
	XWLMB_MIDDLE,
	XWLMB_MOUSE4,
	XWLMB_MOUSE5,
	XWLMB_MOUSE6,
	XWLMB_MOUSE7
};

// window creation constants
enum
{
	XWLW_INVALID,
	XWLW_FULLSCREEN,
};

// event constants
enum
{
	XWLE_INVALID,
	XWLE_MOUSEMOVE,
	XWLE_MOUSEBUTTON_PRESSED,
	XWLE_MOUSEBUTTON_RELEASED,
	XWLE_MOUSEWHEEL,
	XWLE_KEYPRESSED,
	XWLE_KEYRELEASED,
	XWLE_JOYSTICK_MOVE,
	XWLE_JOYSTICKBUTTON_PRESSED,
	XWLE_JOYSTICKBUTTON_RELEASED,
	XWLE_SIZE,
	XWLE_CLOSED,
	XWLE_LOSTFOCUS,
	XWLE_GAINFOCUS,
	XWLE_TEXT
};

// flags
enum
{
	XWL_WINDOWED,
	XWL_FULLSCREEN = 1, // start window full screen
	XWL_NORESIZE = 2, // disable window resizing,
	XWL_OPENGL = 4, // setup opengl rendering
};

typedef struct xwl_displaymode_s
{
	u32 width;
	u32 height;
} xwl_displaymode_t;

typedef struct xwl_window_s
{
	void * handle;
	void * userdata;

#if _WIN32
	HDC dc;
#endif

#if LINUX
	void * context;
#endif

#if __APPLE__
	void * view;
#endif
} xwl_window_t;

typedef struct xwl_windowparams_s
{
	u32 width;
	u32 height;
	u32 flags;
	u32 x;
	u32 y;
	void * userdata;
	char * title;
} xwl_windowparams_t;

typedef struct xwl_event_s
{
	// target window
	xwl_window_t *target;

	// key
	i32 key;
	i32 unicode;

	// size dimension
	i32 width;
	i32 height;

	// mouse pos
	i32 mx;
	i32 my;

	// joystick
	i16 joyid;
	i16 joyaxis;
	i16 joybutton;
	float joypos;

	// event types
	u16 type;

	// -1 is towards the user
	// 1 is away from the user
	i16 wheelDelta;
	i16 button;
	i16 keymods;

} xwl_event_t;


typedef void (*xwl_event_callback)( xwl_event_t * );

// returns 0 on failure
// returns 1 on success
i32 xwl_startup();

// shutdown system
void xwl_shutdown();

// returns 0 if no events are queued
// returns 1 if an event is removed from the queue
i32 xwl_pollevent( xwl_event_t *event );

// returns 0 on failure
// tite is a UTF-8 encoded string
xwl_window_t *xwl_create_window( xwl_windowparams_t *params, const char * title );


// set the event callback
void xwl_set_callback( xwl_event_callback cb );


const char * xwl_key_to_string( i32 key );
const char * xwl_event_to_string( i32 event_type );
const char * xwl_mouse_to_string( i32 mouse );

const char * xwl_get_error();

typedef struct xwl_window_handle_s
{
    xwl_window_t handle;

#ifdef LINUX
    XIC inputContext;
    Atom atomClose;
    XEvent lastKeyRelease;
#endif

} xwl_window_handle_t;

xwl_window_handle_t *xwl_get_unused_window();
void xwl_send_event( xwl_event_t * ev );
void xwl_setup_rendering( xwl_window_t * window );
void xwl_finish();



// -- platform specifics
typedef struct xwl_renderer_settings_s
{
    xwl_window_t * window;

#if LINUX
	XVisualInfo * visual;
	Display * display;
	i32 screen;
#endif

} xwl_renderer_settings_t;


#ifdef __cplusplus
}; // extern "C"
#endif
