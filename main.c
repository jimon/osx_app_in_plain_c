
// we don't need much here
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>

// maybe this is available somewhere in objc runtime?
#if __LP64__ || (TARGET_OS_EMBEDDED && !TARGET_OS_IPHONE) || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
#define NSIntegerEncoding "q"
#define NSUIntegerEncoding "L"
#else
#define NSIntegerEncoding "i"
#define NSUIntegerEncoding "I"
#endif

// this is how they are defined originally
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>
typedef CGPoint NSPoint;
typedef CGRect NSRect;

extern id NSApp;
extern id const NSDefaultRunLoopMode;

bool terminated = false;

// we gonna create objective-c class by hand in runtime, so wow, so hacker!
//@interface AppDelegate : NSObject<NSApplicationDelegate>
//- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
//@end
//@implementation AppDelegate
//- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
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

int main() {
	//@autoreleasepool { // TODO ???

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
	
	//[NSApp setDelegate:dg];
	((void (*)(id, SEL, id))objc_msgSend)(NSApp, sel_registerName("setDelegate:"), dg);
	
	// not needed if we use [NSApp run]
	//[NSApp finishLaunching];
	((void (*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("finishLaunching"));
	
	//id menubar = [[NSMenu alloc] init];
	id menubarAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenu"), sel_registerName("alloc"));
	id menubar = ((id (*)(id, SEL))objc_msgSend)(menubarAlloc, sel_registerName("init"));
	
	//id appMenuItem = [[NSMenuItem alloc] init];
	id appMenuItemAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_registerName("alloc"));
	id appMenuItem = ((id (*)(id, SEL))objc_msgSend)(appMenuItemAlloc, sel_registerName("init"));
	
	//[menubar addItem:appMenuItem];
	((void (*)(id, SEL, id))objc_msgSend)(menubar, sel_registerName("addItem:"), appMenuItem);
	
	//[NSApp setMainMenu:menubar];
	((id (*)(id, SEL, id))objc_msgSend)(NSApp, sel_registerName("setMainMenu:"), menubar);
	
	//id appMenu = [[NSMenu alloc] init];
	id appMenuAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenu"), sel_registerName("alloc"));
	id appMenu = ((id (*)(id, SEL))objc_msgSend)(appMenuAlloc, sel_registerName("init"));
	
	//id appName = [[NSProcessInfo processInfo] processName];
	id processInfo = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSProcessInfo"), sel_registerName("processInfo"));
	id appName = ((id (*)(id, SEL))objc_msgSend)(processInfo, sel_registerName("processName"));
	
	//id quitTitle = [@"Quit " stringByAppendingString:appName];
	id quitTitlePrefixStringAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("alloc"));
	id quitTitlePrefixString = ((id (*)(id, SEL, const char*))objc_msgSend)(quitTitlePrefixStringAlloc, sel_registerName("initWithUTF8String:"), "Quit ");
	id quitTitle = ((id (*)(id, SEL, id))objc_msgSend)(quitTitlePrefixString, sel_registerName("stringByAppendingString:"), appName);
	
	//id quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
	id quitMenuItemKeyAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("alloc"));
	id quitMenuItemKey = ((id (*)(id, SEL, const char*))objc_msgSend)(quitMenuItemKeyAlloc, sel_registerName("initWithUTF8String:"), "q");
	id quitMenuItemAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_registerName("alloc"));
	id quitMenuItem = ((id (*)(id, SEL, id, SEL, id))objc_msgSend)(quitMenuItemAlloc, sel_registerName("initWithTitle:action:keyEquivalent:"), quitTitle, sel_registerName("terminate:"), quitMenuItemKey);
	
	//[appMenu addItem:quitMenuItem];
	((void (*)(id, SEL, id))objc_msgSend)(appMenu, sel_registerName("addItem:"), quitMenuItem);
	
	//[appMenuItem setSubmenu:appMenu];
	((void (*)(id, SEL, id))objc_msgSend)(appMenuItem, sel_registerName("setSubmenu:"), appMenu);
	
	//id window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 200, 200) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask backing:NSBackingStoreBuffered defer:NO];
	NSRect rect = {{0, 0}, {200, 200}};
	id windowAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSWindow"), sel_registerName("alloc"));
	id window = ((id (*)(id, SEL, NSRect, NSUInteger, NSUInteger, BOOL))objc_msgSend)(windowAlloc, sel_registerName("initWithContentRect:styleMask:backing:defer:"), rect, 15, 2, NO);
	
	//[window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
	NSPoint point = {20, 20};
	((void (*)(id, SEL, NSPoint))objc_msgSend)(window, sel_registerName("cascadeTopLeftFromPoint:"), point);
	
	//[window setTitle:@"sup"];
	id titleStringAlloc = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("alloc"));
	id titleString = ((id (*)(id, SEL, const char*))objc_msgSend)(titleStringAlloc, sel_registerName("initWithUTF8String:"), "sup from C");
	((void (*)(id, SEL, id))objc_msgSend)(window, sel_registerName("setTitle:"), titleString);
	
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
			printf("event\n");
			//[NSApp sendEvent:event];
			((void (*)(id, SEL, id))objc_msgSend)(NSApp, sel_registerName("sendEvent:"), event);
			
			//[NSApp updateWindows];
			((void (*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("updateWindows"));
		}
		
		// do runloop stuff
	}

	// or optionally you can use default run loop, and don't forget to disable [NSApp finishLaunching] then
	//[NSApp run];
	//((void (*)(id, SEL))objc_msgSend)(NSApp, sel_registerName("run"));

	printf("gracefully terminated\n");
	
	// } // TODO
	return 0;
}

