#if defined(__OBJC__) && __has_feature(objc_arc)
#error "Can't compile as objective-c code!"
#endif

#ifdef __arm64__
#define abi_objc_msgSend_stret objc_msgSend
#else
#define abi_objc_msgSend_stret objc_msgSend_stret
#endif
#ifdef __i386__
#define abi_abjc_msgSend_fpret objc_msgSend_fpret
#else
#define abi_abjc_msgSend_fpret objc_msgSend
#endif

extern void objc_msgSend();
extern void * NSApp;

#define objc_msgSendSuper_t(RET, ...) ((RET(*)(struct objc_super*, SEL, ##__VA_ARGS__))objc_msgSendSuper)
#define objc_msgSend_t(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))objc_msgSend)
#define objc_msgSend_stret_t(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))abi_objc_msgSend_stret)
#define objc_msgSend_id objc_msgSend_t(id)
#define objc_msgSend_void objc_msgSend_t(void)
#define objc_msgSend_void_id objc_msgSend_t(void, id)
#define objc_msgSend_void_bool objc_msgSend_t(void, bool)

#define sel(NAME) sel_registerName(NAME)
#define class(NAME) ((id)objc_getClass(NAME))
#define makeClass(NAME, SUPER) objc_allocateClassPair((Class)objc_getClass(SUPER), NAME, 0)

// Check here to get the signature right: https://nshipster.com/type-encodings/
#define addMethod(CLASS, NAME, IMPL, SIGNATURE)                       \
    if (!class_addMethod(CLASS, sel(NAME), (IMP)(IMPL), (SIGNATURE))) \
    assert(false)

#define addIvar(CLASS, NAME, SIZE, SIGNATURE)                           \
    if (!class_addIvar(CLASS, NAME, SIZE, rint(log2(SIZE)), SIGNATURE)) \
    assert(false)

#define objc_alloc(CLASS) objc_msgSend_id(class(CLASS), sel("alloc"))

#if __LP64__ || NS_BUILD_32_LIKE_64
#define NSIntegerEncoding "q"
#define NSUIntegerEncoding "L"
#else
#define NSIntegerEncoding "i"
#define NSUIntegerEncoding "I"
#endif

#include <mach/clock.h>
#include <mach/mach.h>

/* #include <AVFoundation/AVFoundation.h> */
#include <CoreFoundation/CoreFoundation.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>
/* #include <CoreFoundation/CFStringEncodingExt.h> */

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#include "draw.h"

typedef CGPoint NSPoint;
typedef CGSize NSSize;
typedef CGRect NSRect;

#define NSApplicationActivationPolicyRegular 0
const NSUInteger NSAllEventMask = NSUIntegerMax;
extern id const NSDefaultRunLoopMode;

