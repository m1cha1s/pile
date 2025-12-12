#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include <unistd.h>
#include <time.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include "draw.h"

#include <exmath.h>

#define GLPROCS \
    X(PFNGLCREATESHADERPROC, glCreateShader) \
    X(PFNGLSHADERSOURCEPROC, glShaderSource) \
    X(PFNGLCOMPILESHADERPROC, glCompileShader) \
    X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
    X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
    X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
    X(PFNGLATTACHSHADERPROC, glAttachShader) \
    X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
    X(PFNGLDELETESHADERPROC, glDeleteShader) \
    X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
    X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
    X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
    X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
    X(PFNGLGENBUFFERSPROC, glGenBuffers) \
    X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
    X(PFNGLBINDBUFFERPROC, glBindBuffer) \
    X(PFNGLBUFFERDATAPROC, glBufferData) \
    X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
    X(PFNGLUSEPROGRAMPROC, glUseProgram) \
    X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
    X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
    X(PFNGLUNIFORM2FPROC, glUniform2f)

#define X(type, name) type name = NULL;
GLPROCS;
#undef X

static Display *display;
static Screen *screen;
static int screenId;
static Window rootWindow;
static Window window;
static int running;
static GLXContext context = 0;
static Atom atomWmDeleteWindow;
static uint64_t lastTime = 0;

static pa_context *audioContext = NULL;
static pa_threaded_mainloop *psTMainloop = NULL;

float DeltaTime = 0;
vec2_t mousePos = {0};

typedef GLXContext (*glxCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);

void D_GL_Init(void);

void D_Init(void)
{
    display = XOpenDisplay( 0 );
    screen = DefaultScreenOfDisplay( display );
    screenId = DefaultScreen( display );

    rootWindow = RootWindow( display, screenId );
//    mainWindow = XCreateSimpleWindow( display, rootWindow, 0, 0, 800, 600, 0, 0, 0 );
    
//    XMapWindow( display, mainWindow );
//    XFlush( display );
    
    GLint majorGLX, minorGLX = 0;
    glXQueryVersion( display, &majorGLX, &minorGLX );
    printf( "GLX Version: %d.%d\n", majorGLX, minorGLX );

    GLint glxAttribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None        
    };

    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig( display, screenId, glxAttribs, &fbcount );
    if (!fbc)
    {
        fprintf(stderr, "Failed to retreive a framebuffer\n" );
        exit( 1 );
    }

    int bestFb = -1, worstFb = -1, bestSampNum = -1, worstSampNum = -1;
    for (int i=0; i < fbcount; ++i)
    {
        XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
        if (vi)
        {
            int sampBuf, samples;
            glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &sampBuf );
            glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES, &samples );

            if ( bestFb < 0 || (sampBuf && samples > bestSampNum) )
            {
                bestFb = i;
                bestSampNum = samples;
            }
            if ( worstFb < 0 || !sampBuf || samples < worstSampNum )
            {
                worstFb = i;
                worstSampNum = samples;
            }
        }
        XFree( vi );
    }

    GLXFBConfig bestFbc = fbc[ bestFb ];
    XFree( fbc );

    XVisualInfo *visual = glXGetVisualFromFBConfig( display, bestFbc );
    if (!visual)
    {
        fprintf(stderr, "Could not create correct visual window.\n" );
        XCloseDisplay( display );
        exit( 1 );
    }

    if (screenId != visual->screen)
    {
        fprintf(stderr, "Screen ID mismatch %d != %d\n", screenId, visual->screen);
        XCloseDisplay( display );
        exit( 1 );
    }

    XSetWindowAttributes windowAttribs = {
        .border_pixel = BlackPixel( display, screenId ),
        .background_pixel = WhitePixel( display, screenId ),
        .override_redirect = True,
        .colormap = XCreateColormap( display, rootWindow, visual->visual, AllocNone ),
        .event_mask = ExposureMask | KeyPressMask|KeyReleaseMask,
    };

    window = XCreateWindow( display, rootWindow, 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs );

    // Redirect Close ???
    atomWmDeleteWindow = XInternAtom( display, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( display, window, &atomWmDeleteWindow, 1 );

    glxCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glxCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

    int ctxAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
    };

    const char *glxExts = glXQueryExtensionsString( display, screenId );

    // TODO extension checkings

    context = glXCreateContextAttribsARB( display, bestFbc, 0, 1, ctxAttribs );

    XSync( display, False );

    if (!glXIsDirect( display, context ))
    {
        printf("Indirect GLX rendering context optained\n");
    }

    glXMakeCurrent( display, window, context );

    printf( "GL Renderer:  %s\n", glGetString(GL_RENDERER) );
    printf( "GL Version:   %s\n", glGetString(GL_VERSION) );
    printf( "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION) );

#define X(type, name) name = (type)glXGetProcAddress( #name ); if (!name) exit(1);
    GLPROCS;
#undef X

    D_GL_Init();

    XClearWindow( display, window );
    XMapRaised( display, window );

    running = 1;
}

void D_Deinit(void)
{
    XCloseDisplay( display );
}

