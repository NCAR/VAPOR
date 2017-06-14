//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		glutil.cpp
//
//	Adaptor:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2004
//
//	Description:  Methods to facilitate use of trackball navigation,
//		adapted from Ken Purcell's code to work in QT window
//
// Copyright (C) 1992  AHPCRC, Univeristy of Minnesota
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program in a file named 'Copying'; if not, write to
// the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139.
//

// Original Author:
//	Ken Chin-Purcell (ken@ahpcrc.umn.edu)
//	Army High Performance Computing Research Center (AHPCRC)
//	Univeristy of Minnesota
//

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cmath>
#include <iostream>
#include <cfloat>
#include <vapor/MyBase.h>
#include <vapor/errorcodes.h>
#include <vapor/glutil.h>

#include <GL/glew.h>

//#include "util.h"

/* The Identity matrix is useful for intializing transformations.
 */
namespace {

GLfloat idmatrix[16] = {
    1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
};
GLdouble idmatrixd[16] = {
    1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
};

};    // namespace

/* Most of the 'v' routines are in the form vsomething(src1, src2, dst),
 * dst can be one of the source vectors.
 */
namespace VAPoR {
void vscale(float *v, float s)
{
    /* Scale the vector v in all directions by s.
     */
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}
void vscale(double *v, double s)
{
    /* Scale the vector v in all directions by s.
     */
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}
void vscale(vector<double> v, double s)
{
    /* Scale the vector v in all directions by s.
     */
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}
// Scale, putting result in another vector
//
void vmult(const float *v, float s, float *w)
{
    w[0] = s * v[0];
    w[1] = s * v[1];
    w[2] = s * v[2];
}
// Scale, putting result in another vector
//
void vmult(const double *v, double s, double *w)
{
    w[0] = s * v[0];
    w[1] = s * v[1];
    w[2] = s * v[2];
}

void vhalf(const float *v1, const float *v2, float *half)
{
    /* Return in 'half' the unit vector
     * half way between v1 and v2.  half can be v1 or v2.
     */
    float len;

    vadd(v2, v1, half);
    len = vlength(half);
    if (len > 0)
        vscale(half, 1 / len);
    else
        vcopy(v1, half);
}

void vcross(const float *v1, const float *v2, float *cross)
{
    /* Vector cross product.
     */
    float temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    vcopy(temp, cross);
}
void vcross(const double *v1, const double *v2, double *cross)
{
    /* Vector cross product.
     */
    double temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    vcopy(temp, cross);
}

void vreflect(const float *in, const float *mirror, float *out)
{
    /* Mirror defines a plane across which in is reflected.
     */
    float temp[3];

    vcopy(mirror, temp);
    vscale(temp, vdot(mirror, in));
    vsub(temp, in, out);
    vadd(temp, out, out);
}

void vtransform(const float *v, GLfloat mat[12], float *vt)
{
    /* Vector transform in software...
     */
    float t[3];

    t[0] = v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2] + mat[3];
    t[1] = v[0] * mat[4] + v[1] * mat[5] + v[2] * mat[6] + mat[7];
    t[2] = v[0] * mat[8] + v[1] * mat[9] + v[2] * mat[10] + mat[11];
    vcopy(t, vt);
}
void vtransform(const float *v, GLfloat mat[12], double *vt)
{
    /* Vector transform in software...
     */
    double t[3];

    t[0] = v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2] + mat[3];
    t[1] = v[0] * mat[4] + v[1] * mat[5] + v[2] * mat[6] + mat[7];
    t[2] = v[0] * mat[8] + v[1] * mat[9] + v[2] * mat[10] + mat[11];
    vcopy(t, vt);
}
void vtransform(const double *v, GLfloat mat[12], double *vt)
{
    /* Vector transform in software...
     */
    double t[3];

    t[0] = v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2] + mat[3];
    t[1] = v[0] * mat[4] + v[1] * mat[5] + v[2] * mat[6] + mat[7];
    t[2] = v[0] * mat[8] + v[1] * mat[9] + v[2] * mat[10] + mat[11];
    vcopy(t, vt);
}

void vtransform(const double *v, GLdouble mat[12], double *vt)
{
    /* Vector transform in software...
     */
    double t[3];

    t[0] = v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2] + mat[3];
    t[1] = v[0] * mat[4] + v[1] * mat[5] + v[2] * mat[6] + mat[7];
    t[2] = v[0] * mat[8] + v[1] * mat[9] + v[2] * mat[10] + mat[11];
    vcopy(t, vt);
}
void vtransform4(const float *v, GLfloat *mat, float *vt)
{
    /* Homogeneous coordinates.
     */
    float t[4];

    t[0] = v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2] + mat[3];
    t[1] = v[0] * mat[4] + v[1] * mat[5] + v[2] * mat[6] + mat[7];
    t[2] = v[0] * mat[8] + v[1] * mat[9] + v[2] * mat[10] + mat[11];
    t[3] = v[0] * mat[12] + v[1] * mat[13] + v[2] * mat[14] + mat[15];
    qcopy(t, vt);
}
void vtransform3(const float *v, float *mat, float *vt)
{
    /* 3x3 matrix multiply
     */
    vt[0] = v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2];
    vt[1] = v[0] * mat[3] + v[1] * mat[4] + v[2] * mat[5];
    vt[2] = v[0] * mat[6] + v[1] * mat[7] + v[2] * mat[8];
}
void vtransform3(const double *v, double *mat, double *vt)
{
    /* 3x3 matrix multiply
     */
    vt[0] = v[0] * mat[0] + v[1] * mat[1] + v[2] * mat[2];
    vt[1] = v[0] * mat[3] + v[1] * mat[4] + v[2] * mat[5];
    vt[2] = v[0] * mat[6] + v[1] * mat[7] + v[2] * mat[8];
}
void vtransform3t(const float *v, float *mat, float *vt)
{
    /* 3x3 matrix multiply, using transpose of matrix
     */
    vt[0] = v[0] * mat[0] + v[1] * mat[3] + v[2] * mat[6];
    vt[1] = v[0] * mat[1] + v[1] * mat[4] + v[2] * mat[7];
    vt[2] = v[0] * mat[2] + v[1] * mat[5] + v[2] * mat[8];
}
// Test whether a planar point is right (or left) of the oriented line from
// pt1 to pt2
bool pointOnRight(double *pt1, double *pt2, double *testPt)
{
    float rhs = pt1[0] * (pt1[1] - pt2[1]) + pt1[1] * (pt2[0] - pt1[0]);
    float test = (pt2[0] - pt1[0]) * testPt[1] + (pt1[1] - pt2[1]) * testPt[0] - rhs;
    return (test < 0.f);
}

void mcopy(GLfloat *m1, GLfloat *m2)
{
    /* Copy a 4x4 matrix
     */
    int row;

    for (row = 0; row < 16; row++) m2[row] = m1[row];
}
void mcopy(double *m1, double *m2)
{
    /* Copy a 4x4 matrix
     */
    int row;

    for (row = 0; row < 16; row++) m2[row] = m1[row];
}

void mmult(GLfloat *m1, GLfloat *m2, GLfloat *prod)
{
    /* Multiply two 4x4 matricies
     */
    int     row, col;
    GLfloat temp[16];

    /*for(row = 0 ; row < 4 ; row++)
       for(col = 0 ; col < 4 ; col++)
        temp[row + col*4] = (m1[row]   * m2[col*4] +
                m1[row+4]  * m2[1+col*4] +
                m1[row+8]  * m2[2+col*4] +
                m1[row+12] * m2[3+col*4]);*/
    /*
     * Use OpenGL style matrix mult -- Wes.
     */
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++) temp[row * 4 + col] = (m1[row * 4] * m2[col] + m1[1 + row * 4] * m2[col + 4] + m1[2 + row * 4] * m2[col + 8] + m1[3 + row * 4] * m2[col + 12]);
    mcopy(temp, prod);
}
void mmult(GLdouble *m1, GLdouble *m2, GLdouble *prod)
{
    /* Multiply two 4x4 matricies
     */
    int      row, col;
    GLdouble temp[16];

    /*
     * Use OpenGL style matrix mult -- Wes.
     */
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++) temp[row * 4 + col] = (m1[row * 4] * m2[col] + m1[1 + row * 4] * m2[col + 4] + m1[2 + row * 4] * m2[col + 8] + m1[3 + row * 4] * m2[col + 12]);
    mcopy(temp, prod);
}

