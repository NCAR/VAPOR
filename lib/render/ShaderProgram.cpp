#include "vapor/glutil.h"    // Must be included first!!!
#include "vapor/ShaderProgram.h"
#include "vapor/FileUtils.h"
#include "vapor/VAssert.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vapor/Shader.h>
#include <vapor/Texture.h>

using namespace VAPoR;
using std::string;
using std::vector;

ShaderProgram::Policy ShaderProgram::UniformNotFoundPolicy = ShaderProgram::Policy::Relaxed;

ShaderProgram::ShaderProgram() : _linked(false), _successStatus(false) {}

ShaderProgram::~ShaderProgram()
{
    if (_id) glDeleteProgram(_id);
    for (int i = 0; i < _shaders.size(); i++)
        if (_shaders[i]) delete _shaders[i];
}

int ShaderProgram::Link()
{
    if (_linked) { return 1; }
    _id = glCreateProgram();
    VAssert(_id);
    for (auto it = _shaders.begin(); it != _shaders.end(); it++) {
        if (*it == nullptr || !(*it)->WasCompilationSuccessful()) { return -1; }
        glAttachShader(_id, (*it)->GetID());
    }
    glLinkProgram(_id);
    glGetProgramiv(_id, GL_LINK_STATUS, &_successStatus);
    _linked = true;

    for (int i = 0; i < _shaders.size(); i++) { delete _shaders[i]; }
    _shaders.clear();

    if (!_successStatus) return -1;

    ComputeSamplerLocations();

    return 1;
}

void ShaderProgram::Bind()
{
    if (WasLinkingSuccessful()) glUseProgram(_id);
}

bool ShaderProgram::IsBound() const { return _id == GetBoundProgramID(); }

void ShaderProgram::UnBind() { glUseProgram(0); }

int ShaderProgram::GetBoundProgramID()
{
    int currentlyBoundProgramId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentlyBoundProgramId);
    return currentlyBoundProgramId;
}

void ShaderProgram::AddShader(Shader *s)
{
    if (_linked) {
        SetErrMsg("Program already linked");
        return;
    }
    _shaders.push_back(s);
}

int ShaderProgram::AddShaderFromSource(unsigned int type, const char *source)
{
    Shader *s = new Shader(type);
    int     ret = s->CompileFromSource(source);
    _shaders.push_back(s);
    return ret;
}

/*
bool ShaderProgram::AddShaderFromFile(unsigned int type, const std::string path)
{
    Shader *s = new Shader(type);
    bool ret = s->CompileFromSource(FileUtils::ReadFileToString(path));
    _shaders.push_back(s);
    return ret;
}
 */

unsigned int ShaderProgram::GetID() const { return _id; }
unsigned int ShaderProgram::WasLinkingSuccessful() const { return _successStatus; }

int ShaderProgram::GetAttributeLocation(const std::string &name) const { return glGetAttribLocation(_id, name.c_str()); }

int ShaderProgram::GetUniformLocation(const std::string &name) const { return glGetUniformLocation(_id, name.c_str()); }

bool ShaderProgram::HasUniform(const std::string &name) const { return GetUniformLocation(name) != -1; }

template<typename T> bool ShaderProgram::SetUniform(const std::string &name, const T &value) const
{
    //    if ((typeid(T) == typeid(int)) && _samplerLocations.count(name)) printf("%s set to %i\n", name.c_str(), *(int*)((void*)&value));

    if (!IsBound()) {
        VAssert(!"Program not bound");
        return false;
    }
    const int location = glGetUniformLocation(_id, name.c_str());
    if (location == -1) {
        // printf("Uniform \"%s\" not found\n", name.c_str());
        if (UniformNotFoundPolicy == Policy::Strict) VAssert(!"Uniform name not found");
        return false;
    }
    SetUniform(location, value);
    return true;
}
template bool ShaderProgram::SetUniform<int>(const std::string &name, const int &value) const;
template bool ShaderProgram::SetUniform<bool>(const std::string &name, const bool &value) const;
template bool ShaderProgram::SetUniform<float>(const std::string &name, const float &value) const;
template bool ShaderProgram::SetUniform<glm::vec2>(const std::string &name, const glm::vec2 &value) const;
template bool ShaderProgram::SetUniform<glm::vec3>(const std::string &name, const glm::vec3 &value) const;
template bool ShaderProgram::SetUniform<glm::vec4>(const std::string &name, const glm::vec4 &value) const;
template bool ShaderProgram::SetUniform<glm::mat4>(const std::string &name, const glm::mat4 &value) const;
template bool ShaderProgram::SetUniform<glm::ivec2>(const std::string &name, const glm::ivec2 &value) const;
template bool ShaderProgram::SetUniform<glm::ivec3>(const std::string &name, const glm::ivec3 &value) const;
template bool ShaderProgram::SetUniform<vector<float>>(const std::string &name, const vector<float> &value) const;

