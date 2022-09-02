#pragma once

#include "vapor/MyBase.h"
#include "vapor/ResourcePath.h"

#ifndef WIN32
    #include <H5PLpublic.h>
#endif

namespace VAPoR {
void SetHDF5PluginPath() {
    string plugins = Wasp::GetSharePath("plugins");

    #ifndef WIN32
        H5PLreplace(plugins.c_str(), 0);
    #else
        plugins = "HDF5_PLUGIN_PATH=" + plugins;
        int rc=_putenv(plugins.c_str());
        if (rc != 0) MyBase::SetErrMsg("Unable to set environtment variable %s", plugins.c_str());
    #endif
}
};    // namespace VAPoR