// 4x4 matrix inversion.  Fixed (AN 4/07) so that it doesn't try to divide by very small
// pivot elements.  Returns 0 if not invertible
int minvert(GLfloat *mat, GLfloat *result)
{
    // Invert a 4x4 matrix

    int   i, j, k;
    float temp;
    float m[8][4];

    mcopy(idmatrix, result);
    // mat[i,j] is row j, col i:
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m[i][j] = mat[i + 4 * j];
            m[i + 4][j] = result[i + 4 * j];
        }
    }

    // Work across by columns (i is col index):

    for (i = 0; i < 4; i++) {
        // Find largest entry in the column, below the diagonal:
        float maxval = 0.f;
        int   pivot = -1;
        for (int rw = i; rw < 4; rw++) {
            if (fabs(m[i][rw]) > maxval) {
                maxval = fabs(m[i][rw]);
                pivot = rw;
            }
        }
        if (pivot < 0) return -1;    // otherwise, can't invert!

        if (pivot != i) {    // Swap i and pivot row:
            for (k = i; k < 8; k++) {
                temp = m[k][i];
                m[k][i] = m[k][pivot];
                m[k][pivot] = temp;
            }
        }

        // Divide original row by pivot element, which is now the [i][i] element:

        for (j = 7; j >= i; j--) m[j][i] /= m[i][i];

        // Subtract other rows, to make row i be the only row with nonzero elt in col i:

        for (j = 0; j < 4; j++)
            if (i != j)
                for (k = 7; k >= i; k--) m[k][j] -= m[k][i] * m[i][j];
    }
    // copy back the last 4 columns:
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) result[i + 4 * j] = m[i + 4][j];
    return 0;
}
int minvert(GLdouble *mat, GLdouble *result)
{
    // Invert a 4x4 matrix

    int    i, j, k;
    double temp;
    double m[8][4];

    mcopy(idmatrixd, result);
    // mat[i,j] is row j, col i:
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m[i][j] = mat[i + 4 * j];
            m[i + 4][j] = result[i + 4 * j];
        }
    }

    // Work across by columns (i is col index):

    for (i = 0; i < 4; i++) {
        // Find largest entry in the column, below the diagonal:
        double maxval = 0.f;
        int    pivot = -1;
        for (int rw = i; rw < 4; rw++) {
            if (abs(m[i][rw]) > maxval) {
                maxval = abs(m[i][rw]);
                pivot = rw;
            }
        }
        if (pivot < 0) return -1;    // otherwise, can't invert!

        if (pivot != i) {    // Swap i and pivot row:
            for (k = i; k < 8; k++) {
                temp = m[k][i];
                m[k][i] = m[k][pivot];
                m[k][pivot] = temp;
            }
        }

        // Divide original row by pivot element, which is now the [i][i] element:

        for (j = 7; j >= i; j--) m[j][i] /= m[i][i];

        // Subtract other rows, to make row i be the only row with nonzero elt in col i:

        for (j = 0; j < 4; j++)
            if (i != j)
                for (k = 7; k >= i; k--) m[k][j] -= m[k][i] * m[i][j];
    }
    // copy back the last 4 columns:
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) result[i + 4 * j] = m[i + 4][j];
    return 0;
}

void qnormal(float *q)
{
    /* Normalize a quaternion
     */
    float s;

    s = 1 / sqrt((double)(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]));
    q[0] *= s;
    q[1] *= s;
    q[2] *= s;
    q[3] *= s;
}
void qinv(const float q1[4], float q2[4])
{
    // Inverse of quaternion is conjugate/norm-square.  4th coeff is real part!
    float mag = q1[0] * q1[0] + q1[1] * q1[1] + q1[2] * q1[2] + q1[3] * q1[3];
    assert(mag > 0.f);
    for (int i = 0; i < 3; i++) q2[i] = -q1[i] / mag;
    q2[3] = q1[3] / mag;
    float reslt[4];
    qmult(q1, q2, reslt);
}

void qmult(const float *q1, const float *q2, float *dest)
{
    /* Multiply two quaternions.  Note quaternion real part is 4th coefficient!
     */
    // static int	count = 0;
    float t1[3], t2[3], t3[3];
    float tf[4];

    vcopy(q1, t1);
    vscale(t1, q2[3]);

    vcopy(q2, t2);
    vscale(t2, q1[3]);

    vcross(q2, q1, t3);
    vadd(t1, t2, tf);
    vadd(t3, tf, tf);
    tf[3] = q1[3] * q2[3] - vdot(q1, q2);

    qcopy(tf, dest);
    // why is this code here?
    // if (++count >= 97) {
    //	count = 0;
    //	qnormal(dest);
    //}
}

void qmult(const double *q1, const double *q2, double *dest)
{
    /* Multiply two quaternions.  Note quaternion real part is 4th coefficient!
     */
    // static int	count = 0;
    double t1[3], t2[3], t3[3];
    double tf[4];

    vcopy(q1, t1);
    vscale(t1, q2[3]);

    vcopy(q2, t2);
    vscale(t2, q1[3]);

    vcross(q2, q1, t3);
    vadd(t1, t2, tf);
    vadd(t3, tf, tf);
    tf[3] = q1[3] * q2[3] - vdot(q1, q2);

    qcopy(tf, dest);
    // why is this code here?
    // if (++count >= 97) {
    //	count = 0;
    //	qnormal(dest);
    //}
}
void qmatrix(const float *q, GLfloat *m)
{
    /* Build a rotation matrix, given a quaternion rotation.
     */
    m[0] = 1 - 2 * (q[1] * q[1] + q[2] * q[2]);
    m[1] = 2 * (q[0] * q[1] - q[2] * q[3]);
    m[2] = 2 * (q[2] * q[0] + q[1] * q[3]);
    m[3] = 0;

    m[4] = 2 * (q[0] * q[1] + q[2] * q[3]);
    m[5] = 1 - 2 * (q[2] * q[2] + q[0] * q[0]);
    m[6] = 2 * (q[1] * q[2] - q[0] * q[3]);
    m[7] = 0;

    m[8] = 2 * (q[2] * q[0] - q[1] * q[3]);
    m[9] = 2 * (q[1] * q[2] + q[0] * q[3]);
    m[10] = 1 - 2 * (q[1] * q[1] + q[0] * q[0]);
    m[11] = 0;

    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}
void qmatrix(const double *q, GLdouble *m)
{
    /* Build a rotation matrix, given a quaternion rotation.
     */
    m[0] = 1 - 2 * (q[1] * q[1] + q[2] * q[2]);
    m[1] = 2 * (q[0] * q[1] - q[2] * q[3]);
    m[2] = 2 * (q[2] * q[0] + q[1] * q[3]);
    m[3] = 0;

    m[4] = 2 * (q[0] * q[1] + q[2] * q[3]);
    m[5] = 1 - 2 * (q[2] * q[2] + q[0] * q[0]);
    m[6] = 2 * (q[1] * q[2] - q[0] * q[3]);
    m[7] = 0;

    m[8] = 2 * (q[2] * q[0] - q[1] * q[3]);
    m[9] = 2 * (q[1] * q[2] + q[0] * q[3]);
    m[10] = 1 - 2 * (q[1] * q[1] + q[0] * q[0]);
    m[11] = 0;

    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}
