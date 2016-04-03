
// this code should work if compiled as C99+ or Objective-C (with or without ARC)

// we don't need much here
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>
#include <OpenGL/gl.h>

// maybe this is available somewhere in objc runtime?
#if __LP64__ || (TARGET_OS_EMBEDDED && !TARGET_OS_IPHONE) || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
#define NSIntegerEncoding "q"
#define NSUIntegerEncoding "L"
#else
#define NSIntegerEncoding "i"
#define NSUIntegerEncoding "I"
#endif

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
// this is how they are defined originally
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>
typedef CGPoint NSPoint;
typedef CGRect NSRect;

extern id NSApp;
extern id const NSDefaultRunLoopMode;
#endif

#if defined(__OBJC__) && __has_feature(objc_arc)
#define ARC_AVAILABLE
#endif

// ABI is a bit different between platforms
#ifdef __arm64__
#define abi_objc_msgSend_stret objc_msgSend
#else
#define abi_objc_msgSend_stret objc_msgSend_stret
#endif
#ifdef __i386__
#define abi_objc_msgSend_fpret objc_msgSend_fpret
#else
#define abi_objc_msgSend_fpret objc_msgSend
#endif

bool terminated = false;
uint32_t windowCount = 0;

// we gonna construct objective-c class by hand in runtime, so wow, so hacker!
//@interface AppDelegate : NSObject<NSApplicationDelegate>
//-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
//@end
//@implementation AppDelegate
//-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
//{
//	terminated = true;
//	return NSTerminateCancel;
//}
//@end
NSUInteger applicationShouldTerminate(id self, SEL _sel, id sender)
{
	printf("requested to terminate\n");
	terminated = true;
	return 0;
}

//@interface WindowDelegate : NSObject<NSWindowDelegate>
//-(void)windowWillClose:(NSNotification*)notification;
//@end
//@implementation WindowDelegate
//-(void)windowWillClose:(NSNotification*)notification
//{
//	(void)notification;
//	assert(windowCount);
//	if(--windowCount == 0)
//		terminated = true;
//}
//@end
void windowWillClose(id self, SEL _sel, id notification)
{
	printf("window will close\n");
	assert(windowCount);
	if(--windowCount == 0)
		terminated = true;
}

