#include <xwl/xwl.h>
#include <xwl/xwl_osx.h>

NSAutoreleasePool *pool = 0;
NSApplication * application = 0;
xwlDelegate *appDelegate = 0;

#define XWLWINDOW xwlWindow
 ////////////////////////////////////////////////////////
u32 LocalizedKeys(unichar ch)
 {
	 switch (ch) 
	 {
		 case 'a':
		 case 'A':                   return XWLK_A;
			 
		 case 'b':
		 case 'B':                   return XWLK_B;
			 
		 case 'c':
		 case 'C':                   return XWLK_C;
			 
		 case 'd':
		 case 'D':                   return XWLK_D;
			 
		 case 'e':
		 case 'E':                   return XWLK_E;
			 
		 case 'f':
		 case 'F':                   return XWLK_F;
			 
		 case 'g':
		 case 'G':                   return XWLK_G;
			 
		 case 'h':
		 case 'H':                   return XWLK_H;
			 
		 case 'i':
		 case 'I':                   return XWLK_I;
			 
		 case 'j':
		 case 'J':                   return XWLK_J;
			 
		 case 'k':
		 case 'K':                   return XWLK_K;
			 
		 case 'l':
		 case 'L':                   return XWLK_L;
			 
		 case 'm':
		 case 'M':                   return XWLK_M;
			 
		 case 'n':
		 case 'N':                   return XWLK_N;
			 
		 case 'o':
		 case 'O':                   return XWLK_O;
			 
		 case 'p':
		 case 'P':                   return XWLK_P;
			 
		 case 'q':
		 case 'Q':                   return XWLK_Q;
			 
		 case 'r':
		 case 'R':                   return XWLK_R;
			 
		 case 's':
		 case 'S':                   return XWLK_S;
			 
		 case 't':
		 case 'T':                   return XWLK_T;
			 
		 case 'u':
		 case 'U':                   return XWLK_U;
			 
		 case 'v':
		 case 'V':                   return XWLK_V;
			 
		 case 'w':
		 case 'W':                   return XWLK_W;
			 
		 case 'x':
		 case 'X':                   return XWLK_X;
			 
		 case 'y':
		 case 'Y':                   return XWLK_Y;
			 
		 case 'z':
		 case 'Z':                   return XWLK_Z;
			 
			 // The kew is not 'localized'.
		 default:                    return XWLK_INVALID;
	 }
 }
 