/*  Convert a 4x4 rotation matrix to a quaternion.  q[3] is real part!
    Code adapted from
    http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion
*/
#define M_EPSILON 0.00000001
void rotmatrix2q(float *m, float *q)
{
    float trace = m[0] + m[5] + m[10] + 1.0f;
    if (trace > M_EPSILON) {
        float s = 0.5f / sqrtf(trace);
        q[3] = 0.25f / s;
        q[0] = (m[9] - m[6]) * s;
        q[1] = (m[2] - m[8]) * s;
        q[2] = (m[4] - m[1]) * s;
    } else {
        if (m[0] > m[5] && m[0] > m[10]) {
            float s = 2.0f * sqrtf(1.0f + m[0] - m[5] - m[10]);
            q[0] = 0.25f * s;
            q[1] = (m[1] + m[4]) / s;
            q[2] = (m[2] + m[8]) / s;
            q[3] = (m[6] - m[9]) / s;    //???? minus?
        } else if (m[5] > m[10]) {
            float s = 2.0f * sqrtf(1.0f + m[5] - m[0] - m[10]);
            q[0] = (m[1] + m[4]) / s;
            q[1] = 0.25f * s;
            q[2] = (m[6] + m[9]) / s;
            q[3] = (m[2] - m[8]) / s;
        } else {
            float s = 2.0f * sqrtf(1.0f + m[10] - m[0] - m[5]);
            q[0] = (m[2] + m[8]) / s;
            q[1] = (m[6] + m[9]) / s;
            q[2] = 0.25f * s;
            q[3] = (m[1] - m[4]) / s;    //?? off by minus??
        }
    }
}

void rotmatrix2q(double *m, double *q)
{
    double trace = m[0] + m[5] + m[10] + 1.0;
    if (trace > M_EPSILON) {
        double s = 0.5 / sqrt(trace);
        q[3] = 0.25 / s;
        q[0] = (m[9] - m[6]) * s;
        q[1] = (m[2] - m[8]) * s;
        q[2] = (m[4] - m[1]) * s;
    } else {
        if (m[0] > m[5] && m[0] > m[10]) {
            double s = 2.0 * sqrt(1.0 + m[0] - m[5] - m[10]);
            q[0] = 0.25 * s;
            q[1] = (m[1] + m[4]) / s;
            q[2] = (m[2] + m[8]) / s;
            q[3] = (m[6] - m[9]) / s;    //???? minus?
        } else if (m[5] > m[10]) {
            double s = 2.0 * sqrt(1.0 + m[5] - m[0] - m[10]);
            q[0] = (m[1] + m[4]) / s;
            q[1] = 0.25 * s;
            q[2] = (m[6] + m[9]) / s;
            q[3] = (m[2] - m[8]) / s;
        } else {
            double s = 2.0 * sqrt(1.0 + m[10] - m[0] - m[5]);
            q[0] = (m[2] + m[8]) / s;
            q[1] = (m[6] + m[9]) / s;
            q[2] = 0.25 * s;
            q[3] = (m[1] - m[4]) / s;    //?? off by minus??
        }
    }
}

void rvec2q(const float rvec[3], float radians, float q[4])
{
    double rvec_normal[3];
    double d;

    d = sqrt(rvec[0] * rvec[0] + rvec[1] * rvec[1] + rvec[2] * rvec[2]);

    if (d != 0.0) {
        rvec_normal[0] = rvec[0] / d;
        rvec_normal[1] = rvec[1] / d;
        rvec_normal[2] = rvec[2] / d;
    } else {
        rvec_normal[0] = 0.0;
        rvec_normal[1] = 0.0;
        rvec_normal[2] = 1.0;
    }

    q[0] = sin(radians / 2.0) * rvec_normal[0];
    q[1] = sin(radians / 2.0) * rvec_normal[1];
    q[2] = sin(radians / 2.0) * rvec_normal[2];
    q[3] = cos(radians / 2.0);
}
void rvec2q(const double rvec[3], double radians, double q[4])
{
    double rvec_normal[3];
    double d;

    d = sqrt(rvec[0] * rvec[0] + rvec[1] * rvec[1] + rvec[2] * rvec[2]);

    if (d != 0.0) {
        rvec_normal[0] = rvec[0] / d;
        rvec_normal[1] = rvec[1] / d;
        rvec_normal[2] = rvec[2] / d;
    } else {
        rvec_normal[0] = 0.0;
        rvec_normal[1] = 0.0;
        rvec_normal[2] = 1.0;
    }

    q[0] = sin(radians / 2.0) * rvec_normal[0];
    q[1] = sin(radians / 2.0) * rvec_normal[1];
    q[2] = sin(radians / 2.0) * rvec_normal[2];
    q[3] = cos(radians / 2.0);
}

float ProjectToSphere(float r, float x, float y)
{
    /* Project an x,y pair onto a sphere of radius r or a hyperbolic sheet
     * if we are away from the center of the sphere.
     *
     * On sphere, 	x*x + y*y + z*z = r*r
     * On hyperbola, 	sqrt(x*x + y*y) * z = 1/2 r*r
     * Tangent at	z = r / sqrt(2)
     */
    float dd, tt;

    dd = x * x + y * y;
    tt = r * r * 0.5;

    if (dd < tt)
        return sqrt((double)(r * r - dd)); /* Inside sphere */
    else
        return tt / sqrt((double)dd); /* On hyperbola */
}

void CalcRotation(float *q, float newX, float newY, float oldX, float oldY, float ballsize)
{
    /* Given old and new mouse positions (scaled to [-1, 1]),
     * Find the rotation quaternion q.
     */
    float p1[3], p2[3]; /* 3D mouse points  */
    float L;            /* sin^2(2 * phi)   */

    /* Check for zero rotation
     */
    if (newX == oldX && newY == oldY) {
        qzero(q);
        return;
    }

    /* Form two vectors based on input points, find rotation axis
     */
    vset(p1, newX, newY, ProjectToSphere(ballsize, newX, newY));
    vset(p2, oldX, oldY, ProjectToSphere(ballsize, oldX, oldY));

    vcross(p1, p2, q); /* axis of rotation from p1 and p2 */

    L = vdot(q, q) / (vdot(p1, p1) * vdot(p2, p2));
    L = sqrt((double)(1 - L));

    vnormal(q);                             /* q' = axis of rotation */
    vscale(q, sqrt((double)((1 - L) / 2))); /* q' = q' * sin(phi) */
    q[3] = sqrt((double)((1 + L) / 2));     /* qs = qs * cos(phi) */
}
void CalcRotation(double *q, double newX, double newY, double oldX, double oldY, double ballsize)
{
    /* Given old and new mouse positions (scaled to [-1, 1]),
     * Find the rotation quaternion q.
     */
    double p1[3], p2[3]; /* 3D mouse points  */
    double L;            /* sin^2(2 * phi)   */

    /* Check for zero rotation
     */
    if (newX == oldX && newY == oldY) {
        qzero(q);
        return;
    }

    /* Form two vectors based on input points, find rotation axis
     */
    vset(p1, newX, newY, ProjectToSphere(ballsize, newX, newY));
    vset(p2, oldX, oldY, ProjectToSphere(ballsize, oldX, oldY));

    vcross(p1, p2, q); /* axis of rotation from p1 and p2 */

    L = vdot(q, q) / (vdot(p1, p1) * vdot(p2, p2));
    L = sqrt((double)(1 - L));

    vnormal(q);                             /* q' = axis of rotation */
    vscale(q, sqrt((double)((1 - L) / 2))); /* q' = q' * sin(phi) */
    q[3] = sqrt((double)((1 + L) / 2));     /* qs = qs * cos(phi) */
}

float ScalePoint(long pt, long origin, long size)
{
    /* Scales integer point to the range [-1, 1]
     */
    float x;

    x = (float)(pt - origin) / (float)size;
    if (x < 0) x = 0;
    if (x > 1) x = 1;

    return 2 * x - 1;
}

