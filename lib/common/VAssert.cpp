#include <vapor/VAssert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <vapor/FileUtils.h>

void Wasp::_VAssertFailed(const char *expr, const char *path, const unsigned int line) {
    const char *fileName = strdup(FileUtils::Basename(std::string(path)).c_str());

    fprintf(stderr, "VAssert(%s) failed in %s, line %i\n", expr, fileName, line);
#ifdef NDEBUG
    exit(1);
#else
    raise(SIGTERM);
#endif
}