////////////////////////////////////////////////////////
u32 NonLocalizedKeys(unsigned short keycode)
{
	// (Some) 0x code based on http://forums.macrumors.com/showthread.php?t=780577
	// Some sf::Key are present twice.
	switch (keycode) 
	{
			// These cases should not be used but anyway...
		case 0x00:                      return XWLK_A;
		case 0x0b:                      return XWLK_B;
		case 0x08:                      return XWLK_C;
		case 0x02:                      return XWLK_D;
		case 0x0e:                      return XWLK_E;
		case 0x03:                      return XWLK_F;
		case 0x05:                      return XWLK_G;
		case 0x04:                      return XWLK_H;
		case 0x22:                      return XWLK_I;
		case 0x26:                      return XWLK_J;
		case 0x28:                      return XWLK_K;
		case 0x25:                      return XWLK_L;
		case 0x2e:                      return XWLK_M;
		case 0x2d:                      return XWLK_N;
		case 0x1f:                      return XWLK_O;
		case 0x23:                      return XWLK_P;
		case 0x0c:                      return XWLK_Q;
		case 0x0f:                      return XWLK_R;
		case 0x01:                      return XWLK_S;
		case 0x11:                      return XWLK_T;
		case 0x20:                      return XWLK_U;
		case 0x09:                      return XWLK_V;
		case 0x0d:                      return XWLK_W;
		case 0x07:                      return XWLK_X;
		case 0x10:                      return XWLK_Y;
		case 0x06:                      return XWLK_Z;
			
			// These cases should not be used but anyway...
		case 0x1d:                      return XWLK_0;
		case 0x12:                      return XWLK_1;
		case 0x13:                      return XWLK_2;
		case 0x14:                      return XWLK_3;
		case 0x15:                      return XWLK_4;
		case 0x17:                      return XWLK_5;
		case 0x16:                      return XWLK_6;
		case 0x1a:                      return XWLK_7;
		case 0x1c:                      return XWLK_8;
		case 0x19:                      return XWLK_9;
			
		case 0x35:                      return XWLK_ESCAPE;
			
			// Modifier keys : never happen with keyDown/keyUp methods (?)
		case 0x3b:                      return XWLK_LCONTROL;
		case 0x38:                      return XWLK_LSHIFT;
		case 0x3a:                      return XWLK_LALT;
		case 0x37:                      return XWLK_LSYSTEM;
		case 0x3e:                      return XWLK_RCONTROL;
		case 0x3c:                      return XWLK_RSHIFT;
		case 0x3d:                      return XWLK_RALT;
		case 0x36:                      return XWLK_RSYSTEM;
			
		case NSMenuFunctionKey:         return XWLK_MENU;
			
		case 0x21:                      return XWLK_LBRACKET;
		case 0x1e:                      return XWLK_RBRACKET;
		case 0x29:                      return XWLK_SEMICOLON;
		case 0x2b:                      return XWLK_COMMA;
		case 0x2f:                      return XWLK_PERIOD;
		case 0x27:                      return XWLK_QUOTE;
		case 0x2c:                      return XWLK_SLASH;
		case 0x2a:                      return XWLK_BACKSLASH;
			
//#warning XWLK_TILDE might be in conflict with some other key.
			// 0x0a is for "Non-US Backslash" according to HID Calibrator,
			// a sample provided by Apple.
		//case 0x0a:                      return XWLK_TILDE;
		case 0x1b:						return XWLK_MINUS;
		case 0x18:                      return XWLK_EQUALS;
		case 0x32:                      return XWLK_TILDE;
		case 0x31:                      return XWLK_SPACE;
		case 0x24:                      return XWLK_RETURN;
		case 0x33:                      return XWLK_BACKSPACE;
		case 0x30:                      return XWLK_TAB;
			
			// Duplicates (see next §).
		case 0x74:                      return XWLK_PAGEUP;
		case 0x79:                      return XWLK_PAGEDN;
		case 0x77:                      return XWLK_END;
		case 0x73:                      return XWLK_HOME;
			
		case NSPageUpFunctionKey:       return XWLK_PAGEUP;
		case NSPageDownFunctionKey:     return XWLK_PAGEDN;
		case NSEndFunctionKey:          return XWLK_END;
		case NSHomeFunctionKey:         return XWLK_HOME;
			
		case NSInsertFunctionKey:       return XWLK_INSERT;
		case NSDeleteFunctionKey:       return XWLK_DELETE;
			
		case 0x45:                      return XWLK_ADD;
		case 0x4e:                      return XWLK_SUBTRACT;
		case 0x43:                      return XWLK_MULTIPLY;
		case 0x4b:                      return XWLK_DIVIDE;
			
			// Duplicates (see next §).
		case 0x7b:                      return XWLK_LEFT;
		case 0x7c:                      return XWLK_RIGHT;
		case 0x7e:                      return XWLK_UP;
		case 0x7d:                      return XWLK_DOWN;
			
		case NSLeftArrowFunctionKey:    return XWLK_LEFT;
		case NSRightArrowFunctionKey:   return XWLK_RIGHT;
		case NSUpArrowFunctionKey:      return XWLK_UP;
		case NSDownArrowFunctionKey:    return XWLK_DOWN;
			
		case 0x52:                      return XWLK_NUMPAD0;
		case 0x53:                      return XWLK_NUMPAD1;
		case 0x54:                      return XWLK_NUMPAD2;
		case 0x55:                      return XWLK_NUMPAD3;
		case 0x56:                      return XWLK_NUMPAD4;
		case 0x57:                      return XWLK_NUMPAD5;
		case 0x58:                      return XWLK_NUMPAD6;
		case 0x59:                      return XWLK_NUMPAD7;
		case 0x5b:                      return XWLK_NUMPAD8;
		case 0x5c:                      return XWLK_NUMPAD9;
			
			// Duplicates (see next §).
		case 0x7a:                      return XWLK_F1;
		case 0x78:                      return XWLK_F2;
		case 0x63:                      return XWLK_F3;
		case 0x76:                      return XWLK_F4;
		case 0x60:                      return XWLK_F5;
		case 0x61:                      return XWLK_F6;
		case 0x62:                      return XWLK_F7;
		case 0x64:                      return XWLK_F8;
		case 0x65:                      return XWLK_F9;
		case 0x6d:                      return XWLK_F10;
		case 0x67:                      return XWLK_F11;
		case 0x6f:                      return XWLK_F12;
		case 0x69:                      return XWLK_F13;
		case 0x6b:                      return XWLK_F14;
		case 0x71:                      return XWLK_F15;
			
		case NSF1FunctionKey:           return XWLK_F1;
		case NSF2FunctionKey:           return XWLK_F2;
		case NSF3FunctionKey:           return XWLK_F3;
		case NSF4FunctionKey:           return XWLK_F4;
		case NSF5FunctionKey:           return XWLK_F5;
		case NSF6FunctionKey:           return XWLK_F6;
		case NSF7FunctionKey:           return XWLK_F7;
		case NSF8FunctionKey:           return XWLK_F8;
		case NSF9FunctionKey:           return XWLK_F9;
		case NSF10FunctionKey:          return XWLK_F10;
		case NSF11FunctionKey:          return XWLK_F11;
		case NSF12FunctionKey:          return XWLK_F12;
		case NSF13FunctionKey:          return XWLK_F13;
		case NSF14FunctionKey:          return XWLK_F14;
		case NSF15FunctionKey:          return XWLK_F15;
			
		case NSPauseFunctionKey:        return XWLK_PAUSE;
			
//#warning keycode 0x1b is not bound to any key.
			// This key is ' on CH-FR, ) on FR and - on US layouts.
			
			// An unknown key.
		default:                        return 0;
	}
}





