#include <stdio.h>
#include <stdlib.h>

#include "draw.h"

int main(int argc, char **argv)
{
    D_Init();
    
    D_SetWindowDims( 800, 600 );
    D_SetWindowTitle( "test" );

    while (D_Running())
    {
        D_ClearColor = 0xffff00ff;
        D_ClearScreen();

        D_FillColor = 0x0000ffff;
        D_DrawFilledRect(10, 10, 50, 50);

        D_Present();
    }

    D_Deinit();
}
