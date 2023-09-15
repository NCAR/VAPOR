#pragma once

#include <vapor/common.h>

namespace Wasp {
COMMON_API void _VAssertFailed(const char *expr, const char *path, const unsigned int line);
}

#define VAssert(expr) ((expr) ? (void)0 : Wasp::_VAssertFailed(#expr, __FILE__, __LINE__))