@implementation xwlDelegate
-(void)keyDown:(NSEvent*)event
{
	NSLog( @"xwlDelegate keyDown" );
}

-(void)keyUp:(NSEvent*)event
{
	NSLog( @"xwlDelegate keyUp" );
}
-(void) noResponderFor: (SEL)eventSelector
{
	NSLog( @"xwlDelegate: noResponderFor event!" );
	//[super noResponderFor:eventSelector];
}

-(void) windowWillMiniaturize:(NSNotification*)notification
{
	NSLog( @"Window will miniaturize" );
}

-(void) windowDidMiniaturize:(NSNotification*)notification
{
	NSLog( @"Window did miniaturize!" );
}

-(BOOL) windowShouldClose:(id)windowIn
{
	xwl_event_t ev = {0};
	ev.type = XWLE_CLOSED;

	xwl_send_event( &ev );
	NSLog( @"Window should close?" );
		
	return YES;
}
-(void) windowDidResignKey:(NSNotification*)notification
{
	xwl_event_t ev = {0};
	ev.type = XWLE_LOSTFOCUS;
	xwl_send_event( &ev );
	
	NSLog( @"Window did resign key %@", [notification object] );
}

-(void) windowDidBecomeKey:(NSNotification*)notification
{
	xwl_event_t ev = {0};
	ev.type = XWLE_GAINFOCUS;
	xwl_send_event( &ev );
	
	NSLog( @"Window did become key %@", [notification object] );
}

-(void)windowDidBecomeMain:(NSNotification*)notification
{
	NSLog( @"windowDidBecomeMain %@", [notification object] );
}

-(void)windowDidResignMain:(NSNotification*)notification
{
	NSLog( @"windowDidResignMain %@", [notification object] );
}

-(void) windowWillClose
{
	NSLog( @"Window will close!" );
}



- (void)windowDidEndSheet:(NSNotification *)notification
{
	NSLog( @"Window did end sheet!" );
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
	NSLog( @"Window Did Deminiaturize!" );
}


-(NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication *)sender;
{
	NSLog( @"ApplicationShouldTerminate" );
	return YES;
}

-(void) populateApplicationMenu:(NSMenu*) aMenu applicationName: (NSString*)applicationName
{
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", nil), applicationName]
								action:@selector(orderFrontStandardAboutPanel:)
						 keyEquivalent:@""];
	[menuItem setTarget:NSApp];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Preferences...", nil)
								action:NULL
						 keyEquivalent:@","];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Services", nil)
								action:NULL
						 keyEquivalent:@""];
	NSMenu * servicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
	[aMenu setSubmenu:servicesMenu forItem:menuItem];
	[NSApp setServicesMenu:servicesMenu];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Hide", nil), applicationName]
								action:@selector(hide:)
						 keyEquivalent:@"h"];
	[menuItem setTarget:NSApp];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Hide Others", nil)
								action:@selector(hideOtherApplications:)
						 keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	[menuItem setTarget:NSApp];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Show All", nil)
								action:@selector(unhideAllApplications:)
						 keyEquivalent:@""];
	[menuItem setTarget:NSApp];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", nil), applicationName]
								action:@selector(terminate:) keyEquivalent:@"q"];
	[menuItem setTarget:NSApp];
}

