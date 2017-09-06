//-- ShaderProgram.h ---------------------------------------------------------
//
// Copyright (C) 2005 Kenny Gruchalla.  All rights reserved.
//
// OGSL Shader program C++ wrapper.
//
//----------------------------------------------------------------------------

#ifndef ShaderProgram_h
#define ShaderProgram_h

#include <vapor/glutil.h>
#include <map>
#include <vapor/MyBase.h>

#ifdef WIN32
#pragma warning(disable : 4251)
#endif

namespace VAPoR {

class RENDER_API ShaderProgram : public Wasp::MyBase {
  public:
    ShaderProgram();
    virtual ~ShaderProgram();

    int LoadVertexShader(string filename);
    int LoadFragmentShader(string filename);
    int LoadShader(string filename, GLenum shaderType);
    int LoadVertexSource(const string &source);
    int LoadFragmentSource(const string &source);

    int LoadVertexSource(const string &source, std::string fileName);
    int LoadFragmentSource(const string &source, std::string fileName);
    int LoadSource(const string &source, GLenum shaderType, std::string fileName);

    int Create();
    int Compile();

    int Enable();
    void Disable();

    GLint UniformLocation(string uniformName);
    GLint AttributeLocation(string attributeName) const;

    static bool Supported();
    GLuint GetProgram();
    void PrintContents();

  protected:
    GLuint _program;
    std::map<std::string, GLuint> _shaderObjects;
};

}; // namespace VAPoR

#endif // ShaderProgram_h
