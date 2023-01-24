#pragma once

#include "vapor/MyBase.h"
#include "vapor/ResourcePath.h"

namespace VAPoR {
void SetHDF5PluginPath() {
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
