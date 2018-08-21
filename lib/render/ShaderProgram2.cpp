#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/ShaderProgram2.h>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace VAPoR;

ShaderProgram2::ShaderProgram2() : _linked(false), _successStatus(false) {}

ShaderProgram2::~ShaderProgram2()
{
    if (_id) glDeleteProgram(_id);
    for (int i = 0; i < _shaders.size(); i++) delete _shaders[i];
}

bool ShaderProgram2::Link()
{
    assert(!_linked);
    _id = glCreateProgram();
    for (auto it = _shaders.begin(); it != _shaders.end(); it++) {
        assert((*it)->WasCompilationSuccessful());
        printf("GL Adding shader %i\n", (*it)->GetID());
        glAttachShader(_id, (*it)->GetID());
    }
    glLinkProgram(_id);
    glGetProgramiv(_id, GL_LINK_STATUS, &_successStatus);
    _linked = true;

    for (int i = 0; i < _shaders.size(); i++) delete _shaders[i];
    _shaders.clear();

    return _successStatus;
}

void ShaderProgram2::Bind()
{
    assert(_linked && WasLinkingSuccessful());
    glUseProgram(_id);
}

bool ShaderProgram2::IsBound() const
{
    int currentlyBoundProgramId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentlyBoundProgramId);
    return _id == currentlyBoundProgramId;
}

void ShaderProgram2::UnBind() { glUseProgram(0); }

void ShaderProgram2::AddShader(Shader *s) { _shaders.push_back(s); }

bool ShaderProgram2::AddShaderFromSource(unsigned int type, const char *source)
{
    Shader *s = new Shader(type);
    bool    ret = s->CompileFromSource(source);
    _shaders.push_back(s);
    return ret;
}

bool ShaderProgram2::AddShaderFromFile(unsigned int type, const std::string path)
{
    Shader *s = new Shader(type);
    bool    ret = s->CompileFromFile(path);
    _shaders.push_back(s);
    return ret;
}

std::string ShaderProgram2::GetLog() const
{
    assert(!_shaders.empty());
    if (_linked) {
        char buf[512];
        glGetProgramInfoLog(_id, 512, NULL, buf);
        return std::string(buf);
    } else {
        if (_shaders.empty())
            return "Not linked and no shaders";
        else
            return _shaders.back()->GetLog();
    }
}

unsigned int ShaderProgram2::GetID() const { return _id; }
unsigned int ShaderProgram2::WasLinkingSuccessful() const { return _successStatus; }

int ShaderProgram2::GetAttributeLocation(const std::string name) const { return glGetAttribLocation(_id, name.c_str()); }

int ShaderProgram2::GetUniformLocation(const std::string name) const { return glGetUniformLocation(_id, name.c_str()); }

template<typename T> bool ShaderProgram2::SetUniform(const std::string name, T value) const
{
    if (!IsBound()) {
        assert(!"Program not bound");
        return false;
    }
    const int location = glGetUniformLocation(_id, name.c_str());
    if (location == -1) {
        assert(!"Uniform name not found");
        return false;
    }
    SetUniform(location, value);
    return true;
}

template bool ShaderProgram2::SetUniform<int>(const std::string name, int value) const;
template bool ShaderProgram2::SetUniform<float>(std::string name, float value) const;
template bool ShaderProgram2::SetUniform<glm::vec2>(std::string name, const glm::vec2 value) const;
template bool ShaderProgram2::SetUniform<glm::vec3>(std::string name, const glm::vec3 value) const;
template bool ShaderProgram2::SetUniform<glm::vec4>(std::string name, const glm::vec4 value) const;
template bool ShaderProgram2::SetUniform<glm::mat4>(std::string name, const glm::mat4 value) const;

void ShaderProgram2::SetUniform(int location, const int value) const { glUniform1i(location, value); }
void ShaderProgram2::SetUniform(int location, const float value) const { glUniform1f(location, value); }
void ShaderProgram2::SetUniform(int location, const glm::vec2 value) const { glUniform2fv(location, 1, glm::value_ptr(value)); }
void ShaderProgram2::SetUniform(int location, const glm::vec3 value) const { glUniform3fv(location, 1, glm::value_ptr(value)); }
void ShaderProgram2::SetUniform(int location, const glm::vec4 value) const { glUniform4fv(location, 1, glm::value_ptr(value)); }
void ShaderProgram2::SetUniform(int location, const glm::mat4 value) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
