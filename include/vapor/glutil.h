//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		glutil.h
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

#ifndef _glutil_h_
#define _glutil_h_

#ifdef __APPLE__
    #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif

#include <vapor/GLInclude.h>
#include <cmath>
#include <vector>
#include <string>
#include <vapor/common.h>

/* These vector and quaternion macros complement similar
 * routines.
 */

#ifdef ArchLinux
    #define sqrtf(fval) ((float)sqrt((double)(fval)))
    #define fabsf(fval) ((float)fabs((double)(fval)))
    #define sinf(fval)  ((float)sin((double)(fval)))
    #define cosf(fval)  ((float)cos((double)(fval)))
    #define tanf(fval)  ((float)tan((double)(fval)))
#endif

//#define vset(a,x,y,z)	(a[0] = x, a[1] = y, a[2] = z)
//#define Verify(expr,estr)	if (!(expr)) BailOut(estr,__FILE__,__LINE__)

#define CallocType(type, i) (type *)calloc(i, sizeof(type))

#define YMAXSTEREO 491
#define YOFFSET    532
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
/*extern GLfloat *idmatrix;*/

namespace VAPoR {

/* glutil.c */

RENDER_API int __CheckGLError(const char *file, int line, const char *msg = 0);

//! Check for any OpenGL errors and return their error codes
//!
//! This function calls glGetError() to test for any OpenGL errors.
//! If no errors are detected the function returns \b true. If one
//! or more errors are detected the function returns false and stores
//! each of the error codes in the \p status vector
//
RENDER_API bool oglStatusOK(std::vector<int> &status);

RENDER_API void doubleToString(const double val, std::string &result, int digits);
//! Decode OpenGL error codes and format them as a string
//!
//! This function takes a vector of error codes (see oglStatusOK) and
//! produces a formatted error string from the list of codes
//
RENDER_API std::string oglGetErrMsg(std::vector<int> status);

//! Returns free RAM in kilobytes
//! Returns -1 if not supported
RENDER_API int oglGetFreeMemory();

//! Test readyness of OpenGL frame buffer, GL_FRAMEBUFFER
//!
RENDER_API bool FrameBufferReady();
};    // namespace VAPoR

#define CheckGLError()       __CheckGLError(__FILE__, __LINE__)
#define CheckGLErrorMsg(msg) __CheckGLError(__FILE__, __LINE__, msg)

#ifndef NDEBUG
    #ifdef Darwin
        #define GL_LEGACY(x) \
            {                \
            }
    #else
        #define GL_LEGACY(x) x
    #endif
    #include <signal.h>
    #define GL_ERR_BREAK() \
        if (CheckGLError()) ::raise(SIGTERM)
#else
    #define GL_LEGACY(x)
    #define GL_ERR_BREAK()
#endif

#endif    // _glutil_h_
