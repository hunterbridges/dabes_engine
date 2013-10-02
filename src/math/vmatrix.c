#include <math.h>
#include <stdio.h>
#include "vmatrix.h"
#include "../util.h"

const VMatrix VMatrixIdentity = {.gl = {
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1
}};

const VVector4 VVector4Zero = {.raw = {0, 0, 0, 0}};

// this file sucks

VVector3 VVector3_normalize(VVector3 vector) {
    float scale = 1.0f / VVector3_length(vector);
    VVector3 v = {.v = {
      vector.v[0] * scale,
      vector.v[1] * scale,
      vector.v[2] * scale
    }};
    return v;
}

float VVector3_length(VVector3 vector) {
    return sqrt(vector.v[0] * vector.v[0] +
                vector.v[1] * vector.v[1] +
                vector.v[2] * vector.v[2]);
}

VMatrix VMatrix_multiply(VMatrix a, VMatrix b) {
    VMatrix mout = {.gl = {
        a.gl[0] * b.gl[0]  + a.gl[4] * b.gl[1]  + a.gl[8] * b.gl[2]   + a.gl[12] * b.gl[3],
        a.gl[1] * b.gl[0]  + a.gl[5] * b.gl[1]  + a.gl[9] * b.gl[2]   + a.gl[13] * b.gl[3],
        a.gl[2] * b.gl[0]  + a.gl[6] * b.gl[1]  + a.gl[10] * b.gl[2]  + a.gl[14] * b.gl[3],
        a.gl[3] * b.gl[0]  + a.gl[7] * b.gl[1]  + a.gl[11] * b.gl[2]  + a.gl[15] * b.gl[3],

        a.gl[0] * b.gl[4]  + a.gl[4] * b.gl[5]  + a.gl[8] * b.gl[6]   + a.gl[12] * b.gl[7],
        a.gl[1] * b.gl[4]  + a.gl[5] * b.gl[5]  + a.gl[9] * b.gl[6]   + a.gl[13] * b.gl[7],
        a.gl[2] * b.gl[4]  + a.gl[6] * b.gl[5]  + a.gl[10] * b.gl[6]  + a.gl[14] * b.gl[7],
        a.gl[3] * b.gl[4]  + a.gl[7] * b.gl[5]  + a.gl[11] * b.gl[6]  + a.gl[15] * b.gl[7],

        a.gl[0] * b.gl[8]  + a.gl[4] * b.gl[9]  + a.gl[8] * b.gl[10]  + a.gl[12] * b.gl[11],
        a.gl[1] * b.gl[8]  + a.gl[5] * b.gl[9]  + a.gl[9] * b.gl[10]  + a.gl[13] * b.gl[11],
        a.gl[2] * b.gl[8]  + a.gl[6] * b.gl[9]  + a.gl[10] * b.gl[10] + a.gl[14] * b.gl[11],
        a.gl[3] * b.gl[8]  + a.gl[7] * b.gl[9]  + a.gl[11] * b.gl[10] + a.gl[15] * b.gl[11],

        a.gl[0] * b.gl[12] + a.gl[4] * b.gl[13] + a.gl[8] * b.gl[14]  + a.gl[12] * b.gl[15],
        a.gl[1] * b.gl[12] + a.gl[5] * b.gl[13] + a.gl[9] * b.gl[14]  + a.gl[13] * b.gl[15],
        a.gl[2] * b.gl[12] + a.gl[6] * b.gl[13] + a.gl[10] * b.gl[14] + a.gl[14] * b.gl[15],
        a.gl[3] * b.gl[12] + a.gl[7] * b.gl[13] + a.gl[11] * b.gl[14] + a.gl[15] * b.gl[15]
    }};
    return mout;
}

VMatrix VMatrix_transpose(VMatrix m) {
    VMatrix mout = {.gl = {
        m.v[0].x, m.v[1].x, m.v[2].x, m.v[3].x,
        m.v[0].y, m.v[1].y, m.v[2].y, m.v[3].y,
        m.v[0].z, m.v[1].z, m.v[2].z, m.v[3].z,
        m.v[0].w, m.v[1].w, m.v[2].w, m.v[3].w
    }};
    return mout;
}

