#pragma once

#include <vector>
#include <string>
#include <glm/fwd.hpp>

#include "vapor/Shader.h"

namespace VAPoR {

//! \class ShaderProgram
//! \ingroup Public_Render
//!
//! \brief Provides a C++ interface to the OpenGL shader program construct
//!
//! \author Stanislaw Jaroszynski
//! \date August, 2018

class RENDER_API ShaderProgram : public Wasp::MyBase {
    unsigned int          _id;
    std::vector<Shader *> _shaders;
    bool                  _linked;
    int                   _successStatus;

public:
    enum class Policy { Strict, Relaxed };
    static Policy UniformNotFoundPolicy;

    ShaderProgram();
    ~ShaderProgram();

    //! \retval 1 is returned on success
    //! \retval -1 is returned on failure
    //!
    int         Link();
    void        Bind();
    bool        IsBound() const;
    static void UnBind();
    static int  GetBoundProgramID();

    void AddShader(Shader *s);

    //! \param[in] type OpenGL shader type enum
    //! \param[in] source GLSL source code
    //!
    //! \retval 1 is returned on success
    //! \retval -1 is returned on failure
    //!
    int AddShaderFromSource(unsigned int type, const char *source);

    unsigned int GetID() const;
    unsigned int WasLinkingSuccessful() const;

    int GetAttributeLocation(const std::string &name) const;
    int GetUniformLocation(const std::string &name) const;

    template<typename T> bool SetUniform(const std::string &name, const T &value) const;
    void                      SetUniform(int location, const int &value) const;
    void                      SetUniform(int location, const float &value) const;
    void                      SetUniform(int location, const glm::vec2 &value) const;
    void                      SetUniform(int location, const glm::vec3 &value) const;
    void                      SetUniform(int location, const glm::vec4 &value) const;
    void                      SetUniform(int location, const glm::mat4 &value) const;

    std::string        GetLog() const;
    void               PrintUniforms() const;
    static const char *GLTypeToString(const unsigned int type);
};

//! \class SmartShaderProgram
//! \ingroup Public_Render
//!
//! \brief Provides a C++ interface to the OpenGL shader program construct
//!
//! \author Stanislaw Jaroszynski
//! \date August, 2018

class SmartShaderProgram {
    ShaderProgram *_program;

    SmartShaderProgram(ShaderProgram *program);

public:
    ~SmartShaderProgram();
    ShaderProgram *operator->() { return _program; }
    bool           IsValid() const;
    friend class ShaderManager;
};
}    // namespace VAPoR