void ViewMatrix(GLfloat *m)
{
    /* Return the total view matrix, including any
     * projection transformation.
     */

    GLfloat mp[16], mv[16];
    glGetFloatv(GL_PROJECTION_MATRIX, mp);
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    mmult(mv, mp, m);
}

int ViewAxis(int *direction)
{
    /* Return the major axis the viewer is looking down.
     * 'direction' indicates which direction down the axis.
     */
    GLfloat view[16];
    int     axis;

    ViewMatrix(view);

    /* The trick is to look down the z column for the largest value.
     * The total view matrix seems to be left hand coordinate
     */

    /*if (fabs((double) view[9]) > fabs((double) view[8]))*/
    if (fabs((double)view[6]) > fabs((double)view[2]))
        axis = 1;
    else
        axis = 0;
    /*if (fabs((double) view[10]) > fabs((double) view[axis+8]))*/
    if (fabs((double)view[10]) > fabs((double)view[2 + axis * 4])) axis = 2;

    if (direction) *direction = view[2 + axis * 4] > 0 ? -1 : 1;
    /**direction = view[axis+8] > 0 ? -1 : 1;*/

    return axis;
}

void StereoPerspective(int fovy, float aspect, float neardist, float fardist, float converge, float eye)
{
    /* The first four arguements act like the perspective command
     * of the gl.  converge is the plane of the screen, and eye
     * is the eye distance from the centerline.
     *
     * Sample values: 320, ???, 0.1, 10.0, 3.0, 0.12
     */
    float left, right, top, bottom;
    float gltan;
    GLint mm;
    glGetIntegerv(GL_MATRIX_MODE, &mm);

    glMatrixMode(GL_PROJECTION);

    gltan = tan((double)(fovy / 2.0 / 10.0 * (M_PI / 180.0)));
    top = gltan * neardist;
    bottom = -top;

    gltan = tan((double)(fovy * aspect / 2.0 / 10.0 * M_PI / 180.0));
    left = -gltan * neardist - eye / converge * neardist;
    right = gltan * neardist - eye / converge * neardist;

    glLoadIdentity();
    glFrustum(left, right, bottom, top, neardist, fardist);
    glTranslatef(-eye, 0.0, 0.0);

    glMatrixMode(mm);
}
/* Make a translation matrix from a vector:
 */
void makeTransMatrix(float *trans, float *mtrx)
{
    for (int i = 0; i < 12; i++) mtrx[i] = 0.f;
    mtrx[0] = 1.f;
    mtrx[5] = 1.f;
    mtrx[10] = 1.f;
    mtrx[15] = 1.f;
    vcopy(trans, mtrx + 12);
}
void makeTransMatrix(double *trans, double *mtrx)
{
    for (int i = 0; i < 12; i++) mtrx[i] = 0.;
    mtrx[0] = 1.;
    mtrx[5] = 1.;
    mtrx[10] = 1.;
    mtrx[15] = 1.;
    for (int i = 0; i < 3; i++) mtrx[i + 12] = (double)trans[i];
}
void makeTransMatrix(const std::vector<double> &trans, double *mtrx)
{
    for (int i = 0; i < 12; i++) mtrx[i] = 0.;
    mtrx[0] = 1.;
    mtrx[5] = 1.;
    mtrx[10] = 1.;
    mtrx[15] = 1.;
    for (int i = 0; i < 3; i++) mtrx[i + 12] = trans[i];
}
/*
 * make a modelview matrix from viewer position, direction, and up vector
 * Vectors must be nonzero
 * side-effect:  will alter input values if not valid.
 */
void makeModelviewMatrix(float *vpos, float *vdir, float *upvec, float *mtrx)
{
    float vtemp[3];
    float left[3] = {-1.f, 0.f, 0.f};
    float ydir[3] = {0.f, 1.f, 0.f};
    float right[3];
    // Normalize the vectors:
    vnormal(upvec);
    vnormal(vdir);
    // Force the up vector to be orthogonal to viewDir
    vcopy(vdir, vtemp);
    vscale(vtemp, vdot(vdir, upvec));
    // Subtract the component of up in the viewdir direction
    vsub(upvec, vtemp, upvec);
    // Make sure it's still valid
    if (vdot(upvec, upvec) == 0.f) {
        // First try up = viewdir x left
        vcross(vdir, left, upvec);
        if (vdot(upvec, upvec) == 0.f) {
            // try viewdir x ydir
            vcross(vdir, ydir, upvec);
        }
    }
    vnormal(upvec);
    // calculate "right" vector:
    vcross(vdir, upvec, right);
    // Construct matrix:
    GLfloat minv[16];
    // Fill in bottom row:
    minv[3] = 0.f;
    minv[7] = 0.f;
    minv[11] = 0.f;
    minv[15] = 1.f;
    // copy in first 3 elements of columns
    vcopy(right, minv);
    vcopy(upvec, minv + 4);
    // third col is neg of viewdir

    vcopy(vdir, minv + 8);
    vscale(minv + 8, -1.f);
    vcopy(vpos, minv + 12);
    int rc = minvert(minv, mtrx);
    assert(rc >= 0);    // Only catch this in debug mode
}
/*
 * make a modelview matrix from viewer position, direction, and up vector
 * Vectors must be nonzero
 * side-effect:  will alter input values if not valid.
 */
void makeModelviewMatrixD(double *vpos, double *vdir, double *upvec, double *mtrx)
{
    double vtemp[3];
    double left[3] = {-1.f, 0.f, 0.f};
    double ydir[3] = {0.f, 1.f, 0.f};
    double right[3];
    double dupvec[3], dvdir[3], dvpos[3];
    for (int i = 0; i < 3; i++) {
        dupvec[i] = upvec[i];
        dvdir[i] = vdir[i];
        dvpos[i] = vpos[i];
    }

    // Normalize the vectors:
    vnormal(dupvec);
    vnormal(dvdir);
    // Force the up vector to be orthogonal to viewDir
    vcopy(dvdir, vtemp);
    vscale(vtemp, vdot(dvdir, dupvec));
    // Subtract the component of up in the viewdir direction
    vsub(dupvec, vtemp, dupvec);
    // Make sure it's still valid
    if (vdot(dupvec, dupvec) == 0.f) {
        // First try up = viewdir x left
        vcross(dvdir, left, dupvec);
        if (vdot(dupvec, dupvec) == 0.f) {
            // try viewdir x ydir
            vcross(dvdir, ydir, dupvec);
        }
    }
    vnormal(dupvec);
    // calculate "right" vector:
    vcross(dvdir, dupvec, right);
    // Construct matrix:
    double minv[16];
    // Fill in bottom row:
    minv[3] = 0.;
    minv[7] = 0.;
    minv[11] = 0.;
    minv[15] = 1.;
    // copy in first 3 elements of columns
    vcopy(right, minv);
    vcopy(dupvec, minv + 4);
    // third col is neg of viewdir

    vcopy(dvdir, minv + 8);
    vscale(minv + 8, -1.);
    vcopy(dvpos, minv + 12);
    int rc = minvert(minv, mtrx);
    assert(rc >= 0);    // Only catch this in debug mode
}
/*
 * make a modelview matrix from viewer position, direction, and up vector
 * Vectors must be nonzero
 * side-effect:  will alter input values if not valid.
 */
