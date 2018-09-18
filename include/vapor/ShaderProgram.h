#pragma once

#include <vector>
#include <string>
#include <glm/fwd.hpp>

#include "vapor/Shader.h"

namespace VAPoR {

class ShaderProgram {
    unsigned int          _id;
    std::vector<Shader *> _shaders;
    bool                  _linked;
    int                   _successStatus;

public:
    enum class Policy { Strict, Relaxed };
    static Policy UniformNotFoundPolicy;

    ShaderProgram();
    ~ShaderProgram();

    bool        Link();
    void        Bind();
    bool        IsBound() const;
    static void UnBind();
    static int  GetBoundProgramID();

    void AddShader(Shader *s);
    bool AddShaderFromSource(unsigned int type, const char *source);
    bool AddShaderFromFile(unsigned int type, const std::string path);

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

class SmartShaderProgram {
    ShaderProgram *_program;

    SmartShaderProgram(ShaderProgram *program);

public:
    ~SmartShaderProgram();
    ShaderProgram *operator->() { return _program; }
    friend class ShaderManager;
};
}    // namespace VAPoR