// from Carbon HIToolbox/Events.h
enum {
    kVK_ANSI_A = 0x00,
    kVK_ANSI_S = 0x01,
    kVK_ANSI_D = 0x02,
    kVK_ANSI_F = 0x03,
    kVK_ANSI_H = 0x04,
    kVK_ANSI_G = 0x05,
    kVK_ANSI_Z = 0x06,
    kVK_ANSI_X = 0x07,
    kVK_ANSI_C = 0x08,
    kVK_ANSI_V = 0x09,
    kVK_ANSI_B = 0x0B,
    kVK_ANSI_Q = 0x0C,
    kVK_ANSI_W = 0x0D,
    kVK_ANSI_E = 0x0E,
    kVK_ANSI_R = 0x0F,
    kVK_ANSI_Y = 0x10,
    kVK_ANSI_T = 0x11,
    kVK_ANSI_1 = 0x12,
    kVK_ANSI_2 = 0x13,
    kVK_ANSI_3 = 0x14,
    kVK_ANSI_4 = 0x15,
    kVK_ANSI_6 = 0x16,
    kVK_ANSI_5 = 0x17,
    kVK_ANSI_Equal = 0x18,
    kVK_ANSI_9 = 0x19,
    kVK_ANSI_7 = 0x1A,
    kVK_ANSI_Minus = 0x1B,
    kVK_ANSI_8 = 0x1C,
    kVK_ANSI_0 = 0x1D,
    kVK_ANSI_RightBracket = 0x1E,
    kVK_ANSI_O = 0x1F,
    kVK_ANSI_U = 0x20,
    kVK_ANSI_LeftBracket = 0x21,
    kVK_ANSI_I = 0x22,
    kVK_ANSI_P = 0x23,
    kVK_ANSI_L = 0x25,
    kVK_ANSI_J = 0x26,
    kVK_ANSI_Quote = 0x27,
    kVK_ANSI_K = 0x28,
    kVK_ANSI_Semicolon = 0x29,
    kVK_ANSI_Backslash = 0x2A,
    kVK_ANSI_Comma = 0x2B,
    kVK_ANSI_Slash = 0x2C,
    kVK_ANSI_N = 0x2D,
    kVK_ANSI_M = 0x2E,
    kVK_ANSI_Period = 0x2F,
    kVK_ANSI_Grave = 0x32,
    kVK_ANSI_KeypadDecimal = 0x41,
    kVK_ANSI_KeypadMultiply = 0x43,
    kVK_ANSI_KeypadPlus = 0x45,
    kVK_ANSI_KeypadClear = 0x47,
    kVK_ANSI_KeypadDivide = 0x4B,
    kVK_ANSI_KeypadEnter = 0x4C,
    kVK_ANSI_KeypadMinus = 0x4E,
    kVK_ANSI_KeypadEquals = 0x51,
    kVK_ANSI_Keypad0 = 0x52,
    kVK_ANSI_Keypad1 = 0x53,
    kVK_ANSI_Keypad2 = 0x54,
    kVK_ANSI_Keypad3 = 0x55,
    kVK_ANSI_Keypad4 = 0x56,
    kVK_ANSI_Keypad5 = 0x57,
    kVK_ANSI_Keypad6 = 0x58,
    kVK_ANSI_Keypad7 = 0x59,
    kVK_ANSI_Keypad8 = 0x5B,
    kVK_ANSI_Keypad9 = 0x5C,
    kVK_Return = 0x24,
    kVK_Tab = 0x30,
    kVK_Space = 0x31,
    kVK_Delete = 0x33,
    kVK_Escape = 0x35,
    kVK_Command = 0x37,
    kVK_Shift = 0x38,
    kVK_CapsLock = 0x39,
    kVK_Option = 0x3A,
    kVK_Control = 0x3B,
    kVK_RightShift = 0x3C,
    kVK_RightOption = 0x3D,
    kVK_RightControl = 0x3E,
    kVK_Function = 0x3F,
    kVK_F17 = 0x40,
    kVK_VolumeUp = 0x48,
    kVK_VolumeDown = 0x49,
    kVK_Mute = 0x4A,
    kVK_F18 = 0x4F,
    kVK_F19 = 0x50,
    kVK_F20 = 0x5A,
    kVK_F5 = 0x60,
    kVK_F6 = 0x61,
    kVK_F7 = 0x62,
    kVK_F3 = 0x63,
    kVK_F8 = 0x64,
    kVK_F9 = 0x65,
    kVK_F11 = 0x67,
    kVK_F13 = 0x69,
    kVK_F16 = 0x6A,
    kVK_F14 = 0x6B,
    kVK_F10 = 0x6D,
    kVK_F12 = 0x6F,
    kVK_F15 = 0x71,
    kVK_Help = 0x72,
    kVK_Home = 0x73,
    kVK_PageUp = 0x74,
    kVK_ForwardDelete = 0x75,
    kVK_F4 = 0x76,
    kVK_End = 0x77,
    kVK_F2 = 0x78,
    kVK_PageDown = 0x79,
    kVK_F1 = 0x7A,
    kVK_LeftArrow = 0x7B,
    kVK_RightArrow = 0x7C,
    kVK_DownArrow = 0x7D,
    kVK_UpArrow = 0x7E
};

