#include "vapor/glutil.h"

#include <stdio.h>
#include <cassert>

#include "vapor/Shader.h"

using namespace VAPoR;

Shader::Shader(unsigned int type) : _id(0), _successStatus(0), _compiled(false), _type(type) {}

Shader::~Shader()
{
    if (_id) printf("GL Freeing shader %i\n", _id);
    if (_id) glDeleteShader(_id);
}

bool Shader::CompileFromSource(const char *source)
{
    assert(!_compiled);
    _id = glCreateShader(_type);
    printf("GL Create Shader %i\n", _id);
    glShaderSource(_id, 1, &source, NULL);
    glCompileShader(_id);
    glGetShaderiv(_id, GL_COMPILE_STATUS, &_successStatus);
    _compiled = true;
    assert(_successStatus);
    return _successStatus;
}

bool Shader::CompileFromFile(const std::string path)
{
    _name = path;
    FILE *f = fopen(path.c_str(), "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        long length = ftell(f);
        rewind(f);
        char *buf = new char[length + 1];
        fread(buf, length, 1, f);
        buf[length] = 0;
        bool ret = CompileFromSource(buf);
        delete[] buf;
        return ret;
    } else {
        // TODO GL
        fprintf(stderr, "File \"%s\" not found\n", path.c_str());
        return false;
    }
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
