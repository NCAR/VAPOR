#pragma once

#ifndef INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
    #error LegacyVectorMath.h is deprecated. Please don't use unless necessary
#endif

#include <vapor/common.h>
#include <vector>
#include <math.h>

namespace VAPoR {

COMMON_API bool   powerOf2(size_t n);
COMMON_API size_t nextPowerOf2(size_t n);

COMMON_API void computeGradientData(int dim[3], int numChan, unsigned char *volume, unsigned char *gradient);
COMMON_API void makeModelviewMatrix(float *vpos, float *vdir, float *upvec, float *matrix);
COMMON_API void makeModelviewMatrixD(double *vpos, double *vdir, double *upvec, double *matrix);
COMMON_API void makeModelviewMatrixD(const std::vector<double> &vpos, const std::vector<double> &vdir, const std::vector<double> &upvec, double *matrix);
COMMON_API void makeTransMatrix(float *transVec, float *matrix);
COMMON_API void makeTransMatrix(double *transVec, double *matrix);
COMMON_API void makeTransMatrix(const std::vector<double> &transVec, double *matrix);
COMMON_API void makeScaleMatrix(const double *scaleVec, double *matrix);
COMMON_API void vscale(float *v, float s);
COMMON_API void vscale(double *v, double s);
COMMON_API void vscale(std::vector<double> v, double s);
COMMON_API void vmult(const float *v, float s, float *w);
COMMON_API void vmult(const double *v, double s, double *w);
COMMON_API void vhalf(const float *v1, const float *v2, float *half);
COMMON_API void vcross(const float *v1, const float *v2, float *cross);
COMMON_API void vcross(const double *v1, const double *v2, double *cross);
COMMON_API void vreflect(const float *in, const float *mirror, float *out);
COMMON_API void vtransform(const float *v, float *mat, float *vt);
COMMON_API void vtransform(const float *v, float *mat, double *vt);
COMMON_API void vtransform(const double *v, float *mat, double *vt);
COMMON_API void vtransform(const double *v, double *mat, double *vt);
COMMON_API void vtransform4(const float *v, float *mat, float *vt);
COMMON_API void vtransform3(const float *v, float *mat, float *vt);
COMMON_API void vtransform3(const double *v, double *mat, double *vt);
COMMON_API void vtransform3t(const float *v, float *mat, float *vt);
COMMON_API bool pointOnRight(double *pt1, double *pt2, const double *testPt);
COMMON_API void mcopy(float *m1, float *m2);
COMMON_API void mcopy(double *m1, double *m2);
COMMON_API void mmult(float *m1, float *m2, float *prod);
COMMON_API void mmult(double *m1, double *m2, double *prod);
COMMON_API int  minvert(const float *mat, float *result);
COMMON_API int  minvert(const double *mat, double *result);

// Some routines to handle 3x3 rotation matrices, represented as 9 floats,
// where the column index increments faster (like in 4x4 case
COMMON_API void mmult33(const double *m1, const double *m2, double *result);

// Same as above, but use the transpose (i.e. inverse for rotations) on the left
COMMON_API void mmultt33(const double *m1Trans, const double *m2, double *result);

// Determine a rotation matrix from (theta, phi, psi) (radians), that is,
// find the rotation matrix that first rotates in (x,y) by psi, then takes the vector (0,0,1)
// to the vector with direction (theta,phi) by rotating by phi in the (x,z) plane and then
// rotating in the (x,y)plane by theta.
COMMON_API void getRotationMatrix(double theta, double phi, double psi, double *matrix);

// Determine a rotation matrix about an axis:
COMMON_API void getAxisRotation(int axis, double rotation, double *matrix);

// Determine the psi, phi, theta from a rotation matrix:
COMMON_API void getRotAngles(double *theta, double *phi, double *psi, const double *matrix);
COMMON_API int  rayBoxIntersect(const float rayStart[3], const float rayDir[3], const float boxExts[6], float results[2]);
COMMON_API int  rayBoxIntersect(const double rayStart[3], const double rayDir[3], const double boxExts[6], double results[2]);
// Determine the minimum and maximum projection of box corners to ray
COMMON_API void rayBoxProject(std::vector<double> rayStart, std::vector<double> rayDir, const double boxExts[6], double results[2]);

COMMON_API void  qnormal(float *q);
COMMON_API void  qinv(const float q1[4], float q2[4]);
COMMON_API void  qmult(const float *q1, const float *q2, float *dest);
COMMON_API void  qmult(const double *q1, const double *q2, double *dest);
COMMON_API void  qmatrix(const float *q, float *m);
COMMON_API void  qmatrix(const double *q, double *m);
COMMON_API float ProjectToSphere(float r, float x, float y);
COMMON_API void  CalcRotation(float *q, float newX, float newY, float oldX, float oldY, float ballsize);
COMMON_API void  CalcRotation(double *q, double newX, double newY, double oldX, double oldY, double ballsize);
COMMON_API float ScalePoint(long pt, long origin, long size);
COMMON_API void  rvec2q(const float rvec[3], float radians, float q[4]);
COMMON_API void  rvec2q(const double rvec[3], double radians, double q[4]);
COMMON_API void  rotmatrix2q(float *m, float *q);
COMMON_API void  rotmatrix2q(double *m, double *q);
COMMON_API float getScale(float *rotmatrix);
COMMON_API void  view2Quat(float vdir[3], float upvec[3], float q[4]);
COMMON_API void  quat2View(float quat[4], float vdir[3], float upvec[3]);
COMMON_API void  qlog(float quat[4], float lquat[4]);
COMMON_API void  qconj(float quat[4], float conj[4]);

COMMON_API void slerp(float quat1[4], float quat2[4], float t, float result[4]);
COMMON_API void squad(float quat1[4], float quat2[4], float s1[4], float s2[4], float t, float result[4]);

COMMON_API void imagQuat2View(const float q[3], float vdir[3], float upvec[3]);

COMMON_API void views2ImagQuats(float vdir1[3], float upvec1[3], float vdir2[3], float upvec2[3], float q1[3], float q2[3]);

inline void vset(float *a, const float x, const float y, const float z)
{
    a[0] = x;
    a[1] = y;
    a[2] = z;
}
inline void vset(double *a, const double x, const double y, const double z)
{
    a[0] = x;
    a[1] = y;
    a[2] = z;
}
inline float  vdot(const float *a, const float *b) { return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]); }
inline float  vdot(const float *a, const double *b) { return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]); }
inline double vdot(const double *a, const double *b) { return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]); }
inline float  vlength(const float *a) { return sqrt(vdot(a, a)); }
inline double vlength(const double *a) { return sqrt(vdot(a, a)); }
inline double vlength(const std::vector<double> a) { return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]); }
inline float  vdist(const float *a, const float *b) { return (sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]))); }
inline double vdist(const double *a, const double *b) { return (sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]))); }
inline void   vnormal(float *a) { vscale(a, 1 / vlength(a)); }
inline void   vnormal(double *a) { vscale(a, 1 / vlength(a)); }
inline void   vnormal(std::vector<double> a) { vscale(a, 1. / vlength(a)); }
inline void   vcopy(const float *a, float *b)
{
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
}
inline void vcopy(const double *a, double *b)
{
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
}
inline void vsub(const double *a, const float *b, float *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}
inline void vsub(const float *a, const float *b, float *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}
inline void vsub(const double *a, const double *b, double *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}
inline void vsub(const double *a, const std::vector<double> b, double *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}
inline void vsub(const std::vector<double> &a, const std::vector<double> &b, double *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}
inline void vadd(const float *a, const float *b, float *c)
{
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}
inline void vadd(const double *a, const double *b, double *c)
{
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}
inline void vzero(float *a) { a[0] = a[1] = a[2] = 0.f; }
inline void vzero(double *a) { a[0] = a[1] = a[2] = 0.; }
inline void qset(float *a, float x, float y, float z, float w)
{
    a[0] = x;
    a[1] = y;
    a[2] = z;
    a[3] = w;
}
inline void qcopy(const float *a, float *b)
{
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
    b[3] = a[3];
}
inline void qcopy(const double *a, double *b)
{
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
    b[3] = a[3];
}
inline void qzero(float *a)
{
    a[0] = a[1] = a[2] = 0;
    a[3] = 1;
}
inline void qzero(double *a)
{
    a[0] = a[1] = a[2] = 0.;
    a[3] = 1.;
}
inline void  qadd(const float *a, const float *b, float *c) { vadd(a, b, c), c[3] = a[3] + b[3]; }
inline float qlength(const float q[4]) { return sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]); }

// Forward declarations for utility functions.
// These should really go in glutil!

int    matrix4x4_inverse(const float *in, float *out);
void   matrix4x4_vec3_mult(const float m[16], const float a[4], float b[4]);
void   adjoint(const float *in, float *out);
double det4x4(const float m[16]);
double det2x2(double a, double b, double c, double d);
double det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3);

};    // namespace VAPoR