-(void) applicationWillFinishLaunching:(NSNotification *)aNotification
{
	NSLog( @"Application will finish launching?" );
	
	NSMenu * mainMenu = [[NSMenu alloc] initWithTitle: @"MainMenu"];
	NSMenuItem * menuItem;
	NSMenu * submenu;
	
	menuItem = [mainMenu addItemWithTitle: @"Apple" action: nil keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:@"Apple"];
	[NSApp performSelector:@selector(setAppleMenu:) withObject: submenu];
	// populate application menu
	
	[self populateApplicationMenu: submenu applicationName: @"Blah"];
	[mainMenu setSubmenu: submenu forItem: menuItem];
	//NSLog( @"Controller: %@", [window windowController] );
	
	//[JJMenuPopulator populateMainMenu];
	
	[NSApp setMainMenu: mainMenu];
	//[[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:@"/Developer/About Xcode Tools.pdf"]];
}

-(void) applicationDidFinishLaunching:(NSNotification*)notification
{
	NSLog( @"Application finished launching?" );
	
	//[self myGameInit];
}


@end

void xwl_osx_startup()
{
	// straight from SFML-2.0
	ProcessSerialNumber psn;
	// Set the process as a normal application so it can get focus.
	
	if (!GetCurrentProcess(&psn)) {
		TransformProcessType(&psn, kProcessTransformToForegroundApplication);
		SetFrontProcess(&psn);
	}
	
	// Tell the application to stop bouncing in the Dock.
	[[NSApplication sharedApplication] finishLaunching];
	// NOTE : This last call won't harm anything even if SFML window was
	// created with an external handle.
	
	pool = [[NSAutoreleasePool alloc] init];
	application = [NSApplication sharedApplication];
	appDelegate = [[xwlDelegate alloc] init];
	
	// set application handler here
	[application setDelegate: appDelegate];
	
	//[NSApp activateIgnoringOtherApps:YES];
	
	// setup the app menu
	[application finishLaunching];
}

void xwl_osx_shutdown()
{	
	[application setDelegate: nil ];
	[application release];
	
	[appDelegate release];
	appDelegate = 0;
}

void xwl_pollevent_osx( xwl_event_t * e )
{
	NSEvent * event = [NSApp nextEventMatchingMask:NSAnyEventMask 
										 untilDate: [NSDate distantPast]
											inMode: NSDefaultRunLoopMode
										   dequeue: YES];
	if ( event != nil )
	{
		//NSLog( @"Debug Event!" );
		// dispatch the event!
		[NSApp sendEvent: event ];
	}
	
	//[event release]; // is this needed?
}


@implementation MyOpenGLView
-(id)initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame: frameRect];
	
	if ( self == nil )
		return nil;

	return self;
}

-(void)lockFocus
{
	[super lockFocus];
	
	if ( [ctx view] != self )
	{
		[ctx setView: self];
	}
	
	[ctx makeCurrentContext];
	
	NSLog( @"Lock Focus!" );
}

-(void)unlockFocus
{
	NSLog( @"unlock focus" );
	[super unlockFocus];
}

-(void)dealloc
{
	[ctx release];
	[super dealloc];
}


-(NSOpenGLContext*)getContext
{
	return ctx;
}

-(void)setContext:(NSOpenGLContext*) context
{
	ctx = context;
}

-(BOOL) isOpaque
{
	return YES;
}


-(BOOL) acceptsFirstResponder
{
	return YES;
}

-(void) noResponderFor: (SEL)eventSelector
{
	NSLog( @"MyOpenGLView: noResponderFor event!" );
	[super noResponderFor:eventSelector];
}

