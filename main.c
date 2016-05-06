
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
	SEL allocSel = sel_registerName("alloc");
	SEL initSel = sel_registerName("init");

	#ifdef ARC_AVAILABLE
	@autoreleasepool
	{
	#else
	//NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	//would be nice to use objc_autoreleasePoolPush instead, but it's not publically available in the headers
	Class NSAutoreleasePoolClass = objc_getClass("NSAutoreleasePool");
	id poolAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSAutoreleasePoolClass, allocSel);
	id pool = ((id (*)(id, SEL))objc_msgSend)(poolAlloc, initSel);
	#endif

	//[NSApplication sharedApplication];
	Class NSApplicationClass = objc_getClass("NSApplication");
	SEL sharedApplicationSel = sel_registerName("sharedApplication");
	((id (*)(Class, SEL))objc_msgSend)(NSApplicationClass, sharedApplicationSel);

	//[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	SEL setActivationPolicySel = sel_registerName("setActivationPolicy:");
	((void (*)(id, SEL, NSInteger))objc_msgSend)(NSApp, setActivationPolicySel, 0);

	//AppDelegate * dg = [[AppDelegate alloc] init];
	Class NSObjectClass = objc_getClass("NSObject");
	Class AppDelegateClass = objc_allocateClassPair(NSObjectClass, "AppDelegate", 0);
	Protocol* NSApplicationDelegateProtocol = objc_getProtocol("NSApplicationDelegate");
	bool resultAddProtoc = class_addProtocol(AppDelegateClass, NSApplicationDelegateProtocol);
	assert(resultAddProtoc);
	SEL applicationShouldTerminateSel = sel_registerName("applicationShouldTerminate:");
	bool resultAddMethod = class_addMethod(AppDelegateClass, applicationShouldTerminateSel, (IMP)applicationShouldTerminate, NSUIntegerEncoding "@:@");
	assert(resultAddMethod);
	id dgAlloc = ((id (*)(Class, SEL))objc_msgSend)(AppDelegateClass, allocSel);
	id dg = ((id (*)(id, SEL))objc_msgSend)(dgAlloc, initSel);

	SEL autoreleaseSel = sel_registerName("autorelease");
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(dg, autoreleaseSel);
	#endif

	//[NSApp setDelegate:dg];
	SEL setDelegateSel = sel_registerName("setDelegate:");
	((void (*)(id, SEL, id))objc_msgSend)(NSApp, setDelegateSel, dg);

	// only needed if we don't use [NSApp run]
	//[NSApp finishLaunching];
	SEL finishLaunchingSel = sel_registerName("finishLaunching");
	((void (*)(id, SEL))objc_msgSend)(NSApp, finishLaunchingSel);

	//id menubar = [[NSMenu alloc] init];
	Class NSMenuClass = objc_getClass("NSMenu");
	id menubarAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSMenuClass, allocSel);
	id menubar = ((id (*)(id, SEL))objc_msgSend)(menubarAlloc, initSel);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(menubar, autoreleaseSel);
	#endif

	//id appMenuItem = [[NSMenuItem alloc] init];
	Class NSMenuItemClass = objc_getClass("NSMenuItem");
	id appMenuItemAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSMenuItemClass, allocSel);
	id appMenuItem = ((id (*)(id, SEL))objc_msgSend)(appMenuItemAlloc, initSel);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(appMenuItem, autoreleaseSel);
	#endif

	//[menubar addItem:appMenuItem];
	SEL addItemSel = sel_registerName("addItem:");
	((void (*)(id, SEL, id))objc_msgSend)(menubar, addItemSel, appMenuItem);

	//[NSApp setMainMenu:menubar];
	SEL setMainMenuSel = sel_registerName("setMainMenu:");
	((id (*)(id, SEL, id))objc_msgSend)(NSApp, setMainMenuSel, menubar);

	//id appMenu = [[NSMenu alloc] init];
	id appMenuAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSMenuClass, allocSel);
	id appMenu = ((id (*)(id, SEL))objc_msgSend)(appMenuAlloc, initSel);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(appMenu, autoreleaseSel);
	#endif

	//id appName = [[NSProcessInfo processInfo] processName];
	Class NSProcessInfoClass = objc_getClass("NSProcessInfo");
	SEL processInfoSel = sel_registerName("processInfo");
	id processInfo = ((id (*)(Class, SEL))objc_msgSend)(NSProcessInfoClass, processInfoSel);
	SEL processNameSel = sel_registerName("processName");
	id appName = ((id (*)(id, SEL))objc_msgSend)(processInfo, processNameSel);

	//id quitTitle = [@"Quit " stringByAppendingString:appName];
	Class NSStringClass = objc_getClass("NSString");
	SEL stringWithUTF8StringSel = sel_registerName("stringWithUTF8String:");
	id quitTitlePrefixString = ((id (*)(Class, SEL, const char*))objc_msgSend)(NSStringClass, stringWithUTF8StringSel, "Quit ");
	SEL stringByAppendingStringSel = sel_registerName("stringByAppendingString:");
	id quitTitle = ((id (*)(id, SEL, id))objc_msgSend)(quitTitlePrefixString, stringByAppendingStringSel, appName);

	//id quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
	id quitMenuItemKey = ((id (*)(Class, SEL, const char*))objc_msgSend)(NSStringClass, stringWithUTF8StringSel, "q");
	id quitMenuItemAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSMenuItemClass, allocSel);
	SEL initWithTitleSel = sel_registerName("initWithTitle:action:keyEquivalent:");
	SEL terminateSel = sel_registerName("terminate:");
	id quitMenuItem = ((id (*)(id, SEL, id, SEL, id))objc_msgSend)(quitMenuItemAlloc, initWithTitleSel, quitTitle, terminateSel, quitMenuItemKey);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(quitMenuItem, autoreleaseSel);
	#endif

	//[appMenu addItem:quitMenuItem];
	((void (*)(id, SEL, id))objc_msgSend)(appMenu, addItemSel, quitMenuItem);

	//[appMenuItem setSubmenu:appMenu];
	SEL setSubmenuSel = sel_registerName("setSubmenu:");
	((void (*)(id, SEL, id))objc_msgSend)(appMenuItem, setSubmenuSel, appMenu);

	//id window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 500, 500) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask backing:NSBackingStoreBuffered defer:NO];
	NSRect rect = {{0, 0}, {500, 500}};
	Class NSWindowClass = objc_getClass("NSWindow");
	id windowAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSWindowClass, allocSel);
	SEL initWithContentRectSel = sel_registerName("initWithContentRect:styleMask:backing:defer:");
	id window = ((id (*)(id, SEL, NSRect, NSUInteger, NSUInteger, BOOL))objc_msgSend)(windowAlloc, initWithContentRectSel, rect, 15, 2, NO);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(window, autoreleaseSel);
	#endif

	// when we are not using ARC, than window will be added to autorelease pool
	// so if we close it by hand (pressing red button), we don't want it to be released for us
	// so it will be released by autorelease pool later
	//[window setReleasedWhenClosed:NO];
	SEL setReleasedWhenClosedSel = sel_registerName("setReleasedWhenClosed:");
	((void (*)(id, SEL, BOOL))objc_msgSend)(window, setReleasedWhenClosedSel, NO);

	windowCount = 1;

	//WindowDelegate * wdg = [[WindowDelegate alloc] init];
	Class WindowDelegateClass = objc_allocateClassPair(NSObjectClass, "WindowDelegate", 0);
	Protocol* NSWindowDelegateProtocol = objc_getProtocol("NSWindowDelegate");
	resultAddProtoc = class_addProtocol(WindowDelegateClass, NSWindowDelegateProtocol);
	assert(resultAddProtoc);
	SEL windowWillCloseSel = sel_registerName("windowWillClose:");
	resultAddMethod = class_addMethod(WindowDelegateClass, windowWillCloseSel, (IMP)windowWillClose,  "v@:@");
	assert(resultAddMethod);
	id wdgAlloc = ((id (*)(Class, SEL))objc_msgSend)(WindowDelegateClass, allocSel);
	id wdg = ((id (*)(id, SEL))objc_msgSend)(wdgAlloc, initSel);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(wdg, autoreleaseSel);
	#endif

	//[window setDelegate:wdg];
	((void (*)(id, SEL, id))objc_msgSend)(window, setDelegateSel, wdg);

	//NSView * contentView = [window contentView];
	SEL contentViewSel = sel_registerName("contentView");
	id contentView = ((id (*)(id, SEL))objc_msgSend)(window, contentViewSel);

	// disable this if you don't want retina support
	//[contentView setWantsBestResolutionOpenGLSurface:YES];
	SEL setWantsBestResolutionOpenGLSurfaceSel = sel_registerName("setWantsBestResolutionOpenGLSurface:");
	((void (*)(id, SEL, BOOL))objc_msgSend)(contentView, setWantsBestResolutionOpenGLSurfaceSel, YES);

	//[window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
	NSPoint point = {20, 20};
	SEL cascadeTopLeftFromPointSel = sel_registerName("cascadeTopLeftFromPoint:");
	((void (*)(id, SEL, NSPoint))objc_msgSend)(window, cascadeTopLeftFromPointSel, point);

	//[window setTitle:@"sup"];
	id titleString = ((id (*)(Class, SEL, const char*))objc_msgSend)(NSStringClass, stringWithUTF8StringSel, "sup from C");
	SEL setTitleSel = sel_registerName("setTitle:");
	((void (*)(id, SEL, id))objc_msgSend)(window, setTitleSel, titleString);

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
	Class NSOpenGLPixelFormatClass = objc_getClass("NSOpenGLPixelFormat");
	id pixelFormatAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSOpenGLPixelFormatClass, allocSel);
	SEL initWithAttributesSel = sel_registerName("initWithAttributes:");
	id pixelFormat = ((id (*)(id, SEL, const uint32_t*))objc_msgSend)(pixelFormatAlloc, initWithAttributesSel, glAttributes);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(pixelFormat, autoreleaseSel);
	#endif

	//NSOpenGLContext * openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	Class NSOpenGLContextClass = objc_getClass("NSOpenGLContext");
	id openGLContextAlloc = ((id (*)(Class, SEL))objc_msgSend)(NSOpenGLContextClass, allocSel);
	SEL initWithFormatSel = sel_registerName("initWithFormat:shareContext:");
	id openGLContext = ((id (*)(id, SEL, id, id))objc_msgSend)(openGLContextAlloc, initWithFormatSel, pixelFormat, nil);
	#ifndef ARC_AVAILABLE
	((void (*)(id, SEL))objc_msgSend)(openGLContext, autoreleaseSel);
	#endif

	//[openGLContext setView:contentView];
	SEL setViewSel = sel_registerName("setView:");
	((void (*)(id, SEL, id))objc_msgSend)(openGLContext, setViewSel, contentView);

	//[window makeKeyAndOrderFront:window];
	SEL makeKeyAndOrderFrontSel = sel_registerName("makeKeyAndOrderFront:");
	((void (*)(id, SEL, id))objc_msgSend)(window, makeKeyAndOrderFrontSel, window);

	//[window setAcceptsMouseMovedEvents:YES];
	SEL setAcceptsMouseMovedEventsSel = sel_registerName("setAcceptsMouseMovedEvents:");
	((void (*)(id, SEL, BOOL))objc_msgSend)(window, setAcceptsMouseMovedEventsSel, YES);

	//[window setBackgroundColor:[NSColor blackColor]];
	Class NSColorClass = objc_getClass("NSColor");
	id blackColor = ((id (*)(Class, SEL))objc_msgSend)(NSColorClass, sel_registerName("blackColor"));
	SEL setBackgroundColorSel = sel_registerName("setBackgroundColor:");
	((void (*)(id, SEL, id))objc_msgSend)(window, setBackgroundColorSel, blackColor);

	// TODO do we really need this?
	//[NSApp activateIgnoringOtherApps:YES];
	SEL activateIgnoringOtherAppsSel = sel_registerName("activateIgnoringOtherApps:");
	((void (*)(id, SEL, BOOL))objc_msgSend)(NSApp, activateIgnoringOtherAppsSel, YES);

	// explicit runloop
	printf("entering runloop\n");

	Class NSDateClass = objc_getClass("NSDate");
	SEL distantPastSel = sel_registerName("distantPast");
	SEL nextEventMatchingMaskSel = sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:");
	SEL frameSel = sel_registerName("frame");
	SEL typeSel = sel_registerName("type");
	SEL buttonNumberSel = sel_registerName("buttonNumber");
	SEL keyCodeSel = sel_registerName("keyCode");
	SEL keyWindowSel = sel_registerName("keyWindow");
	SEL mouseLocationOutsideOfEventStreamSel = sel_registerName("mouseLocationOutsideOfEventStream");
	SEL convertRectToBackingSel = sel_registerName("convertRectToBacking:");
	SEL scrollingDeltaXSel = sel_registerName("scrollingDeltaX");
	SEL scrollingDeltaYSel = sel_registerName("scrollingDeltaY");
	SEL hasPreciseScrollingDeltasSel = sel_registerName("hasPreciseScrollingDeltas");
	SEL modifierFlagsSel = sel_registerName("modifierFlags");
	SEL charactersSel = sel_registerName("characters");
	SEL UTF8StringSel = sel_registerName("UTF8String");
	SEL sendEventSel = sel_registerName("sendEvent:");
	SEL updateWindowsSel = sel_registerName("updateWindows");
	SEL updateSel = sel_registerName("update");
	SEL makeCurrentContextSel = sel_registerName("makeCurrentContext");
	SEL flushBufferSel = sel_registerName("flushBuffer");

	while(!terminated)
	{
		//NSEvent * event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
		id distantPast = ((id (*)(Class, SEL))objc_msgSend)(NSDateClass, distantPastSel);
		id event = ((id (*)(id, SEL, NSUInteger, id, id, BOOL))objc_msgSend)(NSApp, nextEventMatchingMaskSel, NSUIntegerMax, distantPast, NSDefaultRunLoopMode, YES);

		if(event)
		{
			//NSEventType eventType = [event type];
			NSUInteger eventType = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, typeSel);

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
					id currentWindow = ((id (*)(id, SEL))objc_msgSend)(NSApp, keyWindowSel);

					//NSRect adjustFrame = [[currentWindow contentView] frame];
					id currentWindowContentView = ((id (*)(id, SEL))objc_msgSend)(currentWindow, contentViewSel);
					NSRect adjustFrame = ((NSRect (*)(id, SEL))abi_objc_msgSend_stret)(currentWindowContentView, frameSel);

					//NSPoint p = [currentWindow mouseLocationOutsideOfEventStream];
					// NSPoint is small enough to fit a register, so no need for objc_msgSend_stret
					NSPoint p = ((NSPoint (*)(id, SEL))objc_msgSend)(currentWindow, mouseLocationOutsideOfEventStreamSel);

					// map input to content view rect
					if(p.x < 0) p.x = 0;
					else if(p.x > adjustFrame.size.width) p.x = adjustFrame.size.width;
					if(p.y < 0) p.y = 0;
					else if(p.y > adjustFrame.size.height) p.y = adjustFrame.size.height;

					// map input to pixels
					NSRect r = {p.x, p.y, 0, 0};
					//r = [currentWindowContentView convertRectToBacking:r];
					r = ((NSRect (*)(id, SEL, NSRect))abi_objc_msgSend_stret)(currentWindowContentView, convertRectToBackingSel, r);
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
					NSInteger number = ((NSInteger (*)(id, SEL))objc_msgSend)(event, buttonNumberSel);
					printf("mouse other key down : %i\n", (int)number);
					break;
				}
				//case NSOtherMouseUp:
				case 26:
				{
					//NSInteger number = [event buttonNumber];
					NSInteger number = ((NSInteger (*)(id, SEL))objc_msgSend)(event, buttonNumberSel);
					printf("mouse other key up : %i\n", (int)number);
					break;
				}
				//case NSScrollWheel:
				case 22:
				{
					//CGFloat deltaX = [event scrollingDeltaX];
					CGFloat deltaX = ((CGFloat (*)(id, SEL))abi_objc_msgSend_fpret)(event, scrollingDeltaXSel);

					//CGFloat deltaY = [event scrollingDeltaY];
					CGFloat deltaY = ((CGFloat (*)(id, SEL))abi_objc_msgSend_fpret)(event, scrollingDeltaYSel);

					//BOOL precisionScrolling = [event hasPreciseScrollingDeltas];
					BOOL precisionScrolling = ((BOOL (*)(id, SEL))objc_msgSend)(event, hasPreciseScrollingDeltasSel);

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
					NSUInteger modifiers = ((NSUInteger (*)(id, SEL))objc_msgSend)(event, modifierFlagsSel);

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
					id inputText = ((id (*)(id, SEL))objc_msgSend)(event, charactersSel);

					//const char * inputTextUTF8 = [inputText UTF8String];
					const char * inputTextUTF8 = ((const char* (*)(id, SEL))objc_msgSend)(inputText, UTF8StringSel);

					//you can get list of virtual key codes from Carbon HIToolbox/Events.h
					//uint16_t keyCode = [event keyCode];
					uint16_t keyCode = ((unsigned short (*)(id, SEL))objc_msgSend)(event, keyCodeSel);

					printf("key down %u, text '%s'\n", keyCode, inputTextUTF8);
					break;
				}
				//case NSKeyUp:
				case 11:
				{
					//uint16_t keyCode = [event keyCode];
					uint16_t keyCode = ((unsigned short (*)(id, SEL))objc_msgSend)(event, keyCodeSel);

					printf("key up %u\n", keyCode);
					break;
				}
				default:
					break;
			}

			//[NSApp sendEvent:event];
			((void (*)(id, SEL, id))objc_msgSend)(NSApp, sendEventSel, event);

			// if user closes the window we might need to terminate asap
			if(terminated)
				break;

			//[NSApp updateWindows];
			((void (*)(id, SEL))objc_msgSend)(NSApp, updateWindowsSel);
		}

		// do runloop stuff
		//[openGLContext update]; // probably we only need to do it when we resize the window
		((void (*)(id, SEL))objc_msgSend)(openGLContext, updateSel);

		//[openGLContext makeCurrentContext];
		((void (*)(id, SEL))objc_msgSend)(openGLContext, makeCurrentContextSel);

		//NSRect rect = [contentView frame];
		NSRect rect = ((NSRect (*)(id, SEL))abi_objc_msgSend_stret)(contentView, frameSel);

		//rect = [contentView convertRectToBacking:rect];
		rect = ((NSRect (*)(id, SEL, NSRect))abi_objc_msgSend_stret)(contentView, convertRectToBackingSel, rect);

		glViewport(0, 0, rect.size.width, rect.size.height);

		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(1.0f, 0.85f, 0.35f);
		glBegin(GL_TRIANGLES);
		{
			glVertex3f(  0.0f,  0.6f, 0.0f);
			glVertex3f( -0.2f, -0.3f, 0.0f);
			glVertex3f(  0.2f, -0.3f ,0.0f);
		}
		glEnd();

		//[openGLContext flushBuffer];
		((void (*)(id, SEL))objc_msgSend)(openGLContext, flushBufferSel);
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
