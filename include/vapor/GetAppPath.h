//
// $Id$
//
#ifndef _GetAppPath_h_
#define _GetAppPath_h_
#include <vapor/MyBase.h>
#include <vapor/Version.h>

namespace Wasp {

COMMON_API std::string GetAppPath(
    const string &app, const string &name, const vector<string> &paths,
#ifdef _WINDOWS
    //Windows default is backwards slash for separator
    bool forwardSeparator = false
#else
    bool forwardSeparator = true
#endif
);

}; // namespace Wasp

#endif