KeyboardKey KeySymToKey(KeySym ks)
{
    switch (ks)
    {
    case XK_a: return KEY_A;
    case XK_b: return KEY_B;
    case XK_c: return KEY_C;
    case XK_d: return KEY_D;
    case XK_e: return KEY_E;
    case XK_f: return KEY_F;
    case XK_g: return KEY_G;
    case XK_h: return KEY_H;
    case XK_i: return KEY_I;
    case XK_j: return KEY_J;
    case XK_k: return KEY_K;
    case XK_l: return KEY_L;
    case XK_m: return KEY_M;
    case XK_n: return KEY_N;
    case XK_o: return KEY_O;
    case XK_p: return KEY_P;
    case XK_q: return KEY_Q;
    case XK_r: return KEY_R;
    case XK_s: return KEY_S;
    case XK_t: return KEY_T;
    case XK_u: return KEY_U;
    case XK_v: return KEY_V;
    case XK_w: return KEY_W;
    case XK_x: return KEY_X;
    case XK_y: return KEY_Y;
    case XK_z: return KEY_Z;
    }
    return KEY_UNKNOWN;
}

int D_Running(void)
{
    XEvent ev;
    while (XPending( display ) > 0)
    {
        XNextEvent( display, &ev );
        switch (ev.type)
        {
        case ClientMessage: {
            if (ev.xclient.data.l[0] == atomWmDeleteWindow) running = 0;
        } break;
        case KeyPress: {
            unsigned int keyCode = ev.xkey.keycode;
            KeyboardKey key = KeySymToKey(XKeycodeToKeysym(display, keyCode, 0));
            HandleKey(key, 0, 1);
        } break;
        case KeyRelease: {
            unsigned int keyCode = ev.xkey.keycode;
            KeyboardKey key = KeySymToKey(XKeycodeToKeysym(display, keyCode, 0));
            HandleKey(key, 0, 0);
        } break;
        }
    }

    int w,h;
    D_GetWindowDims(&w,&h);
    glViewport(0,0,w,h);

    int mousex, mousey;
    unsigned int mask;
    XQueryPointer(display, window, &rootWindow, &rootWindow, &mousex, &mousey, &mousex, &mousey, &mask);

    mousePos[0] = mousex;
    mousePos[1] = mousey;

    uint64_t now = D_GetTimeNS();

    uint64_t diff = now - lastTime;

    float newDeltaTime = (float)diff/1e9f;
    if (newDeltaTime < 1) DeltaTime = newDeltaTime;

    lastTime = now;

    return running;
}

uint64_t D_GetTimeNS(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t res = (uint64_t)ts.tv_sec*1000000000+(uint64_t)ts.tv_nsec;
    return res;
}

void D_SetWindowDims( int w, int h )
{
    XResizeWindow(display, window, w, h);
}

void D_SetWindowTitle( const char *title )
{
    XStoreName(display, window, title);
}

void D_Present(void)
{
    glXSwapBuffers( display, window ); 
}

void D_GetWindowDims( int *x, int *y )
{
    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);
	*x = attrs.width;
	*y = attrs.height;
}

static void PAContextCallback(pa_context *ctx, void *userData)
{
    pa_context_state_t state = pa_context_get_state(ctx);

    switch (state)
    {
    case PA_CONTEXT_TERMINATED:
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_READY: {
        pa_threaded_mainloop_signal(paTMainloop, 0);
    } break;
    default: brak;
    }
}

static void Stre

void D_InitAudio(int sampleRate)
{
    paTMainloop = pa_threaded_mainloop_new();
    audioContext = pa_context_new(pa_threaded_mainloop_get_api(paTMainloop), "foobar");

    pa_context_set_state_callback(audioContext, PAContextCallback, NULL);

    pa_threaded_mainloop_start(paTMainloop);

    pa_context_connect(audioContext, NULL, PA_CONTEXT_NOFLAGS, NULL);

    pa_context_state_t state;

    pa_threaded_mainloop_lock(paTMainloop);

    state = pa_context_get_state(audioContext);

    while (state != PA_CONTEXT_READY && state != PA_CONTEXT_FAILED && state != PA_CONTEXT_TERMINATED)
    {
        pa_threaded_mainloop_wait(paTMainloop);
        state = pa_context_get_state(audioContext);
    }

    pa_threaded_mainloop_unlock(paTMainloop);

    pa_sample_spec sampleSpec = {
        .format = PA_SAMPLE_FLOAT32NE,
        .rate = sampleRate,
        .channels = 2, // Stereo
    };

    int paError = 0;
    paSimpleContext = pa_simple_new(
        NULL,
        "simpledraw",
        PA_STREAM_PLAYBACK,
        NULL,
        "Audio???",
        &sampleSpec,
        NULL,
        NULL,
        &paError
        );

    if (paSimpleContext == NULL)
    {
        fprintf("Error initializing pulse/pipewire: %s\n", pa_strerror(psError));
        exit(-1);
    }

    
}

void D_DeinitAudio(void)
{
    if (paTMainloop)
    {
        pa_threaded_mainloop_free(paTMainloop);
    }
}

void D_StartAudio(void)
{
}

void D_StopAudio(void)
{
}