void ShaderProgram::SetUniform(int location, const int &value) const { glUniform1i(location, value); }
void ShaderProgram::SetUniform(int location, const float &value) const { glUniform1f(location, value); }
void ShaderProgram::SetUniform(int location, const glm::vec2 &value) const { glUniform2fv(location, 1, glm::value_ptr(value)); }
void ShaderProgram::SetUniform(int location, const glm::vec3 &value) const { glUniform3fv(location, 1, glm::value_ptr(value)); }
void ShaderProgram::SetUniform(int location, const glm::vec4 &value) const { glUniform4fv(location, 1, glm::value_ptr(value)); }
void ShaderProgram::SetUniform(int location, const glm::mat4 &value) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
void ShaderProgram::SetUniform(int location, const glm::ivec2 &value) const { glUniform2iv(location, 1, glm::value_ptr(value)); }
void ShaderProgram::SetUniform(int location, const glm::ivec3 &value) const { glUniform3iv(location, 1, glm::value_ptr(value)); }
void ShaderProgram::SetUniform(int location, const vector<float> &value) const { glUniform1fv(location, value.size(), value.data()); }

template<typename T> void ShaderProgram::SetUniformArray(const std::string &name, int count, const T *values) const { SetUniformArray(GetUniformLocation(name), count, values); }
template void             ShaderProgram::SetUniformArray<int>(const std::string &name, int count, const int *values) const;
template void             ShaderProgram::SetUniformArray<float>(const std::string &name, int count, const float *values) const;
template void             ShaderProgram::SetUniformArray<glm::vec3>(const std::string &name, int count, const glm::vec3 *values) const;
template void             ShaderProgram::SetUniformArray<glm::vec4>(const std::string &name, int count, const glm::vec4 *values) const;

void ShaderProgram::SetUniformArray(int location, int count, const int *values) const { glUniform1iv(location, count, values); }
void ShaderProgram::SetUniformArray(int location, int count, const float *values) const { glUniform1fv(location, count, values); }
void ShaderProgram::SetUniformArray(int location, int count, const glm::vec3 *values) const { glUniform3fv(location, count, (float *)values); }
void ShaderProgram::SetUniformArray(int location, int count, const glm::vec4 *values) const { glUniform4fv(location, count, (float *)values); }

template<typename T> bool ShaderProgram::SetSampler(const std::string &name, const T &value) const
{
    auto itr = _samplerLocations.find(name);
    if (itr == _samplerLocations.end()) return false;
    int textureUnit = itr->second;

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    value.Bind();
    SetUniform(name, textureUnit);

    glActiveTexture(GL_TEXTURE0);

    return true;
}
template bool ShaderProgram::SetSampler<Texture1D>(const std::string &name, const Texture1D &value) const;
template bool ShaderProgram::SetSampler<Texture2D>(const std::string &name, const Texture2D &value) const;
template bool ShaderProgram::SetSampler<Texture3D>(const std::string &name, const Texture3D &value) const;
template bool ShaderProgram::SetSampler<Texture2DArray>(const std::string &name, const Texture2DArray &value) const;

std::string ShaderProgram::GetLog() const
{
    if (_linked) {
        char buf[512];
        glGetProgramInfoLog(_id, 512, NULL, buf);
        return std::string(buf);
    } else {
        if (_shaders.empty())
            return "Cannot linked because there are no shaders";
        else
            return "Cannot link because shader failed to compile";
    }
}

