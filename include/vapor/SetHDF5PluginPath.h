#pragma once

#include <stdlib.h>
#include "vapor/MyBase.h"
#include "vapor/ResourcePath.h"

namespace VAPoR {
void SetHDF5PluginPath() {
    char *existing = getenv("HDF5_PLUGIN_PATH");
    if (existing) {
        printf("Using custom HDF5_PLUGIN_PATH: '%s'\n", existing);
        return;
    }

    int rc=0;
    std::string plugins = "HDF5_PLUGIN_PATH=" + Wasp::GetSharePath("plugins");
    #ifdef WIN32
        rc=_putenv(plugins.c_str());
    #else
        rc = setenv("HDF5_PLUGIN_PATH", Wasp::GetSharePath("plugins").c_str(), 1);
    #endif

    if (rc != 0) Wasp::MyBase::SetErrMsg("Unable to set environtment variable s", plugins.c_str());
}
};    // namespace VAPoR