void makeModelviewMatrixD(const std::vector<double> &vpos, const std::vector<double> &vdir, const std::vector<double> &upvec, double *mtrx)
{
    double vtemp[3];
    double left[3] = {-1.f, 0.f, 0.f};
    double ydir[3] = {0.f, 1.f, 0.f};
    double right[3];
    double dupvec[3], dvdir[3], dvpos[3];
    for (int i = 0; i < 3; i++) {
        dupvec[i] = upvec[i];
        dvdir[i] = vdir[i];
        dvpos[i] = vpos[i];
    }

    // Normalize the vectors:
    vnormal(dupvec);
    vnormal(dvdir);
    // Force the up vector to be orthogonal to viewDir
    vcopy(dvdir, vtemp);
    vscale(vtemp, vdot(dvdir, dupvec));
    // Subtract the component of up in the viewdir direction
    vsub(dupvec, vtemp, dupvec);
    // Make sure it's still valid
    if (vdot(dupvec, dupvec) == 0.f) {
        // First try up = viewdir x left
        vcross(dvdir, left, dupvec);
        if (vdot(dupvec, dupvec) == 0.f) {
            // try viewdir x ydir
            vcross(dvdir, ydir, dupvec);
        }
    }
    vnormal(dupvec);
    // calculate "right" vector:
    vcross(dvdir, dupvec, right);
    // Construct matrix:
    double minv[16];
    // Fill in bottom row:
    minv[3] = 0.;
    minv[7] = 0.;
    minv[11] = 0.;
    minv[15] = 1.;
    // copy in first 3 elements of columns
    vcopy(right, minv);
    vcopy(dupvec, minv + 4);
    // third col is neg of viewdir

    vcopy(dvdir, minv + 8);
    vscale(minv + 8, -1.);
    vcopy(dvpos, minv + 12);
    int rc = minvert(minv, mtrx);
    assert(rc >= 0);    // Only catch this in debug mode
}
void matrix4x4_vec3_mult(const GLfloat m[16], const GLfloat a[4], GLfloat b[4])
{
    b[0] = m[0] * a[0] + m[4] * a[1] + m[8] * a[2] + m[12] * a[3];
    b[1] = m[1] * a[0] + m[5] * a[1] + m[9] * a[2] + m[13] * a[3];
    b[2] = m[2] * a[0] + m[6] * a[1] + m[10] * a[2] + m[14] * a[3];
    b[3] = m[3] * a[0] + m[7] * a[1] + m[11] * a[2] + m[15] * a[3];

    if (b[3] != 0.0 && b[3] != 1.0) {
        b[0] /= b[3];
        b[1] /= b[3];
        b[2] /= b[3];
        b[3] = 1.0;
    }
}

//
// Matrix Inversion
// Adapted from Richard Carling
// from "Graphics Gems", Academic Press, 1990
//

#define SMALL_NUMBER 1.e-8
/*
 *   matrix4x4_inverse( original_matrix, inverse_matrix )
 *
 *    calculate the inverse of a 4x4 matrix
 *
 *     -1
 *     A  = ___1__ adjoint A
 *         det A
 */

int matrix4x4_inverse(const GLfloat *in, GLfloat *out)
{
    int    i, j;
    double det;
    double det4x4(const GLfloat m[16]);
    void   adjoint(const GLfloat *in, GLfloat *out);

    /* calculate the adjoint matrix */

    adjoint(in, out);

    /*  calculate the 4x4 determinant
     *  if the determinant is zero,
     *  then the inverse matrix is not unique.
     */

    det = det4x4(in);

    if (fabs(det) < SMALL_NUMBER) {
        //	Singular matrix, no inverse!
        return (-1);
    }

    /* scale the adjoint matrix to get the inverse */

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) { out[i * 4 + j] = out[i * 4 + j] / det; }
    }
    return (0);
}

/*
 *   adjoint( original_matrix, inverse_matrix )
 *
 *     calculate the adjoint of a 4x4 matrix
 *
 *      Let  a   denote the minor determinant of matrix A obtained by
 *           ij
 *
 *      deleting the ith row and jth column from A.
 *
 *                    i+j
 *     Let  b   = (-1)    a
 *          ij            ji
 *
 *    The matrix B = (b  ) is the adjoint of A
 *                     ij
 */

void adjoint(const GLfloat *in, GLfloat *out)
{
    double a1, a2, a3, a4, b1, b2, b3, b4;
    double c1, c2, c3, c4, d1, d2, d3, d4;
    double det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3);

    /* assign to individual variable names to aid  */
    /* selecting correct values  */

    a1 = in[0 * 4 + 0];
    b1 = in[0 * 4 + 1];
    c1 = in[0 * 4 + 2];
    d1 = in[0 * 4 + 3];

    a2 = in[1 * 4 + 0];
    b2 = in[1 * 4 + 1];
    c2 = in[1 * 4 + 2];
    d2 = in[1 * 4 + 3];

    a3 = in[2 * 4 + 0];
    b3 = in[2 * 4 + 1];
    c3 = in[2 * 4 + 2];
    d3 = in[2 * 4 + 3];

    a4 = in[3 * 4 + 0];
    b4 = in[3 * 4 + 1];
    c4 = in[3 * 4 + 2];
    d4 = in[3 * 4 + 3];

    /* row column labeling reversed since we transpose rows & columns */

    out[0 * 4 + 0] = det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4);
    out[1 * 4 + 0] = -det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4);
    out[2 * 4 + 0] = det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4);
    out[3 * 4 + 0] = -det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);

    out[0 * 4 + 1] = -det3x3(b1, b3, b4, c1, c3, c4, d1, d3, d4);
    out[1 * 4 + 1] = det3x3(a1, a3, a4, c1, c3, c4, d1, d3, d4);
    out[2 * 4 + 1] = -det3x3(a1, a3, a4, b1, b3, b4, d1, d3, d4);
    out[3 * 4 + 1] = det3x3(a1, a3, a4, b1, b3, b4, c1, c3, c4);

    out[0 * 4 + 2] = det3x3(b1, b2, b4, c1, c2, c4, d1, d2, d4);
    out[1 * 4 + 2] = -det3x3(a1, a2, a4, c1, c2, c4, d1, d2, d4);
    out[2 * 4 + 2] = det3x3(a1, a2, a4, b1, b2, b4, d1, d2, d4);
    out[3 * 4 + 2] = -det3x3(a1, a2, a4, b1, b2, b4, c1, c2, c4);

    out[0 * 4 + 3] = -det3x3(b1, b2, b3, c1, c2, c3, d1, d2, d3);
    out[1 * 4 + 3] = det3x3(a1, a2, a3, c1, c2, c3, d1, d2, d3);
    out[2 * 4 + 3] = -det3x3(a1, a2, a3, b1, b2, b3, d1, d2, d3);
    out[3 * 4 + 3] = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
}
/*
 * double = det4x4( matrix )
 *
 * calculate the determinant of a 4x4 matrix.
 */
double det4x4(const GLfloat m[16])
{
    double ans;
    double a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

    double det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3);

    /* assign to individual variable names to aid selecting */
    /*  correct elements */

    a1 = m[0 * 4 + 0];
    b1 = m[0 * 4 + 1];
    c1 = m[0 * 4 + 2];
    d1 = m[0 * 4 + 3];

    a2 = m[1 * 4 + 0];
    b2 = m[1 * 4 + 1];
    c2 = m[1 * 4 + 2];
    d2 = m[1 * 4 + 3];

    a3 = m[2 * 4 + 0];
    b3 = m[2 * 4 + 1];
    c3 = m[2 * 4 + 2];
    d3 = m[2 * 4 + 3];

    a4 = m[3 * 4 + 0];
    b4 = m[3 * 4 + 1];
    c4 = m[3 * 4 + 2];
    d4 = m[3 * 4 + 3];

    ans = a1 * det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4) - b1 * det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4) + c1 * det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4)
        - d1 * det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);
    return ans;
}

/*
 * double = det3x3(  a1, a2, a3, b1, b2, b3, c1, c2, c3 )
 *
 * calculate the determinant of a 3x3 matrix
 * in the form
 *
 *     | a1,  b1,  c1 |
 *     | a2,  b2,  c2 |
 *     | a3,  b3,  c3 |
 */