int main()
{
	#ifdef ARC_AVAILABLE
	@autoreleasepool
	{
	#else
	//NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	//would be nice to use objc_autoreleasePoolPush instead, but it's not publically available in the headers
	id poolAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSAutoreleasePool"), sel_registerName("alloc"));
	id pool = ((id (*)(id, SEL))objc_msgSend)(poolAlloc, sel_registerName("init"));
	#endif
	
	//[NSApplication sharedApplication];
	((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
	
	//[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	((void (*)(id, SEL, NSInteger))objc_msgSend)(NSApp, sel_registerName("setActivationPolicy:"), 0);
	
	//AppDelegate * dg = [[AppDelegate alloc] init];
	Class appDelegateClass = objc_allocateClassPair((Class)objc_getClass("NSObject"), "AppDelegate", 0);
	bool resultAddProtoc = class_addProtocol(appDelegateClass, objc_getProtocol("NSApplicationDelegate"));
	assert(resultAddProtoc);
	bool resultAddMethod = class_addMethod(appDelegateClass, sel_registerName("applicationShouldTerminate:"), (IMP)applicationShouldTerminate, NSUIntegerEncoding "@:@");
	assert(resultAddMethod);
	id dgAlloc = ((id (*)(id, SEL))objc_msgSend)((id)appDelegateClass, sel_registerName("alloc"));
	id dg = ((id (*)(id, SEL))objc_msgSend)(dgAlloc, sel_registerName("init"));
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(dg, sel_registerName("autorelease"));
	#endif
	
	//[NSApp setDelegate:dg];
	((void (*)(id, SEL, id))objc_msgSend)(NSApp, sel_registerName("setDelegate:"), dg);
	
	// only needed if we don't use [NSApp run]
	//[NSApp finishLaunching];
	((void (*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("finishLaunching"));
	
	//id menubar = [[NSMenu alloc] init];
	id menubarAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenu"), sel_registerName("alloc"));
	id menubar = ((id (*)(id, SEL))objc_msgSend)(menubarAlloc, sel_registerName("init"));
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(menubar, sel_registerName("autorelease"));
	#endif

	//id appMenuItem = [[NSMenuItem alloc] init];
	id appMenuItemAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_registerName("alloc"));
	id appMenuItem = ((id (*)(id, SEL))objc_msgSend)(appMenuItemAlloc, sel_registerName("init"));
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(appMenuItem, sel_registerName("autorelease"));
	#endif

	//[menubar addItem:appMenuItem];
	((void (*)(id, SEL, id))objc_msgSend)(menubar, sel_registerName("addItem:"), appMenuItem);
	
	//[NSApp setMainMenu:menubar];
	((id (*)(id, SEL, id))objc_msgSend)(NSApp, sel_registerName("setMainMenu:"), menubar);
	
	//id appMenu = [[NSMenu alloc] init];
	id appMenuAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenu"), sel_registerName("alloc"));
	id appMenu = ((id (*)(id, SEL))objc_msgSend)(appMenuAlloc, sel_registerName("init"));
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(appMenu, sel_registerName("autorelease"));
	#endif
	
	//id appName = [[NSProcessInfo processInfo] processName];
	id processInfo = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSProcessInfo"), sel_registerName("processInfo"));
	id appName = ((id (*)(id, SEL))objc_msgSend)(processInfo, sel_registerName("processName"));
	
	//id quitTitle = [@"Quit " stringByAppendingString:appName];
	id quitTitlePrefixString = ((id (*)(id, SEL, const char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "Quit ");
	id quitTitle = ((id (*)(id, SEL, id))objc_msgSend)(quitTitlePrefixString, sel_registerName("stringByAppendingString:"), appName);
	
	//id quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
	id quitMenuItemKey = ((id (*)(id, SEL, const char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "q");
	id quitMenuItemAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_registerName("alloc"));
	id quitMenuItem = ((id (*)(id, SEL, id, SEL, id))objc_msgSend)(quitMenuItemAlloc, sel_registerName("initWithTitle:action:keyEquivalent:"), quitTitle, sel_registerName("terminate:"), quitMenuItemKey);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(quitMenuItem, sel_registerName("autorelease"));
	#endif

	//[appMenu addItem:quitMenuItem];
	((void (*)(id, SEL, id))objc_msgSend)(appMenu, sel_registerName("addItem:"), quitMenuItem);
	
	//[appMenuItem setSubmenu:appMenu];
	((void (*)(id, SEL, id))objc_msgSend)(appMenuItem, sel_registerName("setSubmenu:"), appMenu);
	
	//id window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 500, 500) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask backing:NSBackingStoreBuffered defer:NO];
	NSRect rect = {{0, 0}, {500, 500}};
	id windowAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSWindow"), sel_registerName("alloc"));
	id window = ((id (*)(id, SEL, NSRect, NSUInteger, NSUInteger, BOOL))objc_msgSend)(windowAlloc, sel_registerName("initWithContentRect:styleMask:backing:defer:"), rect, 15, 2, NO);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(window, sel_registerName("autorelease"));
	#endif
	
	// when we are not using ARC, than window will be added to autorelease pool
	// so if we close it by hand (pressing red button), we don't want it to be released for us
	// so it will be released by autorelease pool later
	//[window setReleasedWhenClosed:NO];
	((void (*)(id, SEL, BOOL))objc_msgSend)(window, sel_registerName("setReleasedWhenClosed:"), NO);

	windowCount = 1;
	
	//WindowDelegate * wdg = [[WindowDelegate alloc] init];
	Class WindowDelegateClass = objc_allocateClassPair((Class)objc_getClass("NSObject"), "WindowDelegate", 0);
	resultAddProtoc = class_addProtocol(WindowDelegateClass, objc_getProtocol("NSWindowDelegate"));
	assert(resultAddProtoc);
	resultAddMethod = class_addMethod(WindowDelegateClass, sel_registerName("windowWillClose:"), (IMP)windowWillClose,  "v@:@");
	assert(resultAddMethod);
	id wdgAlloc = ((id (*)(id, SEL))objc_msgSend)((id)WindowDelegateClass, sel_registerName("alloc"));
	id wdg = ((id (*)(id, SEL))objc_msgSend)(wdgAlloc, sel_registerName("init"));
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(wdg, sel_registerName("autorelease"));
	#endif

	//[window setDelegate:wdg];
	((void (*)(id, SEL, id))objc_msgSend)(window, sel_registerName("setDelegate:"), wdg);
	
	//NSView * contentView = [window contentView];
	id contentView = ((id (*)(id, SEL))objc_msgSend)(window, sel_registerName("contentView"));

	// disable this if you don't want retina support
	//[contentView setWantsBestResolutionOpenGLSurface:YES];
	((void (*)(id, SEL, BOOL))objc_msgSend)(contentView, sel_registerName("setWantsBestResolutionOpenGLSurface:"), YES);
	
	//[window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
	NSPoint point = {20, 20};
	((void (*)(id, SEL, NSPoint))objc_msgSend)(window, sel_registerName("cascadeTopLeftFromPoint:"), point);
	
	//[window setTitle:@"sup"];
	id titleString = ((id (*)(id, SEL, const char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "sup from C");
	((void (*)(id, SEL, id))objc_msgSend)(window, sel_registerName("setTitle:"), titleString);
	
	//NSOpenGLPixelFormatAttribute glAttributes[] =
	//{
	//	NSOpenGLPFAColorSize, 24,
	//	NSOpenGLPFAAlphaSize, 8,
	//	NSOpenGLPFADoubleBuffer,
	//	NSOpenGLPFAAccelerated,
	//	NSOpenGLPFANoRecovery,
	//	NSOpenGLPFASampleBuffers, 1,
	//	NSOpenGLPFASamples, 4,
	//	NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy, // or NSOpenGLProfileVersion3_2Core
	//	0
	//};
	uint32_t glAttributes[] =
	{
		8, 24,
		11, 8,
		5,
		73,
		72,
		55, 1,
		56, 4,
		99, 0x1000, // or 0x3200
		0
	};
	
	//NSOpenGLPixelFormat * pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttributes];
	id pixelFormatAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSOpenGLPixelFormat"), sel_registerName("alloc"));
	id pixelFormat = ((id (*)(id, SEL, const uint32_t*))objc_msgSend)(pixelFormatAlloc, sel_registerName("initWithAttributes:"), glAttributes);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(pixelFormat, sel_registerName("autorelease"));
	#endif

	//NSOpenGLContext * openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	id openGLContextAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSOpenGLContext"), sel_registerName("alloc"));
	id openGLContext = ((id (*)(id, SEL, id, id))objc_msgSend)(openGLContextAlloc, sel_registerName("initWithFormat:shareContext:"), pixelFormat, nil);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(openGLContext, sel_registerName("autorelease"));
	#endif
	
	//[openGLContext setView:contentView];
	((void (*)(id, SEL, id))objc_msgSend)(openGLContext, sel_registerName("setView:"), contentView);
	
	//[window makeKeyAndOrderFront:window];
	((void (*)(id, SEL, id))objc_msgSend)(window, sel_registerName("makeKeyAndOrderFront:"), window);
	
	//[window setAcceptsMouseMovedEvents:YES];
	((void (*)(id, SEL, BOOL))objc_msgSend)(window, sel_registerName("setAcceptsMouseMovedEvents:"), YES);
	
	//[window setBackgroundColor:[NSColor blackColor]];
	id blackColor = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSColor"), sel_registerName("blackColor"));
	((void (*)(id, SEL, id))objc_msgSend)(window, sel_registerName("setBackgroundColor:"), blackColor);
	
	// TODO do we really need this?
	//[NSApp activateIgnoringOtherApps:YES];
	((void (*)(id, SEL, BOOL))objc_msgSend)(NSApp, sel_registerName("activateIgnoringOtherApps:"), YES);
	
	// explicit runloop
	printf("entering runloop\n");
	while(!terminated)
	{
		//NSEvent * event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
		id distantPast = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"), sel_registerName("distantPast"));
		id event = ((id (*)(id, SEL, NSUInteger, id, id, BOOL))objc_msgSend)(NSApp, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"), NSUIntegerMax, distantPast, NSDefaultRunLoopMode, YES);
		
		if(event)
		{
			//NSEventType eventType = [event type];
			NSUInteger eventType = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, sel_registerName("type"));
			
			switch(eventType)
			{
			//case NSMouseMoved:
			//case NSLeftMouseDragged:
			//case NSRightMouseDragged:
			//case NSOtherMouseDragged:
			case 5:
			case 6:
			case 7:
			case 27:
			{
				//NSWindow * currentWindow = [NSApp keyWindow];
				id currentWindow = ((id (*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("keyWindow"));

				//NSRect adjustFrame = [[currentWindow contentView] frame];
				id currentWindowContentView = ((id (*)(id, SEL))objc_msgSend)(currentWindow, sel_registerName("contentView"));
				NSRect adjustFrame = ((NSRect (*)(id, SEL))abi_objc_msgSend_stret)(currentWindowContentView, sel_registerName("frame"));
				
				//NSPoint p = [currentWindow mouseLocationOutsideOfEventStream];
				// NSPoint is small enough to fit a register, so no need for objc_msgSend_stret
				NSPoint p = ((NSPoint (*)(id, SEL))objc_msgSend)(currentWindow, sel_registerName("mouseLocationOutsideOfEventStream"));
				
				// map input to content view rect
				if(p.x < 0) p.x = 0;
				else if(p.x > adjustFrame.size.width) p.x = adjustFrame.size.width;
				if(p.y < 0) p.y = 0;
				else if(p.y > adjustFrame.size.height) p.y = adjustFrame.size.height;

				// map input to pixels
				NSRect r = {p.x, p.y, 0, 0};
				//r = [currentWindowContentView convertRectToBacking:r];
				r = ((NSRect (*)(id, SEL, NSRect))abi_objc_msgSend_stret)(currentWindowContentView, sel_registerName("convertRectToBacking:"), r);
				p = r.origin;
				
				printf("mouse moved to %f %f\n", p.x, p.y);
				break;
			}
			//case NSLeftMouseDown:
			case 1:
				printf("mouse left key down\n");
				break;
			//case NSLeftMouseUp:
			case 2:
				printf("mouse left key up\n");
				break;
			//case NSRightMouseDown:
			case 3:
				printf("mouse right key down\n");
				break;
			//case NSRightMouseUp:
			case 4:
				printf("mouse right key up\n");
				break;
			//case NSOtherMouseDown:
			case 25:
			{
				// number == 2 is a middle button

				//NSInteger number = [event buttonNumber];
				NSInteger number = ((NSInteger (*)(id, SEL))objc_msgSend)(event, sel_registerName("buttonNumber"));
				printf("mouse other key down : %i\n", (int)number);
				break;
			}
			//case NSOtherMouseUp:
			case 26:
			{
				//NSInteger number = [event buttonNumber];
				NSInteger number = ((NSInteger (*)(id, SEL))objc_msgSend)(event, sel_registerName("buttonNumber"));
				printf("mouse other key up : %i\n", (int)number);
				break;
			}
			//case NSScrollWheel:
			case 22:
			{
				//CGFloat deltaX = [event scrollingDeltaX];
				CGFloat deltaX = ((CGFloat (*)(id, SEL))abi_objc_msgSend_fpret)(event, sel_registerName("scrollingDeltaX"));

				//CGFloat deltaY = [event scrollingDeltaY];
				CGFloat deltaY = ((CGFloat (*)(id, SEL))abi_objc_msgSend_fpret)(event, sel_registerName("scrollingDeltaY"));
				
				//BOOL precisionScrolling = [event hasPreciseScrollingDeltas];
				BOOL precisionScrolling = ((BOOL (*)(id, SEL))objc_msgSend)(event, sel_registerName("hasPreciseScrollingDeltas"));

				if(precisionScrolling)
				{
					deltaX *= 0.1f; // similar to glfw
					deltaY *= 0.1f;
				}

				if(fabs(deltaX) > 0.0f || fabs(deltaY) > 0.0f)
					printf("mouse scroll wheel delta %f %f\n", deltaX, deltaY);
				break;
			}
			//case NSFlagsChanged:
			case 12:
			{
				//NSEventModifierFlags modifiers = [event modifierFlags];
				NSUInteger modifiers = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, sel_registerName("modifierFlags"));
				
				// based on NSEventModifierFlags
				struct
				{
					union
					{
						struct
						{
							uint8_t alpha_shift:1;
							uint8_t shift:1;
							uint8_t control:1;
							uint8_t alternate:1;
							uint8_t command:1;
							uint8_t numeric_pad:1;
							uint8_t help:1;
							uint8_t function:1;
						};
						uint8_t mask;
					};
				} keys;
				
				//keys.mask = (modifiers & NSDeviceIndependentModifierFlagsMask) >> 16;
				keys.mask = (modifiers & 0xffff0000UL) >> 16;
				
				printf("mod keys : mask %03u state %u%u%u%u%u%u%u%u\n", keys.mask, keys.alpha_shift, keys.shift, keys.control, keys.alternate, keys.command, keys.numeric_pad, keys.help, keys.function);
				break;
			}
			//case NSKeyDown:
			case 10:
			{
				//NSString * inputText = [event characters];
				id inputText = ((id (*)(id, SEL))objc_msgSend)(event, sel_registerName("characters"));
				
				//const char * inputTextUTF8 = [inputText UTF8String];
				const char * inputTextUTF8 = ((const char* (*)(id, SEL))objc_msgSend)(inputText, sel_registerName("UTF8String"));
				
				//you can get list of virtual key codes from Carbon HIToolbox/Events.h
				//uint16_t keyCode = [event keyCode];
				uint16_t keyCode = ((unsigned short (*)(id, SEL))objc_msgSend)(event, sel_registerName("keyCode"));

				printf("key down %u, text '%s'\n", keyCode, inputTextUTF8);
				break;
			}
			//case NSKeyUp:
			case 11:
			{
				//uint16_t keyCode = [event keyCode];
				uint16_t keyCode = ((unsigned short (*)(id, SEL))objc_msgSend)(event, sel_registerName("keyCode"));
				
				printf("key up %u\n", keyCode);
				break;
			}
			default:
				break;
			}
			
			//[NSApp sendEvent:event];
			((void (*)(id, SEL, id))objc_msgSend)(NSApp, sel_registerName("sendEvent:"), event);
			
			// if user closes the window we might need to terminate asap
			if(terminated)
				break;
			
			//[NSApp updateWindows];
			((void (*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
		}
		
		// do runloop stuff
		//[openGLContext update]; // probably we only need to do it when we resize the window
		((void (*)(id, SEL))objc_msgSend)(openGLContext, sel_registerName("update"));
		
		//[openGLContext makeCurrentContext];
		((void (*)(id, SEL))objc_msgSend)(openGLContext, sel_registerName("makeCurrentContext"));
		
		//NSRect rect = [contentView frame];
		NSRect rect = ((NSRect (*)(id, SEL))abi_objc_msgSend_stret)(contentView, sel_registerName("frame"));
		
		//rect = [contentView convertRectToBacking:rect];
		rect = ((NSRect (*)(id, SEL, NSRect))abi_objc_msgSend_stret)(contentView, sel_registerName("convertRectToBacking:"), rect);
		
		glViewport(0, 0, rect.size.width, rect.size.height);
		
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(1.0f, 0.85f, 0.35f);
		glBegin(GL_TRIANGLES);
		{
			glVertex3f(  0.0,  0.6, 0.0);
			glVertex3f( -0.2, -0.3, 0.0);
			glVertex3f(  0.2, -0.3 ,0.0);
		}
		glEnd();
		
		//[openGLContext flushBuffer];
		((void (*)(id, SEL))objc_msgSend)(openGLContext, sel_registerName("flushBuffer"));
	}
		
	printf("gracefully terminated\n");
	
	#ifdef ARC_AVAILABLE
	}
	#else
	//[pool drain];
	((void (*)(id, SEL))objc_msgSend)(pool, sel_registerName("drain"));
	#endif
	return 0;
}
