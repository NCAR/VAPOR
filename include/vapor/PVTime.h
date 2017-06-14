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

#ifndef _PVTime_h_
#define _PVTime_h_

#include <cmath>
#include <string>
#include <ctime>
#ifndef WIN32
    #include <stdint.h>
#endif
#include <vapor/common.h>

#ifndef TIME64_T
    #ifdef WIN32
        #define TIME64_T __int64
    #else
        #define TIME64_T int64_t
    #endif
#endif

using namespace std;

namespace Wasp {

COMMON_API TIME64_T   MkTime64(struct tm *t);
COMMON_API struct tm *LocalTime64_r(const TIME64_T *t, struct tm *p);
COMMON_API struct tm *GmTime64_r(const TIME64_T *t, struct tm *p);

};    // namespace Wasp

#endif    // _PVTime_h_
