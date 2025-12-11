#include "draw_math.h"
#include <math.h>
#include "draw_colorspace.h"

void SRGBToOkLab(vec3_t c) 
{
    float l = 0.4122214708f * c[0] + 0.5363325363f * c[1] + 0.0514459929f * c[2];
	float m = 0.2119034982f * c[0] + 0.6806995451f * c[1] + 0.1073969566f * c[2];
	float s = 0.0883024619f * c[0] + 0.2817188376f * c[1] + 0.6299787005f * c[2];

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    c[0] = Clamp(0, 1, 0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_);
    c[1] = Clamp(-0.5, 0.5, 1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_);
    c[2] = Clamp(-0.5, 0.5, 0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_);
}

void OkLabToSRGB(vec3_t c) 
{
    float l_ = c[0] + 0.3963377774f * c[1] + 0.2158037573f * c[2];
    float m_ = c[0] - 0.1055613458f * c[1] - 0.0638541728f * c[2];
    float s_ = c[0] - 0.0894841775f * c[1] - 1.2914855480f * c[2];

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

    c[0] = Clamp(0, 1, +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s);
    c[1] = Clamp(0, 1, -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s);
    c[2] = Clamp(0, 1, -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s);
}

void OkLChToOkLab(vec3_t c, float L, float C, float h)
{
    c[0] = L;
    c[1] = Clamp(-0.4, 0.4, C)*cosf(h);
    c[2] = Clamp(-0.4, 0.4, C)*sinf(h);
}

uint32_t Vec3ToHexColor(vec3_t c)
{
    return (uint32_t)(255*c[0]) << 24 | (uint32_t)(255*c[1]) << 16 |(uint32_t)(255*c[2]) << 8;
}
