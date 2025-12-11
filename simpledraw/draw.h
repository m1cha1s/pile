#ifndef _DRAW_H
#define _DRAW_H

#include "draw_math.h"
#include "draw_colorspace.h"
#include <stdint.h>

#ifndef true
#define true (1)
#endif

#ifndef false
#define false (0)
#endif

#define ArrayCount(arr) (sizeof((arr))/sizeof(*(arr)))

extern float D_StrokeWidth;

extern float DeltaTime;

extern vec2_t mousePos;

void D_Init(void);
void D_Deinit(void);

void D_InitAudio(int sampleRate);
void D_StartAudio(void);
void D_StopAudio(void);
void D_DeinitAudio(void);

int D_Running(void);

typedef enum ColorType {
    D_COLOR_FILL,
    D_COLOR_CLEAR,
    
    D_COLOR_TYPE_COUNT,
} ColorType;

void D_SetColor(ColorType type, float r, float g, float b, float a);
void D_SetColorHex(ColorType type, uint32_t color);
void D_SetColorVec4(ColorType type, vec4_t color);

void D_SetWindowDims( int w, int h );
void D_GetWindowDims( int *w, int *h );
void D_SetWindowTitle( const char *title );

void D_ClearScreen(void);
void D_Present(void);

#define D_DrawFilledRectV(pos,dim) D_DrawFilledRect((pos)[0], (pos)[1], (dim)[0], (dim)[1])
void D_DrawFilledRect(float x, float y, float w, float h);
#define D_DrawFilledCircleV(pos, r, segCount) D_DrawFilledCircle((pos)[0], (pos)[1], (r), (segCount))
void D_DrawFilledCircle(float x, float y, float r, int segCount);
#define D_DrawLineV(p1,p2) D_DrawLine((p1)[0], (p1)[1], (p2)[0], (p2)[1])
void D_DrawLine(float x1, float y1, float x2, float y2);

void D_BeginSegmentedLine(void);
#define D_DrawSegmentV(p1,p2) D_DrawSegment((p1)[0], (p1)[1], (p2)[0], (p2)[1])
void D_DrawSegment(float x1, float y1, float x2, float y2);
void D_EndSegmentedLine(void);

#define D_DrawBezierV(p1,p2,p3,p4,segCount) D_DrawBezier((p1)[0], (p1)[1], (p2)[0], (p2)[1], (p3)[0], (p3)[1], (p4)[0], (p4)[1], (segCount))
void D_DrawBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int segCount);

uint64_t D_GetTimeNS(void);

typedef enum KeyboardKey {
    KEY_UNKNOWN,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
} KeyboardKey;

// Implemented by the app

void HandleKey(KeyboardKey key, int c, int pressed);
void HandleMouse(int x, int y, int dx, int dy, int scrollx, int scrolly);
void HandleButton(int button, int pressed);
void HandleAudio(float *outBuffer, int framesToWrite);

#endif

#ifdef DRAW_IMPLEMENTATION
#undef DRAW_IMPLEMENTATION

#if defined(__APPLE__)
#include "draw_cocoa.c"
#elif defined(__linux__)
#include "draw_x11.c"
#else
#error "Platform not supported!"
#endif

#ifdef DRAW_GL
#include "draw_gl.c"
#endif

#include "draw_math.c"
#include "draw_mem.c"
#include "draw_colorspace.c"

#endif