-(void) windowResized:(NSNotification*) notification
{
	xwl_event_t ev = {0};
	XWLWINDOW * wnd = (XWLWINDOW*)[notification object];
	ev.type = XWLE_SIZE;
	
	ev.width = [[wnd contentView] frame].size.width;
	ev.height = [[wnd contentView] frame].size.height;
	
	if ( [wnd render] != nil )
		[[[wnd render] getContext] update];
	
	xwl_send_event( &ev );
	
	
	NSLog( @"windowResized" );
}

void dispatchMouseMoveEvent(NSEvent * theEvent)
{
	xwl_event_t ev = {0};
	u16 titleBarHeight;
	u32 fixedHeight;
	XWLWINDOW * wnd = (XWLWINDOW*)[theEvent window];
	NSPoint pt = [[theEvent window] mouseLocationOutsideOfEventStream];
	//NSPoint loc = [self convertPoint:[theEvent locationInWindow] fromView: nil];
	//printf( "x: %g, y: %g\n", loc.x, loc.y );
	
	// subtract the window height from the contentView height
	// this will give us the size of the title bar
	titleBarHeight = [wnd frame].size.height - [[wnd contentView] frame].size.height;	
	
	fixedHeight = [wnd frame].size.height - titleBarHeight;
	
	ev.type = XWLE_MOUSEMOVE;
	ev.mx = pt.x;
	ev.my = fixedHeight - pt.y; // top left is origin
	
	if ( ev.mx >= 0 && ev.my >= 0 && (ev.mx <= [wnd frame].size.width) && (ev.my <= fixedHeight) )
	{
		xwl_send_event( &ev );
	}
}

-(void) mouseMoved:(NSEvent *) theEvent
{
	dispatchMouseMoveEvent( theEvent );
	
	//NSLog(@"mouseMoved");
}

-(void) mouseDown:(NSEvent *) theEvent
{
	xwl_event_t ev = {0};
	ev.type = XWLE_MOUSEBUTTON_PRESSED;
	ev.button = XWLMB_LEFT;
	xwl_send_event( &ev );
}

-(void) mouseUp:(NSEvent *) theEvent
{
	xwl_event_t ev = {0};
	ev.type = XWLE_MOUSEBUTTON_RELEASED;
	ev.button = XWLMB_LEFT;
	xwl_send_event( &ev );
}

-(void) mouseDragged:(NSEvent *) theEvent
{
	dispatchMouseMoveEvent( theEvent );
	
	//NSLog( @"mouseDragged" );
}


-(void)rightMouseDown:(NSEvent *) event
{
	xwl_event_t ev = {0};
	ev.type = XWLE_MOUSEBUTTON_PRESSED;
	ev.button = XWLMB_RIGHT;
	xwl_send_event( &ev );
}

-(void) rightMouseUp:(NSEvent *) theEvent
{
	xwl_event_t ev = {0};
	ev.type = XWLE_MOUSEBUTTON_RELEASED;
	ev.button = XWLMB_RIGHT;
	xwl_send_event( &ev );
}

-(void) rightMouseDragged:(NSEvent *) theEvent
{
	dispatchMouseMoveEvent( theEvent );
	//NSLog( @"rightMouseDragged" );
}

-(void) otherMouseDown:(NSEvent *) theEvent
{
	xwl_event_t ev = {0};
	ev.type = XWLE_MOUSEBUTTON_PRESSED;
	ev.button = XWLMB_MIDDLE;
	xwl_send_event( &ev );
}

-(void) otherMouseUp:(NSEvent *) theEvent
{
	xwl_event_t ev = {0};
	ev.type = XWLE_MOUSEBUTTON_RELEASED;
	ev.button = XWLMB_MIDDLE;
	xwl_send_event( &ev );
}


-(void) otherMouseDragged:(NSEvent *) theEvent
{
	dispatchMouseMoveEvent( theEvent );
	//NSLog( @"otherMouseDragged" );
}

-(void) scrollWheel:(NSEvent *) theEvent
{
	xwl_event_t ev = {0};
	ev.type = XWLE_MOUSEWHEEL;
	// -1 is towards the user
	// 1 is away from the user	
	ev.wheelDelta = ([theEvent deltaY] > 0) ? 1 : -1;
	xwl_send_event( &ev );
	//NSLog( @"scrollWheel %g %g %g", [theEvent deltaX], [theEvent deltaY], [theEvent deltaZ] );
}

