#include "vapor/glutil.h"

#include <stdio.h>
#include <cassert>

#include "vapor/Shader.h"

using namespace VAPoR;

Shader::Shader(unsigned int type) : _id(0), _successStatus(0), _compiled(false), _type(type) {}

Shader::~Shader()
{
    if (_id) glDeleteShader(_id);
}

int Shader::CompileFromSource(const std::string &source)
{
    assert(!_compiled);
    char *buffer = new char[source.length() + 1];
    strcpy(buffer, source.c_str());
    _id = glCreateShader(_type);
    glShaderSource(_id, 1, &buffer, NULL);
    glCompileShader(_id);
    glGetShaderiv(_id, GL_COMPILE_STATUS, &_successStatus);
    _compiled = true;
    delete[] buffer;

    if (!_successStatus) {
        SetErrMsg("--------------- Shader Compilation Failed ---------------\n"
                  "%s"
                  "---------------------------------------------------------\n",
                  GetLog().c_str());
        return -1;
    }
    return 1;
}

std::string Shader::GetLog() const
{
    char buf[512];
    glGetShaderInfoLog(_id, 512, NULL, buf);
    return std::string(buf);
}

unsigned int Shader::GetID() const { return _id; }
unsigned int Shader::GetType() const { return _type; }
bool         Shader::WasCompilationSuccessful() const { return _successStatus; }