double det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3)
{
    double ans;
    double det2x2(double a, double b, double c, double d);

    ans = a1 * det2x2(b2, b3, c2, c3) - b1 * det2x2(a2, a3, c2, c3) + c1 * det2x2(a2, a3, b2, b3);
    return ans;
}

/*
 * double = det2x2( double a, double b, double c, double d )
 *
 * calculate the determinant of a 2x2 matrix.
 */

double det2x2(double a, double b, double c, double d)
{
    double ans;
    ans = a * d - b * c;
    return ans;
}

bool oglStatusOK(vector<int> &status)
{
    GLenum glErr;

    while ((glErr = glGetError()) != GL_NO_ERROR) { status.push_back((int)glErr); }
    if (status.size()) return (false);

    return (true);
}

string oglGetErrMsg(vector<int> status)
{
    string msg;
    for (int i = 0; i < status.size(); i++) {
        char *s = (char *)gluErrorString((GLenum)status[i]);
        msg += (string)s;
        msg += "\n";
    }
    return (msg);
}

int printOglError(const char *file, int line, const char *msg)
{
    //
    // Returns -1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();

    while (glErr != GL_NO_ERROR) {
#ifdef DEBUG
        std::cout << "glError: " << gluErrorString(glErr) << std::endl << "         " << file << " : " << line << std::endl;
#endif
        if (msg) {
            Wasp::MyBase::SetErrMsg("glError: %s\n", msg);
            msg = NULL;
        }
        Wasp::MyBase::SetErrMsg("glError: %s\n         %s : %d", gluErrorString(glErr), file, line);

        retCode = -1;

        glErr = glGetError();
    }

    return retCode;
}

bool FrameBufferReady()
{
    GLuint fbstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fbstatus != GL_FRAMEBUFFER_COMPLETE) { return (false); }

    // Not sure if this is necessary
    //
    vector<int> errcodes;
    bool        status = oglStatusOK(errcodes);

    if (status) return true;

    // If error, check for invalid frame buffer
    //
    for (int i = 0; i < errcodes.size(); i++) {
        if (errcodes[i] == GL_INVALID_FRAMEBUFFER_OPERATION) return false;
    }

    return (true);
}

/*
 * Return true, if n is a power of 2.
 */
bool powerOf2(size_t n) { return (n & (n - 1)) == 0; }

/*
 * Return the next power of 2 that is equal or larger than n
 */
size_t nextPowerOf2(size_t n)
{
    if (powerOf2(n)) return n;

    size_t p;

    for (int i = 63; i >= 0; i--) {
        p = n & ((size_t)1 << i);

        if (p) {
            p = ((size_t)1 << (i + 1));
            break;
        }
    }

    return p;
}

// Some routines to handle 3x3 rotation matrices, represented as 9 floats,
// where the column index increments faster
void mmult33(const double *m1, const double *m2, double *result)
{
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) { result[row * 3 + col] = (m1[row * 3] * m2[col] + m1[1 + row * 3] * m2[col + 3] + m1[2 + row * 3] * m2[col + 6]); }
    }
}

// Same as above, but use the transpose (i.e. inverse for rotations) on the left
void mmultt33(const double *m1Trans, const double *m2, double *result)
{
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) { result[row * 3 + col] = (m1Trans[row] * m2[col] + m1Trans[3 + row] * m2[col + 3] + m1Trans[6 + row] * m2[col + 6]); }
    }
}
// Determine a rotation matrix from (theta, phi, psi) (radians), that is,
// find the rotation matrix that first rotates in (x,y) by psi, then takes the vector (0,0,1)
// to the vector with direction (theta,phi) by rotating by phi in the (x,z) plane and then
// rotating in the (x,y)plane by theta.
void getRotationMatrix(double theta, double phi, double psi, double *matrix)
{
    // do the theta-phi rotation first:
    double mtrx1[9], mtrx2[9];
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double cosPhi = cos(phi);
    double sinPhi = sin(phi);
    double cosPsi = cos(psi);
    double sinPsi = sin(psi);
    // specify mtrx1 as an (X,Z) rotation by -phi, followed by an (X,Y) rotation by theta:
    mtrx1[0] = cosTheta * cosPhi;
    mtrx1[1] = -sinTheta;
    mtrx1[2] = cosTheta * sinPhi;
    // 2nd row:
    mtrx1[3] = sinTheta * cosPhi;
    mtrx1[4] = cosTheta;
    mtrx1[5] = sinTheta * sinPhi;
    // 3rd row:
    mtrx1[6] = -sinPhi;
    mtrx1[7] = 0.f;
    mtrx1[8] = cosPhi;
    // mtrx2 is a rotation by psi in the x,y plane:
    mtrx2[0] = cosPsi;
    mtrx2[1] = -sinPsi;
    mtrx2[2] = 0.f;
    mtrx2[3] = sinPsi;
    mtrx2[4] = cosPsi;
    mtrx2[5] = 0.f;
    mtrx2[6] = 0.f;
    mtrx2[7] = 0.f;
    mtrx2[8] = 1.f;
    mmult33(mtrx1, mtrx2, matrix);
}

// Determine a rotation matrix about an axis:
COMMON_API void getAxisRotation(int axis, double rotation, double *matrix)
{
    for (int col = 0; col < 3; col++) {
        for (int row = 0; row < 3; row++) {
            if (row == axis && col == axis)
                matrix[col + row * 3] = 1.f;
            else if (row == axis || col == axis)
                matrix[col + row * 3] = 0.f;
            else {
                if (row == col)
                    matrix[col + row * 3] = cos(rotation);
                else if (row > col)
                    matrix[col + row * 3] = sin(rotation);
                else
                    matrix[col + row * 3] = -sin(rotation);
            }
        }
    }
}