enum {
    NSWindowStyleMaskTitled = 1 << 0,
    NSWindowStyleMaskClosable =  1 << 1,
    NSWindowStyleMaskMiniaturizable = 1 << 2,
    NSWindowStyleMaskResizable = 1 << 3,
    NSWindowStyleRegular = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable,
    NSWindowStyleMaskFullSizeContentView = 1 << 15,
};

#define NSTrackingMouseEnteredAndExited 1
#define NSTrackingActiveInKeyWindow 0x20
#define NSTrackingInVisibleRect 0x200

enum {
    NSLeftMouseDown = 1,
    NSLeftMouseUp = 2,
    NSRightMouseDown = 3,
    NSRightMouseUp = 4,
    NSMouseMoved = 5,
    NSOtherMouseDown = 25,
    NSOtherMouseUp = 26,
    NSModifiersChanged = 12,
    NSKeyDown = 10,
    NSKeyDownMask = 1 << NSKeyDown,
    NSKeyUp = 11,
    NSKeyUpMask = 1 << NSKeyUp,
};

static int running;
static int didResize;
static id pool;
static id window;
static id openglContext;
static clock_serv_t clockServ;
static uint64_t lastTime = 0;

float DeltaTime = 0;
Vector2 mousePos;

void *InternalCocoa(id window) { return NULL; }

static NSUInteger AppShouldTerminate(id self, SEL _sel, id notification)
{
    running = 0;
    return 0;
}

static void WindowWillClose(id self, SEL _sel, id notification)
{
    NSUInteger value = 1;
    object_setInstanceVariable(self, "closed", (void*)value);
}

static void WindowDidResize(id self, SEL _sel, id notification)
{
    didResize = 1;
}

static void WindowDidEnterFullscreen(id self, SEL _sel, id notification) {}
static void WindowDidBecomeKey(id self, SEL _sel, id notification) {}
static void MouseEntered(id self, SEL _sel, id notification) {}
static void MouseExited(id self, SEL _sel, id notification) {}

static int IsWindowClosed(id window)
{
    id wdg = objc_msgSend_id(window, sel("delegate"));
    if (!wdg) return 0;
    NSUInteger value = 0;
    object_getInstanceVariable(wdg, "closed", (void**)&value);
    return value;
}

static int IsWindowVisible(id window)
{
    id wdg = objc_msgSend_id(window, sel("delegate"));
    if (!wdg) return 0;
    NSUInteger value = 0;
    object_getInstanceVariable(wdg, "visible", (void **) &value);
    return value;
}

static id PushPool(void)
{
    id pool = objc_msgSend_id(class("NSAutoreleasePool"), sel("alloc"));
    return objc_msgSend_id(pool, sel("init"));
}

static void PopPool(id pool)
{
    objc_msgSend_id(pool, sel("drain"));
}

static NSSize WindowSize(id window)
{
    id contentView = objc_msgSend_id(window, sel("contentView"));
    NSRect rect = objc_msgSend_stret_t(NSRect)(contentView, sel("frame"));
    return rect.size;
}

static NSSize ContentBackingSize(id window)
{
    id contentView = objc_msgSend_id(window, sel("contentView"));
    NSRect rect = objc_msgSend_stret_t(NSRect)(contentView, sel("frame"));
    rect = objc_msgSend_stret_t(NSRect, NSRect)(contentView, sel("convertRectToBacking:"), rect);

    return rect.size;
}

void D_GL_Init(void);

