#ifndef BASE_MATH_H
#define BASE_MATH_H

#include "base.h"

typedef union Vector2 {
    struct {
        f32 x, y;
    };
    f32 e[2];
} Vector2;

#define v2_spread(a) (a).x, (a).y
Vector2 v2_add(Vector2 a, Vector2 b);
Vector2 v2_sub(Vector2 a, Vector2 b);
Vector2 v2_scale(Vector2 a, f32 scale);
f32 v2_dot(Vector2 a, Vector2 b);
//Vector2 v2_cross(Vector2 a, Vector2 b);
f32 v2_mag(Vector2 a);
Vector2 v2_norm(Vector2 a);

typedef union Vector3 {
    struct {
        f32 x, y, z;
    };
    struct {
        Vector2 xy;
        f32 _unused0;
    };
    struct {
        f32 _unused1;
        Vector2 yz;
    };
    f32 e[3];
} Vector3;

#define v3_spread(a) (a).x, (a).y, (a).z
Vector3 v3_add(Vector3 a, Vector3 b);
Vector3 v3_sub(Vector3 a, Vector3 b);
Vector3 v3_scale(Vector3 a, f32 scale);
f32 v3_dot(Vector3 a, Vector3 b);
Vector3 v3_cross(Vector3 a, Vector3 b);
f32 v3_mag(Vector3 a);
Vector3 v3_norm(Vector3 a);

typedef union Vector4 {
    struct {
        f32 x, y, z, w;
    };
    struct {
        Vector2 xy, zw;
    };
    struct {
        f32 _unused0;
        Vector2 yz;
        f32 _unused1;
    };
    struct {
        Vector3 xyz;
        f32 _unused2;
    };
    struct {
        f32 _unused4;
        Vector3 yzw;
    };
    f32 e[4];
} Vector4;

#define v4_spread(a) (a).x, (a).y, (a).z, (a).w
Vector4 v4_add(Vector4 a, Vector4 b);
Vector4 v4_sub(Vector4 a, Vector4 b);
Vector4 v4_scale(Vector4 a, f32 scale);
f32 v4_dot(Vector4 a, Vector4 b);
f32 v4_mag(Vector4 a);
Vector4 v4_norm(Vector4 a);

typedef union Matrix4x4 {
    struct {
        Vector4 cols[4];
    };
    f32 x[4][4];
    f32 e[16];
} Matrix4x4;

Matrix4x4 m4_scale(f32 x, f32 y, f32 z, f32 w);
#define m4_scale_v4(v) (m4_scale(v4_spread((v)))
#define m4_scale_v3(v) (m4_scale(v3_spread((v)), 1))
#define m4_ident() (m4_scale(1,1,1,1))
Matrix4x4 m4_translation(f32 x, f32 y, f32 z);
#define m4_translation_v(v) (m4_translation(v4_spread((v))))
Vector4 m4_v4_mul(Matrix4x4 m, Vector4 a);
Matrix4x4 m4_m4_mul(Matrix4x4 a, Matrix4x4 b);

#endif //BASE_MATH_H
