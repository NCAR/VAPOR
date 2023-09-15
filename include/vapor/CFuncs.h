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

namespace Wasp {

COMMON_API void Splitpath(std::string path, std::string &volume, std::string &dir, std::string &file, bool nofile);

COMMON_API double GetTime();

COMMON_API int MkDirHier(const std::string &dir);

COMMON_API std::string GetEnvironmentalVariable(const std::string &name);

};    // namespace Wasp

#endif    // _CFuncs_h_