void D_Init(void)
{
    pool = PushPool();

    objc_msgSend_id(class("NSApplication"), sel("sharedApplication"));
    objc_msgSend_t(void, NSInteger)(NSApp, sel("setActivationPolicy:"), NSApplicationActivationPolicyRegular);

    Class appDelegateClass = makeClass("AppDelegate", "NSObject");
    addMethod(appDelegateClass, "applicationShouldTerminate", &AppShouldTerminate, NSUIntegerEncoding "@:@");
    id dgAlloc = objc_msgSend_id((id)appDelegateClass, sel("alloc"));
    id dg = objc_msgSend_id(dgAlloc, sel("init"));

    objc_msgSend_void_id(NSApp, sel("setDelegate:"), dg);
    objc_msgSend_void(NSApp, sel("finishLaunching"));

    id menuBar = objc_alloc("NSMenu");
    menuBar = objc_msgSend_id(menuBar, sel("init"));

    id appMenuItem = objc_alloc("NSMenuItem");
    appMenuItem = objc_msgSend_id(appMenuItem, sel("init"));

    objc_msgSend_void_id(menuBar, sel("addItem:"), appMenuItem);
    objc_msgSend_t(id, id)(NSApp, sel("setMainMenu:"), menuBar);

    id processInfo = objc_msgSend_id(class("NSProcessInfo"), sel("processInfo"));
    id appName = objc_msgSend_id(processInfo, sel("processName"));

    id appMenu = objc_alloc("NSMenu");
    appMenu = objc_msgSend_t(id, id)(appMenu, sel("initWithTitle:"), appName);

    id quitTitlePrefixString = objc_msgSend_t(id, const char *)(class("NSString"), sel("stringWithUTF8String:"), "Quit ");
    id quitTitle = objc_msgSend_t(id, id)(quitTitlePrefixString, sel("stringByAppendingString:"), appName);

    id quitMenuItemKey = objc_msgSend_t(id, const char *)(class("NSString"), sel("stringWithUTF8String:"), "q");
    id quitMenuItem = objc_alloc("NSMenuItem");
    quitMenuItem = objc_msgSend_t(id, id, SEL, id)(quitMenuItem, sel("initWithTitle:action:keyEquivalent:"), quitTitle, sel("terminate:"), quitMenuItemKey);

    objc_msgSend_void_id(appMenu, sel("addItem:"), quitMenuItem);
    objc_msgSend_void_id(appMenuItem, sel("setSubmenu:"), appMenu);

    // Initializing the window

    NSUInteger windowStyleMask = NSWindowStyleRegular & ~NSWindowStyleMaskMiniaturizable;
    int windowScale = 1;

    NSRect rect = { { 0, 0 }, { 800, 600 } };
    id windowAlloc = objc_msgSend_id(class("NSWindow"), sel("alloc"));
    window = objc_msgSend_t(id, NSRect, NSUInteger, NSUInteger, BOOL)(windowAlloc, sel("initWithContentRect:styleMask:backing:defer:"), rect, windowStyleMask, 2, NO);

    objc_msgSend_void_bool(window, sel("setReleasedWhenClosed:"), NO);

    Class windowDelegateClass = objc_allocateClassPair((Class)objc_getClass("NSObject"), "WindowDelegate", 0);
    addIvar(windowDelegateClass, "closed", sizeof(NSUInteger), NSUIntegerEncoding);
    addIvar(windowDelegateClass, "visible", sizeof(NSUInteger), NSUIntegerEncoding);
    addMethod(windowDelegateClass, "windowWillClose:", &WindowWillClose, "v@:@");
    addMethod(windowDelegateClass, "windowDidEnterFullscreen:", &WindowDidEnterFullscreen, "v@:@");
    addMethod(windowDelegateClass, "windowDidResize:", &WindowDidResize, "v@:@");
    addMethod(windowDelegateClass, "mouseEntered:", &MouseEntered, "v@:@");
    addMethod(windowDelegateClass, "mouseExited:", &MouseExited, "v@:@");

    id wdgAlloc = objc_msgSend_id((id)windowDelegateClass, sel("alloc"));
    id wdg = objc_msgSend_id(wdgAlloc, sel("init"));

    if (0)
    {
        objc_msgSend_void_id(window, sel("toggleFullscreen:"), window);
    }
    else
    {
        NSUInteger value = 1;
        object_setInstanceVariable(wdg, "visible", (void*)value);
    }

    objc_msgSend_void_id(window, sel("setDelegate:"), wdg);

    id contentView = objc_msgSend_id(window, sel("contentView"));
    int wantsHighRest = 1;
    objc_msgSend_void_bool(contentView, sel("setWantsBestResolutionOpenGLSurface:"), wantsHighRest);

    NSPoint point = { 20, 20 };
    objc_msgSend_t(void, NSPoint)(window, sel("cascadeTopLeftFromPoint:"), point);

    id titleString = objc_msgSend_t(id, const char *)(class("NSString"), sel("stringWithUTF8String:"), "!!!FIXME!!!");
    objc_msgSend_void_id(window, sel("setTitle:"), titleString);

    uint32_t glAttribs[] = {
            8, 24,
            11, 8,
            5,
            73,
            55, 1,
            56, 4,
            99, 0x3200,
            0
    };

    id pixelFormat = objc_alloc("NSOpenGLPixelFormat");
    pixelFormat = objc_msgSend_t(id, const uint32_t *)(pixelFormat, sel("initWithAttributes:"), glAttribs);
    objc_msgSend_void(pixelFormat, sel("autorelease"));

    openglContext = objc_alloc("NSOpenGLContext");
    openglContext = objc_msgSend_t(id, id, id)(openglContext, sel("initWithFormat:shareContext:"), pixelFormat, nil);
    objc_msgSend_void_id(openglContext, sel("setView:"), contentView);
    objc_msgSend_void_id(window, sel("makeKeyAndOrderFront:"), contentView);
    objc_msgSend_void_bool(window, sel("setAcceptsMouseMovedEvents:"), YES);

    id blackColor = objc_msgSend_id(class("NSColor"), sel("blackColor"));
    objc_msgSend_void_id(window, sel("setBackgroundColor:"), blackColor);

    objc_msgSend_void_bool(NSApp, sel("activateIgnoringOtherApps:"), YES);

    NSSize windowContentSize = ContentBackingSize(window);
    int trackingFlags = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect;
    id trackingArea = objc_msgSend_id(class("NSTrackingArea"), sel("alloc"));
    trackingArea = objc_msgSend_t(id, NSRect, int, id, id)(trackingArea, sel("initWithRect:options:owner:userInfo:"), rect, trackingFlags, wdg, 0);

    objc_msgSend_void(openglContext, sel("makeCurrentContext"));

    host_get_clock_service(mach_host_self(), CLOCK_REALTIME, &clockServ);

    D_GL_Init();

    running = 1;
}

