#include "draw.h"

#include <math.h>
#include <string.h>

const float EPSILON = 1e-6f;

float Lerp(float a, float b, float t)
{
    return a+t*(b-a);
}

void FiLerp(float *a, float b, float decay) {
    *a = b+((*a)-b)*exp(-decay*DeltaTime);
}

float Random(void)
{
    return (float)((rand() % 200)-100)/100.f;
}

float Mat3Det(const mat3_t m)
{
    return m[0][0] * (m[1][1]*m[2][2]-m[1][2]*m[2][1]) -
           m[0][1] * (m[1][0]*m[2][2]-m[1][2]*m[2][0]) +
           m[0][2] * (m[1][0]*m[2][1]-m[1][1]*m[2][0]);
}

void Mat3Invert(const mat3_t m, mat3_t out)
{
    float det = Mat3Det(m);

    if (fabsf(det) < EPSILON) return;

    float invDet = 1.f/det;

    out[0][0] = (m[1][1]*m[2][2]-m[1][2]*m[2][1]) * invDet;
    out[0][1] = (m[0][2]*m[2][1]-m[0][1]*m[2][2]) * invDet;
    out[0][2] = (m[0][1]*m[1][2]-m[0][2]*m[1][1]) * invDet;

    out[1][0] = (m[1][2]*m[2][0]-m[1][0]*m[2][2]) * invDet;
    out[1][1] = (m[0][0]*m[2][2]-m[0][2]*m[2][0]) * invDet;
    out[1][2] = (m[0][2]*m[1][0]-m[0][0]*m[1][2]) * invDet;

    out[2][0] = (m[1][0]*m[2][1]-m[1][1]*m[2][0]) * invDet;
    out[2][1] = (m[0][1]*m[2][0]-m[0][0]*m[2][1]) * invDet;
    out[2][2] = (m[0][0]*m[1][1]-m[0][1]*m[1][0]) * invDet;
}

void Mat3Vec3Mul(const mat3_t m, const vec3_t a, vec3_t res)
{
    for (int y = 0; y < 3; ++y)
    {
        res[y] = 0;
        for (int i = 0; i < 3; ++i)
        {
            res[y] += m[y][i] * a[i];
        }
    }
}
