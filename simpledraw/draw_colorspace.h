#ifndef _DRAW_COLORSPACE_H
#define _DRAW_COLORSPACE_H

#include "draw_math.h"
#include <stdint.h>

void SRGBToOkLab(vec3_t c);
void OkLabToSRGB(vec3_t c);
void OkLChToOkLab(vec3_t c, float L, float C, float h);

uint32_t Vec3ToHexColor(vec3_t c);

#endif