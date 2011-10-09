#pragma once

#include <xwl/xwl.h>

void xwl_osx_startup();
void xwl_osx_shutdown();

xwl_window_handle_t *xwl_create_osx_window( xwl_windowparams_t * params, const char * title );
void xwl_pollevent_osx( xwl_event_t * event );
void xwl_setup_osx_rendering( xwl_window_t * window, u32 * attribs );
void xwl_osx_finish( xwl_window_t * window );
void *xwl_osx_rendering_context( xwl_window_t * window );

#if __OBJC__

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#import "EAGLView.h"


@interface xwlWindow : UIWindow
{
@public
	xwl_window_handle_t * xwlhandle;
	EAGLView * render;
}

@property (nonatomic) xwl_window_handle_t * xwlhandle;
@property (nonatomic,retain) EAGLView * render;
@end

@interface xwlDelegate : NSObject <UIApplicationDelegate, UIAccelerometerDelegate> {
    xwlWindow *window;
	EAGLView *glView;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;
@end



#elif TARGET_OS_MAC

#import <Cocoa/Cocoa.h>
@interface MyOpenGLView : NSView
{
@public
	NSOpenGLContext * ctx;
}

@property (nonatomic, retain) NSOpenGLContext * context;

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

#endif