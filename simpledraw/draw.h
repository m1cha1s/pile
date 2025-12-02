#ifndef _DRAW_H
#define _DRAW_H

#include <exmath.h>
#include <stdint.h>

#ifndef true
#define true (1)
#endif

#ifndef false
#define false (0)
#endif

extern uint32_t D_ClearColor;
extern uint32_t D_FillColor;
extern float D_StrokeWidth;

extern float DeltaTime;

extern Vector2 mousePos;

void D_Init(void);
void D_Deinit(void);

int D_Running(void);

void D_SetWindowDims( int w, int h );
void D_GetWindowDims( int *w, int *h );
void D_SetWindowTitle( const char *title );

void D_ClearScreen(void);
void D_Present(void);

void D_DrawFilledRect(float x, float y, float w, float h);
void D_DrawLine(float x1, float y1, float x2, float y2);

uint64_t D_GetTimeNS(void);

typedef enum KeyboardKey {
    KEY_UNKNOWN,
    KEY_W,
    KEY_S,
    KEY_A,
    KEY_D,
} KeyboardKey;

// Implemented by the app

void HandleKey(KeyboardKey key, int c, int pressed);
void HandleMouse(int x, int y, int dx, int dy, int scrollx, int scrolly);
void HandleButton(int button, int pressed);



#endif
