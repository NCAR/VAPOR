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
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>

#include <GL/glew.h>

//#include "util.h"

/* Most of the 'v' routines are in the form vsomething(src1, src2, dst),
 * dst can be one of the source vectors.
 */
namespace VAPoR {

void ViewMatrix(float *m)
{
    /* Return the total view matrix, including any
     * projection transformation.
     */

    float mp[16], mv[16];
    glGetFloatv(GL_PROJECTION_MATRIX, mp);
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    mmult(mv, mp, m);
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

int ViewAxis(int *direction)
{
    /* Return the major axis the viewer is looking down.
     * 'direction' indicates which direction down the axis.
     */
    float view[16];
    int   axis;

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
    for (int i = 0; i < status.size(); i++) msg += "Error #" + std::to_string(status[i]) + "\n";
    return msg;
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
        Wasp::MyBase::SetErrMsg("glError: %i\n\t%s : %d", glErr, file, line);

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

};    // namespace VAPoR
