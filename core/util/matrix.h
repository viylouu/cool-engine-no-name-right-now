#pragma once

#include "cvec.h"

/* TYPES */

typedef float mat4[16];

/* FUNCS */

void mat4_make(
    mat4* mat,
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33
);

void mat4_multiply(mat4* out, const mat4 a, const mat4 b);

void mat4_ortho(
    mat4* mat, 
    float left, float right,
    float bottom, float top,
    float near, float far
);

void mat4_set_identity(mat4* mat);
mat4* mat4_get_identity(void);

void mat4_translate(mat4* mat, float x, float y, float z);
void mat4_scale(mat4* mat, float x, float y, float z);

void mat4_rotateX(mat4* mat, float ang);
void mat4_rotateY(mat4* mat, float ang);
void mat4_rotateZ(mat4* mat, float ang);

void mat4_multiply_vector(vec4* out, const mat4 a, vec4 b);
