#ifndef _DRAW_H
#define _DRAW_H

#include <stdint.h>

extern uint32_t D_ClearColor;
extern uint32_t D_FillColor;

void D_Init(void);
void D_Deinit(void);

int D_Running(void);

void D_SetWindowDims( int w, int h );
void D_SetWindowTitle( const char *title );

void D_ClearScreen(void);
void D_Present(void);

void D_DrawFilledRect(int x, int y, int w, int h);

#endif
