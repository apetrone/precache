#pragma once

#include <xwl/xwl.h>

void xwl_osx_startup();
void xwl_osx_shutdown();

xwl_window_handle_t *xwl_create_osx_window( xwl_windowparams_t * params );
void xwl_pollevent_osx( xwl_event_t * event );
void xwl_setup_osx_rendering( xwl_window_t * window );
void xwl_osx_finish( xwl_window_t * window );
#if __OBJC__

#import <Cocoa/Cocoa.h>


@interface MyOpenGLView : NSView
{
	NSOpenGLContext * ctx;
}
-(id)initWithFrame:(NSRect)frameRect;
-(void)dealloc;
-(NSOpenGLContext*) getContext;
-(void)setContext:(NSOpenGLContext*)context;
-(BOOL) isOpaque;
@end

@interface xwlWindow : NSWindow
{
@public
	xwl_window_handle_t * xwlhandle;
	MyOpenGLView * render;
}

@property (nonatomic) xwl_window_handle_t *xwlhandle;
@property (nonatomic, retain) MyOpenGLView *render;

-(BOOL) canBecomeKeyWindow;
-(BOOL) canBecomeMainWindow;

@end



@interface xwlDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
{}
@end


#endif