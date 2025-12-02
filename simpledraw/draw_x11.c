#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <unistd.h>

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

float DeltaTime = 0;
Vector2 mousePos = {0};

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
        .event_mask = ExposureMask,
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

int D_Running(void)
{
    XEvent ev;
    while (XPending( display ) > 0)
    {
        XNextEvent( display, &ev );
        if (ev.type == ClientMessage)
        {
            if (ev.xclient.data.l[0] == atomWmDeleteWindow) running = 0;
        }
    }

    int mousex, mousey;
    unsigned int mask;
    XQueryPointer(display, window, &rootWindow, &rootWindow, &mousex, &mousey, &mousex, &mousey, &mask);

    mousePos.x = mousex;
    mousePos.y = mousey;

    return running;
}

void D_SetWindowDims( int w, int h )
{
}

void D_SetWindowTitle( const char *title )
{
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

#include "draw_gl.c"