-(void) keyDown:(NSEvent *) event
{	
	xwl_event_t ev = {0};
	XWLWINDOW * wnd = (XWLWINDOW*)[event window];
	NSString *string = [event charactersIgnoringModifiers];

	// send text event with unicode
	if ( [[event characters] length] && ![event isARepeat] )
	{
		unichar code = [[event characters] characterAtIndex:0];
		
		// Codes from 0xF700 to 0xF8FF are non text keys (see NSEvent.h)
		// 0x35 is the Escape key	
		// not escape, or any other non-text keys
		if ( [event keyCode] != 0x35 && (code < 0xF700 || code > 0xF8FF))
		{
			
			NSString * str;
			NSText *text = [wnd fieldEditor: YES forObject: wnd];
			[text interpretKeyEvents:[NSArray arrayWithObject:event]];
			
			str = [text string];
			if ( [str length] > 0 )
			{
				// send text event
				ev.type = XWLE_TEXT;
				ev.unicode = [str characterAtIndex: 0];
				xwl_send_event( &ev );
				
				// we must reset this here, otherwise we don't get different character codes
				[text setString: @""];
			}
		}
	}
	else
	{
		//[super keyDown: event];
	}
	
	if ( ![event isARepeat] )
	{
		memset( &ev, 0, sizeof(xwl_event_t) );
		ev.type = XWLE_KEYPRESSED;
		
		if ([string length] > 0)
		{
			ev.key = LocalizedKeys([string characterAtIndex:0]);
		}
		
		// the key is not a localized one...
		if (ev.key == XWLK_INVALID)
		{
			ev.key = NonLocalizedKeys([event keyCode]);
		}
		
		if (ev.key == XWLK_INVALID)
		{
			NSLog( @"Unknown key code: %i", [event keyCode] );
		}
		
		
		// send the KeyPressed event
		xwl_send_event( &ev );
	}
	else
	{
		//[super keyDown: event];
	}
	
	
	/*
	 NSUInteger modifierFlags = [event modifierFlags];
	 if ( modifierFlags & NSAlternateKeyMask )
	 NSLog( @"alt" );
	 if ( modifierFlags & NSControlKeyMask)
	 NSLog( @"control" );
	 if ( modifierFlags & NSShiftKeyMask )
	 NSLog( @"shift" );
	 if ( modifierFlags & NSCommandKeyMask )
	 NSLog( @"system" );
	 */

	NSLog( @"MyOpenGLView keyDown" );
	//[super keyDown: event];
}

-(BOOL)resignFirstResponder
{
	return NO;
}

-(void) keyUp:(NSEvent *) event
{
#if 1
	xwl_event_t ev = {0};
	NSString *string = [event charactersIgnoringModifiers];
	
	if ( ![event isARepeat] )
	{
		memset( &ev, 0, sizeof(xwl_event_t) );
		ev.type = XWLE_KEYRELEASED;
		
		if ([string length] > 0)
		{
			ev.key = LocalizedKeys([string characterAtIndex:0]);
		}
		
		// the key is not a localized one...
		if (ev.key == XWLK_INVALID)
		{
			ev.key = NonLocalizedKeys([event keyCode]);
		}
		
		if (ev.key == XWLK_INVALID)
		{
			NSLog( @"Unknown key code: %i", [event keyCode] );
		}
		
		
		// send the KeyPressed event
		xwl_send_event( &ev );
	}
	else
	{
		//[super keyUp: event];
	}
#endif
	NSLog( @"MyOpenGLView keyUp" );
	[super keyUp: event];
}

-(void)flagsChanged:(NSEvent *)event
{
	// TODO: generate key events for these
	NSUInteger modifierFlags = [event modifierFlags];
	if ( modifierFlags & NSAlternateKeyMask )
		NSLog( @"alt" );
	if ( modifierFlags & NSControlKeyMask)
		NSLog( @"control" );
	if ( modifierFlags & NSShiftKeyMask )
		NSLog( @"shift" );
	if ( modifierFlags & NSCommandKeyMask )
		NSLog( @"system" );	
	
	NSLog( @"Flags Changed!" );
}

-(void)mouseEntered:(NSEvent *)theEvent
{
	NSLog( @"mouseEntered");
}

-(void)mouseExited:(NSEvent *)theEvent
{
	NSLog( @"mouseExited" );
}

@end


