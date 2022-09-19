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

#include "vapor/VAssert.h"
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

/* Most of the 'v' routines are in the form vsomething(src1, src2, dst),
 * dst can be one of the source vectors.
 */
namespace VAPoR {

bool oglStatusOK(vector<int> &status)
{
    GLenum glErr;

    while ((glErr = glGetError()) != GL_NO_ERROR) { status.push_back((int)glErr); }
    if (status.size()) return (false);

    return (true);
}

static const char *_glErrorStr(int err)
{
    switch (err) {
#define ERR_TO_STR(E) case E: return #E
        ERR_TO_STR(GL_NO_ERROR);
        ERR_TO_STR(GL_INVALID_ENUM);
        ERR_TO_STR(GL_INVALID_VALUE);
        ERR_TO_STR(GL_INVALID_OPERATION);
        ERR_TO_STR(GL_INVALID_FRAMEBUFFER_OPERATION);
        ERR_TO_STR(GL_OUT_OF_MEMORY);
//        ERR_TO_STR(GL_STACK_UNDERFLOW);
//        ERR_TO_STR(GL_STACK_OVERFLOW);
    default: return "GL_UNKNOWN_ERROR";
    }
}

string oglGetErrMsg(vector<int> status)
{
    string msg;
    for (auto e : status) {
        msg += "OpenGL Error: ";
        msg += _glErrorStr(e);
        msg += " (#" + std::to_string(e) + ")\n";
    }
    return msg;
}

int oglGetFreeMemory()
{
#define TEXTURE_FREE_MEMORY_ATI 0x87FC
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
    GLint buf[4];
    vector<int> _;

    memset(buf, 0, sizeof(buf));
    glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, buf);
    if (buf[0] > 0)
        return buf[0];

    oglStatusOK(_);

    memset(buf, 0, sizeof(buf));
    glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, buf);
    if (buf[0] > 0)
        return buf[0];

    oglStatusOK(_);

    return -1;
}

int __CheckGLError(const char *file, int line, const char *msg)
{
    //
    // Returns -1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();

    while (glErr != GL_NO_ERROR) {
#ifndef NDEBUG
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
//        std::cout << "glError: " << gluErrorString(glErr) << std::endl << "         " << file << " : " << line << std::endl;
    #pragma GCC diagnostic pop
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
