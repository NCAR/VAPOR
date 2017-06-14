//
//      $Id$
//
//************************************************************************
//								*
//		     Copyright (C)  2004			*
//     University Corporation for Atmospheric Research		*
//		     All Rights Reserved			*
//								*
//************************************************************************/
//
//	File:
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Mon Nov 29 11:49:03 MST 2004
//
//	Description:	A collection of common C system routines that
//					aren't always portable across OS platforms
//

#ifndef _CFuncs_h_
#define _CFuncs_h_

#include <cmath>
#include <string>
#include <vapor/common.h>

using namespace std;

namespace Wasp {

COMMON_API const char *Basename(const char *path);
COMMON_API string Basename(const string &path);

COMMON_API string Dirname(const string &path);

COMMON_API string Catpath(string volume, string dir, string file);

COMMON_API void Splitpath(
    string path, string &volume, string &dir, string &file, bool nofile);

COMMON_API bool IsAbsPath(string path);

COMMON_API double GetTime();

COMMON_API int MkDirHier(const string &dir);

}; // namespace Wasp

#endif // _CFuncs_h_
