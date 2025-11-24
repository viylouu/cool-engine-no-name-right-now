#include "matrix.h"

#include <string.h>
#include <math.h>

/* FUNCS */

void mat4_make(
    mat4* mat,
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33
) {
    float tmp[16] = {
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33
    };
    memcpy(*mat, tmp, sizeof(tmp));
}

void mat4_multiply(mat4* out, const mat4 a, const mat4 b) {
    mat4 result;
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++) {
            result[row*4 + col] =
                a[row*4 + 0] * b[0*4 + col] +
                a[row*4 + 1] * b[1*4 + col] +
                a[row*4 + 2] * b[2*4 + col] +
                a[row*4 + 3] * b[3*4 + col];
        }

    memcpy(*out, result, sizeof(result));
}

void mat4_ortho(
    mat4* mat, 
    float left, float right,
    float bottom, float top,
    float near, float far
) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;

    mat4_make(mat, 
        2.f/rl, 0, 0, 0,
        0, 2.f/tb, 0, 0,
        0, 0,-2.f/fn, 0,
        -(right+left) / rl, -(top+bottom) / tb, -(far+near) / fn, 1
        );
}

void mat4_set_identity(mat4* mat) {
     mat4_make(mat, 
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
}

void mat4_translate(
    mat4* mat,
    float x, float y, float z
) {
    mat4_make(mat,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
        );
}

void mat4_scale(
    mat4* mat,
    float x, float y, float z
) {
    mat4_make(mat,
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
        );
}

void mat4_rotateX(mat4* mat, float ang) {
    float c = cosf(ang);
    float s = sinf(ang);

    mat4_make(mat,
        1, 0, 0, 0,
        0, c,-s, 0,
        0, s, c, 0,
        0, 0, 0, 1
        );
}

void mat4_rotateY(mat4* mat, float ang) {
    float c = cosf(ang);
    float s = sinf(ang);

    mat4_make(mat,
        c, 0, s, 0,
        0, 1, 0, 0,
       -s, 0, c, 0,
        0, 0, 0, 1
        );
}

void mat4_rotateZ(mat4* mat, float ang) {
    float c = cosf(ang);
    float s = sinf(ang);

    mat4_make(mat,
        c,-s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
}

void mat4_multiply_vector(vec4* out, const mat4 a, vec4 b) {
    out->x = a[0]*b.x + a[1]*b.y + a[2]*b.z + a[3]*b.w;
    out->y = a[4]*b.x + a[5]*b.y + a[6]*b.z + a[7]*b.w;
    out->z = a[8]*b.x + a[9]*b.y + a[10]*b.z + a[11]*b.w;
    out->w = a[12]*b.x + a[13]*b.y + a[14]*b.z + a[15]*b.w;
}