// Determine the psi, phi, theta (radians!) from a rotation matrix.
// the rotation matrix rotates about the z-axis by psi,
// then takes the z-axis to the unit vector with spherical
// coordinates theta and phi.
void getRotAngles(double *theta, double *phi, double *psi, const double *matrix)
{
    // First find phi and theta by looking at matrix applied to 0,0,1:
    double vec[3];
    double tempPhi, tempTheta;
    double tempPsi = 0.;
    double tMatrix1[9], tMatrix2[9];
    vec[0] = matrix[2];
    vec[1] = matrix[5];
    vec[2] = matrix[8];
    // float cosPhi = vec[2];
    tempPhi = acos(vec[2]);    // unique angle between 0 and pi
    // now project vec[0], vec[1] to x,y plane:

    double normsq = (vec[0] * vec[0] + vec[1] * vec[1]);
    if (normsq == 0.)
        tempTheta = 0.;
    else {
        tempTheta = acos(vec[0] / sqrt(normsq));
        // If sin(theta)<0 then theta is negative:
        if (vec[1] < 0) tempTheta = -tempTheta;
    }
    // Find the transformation determined by theta, phi:
    getRotationMatrix(tempTheta, tempPhi, tempPsi, tMatrix1);

    // Apply the inverse of this to the input matrix:
    mmultt33(tMatrix1, matrix, tMatrix2);
    // Now the resulting matrix is a rotation by psi
    // Cos psi and sin psi are in the first column:
    if (abs(tMatrix2[0]) > 1.f) {
        if (tMatrix2[0] > 0.f)
            tMatrix2[0] = 1.f;
        else
            tMatrix2[0] = -1.f;
    }
    tempPsi = acos(tMatrix2[0]);
    if (tMatrix2[3] < 0.f) tempPsi = -tempPsi;

    *theta = tempTheta;
    *phi = tempPhi;
    *psi = tempPsi;
    return;
}
// Intersect a ray with an axis-aligned box.
// Ray is specified by start point and direction vector.
// Box is specified by 6 floats (extents)
// Return value is number of intersections found,
// Results are specified as floats, increasing order, indicating the position R on the ray where
// intersection = rayStart+R*rayDir.
int rayBoxIntersect(const float rayStart[3], const float rayDir[3], const float boxExts[6], float results[2])
{
    // Loop over axes of cube.  Intersect faces with the ray, then test if it's inside box extents:
    int numfound = 0;
    for (int axis = 0; axis < 3; axis++) {
        // Points along ray are rayStart+t*rayDir.
        // To intersect face, rayStart+t*rayDir has axis coordinate equal to boxExts[axis] or boxExts[axis+3]
        // so that t = (boxExts[axis+(0 or 3)] - rayStart[axis])/rayDir[axis];
        if (rayDir[axis] == 0.f) continue;    // Plane is parallel to ray
        // check front and back intersections:
        for (int frontBack = 0; frontBack < 4; frontBack += 3) {
            float t = (boxExts[axis + frontBack] - rayStart[axis]) / rayDir[axis];
            // Check to see if point is within other two box bounds
            float intersectPoint[3];
            for (int j = 0; j < 3; j++) { intersectPoint[j] = rayStart[j] + t * rayDir[j]; }
            bool pointOK = true;
            for (int otherCoord = 0; otherCoord < 3; otherCoord++) {
                if (otherCoord == axis) continue;
                if (intersectPoint[otherCoord] < boxExts[otherCoord]) {
                    pointOK = false;
                    break;
                }
                if (intersectPoint[otherCoord] > boxExts[otherCoord + 3]) {
                    pointOK = false;
                    break;
                }
            }
            if (pointOK) {
                // Found an intersection!
                results[numfound++] = t;
                // order the points in increasing t:
                if (numfound == 2 && (results[1] < results[0])) {
                    float temp = results[0];
                    results[0] = results[1];
                    results[1] = temp;
                }
                if (numfound == 2) return numfound;
            }
        }
    }
    return numfound;
}
int rayBoxIntersect(const double rayStart[3], const double rayDir[3], const double boxExts[6], double results[2])
{
    // Loop over axes of cube.  Intersect faces with the ray, then test if it's inside box extents:
    // Return at most two intersections, the max and min along the ray.
    int    numfound = 0;
    double tempResults[6];
    double epsilons[3];    // Error tolerance is size/10^6
    for (int i = 0; i < 3; i++) epsilons[i] = (boxExts[i + 3] - boxExts[i]) * 1.e-6;
    for (int axis = 0; axis < 3; axis++) {
        if (numfound == 6) break;
        // Points along ray are rayStart+t*rayDir.
        // To intersect face, rayStart+t*rayDir has axis coordinate equal to boxExts[axis] or boxExts[axis+3]
        // so that t = (boxExts[axis+(0 or 3)] - rayStart[axis])/rayDir[axis];
        if (rayDir[axis] == 0.) continue;    // Plane is parallel to ray
        // check front and back intersections:
        for (int frontBack = 0; frontBack < 4; frontBack += 3) {
            float t = (boxExts[axis + frontBack] - rayStart[axis]) / rayDir[axis];
            // Check to see if point is within other two box bounds
            float intersectPoint[3];
            for (int j = 0; j < 3; j++) { intersectPoint[j] = rayStart[j] + t * rayDir[j]; }
            bool pointOK = true;
            for (int otherCoord = 0; otherCoord < 3; otherCoord++) {
                if (otherCoord == axis) continue;
                if (intersectPoint[otherCoord] < boxExts[otherCoord] - epsilons[otherCoord]) {
                    pointOK = false;
                    break;
                }
                if (intersectPoint[otherCoord] > boxExts[otherCoord + 3] + epsilons[otherCoord]) {
                    pointOK = false;
                    break;
                }
            }
            if (pointOK) {
                // Found an intersection!
                tempResults[numfound++] = t;
            }
            if (numfound == 6) break;
        }
    }
    if (numfound == 0) return 0;
    // Find the max and min t
    double mint = DBL_MAX;
    double maxt = -DBL_MAX;

    for (int i = 0; i < numfound; i++) {
        if (tempResults[i] < mint) { mint = tempResults[i]; }
        if (tempResults[i] > maxt) { maxt = tempResults[i]; }
    }
    results[0] = mint;
    if (numfound == 1) return 1;
    results[1] = maxt;
    return 2;
}
// Project box corners to ray
// Ray is specified by start point and direction vector.
// Box is specified by 6 doubles (extents)
// Results are specified as doubles, increasing order, indicating the position R on the ray where
// the corner is projected
void rayBoxProject(vector<double> rayStart, vector<double> rayDir, const double boxExts[6], double results[2])
{
    // For each box corner,
    // project to line of view
    double cor[3], wrk[3], dir[3];
    double maxProj = -DBL_MAX;
    double minProj = DBL_MAX;
    // Make sure rayDir is normalized:
    for (int i = 0; i < 3; i++) dir[i] = rayDir[i];
    vnormal(dir);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) { cor[j] = ((i >> j) & 1) ? boxExts[j + 3] : boxExts[j]; }

        vsub(cor, rayStart, wrk);

        double mdist = vdot(wrk, dir);
        if (minProj > mdist) { minProj = mdist; }
        if (maxProj < mdist) { maxProj = mdist; }
    }
    results[0] = minProj;
    results[1] = maxProj;
}
// Convert a camera view to a quaternion, for linear interpolation of viewpoints.

void view2Quat(float vdir[3], float upvec[3], float q[4])
{
    float vtemp[3];
    float left[3] = {-1.f, 0.f, 0.f};
    float ydir[3] = {0.f, 1.f, 0.f};
    float right[3];
    // Normalize the vectors:
    vnormal(upvec);
    vnormal(vdir);
    // Force the up vector to be orthogonal to viewDir
    vcopy(vdir, vtemp);
    vscale(vtemp, vdot(vdir, upvec));
    // Subtract the component of up in the viewdir direction
    vsub(upvec, vtemp, upvec);
    // Make sure it's still valid
    if (vdot(upvec, upvec) == 0.f) {
        // First try up = viewdir x left
        vcross(vdir, left, upvec);
        if (vdot(upvec, upvec) == 0.f) {
            // try viewdir x ydir
            vcross(vdir, ydir, upvec);
        }
    }
    vnormal(upvec);
    // calculate "right" vector:
    vcross(vdir, upvec, right);
    // Construct 4x4 matrix.  Just because rotmatrix2q expects 4x4
    float minv[16];
    // Bottom row, right column not needed.
    minv[3] = 0.f;
    minv[7] = 0.f;
    minv[11] = 0.f;
    minv[15] = 1.f;
    // copy in first 3 elements of columns
    vcopy(right, minv);
    vcopy(upvec, minv + 4);
    // third col is neg of viewdir

    vcopy(vdir, minv + 8);
    vscale(minv + 8, -1.f);
    vzero(minv + 12);
    minv[15] = 1.f;
    rotmatrix2q(minv, q);
}

// Convert two camera views to two pure imaginary quaternion, for linear interpolation of viewpoints.
// If dot product is negative, the first quaternion is negated, to prevent interpolating the long way around.
void views2ImagQuats(float vdir1[3], float upvec1[3], float vdir2[3], float upvec2[3], float q1[3], float q2[3])
{
    float quat1[4], quat2[4];
    view2Quat(vdir1, upvec1, quat1);
    view2Quat(vdir2, upvec2, quat2);
    float dotprod = 0.;
    for (int i = 0; i < 4; i++) dotprod += quat1[i] * quat2[i];
    if (dotprod < 0.f)
        for (int i = 0; i < 4; i++) quat1[i] = -quat1[i];
    qnormal(quat2);    // force full quaternion to be norm-1 (correct round-off error)
    qnormal(quat1);
    float mag = vlength(quat1);    // norm of imaginary part
    float re = acos(quat1[3]);
    if (mag == 0.f)
        for (int i = 0; i < 3; i++) q1[i] = 0.f;
    else
        for (int i = 0; i < 3; i++) q1[i] = quat1[i] * re / mag;
    mag = vlength(quat2);    // norm of imaginary part
    re = acos(quat2[3]);
    if (mag == 0.f)
        for (int i = 0; i < 3; i++) q2[i] = 0.f;
    else
        for (int i = 0; i < 3; i++) q2[i] = quat2[i] * re / mag;
    return;
}
// Convert a pure-imaginary quaternion to camera view, for linear interpolation of viewpoints.