MyOpenGLView* setup_rendering( XWLWINDOW * handle )
{
	NSOpenGLContext * ctx;
	NSOpenGLPixelFormat * format;
	MyOpenGLView * view;
	
	// choose pixel format
	NSOpenGLPixelFormatAttribute attribs[] = { NSOpenGLPFAClosestPolicy, NSOpenGLPFADoubleBuffer, NSOpenGLPFAAccelerated, NSOpenGLPFAWindow, NSOpenGLPFAColorSize, 24, 0 };
	format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];
	
	if ( format == nil )
	{
		NSLog( @"Unable to create pixel format!" );
		return 0;
	}
	
	// create opengl context
	ctx = [[NSOpenGLContext alloc] initWithFormat: format shareContext: NO];
	if ( ctx == nil )
	{
		NSLog( @"Unable to create opengl context!" );
		return 0;
	}
	[ctx retain];
	
	// create the custom view
	view = [[MyOpenGLView alloc] initWithFrame: [[handle contentView] frame] ];
	[view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	
	// set the context of that view
	[view setContext: ctx ];
	
	// context is now 'current', so glGetString calls succeed.
	[ctx makeCurrentContext];
	
	// make sure it sends the windowResized event
	[[NSNotificationCenter defaultCenter] addObserver: view 
											 selector:@selector(windowResized:) name:NSWindowDidResizeNotification 
											   object: handle];	
	
	// make the view this window's first responder and add it as a subview
	[handle makeFirstResponder: view];
	[[handle contentView] addSubview: view];
	
	return view;
}

void xwl_setup_osx_rendering( xwl_window_t * window )
{
	MyOpenGLView * view;
	view = setup_rendering( window->handle );
	
	if ( view )
	{
		((xwlWindow*)window->handle).render = view;
		window->view = view;
	}
}

void xwl_osx_finish( xwl_window_t * window )
{
	if ( !window || !window->view )
		return;

	[[((MyOpenGLView*)window->view) getContext] flushBuffer];
}



xwl_window_handle_t *xwl_create_osx_window( xwl_windowparams_t * params, const char * title )
{
	NSRect frame;
	NSPoint origin;
	i32 windowMask;
	xwl_window_handle_t *wh = 0;
	XWLWINDOW * handle;

	// full screen
	
	if ( params->flags & XWL_FULLSCREEN )
	{
		windowMask = NSBorderlessWindowMask;
	}
	else
	{
		windowMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
	}

	if ( params->flags & XWL_NORESIZE )
	{
		// remove this flag
		windowMask &= ~NSResizableWindowMask;
	}
	
	
	frame = NSMakeRect( 0, 0, params->width, params->height );
	
	handle = [[XWLWINDOW alloc] initWithContentRect: frame styleMask: windowMask backing: NSBackingStoreBuffered defer: NO];
	[handle autorelease];
	
	// uncomment the next line to see a red color when the window is initially created
	//[handle setBackgroundColor: [NSColor redColor]];
	[handle setBackgroundColor: [NSColor blackColor]];
	[handle makeKeyAndOrderFront: nil];
	[handle makeMainWindow];
	[handle makeKeyWindow];
	[handle orderFront: nil];
	
	[handle setTitle: [NSString stringWithUTF8String: title] ];
	[handle setAcceptsMouseMovedEvents: YES];
	[handle setReleasedWhenClosed: NO];

	[handle setDelegate: appDelegate ];

	origin = NSMakePoint( 0, 0 );
	
	[handle center];
	[handle setFrameOrigin: origin];
	
	
	// full screen
	if ( params->flags & XWL_FULLSCREEN )
	{
		[handle setLevel: CGShieldingWindowLevel() ];
	}
		
	if ( handle )
	{
		wh = xwl_get_unused_window();
		wh->handle.handle = handle;
	}

	
	return wh;
}


@implementation xwlWindow
@synthesize xwlhandle;
@synthesize render;

/* this is required when using the styleMask: NSBorderlessWindowMask */
/* By default NSBorderlessWindowMask windows report that they cannot become the key (target of keyboard input) */
-(BOOL) canBecomeKeyWindow
{
	return YES;
}

/* By default NSBorderlessWindowMask windows report that they cannot become the main window without this override */
-(BOOL) canBecomeMainWindow
{
	return YES;
}
@end
