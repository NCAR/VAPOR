#pragma once

#include "vapor/IResourceManager.h"
#include "vapor/ShaderProgram.h"
#include <string>

namespace VAPoR {

//! \class ShaderManager
//! \ingroup Public_Render
//!
//! \brief Resource management class for shaders
//!
//! \author Stanislaw Jaroszynski
//! \date    August, 2018

class RENDER_API ShaderManager : public IResourceManager<std::string, ShaderProgram> {
    std::map<std::string, long> _modifiedTimes;

    std::vector<std::string> _getSourceFilePaths(const std::string &name) const;
    bool                     _wasFileModified(const std::string &path) const;

public:
    ShaderProgram *    GetShader(const std::string &name);
    SmartShaderProgram GetSmartShader(const std::string &name);
    int                LoadResourceByKey(const std::string &name);

    //! \param[in] path to GLSL source code file
    //!
    //! \retval Shader* is returned on success
    //! \retval nullptr is returned on failure
    //!
    static Shader *CompileNewShaderFromFile(const std::string &path);

    //! Returns an OpenGL shader type enum based on the file extension.
    //! Valid extensions are .vert, .frag, and .geom
    //!
    //! \param[in] path to GLSL source code file
    //!
    static unsigned int GetShaderTypeFromPath(const std::string &path);
};

}    // namespace VAPoR
