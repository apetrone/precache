#include <xwl/xwl.h>
#include <stdio.h>

// temporary OpenGL tests
#if _WIN32
#include <windows.h>
#include <gl/gl.h>
#pragma comment( lib, "opengl32.lib" )
#elif LINUX
#include <GL/gl.h>
#include <GL/glx.h>
#elif __APPLE__
#if TARGET_OS_IPHONE
#elif TARGET_OS_MAC
	#include <OpenGL/OpenGL.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if _WIN32
i32 xwl_renderer_startup( xwl_renderer_settings_t * settings, u32 * attribs )
{
	HGLRC glrc;
	PIXELFORMATDESCRIPTOR pfd = {0};
	i32 pixelFormat = -1;
	HWND handle;

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.cAlphaBits = 8;
	pfd.dwLayerMask = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	if ( !settings->window || !settings->window->handle )
		return 0;

	handle = (HWND)settings->window->handle;

	// get a DC to our window
	settings->window->dc = GetDC( handle );

	// choose the pixel format
	pixelFormat = ChoosePixelFormat( settings->window->dc, &pfd );

	if ( pixelFormat == 0 )
		return 0;

	// set the pixel format
	SetPixelFormat( settings->window->dc, pixelFormat, &pfd );

	// create a rendering context from it
	glrc = wglCreateContext( settings->window->dc );

	// make that context current
	wglMakeCurrent( settings->window->dc, glrc );

	return 1;
}

void xwl_renderer_post( xwl_renderer_settings_t * settings )
{
	if ( !settings->window )
		return;

	SwapBuffers( settings->window->dc );
}

void xwl_renderer_shutdown( xwl_renderer_settings_t * settings )
{
	HGLRC rc;

	if ( !settings->window )
		return;

	rc = wglGetCurrentContext();

	wglMakeCurrent( 0, 0 );

	wglDeleteContext( rc );

	ReleaseDC( (HWND)settings->window->handle, settings->window->dc );
}
	
void xwl_renderer_activate( xwl_renderer_settings_t * settings )
{
}
#endif


#if LINUX
i32 xwl_renderer_startup( xwl_renderer_settings_t * settings, u32 * attribs )
{
    XWindowAttributes att;
    Window window = (Window)settings->window;

     GLint   attrib[] = {GLX_RGBA, GLX_DOUBLEBUFFER, // need double buffering
                            GLX_DEPTH_SIZE, 16,    // put in the depth size
                                                        // that was passed to us
                            GLX_RED_SIZE, 8,            // 8 bits pretty standard for our RGBA
                            GLX_GREEN_SIZE, 8,
                            GLX_BLUE_SIZE,8,
                            GLX_ALPHA_SIZE, 8,
                            None };

    settings->visual = glXChooseVisual( settings->display, settings->screen, attrib );
    if ( !settings->visual )
    {
        printf( "Unable to get visual!\n" );
        return 0;
    }

    settings->window->context = glXCreateContext( settings->display, settings->visual, 0, 1 );

    if ( !settings->window->context )
    {
        printf( "Unable to create context!\n" );
        return 0;
    }

    if ( !glXIsDirect( settings->display, settings->window->context ) )
    {
        printf( "Rendering is NOT direct.\n" );
        return 0;
    }

	return 1;
}

void xwl_renderer_post( xwl_renderer_settings_t * settings )
{
    if ( !settings || !settings->display || !settings->window )
        return;

    glXSwapBuffers( settings->display, (GLXDrawable)settings->window->handle );
}

void xwl_renderer_shutdown( xwl_renderer_settings_t * settings )
{
    glXMakeCurrent( settings->display, None, 0 );
    glXDestroyContext( settings->display, settings->window->context );
}
	
void xwl_renderer_activate( xwl_renderer_settings_t * settings )
{
}
#endif




#if __APPLE__
void xwl_pollevent_osx( xwl_event_t * event );
void xwl_setup_osx_rendering( xwl_window_t * window, u32 * attribs );
void xwl_osx_finish( xwl_window_t * window );
void xwl_osx_activate( xwl_window_t * window );

i32 xwl_renderer_startup( xwl_renderer_settings_t * settings, u32 * attribs )
{
	// setup an opengl view
	xwl_setup_osx_rendering( settings->window, attribs );

	return 1;
}

void xwl_renderer_post( xwl_renderer_settings_t * settings )
{
	xwl_osx_finish( settings->window );
}

void xwl_renderer_shutdown( xwl_renderer_settings_t * settings )
{

}

void xwl_renderer_activate( xwl_renderer_settings_t * settings )
{
	xwl_osx_activate( settings->window );
}
#endif

#ifdef __cplusplus
}; // extern "C"
#endif
