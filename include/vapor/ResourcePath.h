#pragma once

#include "vapor/MyBase.h"
#include <string>

namespace Wasp {
COMMON_API std::string GetResourcePath(const std::string &name);
COMMON_API std::string GetSharePath(const std::string &name);

//! Returns full python installation path
//! e.g. /home/lib/python2.7
COMMON_API std::string GetPythonPath();

//! Returns python version
//! e.g. 3.6
COMMON_API std::string GetPythonVersion();

//! Returns python home
//! e.g. if python if installed in /home/lib/python2.7
//! this will return /home on Linux/Mac but it will return
//! /home/lib/python2.7 on Windows
COMMON_API std::string GetPythonDir();
};    // namespace Wasp
