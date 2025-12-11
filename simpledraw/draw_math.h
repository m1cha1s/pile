#ifndef _DRAW_MATH_H
#define _DRAW_MATH_H

#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
#define Clamp(min,max,x) (Max((min), Min((x), (max))))

typedef float vec2_t[2];
typedef float vec3_t[3];
typedef float vec4_t[4];
typedef float mat3_t[3][3];
typedef float mat4_t[4][4];

#define Vector2Copy(a, b) ((b)[0]=(a)[0], (b)[1]=(a)[1])
#define Vector3Copy(a, b) ((b)[0]=(a)[0], (b)[1]=(a)[1], (b)[2]=(a)[2])

float Lerp(float a, float b, float t);
void FiLerp(float *a, float b, float decay);
float Random(void);

float Mat3Det(const mat3_t m);
void Mat3Invert(const mat3_t m, mat3_t out);
void Mat3Vec3Mul(const mat3_t m, const vec3_t a, vec3_t res);

#endif