#ifndef __vmatrix_h
#define __vmatrix_h

#ifdef DABES_IOS
#include <GLKit/GLKMath.h>
#include <OpenGLES/ES2/gl.h>
#include <QuartzCore/CoreAnimation.h>
#endif

typedef union VVector3 {
  struct { float x, y, z; };
  struct { float r, g, b; };
  struct { float s, t, p; };
  float v[3];
} VVector3;

VVector3 VVector3_normalize(VVector3 vector);
float VVector3_length(VVector3 vector);

typedef union VMatrix {
    struct {
      float m11, m12, m13, m14;
      float m21, m22, m23, m24;
      float m31, m32, m33, m34;
      float m41, m42, m43, m44;
    } gfx;
    float gl[16];
#ifdef DABES_IOS
    CATransform3D ca;
    GLKMatrix4 glk;
#endif
} VMatrix;

extern const VMatrix VMatrixIdentity;

VMatrix VMatrix_multiply(VMatrix a, VMatrix b);
VMatrix VMatrix_scale(VMatrix matrix, double x, double y, double z);
VMatrix VMatrix_make_rotation(float radians, float x, float y, float z);
VMatrix VMatrix_rotate(VMatrix matrix, double rot_degs, double x, double y,
                       double z);
VMatrix VMatrix_translate(VMatrix matrix, double tx, double ty, double tz);
VMatrix VMatrix_make_ortho(float left, float right, float top,
                           float bottom, float near, float far);

#endif