void ShaderProgram::PrintUniforms() const
{
    GLint  count;
    GLint  size;
    GLenum type;
    char   name[64];
    int    nameLength;

    glGetProgramiv(_id, GL_ACTIVE_UNIFORMS, &count);
    for (int i = 0; i < count; i++) {
        glGetActiveUniform(_id, i, 64, &nameLength, &size, &type, name);
        printf("%s %s\n", GLTypeToString(type), name);
    }
}

void ShaderProgram::ComputeSamplerLocations()
{
    GLint  count;
    GLint  size;
    GLenum type;
    char   name[128];
    int    nameLength;
    int    samplerId = 1;    // Starting at 1 fixes some super weird GL bug

    glGetProgramiv(_id, GL_ACTIVE_UNIFORMS, &count);
    for (int i = 0; i < count; i++) {
        glGetActiveUniform(_id, i, 128, &nameLength, &size, &type, name);
        if (IsGLTypeSampler(type)) _samplerLocations[string(name)] = samplerId++;
    }
    VAssert(samplerId <= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS - 1);
}

const char *ShaderProgram::GLTypeToString(const unsigned int type)
{
    switch (type) {
    case GL_FLOAT: return "float";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_DOUBLE: return "double";
    case GL_DOUBLE_VEC2: return "dvec2";
    case GL_DOUBLE_VEC3: return "dvec3";
    case GL_DOUBLE_VEC4: return "dvec4";
    case GL_INT: return "int";
    case GL_INT_VEC2: return "ivec2";
    case GL_INT_VEC3: return "ivec3";
    case GL_INT_VEC4: return "ivec4";
    case GL_UNSIGNED_INT: return "unsigned int";
    case GL_UNSIGNED_INT_VEC2: return "uvec2";
    case GL_UNSIGNED_INT_VEC3: return "uvec3";
    case GL_UNSIGNED_INT_VEC4: return "uvec4";
    case GL_BOOL: return "bool";
    case GL_BOOL_VEC2: return "bvec2";
    case GL_BOOL_VEC3: return "bvec3";
    case GL_BOOL_VEC4: return "bvec4";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    case GL_FLOAT_MAT2x3: return "mat2x3";
    case GL_FLOAT_MAT2x4: return "mat2x4";
    case GL_FLOAT_MAT3x2: return "mat3x2";
    case GL_FLOAT_MAT3x4: return "mat3x4";
    case GL_FLOAT_MAT4x2: return "mat4x2";
    case GL_FLOAT_MAT4x3: return "mat4x3";
    case GL_DOUBLE_MAT2: return "dmat2";
    case GL_DOUBLE_MAT3: return "dmat3";
    case GL_DOUBLE_MAT4: return "dmat4";
    case GL_DOUBLE_MAT2x3: return "dmat2x3";
    case GL_DOUBLE_MAT2x4: return "dmat2x4";
    case GL_DOUBLE_MAT3x2: return "dmat3x2";
    case GL_DOUBLE_MAT3x4: return "dmat3x4";
    case GL_DOUBLE_MAT4x2: return "dmat4x2";
    case GL_DOUBLE_MAT4x3: return "dmat4x3";
    case GL_SAMPLER_1D: return "sampler1D";
    case GL_SAMPLER_2D: return "sampler2D";
    case GL_SAMPLER_3D: return "sampler3D";
    case GL_SAMPLER_CUBE: return "samplerCube";
    case GL_SAMPLER_1D_SHADOW: return "sampler1DShadow";
    case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
    case GL_SAMPLER_1D_ARRAY: return "sampler1DArray";
    case GL_SAMPLER_2D_ARRAY: return "sampler2DArray";
    case GL_SAMPLER_1D_ARRAY_SHADOW: return "sampler1DArrayShadow";
    case GL_SAMPLER_2D_ARRAY_SHADOW: return "sampler2DArrayShadow";
    case GL_SAMPLER_2D_MULTISAMPLE: return "sampler2DMS";
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return "sampler2DMSArray";
    case GL_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow";
    case GL_SAMPLER_BUFFER: return "samplerBuffer";
    case GL_SAMPLER_2D_RECT: return "sampler2DRect";
    case GL_SAMPLER_2D_RECT_SHADOW: return "sampler2DRectShadow";
    case GL_INT_SAMPLER_1D: return "isampler1D";
    case GL_INT_SAMPLER_2D: return "isampler2D";
    case GL_INT_SAMPLER_3D: return "isampler3D";
    case GL_INT_SAMPLER_CUBE: return "isamplerCube";
    case GL_INT_SAMPLER_1D_ARRAY: return "isampler1DArray";
    case GL_INT_SAMPLER_2D_ARRAY: return "isampler2DArray";
    case GL_INT_SAMPLER_2D_MULTISAMPLE: return "isampler2DMS";
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "isampler2DMSArray";
    case GL_INT_SAMPLER_BUFFER: return "isamplerBuffer";
    case GL_INT_SAMPLER_2D_RECT: return "isampler2DRect";
    case GL_UNSIGNED_INT_SAMPLER_1D: return "usampler1D";
    case GL_UNSIGNED_INT_SAMPLER_2D: return "usampler2D";
    case GL_UNSIGNED_INT_SAMPLER_3D: return "usampler3D";
    case GL_UNSIGNED_INT_SAMPLER_CUBE: return "usamplerCube";
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return "usampler2DArray";
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return "usampler2DArray";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return "usampler2DMS";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "usampler2DMSArray";
    case GL_UNSIGNED_INT_SAMPLER_BUFFER: return "usamplerBuffer";
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return "usampler2DRect";
#ifdef GL_IMAGE_1D
    case GL_IMAGE_1D: return "image1D";
    case GL_IMAGE_2D: return "image2D";
    case GL_IMAGE_3D: return "image3D";
    case GL_IMAGE_2D_RECT: return "image2DRect";
    case GL_IMAGE_CUBE: return "imageCube";
    case GL_IMAGE_BUFFER: return "imageBuffer";
    case GL_IMAGE_1D_ARRAY: return "image1DArray";
    case GL_IMAGE_2D_ARRAY: return "image2DArray";
    case GL_IMAGE_2D_MULTISAMPLE: return "image2DMS";
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return "image2DMSArray";
    case GL_INT_IMAGE_1D: return "iimage1D";
    case GL_INT_IMAGE_2D: return "iimage2D";
    case GL_INT_IMAGE_3D: return "iimage3D";
    case GL_INT_IMAGE_2D_RECT: return "iimage2DRect";
    case GL_INT_IMAGE_CUBE: return "iimageCube";
    case GL_INT_IMAGE_BUFFER: return "iimageBuffer";
    case GL_INT_IMAGE_1D_ARRAY: return "iimage1DArray";
    case GL_INT_IMAGE_2D_ARRAY: return "iimage2DArray";
    case GL_INT_IMAGE_2D_MULTISAMPLE: return "iimage2DMS";
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "iimage2DMSArray";
    case GL_UNSIGNED_INT_IMAGE_1D: return "uimage1D";
    case GL_UNSIGNED_INT_IMAGE_2D: return "uimage2D";
    case GL_UNSIGNED_INT_IMAGE_3D: return "uimage3D";
    case GL_UNSIGNED_INT_IMAGE_2D_RECT: return "uimage2DRect";
    case GL_UNSIGNED_INT_IMAGE_CUBE: return "uimageCube";
    case GL_UNSIGNED_INT_IMAGE_BUFFER: return "uimageBuffer";
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY: return "uimage1DArray";
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: return "uimage2DArray";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE: return "uimage2DMS";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "uimage2DMSArray";
    case GL_UNSIGNED_INT_ATOMIC_COUNTER: return "atomic_uint";
#endif
    default: return "INVALID ENUM";
    }
}

bool ShaderProgram::IsGLTypeSampler(const unsigned int type)
{
    switch (type) {
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_SAMPLER_BUFFER:
    case GL_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_INT_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return true;
    default: return false;
    }
}

SmartShaderProgram::SmartShaderProgram(ShaderProgram *program) : _program(program)
{
    if (_program) _program->Bind();
}

SmartShaderProgram::~SmartShaderProgram()
{
    if (_program) _program->UnBind();
}

bool SmartShaderProgram::IsValid() const { return _program; }