void D_Deinit(void)
{
    if (!IsWindowClosed(window) && running)
    {
        objc_msgSend_void(window, sel("close"));
    }

    id wdg = objc_msgSend_id(window, sel("delegate"));
    objc_msgSend_void(wdg, sel("release"));
    objc_msgSend_void(openglContext, sel("release"));
    objc_msgSend_void(window, sel("release"));
}

void D_GetWindowDims( int *w, int *h )
{
    NSSize s = WindowSize(window);
    *w = s.width;
    *h = s.height;
}

static KeyboardKey ConvertKeyCode(uint16_t keyCode)
{
    switch (keyCode)
    {
        case kVK_ANSI_W: return KEY_W;
        case kVK_ANSI_S: return KEY_S;
        case kVK_ANSI_A: return KEY_A;
        case kVK_ANSI_D: return KEY_D;
        default: break;
    }
    return KEY_UNKNOWN;
}

static void HandleEvents(id event)
{
    if (!event) return;

    NSUInteger type = objc_msgSend_t(NSUInteger)(event, sel("type"));

    switch (type)
    {
        case NSMouseMoved: {
//            NSPoint loc = objc_msgSend_t(NSPoint)(event, sel("locationInWindow"));
            NSPoint loc = objc_msgSend_t(NSPoint)(window, sel("mouseLocationOutsideOfEventStream"));

            id view = objc_msgSend_id(window, sel("contentView"));
            NSRect adjustFrame = objc_msgSend_stret_t(NSRect)(view, sel("frame"));

            NSRect r = {loc, {0, 0}};
//            r = objc_msgSend_stret_t(NSRect, NSRect)(view, sel("convertRectToBacking:"), r);
//            loc = r.origin;

            NSSize s = WindowSize(window);

            mousePos.x = loc.x;
            mousePos.y = s.height-loc.y;
        } break;
        case NSLeftMouseDown: {

        } break;
        case NSLeftMouseUp: {

        } break;
        case NSRightMouseDown: {

        } break;
        case NSRightMouseUp: {

        } break;
        case NSOtherMouseDown: {

        } break;
        case NSOtherMouseUp: {

        } break;
        case NSModifiersChanged: {

        } break;
        case NSKeyDown: {
            uint16_t keyCode = objc_msgSend_t(uint16_t)(event, sel("keyCode"));
            KeyboardKey key = ConvertKeyCode(keyCode);

            id chars = objc_msgSend_t(id)(event, sel("characters"));
            uint8_t *zstr = objc_msgSend_t(uint8_t*)(chars, sel("UTF8String"));
            int zstrLen = strlen(zstr);

            int c = zstrLen ? zstr[0] : 0;

            HandleKey(key, c, 1);

            if (key != KEY_UNKNOWN) return;
        } break;
        case NSKeyUp: {
            uint16_t keyCode = objc_msgSend_t(uint16_t)(event, sel("keyCode"));
            KeyboardKey key = ConvertKeyCode(keyCode);

            id chars = objc_msgSend_t(id)(event, sel("characters"));
            uint8_t *zstr = objc_msgSend_t(uint8_t*)(chars, sel("UTF8String"));
            int zstrLen = strlen(zstr);

            int c = zstrLen ? zstr[0] : 0;

            HandleKey(key, c, 0);

            if (key != KEY_UNKNOWN) return;
        } break;
        default: break;
    }

ExitWithPipe:
    objc_msgSend_void_id(NSApp, sel("sendEvent:"), event);
}