void imagQuat2View(const float q[3], float vdir[3], float upvec[3])
{
    float quat[4];
    float mtrx[16];
    // First, calc exponential of q:
    float mag = vlength(q);
    quat[3] = cos(mag);
    if (mag > 0.f) {
        float s = sin(mag) / mag;
        for (int i = 0; i < 3; i++) quat[i] = q[i] * s;
    } else {
        for (int i = 0; i < 3; i++) quat[i] = 0.f;
    }
    // then convert quat to a matrix
    qmatrix(quat, mtrx);
    // extract rows:
    vcopy(mtrx + 4, upvec);
    vcopy(mtrx + 8, vdir);
    vscale(vdir, -1.f);
}
void quat2View(float quat[4], float vdir[3], float upvec[3])
{
    float mtrx[16];
    // convert quat to a matrix
    qmatrix(quat, mtrx);
    // extract rows:
    vcopy(mtrx + 4, upvec);
    vcopy(mtrx + 8, vdir);
    vscale(vdir, -1.f);
}
// Spherical linear interpolation between two unit quaternions.  t is between 0 and 1.
void slerp(float quat0[4], float quat1[4], float t, float result[4])
{
    // Wikipedia:  slerp = q0*(q0^-1 * q1)**t
    float q0inv[4], res1[4], res2[4], V[3];
    // make sure they are unit quaternion:
    qnormal(quat0);
    qnormal(quat1);

    // calculate quat0 inv
    for (int i = 0; i < 3; i++) q0inv[i] = -quat0[i];
    q0inv[3] = quat0[3];
    qmult(q0inv, quat1, res1);
    // now take res1 to power t:
    // First express res1 as cos theta + V sin theta, = exp(theta*V)
    if (res1[3] > 1.0) res1[3] = 1.0;
    if (res1[3] < -1.0) res1[3] = -1.0;
    double theta = acos((double)res1[3]);
    float  sinTheta = sin(theta);

    if (sinTheta == 0.f) {
        vzero(V);
    } else {
        vcopy(res1, V);
        vscale(V, 1. / sinTheta);
    }
    // Now take to power t, i.e. cos(t*theta)+sin(t*theta)*V
    res2[3] = cos(t * theta);
    for (int i = 0; i < 3; i++) res2[i] = V[i] * sin(t * theta);

    // then multiply on the left by quat0:
    qmult(quat0, res2, result);
}
// Logarithm of a unit quaternion
void qlog(float quat[4], float quatlog[4])
{
    // express quat as cos theta + V sin theta, = exp(theta*V)
    float V[3];
    if (quat[3] > 1.0) quat[3] = 1.0;
    if (quat[3] < -1.0) quat[3] = -1.0;
    double theta = acos((double)quat[3]);
    float  sinTheta = sin(theta);
    if (sinTheta == 0.f) {
        vzero(V);
    } else {
        vcopy(quat, V);
        vscale(V, theta / sinTheta);
    }
    vcopy(V, quatlog);
    quatlog[3] = 0.f;
}
void squad(float quat1[4], float quat2[4], float s1[4], float s2[4], float t, float result[4])
{
    float qa[4], qb[4];
    slerp(quat1, quat2, t, qa);
    slerp(s1, s2, t, qb);
    slerp(qa, qb, 2. * t * (1. - t), result);
}
void qconj(float quat[4], float conj[4])
{
    conj[3] = quat[3];
    conj[0] = -quat[0];
    conj[1] = -quat[1];
    conj[2] = -quat[2];
}
void doubleToString(const double d, string &s, int digits)
{
    char buf[30];
    sprintf(buf, "%-.*G", digits, d);
    string ss(buf);
    s = ss;
}
#define DEAD
#ifdef DEAD

    // Macros
    #define LINEAR_INDEX(dim, x, y, z) ((x) + (dim[0]) * ((y) + (z) * (dim[1])))
    #define sqr(x)                     ((x) * (x))

void computeGradientData(int dim[3], int numChan, unsigned char *volume, unsigned char *gradient)
{
    for (int z = 0; z < dim[2]; z++)
        for (int y = 0; y < dim[1]; y++)
            for (int x = 0; x < dim[0]; x++) {
                float gradient_temp[3];

                // The following code computes the gradient for the volume data.
                // For volumes with more than one channel, only the first channel
                // is used to compute the gradient! The voxel index computation is
                // very inefficient and can be optimized a lot using a simple
                // incremental computation.

                // Handle border cases correctly by using forward and backward
                // differencing for the boundaries.
                if (x == 0)
                    // forward differencing
                    gradient_temp[0] = ((float)volume[numChan * LINEAR_INDEX(dim, x + 1, y, z)] - (float)volume[numChan * LINEAR_INDEX(dim, x, y, z)]);
                else if (x == dim[0] - 1)
                    // backward differencing
                    gradient_temp[0] = ((float)volume[numChan * LINEAR_INDEX(dim, x, y, z)] - (float)volume[numChan * LINEAR_INDEX(dim, x - 1, y, z)]);
                else
                    // central differencing
                    gradient_temp[0] = (((float)volume[numChan * LINEAR_INDEX(dim, x + 1, y, z)] - (float)volume[numChan * LINEAR_INDEX(dim, x - 1, y, z)]) / 2.0);

                if (y == 0)
                    gradient_temp[1] = ((float)volume[numChan * LINEAR_INDEX(dim, x, y + 1, z)] - (float)volume[numChan * LINEAR_INDEX(dim, x, y, z)]);
                else if (y == dim[1] - 1)
                    gradient_temp[1] = ((float)volume[numChan * LINEAR_INDEX(dim, x, y, z)] - (float)volume[numChan * LINEAR_INDEX(dim, x, y - 1, z)]);
                else
                    gradient_temp[1] = (((float)volume[numChan * LINEAR_INDEX(dim, x, y + 1, z)] - (float)volume[numChan * LINEAR_INDEX(dim, x, y - 1, z)]) / 2.0);

                if (z == 0)
                    gradient_temp[2] = ((float)volume[numChan * LINEAR_INDEX(dim, x, y, z + 1)] - (float)volume[numChan * LINEAR_INDEX(dim, x, y, z)]);
                else if (z == dim[2] - 1)
                    gradient_temp[2] = ((float)volume[numChan * LINEAR_INDEX(dim, x, y, z)] - (float)volume[numChan * LINEAR_INDEX(dim, x, y, z - 1)]);
                else
                    gradient_temp[2] = (((float)volume[numChan * LINEAR_INDEX(dim, x, y, z + 1)] - (float)volume[numChan * LINEAR_INDEX(dim, x, y, z - 1)]) / 2.0);

                // compute the magintude for the gradient
                double mag = (sqrt(sqr(gradient_temp[0]) + sqr(gradient_temp[1]) + sqr(gradient_temp[2])));

                // avoid any divide by zeros!
                if (mag > 0.01) {
                    gradient_temp[0] /= mag;
                    gradient_temp[1] /= mag;
                    gradient_temp[2] /= mag;
                } else {
                    gradient_temp[0] = gradient_temp[1] = gradient_temp[2] = 0.0;
                }

                // Map the floating point gradient values to the appropriate range
                // in unsigned byte
                unsigned long index = 4 * (LINEAR_INDEX(dim, x, y, z));
                for (int i = 0; i < 3; i++) { gradient[index + i] = (signed char)floor(gradient_temp[i] * 128.0); }

                // Set the alpha to be 1.0
                gradient[index + 3] = 255;
            }
}

#endif
};    // namespace VAPoR