VMatrix VMatrix_scale(VMatrix matrix, double x, double y, double z) {
    VMatrix scaled = {.gl = {
      matrix.gl[0] * x, matrix.gl[1] * x, matrix.gl[2] * x, matrix.gl[3] * x,
      matrix.gl[4] * y, matrix.gl[5] * y, matrix.gl[6] * y, matrix.gl[7] * y,
      matrix.gl[8] * z, matrix.gl[9] * z, matrix.gl[10] * z, matrix.gl[11] * z,
      matrix.gl[12], matrix.gl[13], matrix.gl[14], matrix.gl[15]
    }};
    return scaled;
}


VMatrix VMatrix_make_rotation(float radians, float x, float y, float z) {
    VVector3 v = {.v = {x, y, z}};
    v = VVector3_normalize(v);
    float cos = cosf(radians);
    float cosp = 1.0f - cos;
    float sin = sinf(radians);

    VMatrix m = {.gl = {
      cos + cosp * v.v[0] * v.v[0],
      cosp * v.v[0] * v.v[1] + v.v[2] * sin,
      cosp * v.v[0] * v.v[2] - v.v[1] * sin,
      0.0f,
      cosp * v.v[0] * v.v[1] - v.v[2] * sin,
      cos + cosp * v.v[1] * v.v[1],
      cosp * v.v[1] * v.v[2] + v.v[0] * sin,
      0.0f,
      cosp * v.v[0] * v.v[2] + v.v[1] * sin,
      cosp * v.v[1] * v.v[2] - v.v[0] * sin,
      cos + cosp * v.v[2] * v.v[2],
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      1.0f
    }};

    return m;
}

VMatrix VMatrix_rotate(VMatrix matrix, double rot_degs, double x, double y,
                       double z) {
    float radians =  rot_degs * M_PI / 180.0;
    VMatrix rotation = VMatrix_make_rotation(radians, x, y, z);
    VMatrix rotated = VMatrix_multiply(matrix, rotation);
    return rotated;
}

VMatrix VMatrix_translate(VMatrix matrix, double tx, double ty, double tz) {
    VMatrix xformed = {.gl = {
        matrix.gl[0], matrix.gl[1], matrix.gl[2], matrix.gl[3],
        matrix.gl[4], matrix.gl[5], matrix.gl[6], matrix.gl[7],
        matrix.gl[8], matrix.gl[9], matrix.gl[10], matrix.gl[11],
        matrix.gl[0] * tx + matrix.gl[4] * ty + matrix.gl[8] * tz + matrix.gl[12],
        matrix.gl[1] * tx + matrix.gl[5] * ty + matrix.gl[9] * tz + matrix.gl[13],
        matrix.gl[2] * tx + matrix.gl[6] * ty + matrix.gl[10] * tz + matrix.gl[14],
        matrix.gl[15]
    }};
    return xformed;
}

VMatrix VMatrix_make_ortho(float left, float right, float top,
                           float bottom, float near, float far) {
  VMatrix ortho = {.gl = {
    2 / (right - left),    0,                  0,                 -1 * (right + left) / (right - left),
    0,                     2 / (top - bottom), 0,                 -1 * (top + bottom) / (top - bottom),
    0,                     0,                  -2 / (far - near), far * near / (far - near),
    0,                     0,                  0,                 1
  }};
  return ortho;
}

VMatrix VMatrix_make_perspective(float fov_radians, float aspect, float near, float far)
{
  float cotan = 1.0f / tanf(fov_radians / 2.0f);
  
  VMatrix pers = {.gl = { cotan / aspect, 0.0f, 0.0f, 0.0f,
    0.0f, cotan, 0.0f, 0.0f,
    0.0f, 0.0f, (far + near) / (near - far), -1.0f,
    0.0f, 0.0f, (2.0f * far * near) / (near - far), 0.0f }};
  
  return pers;
}

VMatrix VMatrix_make_frustum(float left, float right, float top,
                             float bottom, float near, float far) {
  VMatrix frustum = {.gl = {
    2.0 * near / (right - left),     0,                               0,                                  0,
    0,                               2.0 * near / (top - bottom),     0,                                  0,
    (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near),       -1,
    0,                               0,                               (-2.0 * far * near) / (far - near), 0
  }};
  return frustum;
}

int VMatrix_is_equal(VMatrix a, VMatrix b) {
    int i = 0;
    for (i = 0; i < 16; i++) {
        if (!fequal(a.gl[i], b.gl[i])) return 0;
    }
    return 1;
}