int D_Running(void)
{
    uint64_t now = D_GetTimeNS();

    uint64_t diff = now - lastTime;

    float newDeltaTime = (float)diff/1e9f;
    if (newDeltaTime < 1) DeltaTime = newDeltaTime;

    lastTime = now;

    PopPool(pool);
    pool = PushPool();

    if (!running || IsWindowClosed(window)) return 0;

    id keyWindow = objc_msgSend_id(NSApp, sel("keyWindow"));
    NSUInteger eventMask = NSAllEventMask;

    if (keyWindow == window)
    {

    }
    else
    {
        eventMask &= ~(NSKeyDownMask | NSKeyUpMask);
    }

    id event = 0;
    int visible = 0;

    do {
        event = objc_msgSend_t(id, NSUInteger, id, id, BOOL)(NSApp, sel("nextEventMatchingMask:untilDate:inMode:dequeue:"), eventMask, nil, NSDefaultRunLoopMode, YES);
        if (event)
        {
            HandleEvents(event);
        }
        else
        {
            visible = IsWindowVisible(window);
        }
    } while (event != 0 || !visible);

    objc_msgSend_void(NSApp, sel("updateWindows"));
    objc_msgSend_void(openglContext, sel("update"));
    objc_msgSend_void(openglContext, sel("makeCurrentContext"));

    return running;
}

void D_Present(void)
{
    objc_msgSend_void(openglContext, sel("flushBuffer"));
    objc_msgSend_void(class("NSOpenGLContext"), sel("clearCurrentContext"));
}

void D_SetWindowDims( int w, int h )
{
    NSRect r = { {10, 10}, {w, h} };
    objc_msgSend_t(void, NSRect, BOOL)(window, sel("setFrame:display:"), r, 1);
}

void D_SetWindowTitle( const char *title )
{
    id titleString = objc_msgSend_t(id, const char *)(class("NSString"), sel("stringWithUTF8String:"), title);
    objc_msgSend_void_id(window, sel("setTitle:"), titleString);
}

uint64_t D_GetTimeNS(void)
{
    mach_timespec_t ts;
    clock_get_time(clockServ, &ts);
    uint64_t res = (uint64_t)ts.tv_sec*1000000000+(uint64_t)ts.tv_nsec;
    return res;
}

#include "draw_gl.c"
