#ifndef __vmatrix_h
#define __vmatrix_h

#ifdef DABES_IOS
#include <GLKit/GLKMath.h>
#include <OpenGLES/ES2/gl.h>
#include <QuartzCore/CoreAnimation.h>
#endif
#ifdef DABES_MAC
#include <OpenGL/OpenGL.h>
#include <GLKit/GLKMath.h>
#include <QuartzCore/CATransform3D.h>
#endif

typedef union VVector3 {
  struct { float x, y, z; };
  struct { float r, g, b; };
  struct { float s, t, p; };
  float v[3];
} VVector3;

VVector3 VVector3_normalize(VVector3 vector);
float VVector3_length(VVector3 vector);

typedef union VVector4 {
  struct {
    float x;
    float y;
    float z;
    float w;
  };
  struct {
    float r;
    float g;
    float b;
    float a;
  };
  float raw[4];
} VVector4;

extern const VVector4 VVector4Zero;

static const VVector4 VVector4ClearColor = {.raw = {0.0, 0.0, 0.0, 0.0}};
static const VVector4 VVector4BlackColor = {.raw = {0.0, 0.0, 0.0, 1.0}};
static const VVector4 VVector4WhiteColor = {.raw = {1.0, 1.0, 1.0, 1.0}};

typedef union VMatrix {
    struct {
      float m11, m12, m13, m14;
      float m21, m22, m23, m24;
      float m31, m32, m33, m34;
      float m41, m42, m43, m44;
    } gfx;
    float gl[16];
    VVector4 v[4];
#if defined(DABES_IOS) || defined(DABES_MAC)
    CATransform3D ca;
#endif
#ifdef DABES_IOS
    GLKMatrix4 glk;
#endif
} VMatrix;

extern const VMatrix VMatrixIdentity;

VMatrix VMatrix_multiply(VMatrix a, VMatrix b);
VMatrix VMatrix_transpose(VMatrix m);
VMatrix VMatrix_scale(VMatrix matrix, double x, double y, double z);
VMatrix VMatrix_make_rotation(float radians, float x, float y, float z);
VMatrix VMatrix_rotate(VMatrix matrix, double rot_degs, double x, double y,
                       double z);
VMatrix VMatrix_translate(VMatrix matrix, double tx, double ty, double tz);
VMatrix VMatrix_make_ortho(float left, float right, float top,
                           float bottom, float near, float far);
VMatrix VMatrix_make_perspective(float fov_radians, float aspect, float near,
                                 float far);
VMatrix VMatrix_make_frustum(float left, float right, float top,
                             float bottom, float near, float far);
int VMatrix_is_equal(VMatrix a, VMatrix b);

#endif
