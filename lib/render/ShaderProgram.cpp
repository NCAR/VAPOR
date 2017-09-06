//-- ShaderProgram.cpp -------------------------------------------------------
//
// Copyright (C) 2005 Kenny Gruchalla.  All rights reserved.
//
// OGSL Shader program C++ wrapper.
//
// Changelog:
// 06/30/2011 - Yannick Polius
// Modified to accomodate the new VAPOR shader architecture
//
//----------------------------------------------------------------------------

#include <vapor/glutil.h> // Must be included first!!!
#include <vapor/ShaderProgram.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace VAPoR;
using namespace Wasp;

//----------------------------------------------------------------------------
// Default constructor
//----------------------------------------------------------------------------
ShaderProgram::ShaderProgram() {
    _program = 0;
    _shaderObjects.clear();
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
ShaderProgram::~ShaderProgram() {
    printOpenGLError();
    for (std::map<std::string, GLuint>::const_iterator iter = _shaderObjects.begin();
         iter != _shaderObjects.end(); iter++) {

        GLuint shader = iter->second;
#ifdef DEBUG
        std::cout << "Attempting to delete shader obj: " << shader << " prog: " << _program << std::endl;
#endif
        glDetachShader(_program, shader);
#ifdef DEBUG
        if (printOpenGLError() != 0)
            std::cout << "Delete " << shader << " FAILED" << std::endl;
#endif
        if (GLEW_VERSION_2_0) {
            glDeleteShader(shader);
        } else {
            glDeleteObjectARB(shader);
        }
    }

    printOpenGLError();
    if (GLEW_VERSION_2_0) {
        glDeleteProgram(_program);
    } else {
        glDeleteObjectARB(_program);
    }
    _shaderObjects.clear();
    printOpenGLError();
}

//----------------------------------------------------------------------------
// Load vertex shader source from a file.
//----------------------------------------------------------------------------
int ShaderProgram::LoadVertexShader(string filename) {
    return LoadShader(filename, GL_VERTEX_SHADER);
}

//----------------------------------------------------------------------------
// Load fragment shader source from a file.
//----------------------------------------------------------------------------
int ShaderProgram::LoadFragmentShader(string filename) {
    return LoadShader(filename, GL_FRAGMENT_SHADER);
}

//----------------------------------------------------------------------------
// Load shader source from a file.
//
// The shader is assumed to live in the executable directory.
//----------------------------------------------------------------------------
int ShaderProgram::LoadShader(string filename, GLenum shaderType) {

    //
    // Read the file
    //
    ifstream in;
    in.open(filename);
    if (!in) {
        SetErrMsg("fopen(%s) : %M", filename.c_str());
        return (-1);
    }

    std::ostringstream contents;
    contents << in.rdbuf();
    if (!in) {
        SetErrMsg("Error reading file %s : %M", filename.c_str());
        return (-1);
    }

    in.close();
    string str = contents.str();

    const GLchar *sourceBuffer = (const GLchar *)str.c_str();
    int size = str.size();

    GLuint shader;
    if (GLEW_VERSION_2_0) {
        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &sourceBuffer, &size);
        if (shader == 0) {
            SetErrMsg("glCreateShader()");
            return (-1);
        }

        glAttachShader(_program, shader);

        if (printOpenGLError() < 0)
            return (-1);
    } else {
        shader = glCreateShaderObjectARB(shaderType);
        glShaderSourceARB(shader, 1, &sourceBuffer, &size);
        if (shader == 0) {
            SetErrMsg("glCreateShader()");
            return (-1);
        }

        glAttachObjectARB(_program, shader);

        if (printOpenGLError() < 0)
            return (-1);
    }

    _shaderObjects[std::string(filename)] = shader;

    SetDiagMsg("creating shader obj: %d, program: %d", shader, _program);

    return 0;
}

//----------------------------------------------------------------------------
// Load vertex shader source from a string.
//----------------------------------------------------------------------------
int ShaderProgram::LoadVertexSource(const string &source, std::string fileName) {
    return LoadSource(source, GL_VERTEX_SHADER, fileName);
}
//----------------------------------------------------------------------------
// Load vertex shader source from a string.
//----------------------------------------------------------------------------
int ShaderProgram::LoadVertexSource(const string &source) {
    return LoadSource(source, GL_VERTEX_SHADER, "");
}
//----------------------------------------------------------------------------
// Load fragment shader source from a file.
//----------------------------------------------------------------------------
int ShaderProgram::LoadFragmentSource(const string &source) {
    return LoadSource(source, GL_FRAGMENT_SHADER, "");
}
//----------------------------------------------------------------------------
// Load fragment shader source from a file.
//----------------------------------------------------------------------------
int ShaderProgram::LoadFragmentSource(const string &source, std::string fileName) {
    return LoadSource(source, GL_FRAGMENT_SHADER, fileName);
}

//----------------------------------------------------------------------------
// Create the shader from the char* source
//----------------------------------------------------------------------------
int ShaderProgram::LoadSource(const string &source, GLenum shaderType, std::string fileName) {
    GLuint shader;

    //
    // Create a shader object, and load it's source
    //
    int length = source.size();
    const GLchar *sourceBuffer = (const GLchar *)source.c_str();

    if (GLEW_VERSION_2_0) {
        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &sourceBuffer, &length);
        if (shader == 0) {
            SetErrMsg("glCreateShader()");
            return (-1);
        }

        //
        // Attach the shader
        //
        glAttachShader(_program, shader);
        if (printOpenGLError() != 0)
            return (-1);
    } else {
        shader = glCreateShaderObjectARB(shaderType);
        glShaderSourceARB(shader, 1, &sourceBuffer, &length);
        if (shader == 0) {
            SetErrMsg("glCreateShader()");
            return (-1);
        }

        //
        // Attach the shader
        //
        glAttachObjectARB(_program, shader);
        if (printOpenGLError() != 0)
            return (-1);
    }
    _shaderObjects[fileName] = shader;

    SetDiagMsg("creating shader obj: %d, program: %d", shader, _program);
    return true;
}

//----------------------------------------------------------------------------
// Create the shader program
//----------------------------------------------------------------------------
int ShaderProgram::Create() {
    if (GLEW_VERSION_2_0) {
        _program = glCreateProgram();
    } else {
        _program = glCreateProgramObjectARB();
    }

    if (printOpenGLError() != 0)
        return (-1);
    return (true);
}

//----------------------------------------------------------------------------
// Compile and link the shader program
//----------------------------------------------------------------------------
int ShaderProgram::Compile() {
    //
    // Compile all the shaders
    //

    bool compileSuccess = true;
    std::string shaderErrors("");
    for (std::map<std::string, GLuint>::const_iterator iter = _shaderObjects.begin();
         iter != _shaderObjects.end(); ++iter) {

        GLuint shader = iter->second;
        GLint infologLength = 0;
        GLint shaderCompiled;

        if (GLEW_VERSION_2_0) {
            //
            // Compile a single shader
            //
            glCompileShader(shader);
            glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
            if (printOpenGLError() != 0)
                return (-1);

            //
            // Print log information
            //
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
            if (printOpenGLError() != 0)
                return (-1);

            if (shaderCompiled != GL_TRUE && infologLength > 1) {
                GLchar *infoLog = new GLchar[infologLength];
                int nWritten = 0;

                glGetShaderInfoLog(shader, infologLength, &nWritten, infoLog);

                std::string output = iter->first + ":\t\n " + (char *)infoLog;

                shaderErrors += output;

                delete[] infoLog;
                infoLog = NULL;
            }
        } else {
            //
            // Compile a single shader
            //
            glCompileShaderARB(shader);
            glGetObjectParameterivARB(shader,
                                      GL_OBJECT_COMPILE_STATUS_ARB,
                                      &shaderCompiled);
            if (printOpenGLError() != 0)
                return (-1);

            //
            // Print log information
            //
            glGetObjectParameterivARB(shader,
                                      GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                      &infologLength);
            if (printOpenGLError() != 0)
                return (-1);

            if (shaderCompiled != GL_TRUE && infologLength > 1) {
                GLchar *infoLog = new GLchar[infologLength];
                int nWritten = 0;

                glGetInfoLogARB(shader, infologLength, &nWritten, infoLog);

                std::string output = iter->first + ":\t\n " + (char *)infoLog;

                shaderErrors += output;
                delete[] infoLog;
                infoLog = NULL;
            }
        }

        //if (!compiled)
        if (shaderCompiled != GL_TRUE) {
            compileSuccess = false;
        }
    }

    if (!compileSuccess) {
        //print out errrors and exit
        SetErrMsg(
            "Shader InfoLog:\n%s", (GLchar *)shaderErrors.c_str());
        return -1;
    }

    //
    // Link the program
    //
    GLint linked = 0;

    if (GLEW_VERSION_2_0) {
        glLinkProgram(_program);
        glGetProgramiv(_program, GL_LINK_STATUS, &linked);
        if (printOpenGLError() != 0)
            return (-1);

        //
        // Print log information
        //
        GLint infologLength = 0;
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infologLength);
        if (printOpenGLError() != 0)
            return (-1);

        if (linked != GL_TRUE && infologLength > 1) {
            GLchar *infoLog = new GLchar[infologLength];
            int nWritten = 0;

            glGetProgramInfoLog(_program, infologLength, &nWritten, infoLog);

            SetErrMsg("ShaderProgram Link Error: %s\n", infoLog);

            delete[] infoLog;
            infoLog = NULL;
        }
    } else {
        glLinkProgramARB(_program);
        glGetObjectParameterivARB(_program, GL_OBJECT_LINK_STATUS_ARB, &linked);
        if (printOpenGLError() != 0)
            return (-1);

        //
        // Print log information
        //
        GLint infologLength = 0;
        glGetObjectParameterivARB(_program,
                                  GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);
        if (printOpenGLError() != 0)
            return (-1);

        if (linked != GL_TRUE && infologLength > 1) {
            GLchar *infoLog = new GLchar[infologLength];
            int nWritten = 0;

            glGetInfoLogARB(_program, infologLength, &nWritten, infoLog);

            SetErrMsg("ShaderProgram Link Error: %s\n", infoLog);

            delete[] infoLog;
            infoLog = NULL;
        }
    }

    if (!linked)
        return (-1);
    return (0);
}

//----------------------------------------------------------------------------
// Enable the shader program.
//----------------------------------------------------------------------------
int ShaderProgram::Enable() {
    if (GLEW_VERSION_2_0) {
        glUseProgram(_program);
    } else {
        glUseProgramObjectARB(_program);
    }

    if (printOpenGLError() != 0)
        return (-1);
    return (0);
}

//----------------------------------------------------------------------------
// Disable the shader program.
//----------------------------------------------------------------------------
void ShaderProgram::Disable() {
    if (GLEW_VERSION_2_0) {
        glUseProgram(0);
    } else {
        glUseProgramObjectARB(0);
    }

    printOpenGLError();
}

//----------------------------------------------------------------------------
// Find uniform location in the shader.
//----------------------------------------------------------------------------
GLint ShaderProgram::UniformLocation(string uniformName) {
    GLint location;
    if (GLEW_VERSION_2_0) {
        location = glGetUniformLocation(_program, uniformName.c_str());
    } else {
        location = glGetUniformLocationARB(_program, uniformName.c_str());
    }
    if (location == -1) {
        SetErrMsg("uniform \"%s\" not found.\n", uniformName.c_str());
    }

    printOpenGLError();

    return location;
}

//----------------------------------------------------------------------------
// Find attribute location in the shader.
//----------------------------------------------------------------------------
GLint ShaderProgram::AttributeLocation(string attributeName) const {
    GLint location;
    if (GLEW_VERSION_2_0) {
        location = glGetAttribLocation(_program, attributeName.c_str());
    } else {
        location = glGetAttribLocationARB(_program, attributeName.c_str());
    }
    if (location == -1) {
        SetErrMsg("attribute \"%s\" not found.\n", attributeName.c_str());
    }

    printOpenGLError();

    return location;
}

//----------------------------------------------------------------------------
// Determine if shader programming is supported.
//----------------------------------------------------------------------------
bool ShaderProgram::Supported() {
    return (GLEW_VERSION_2_0 || GLEW_ARB_shader_objects);
}

GLuint ShaderProgram::GetProgram() {
    return _program;
}

void ShaderProgram::PrintContents() {
    std::cout << "Program " << (int)_program << " Shader Objects: \n\t";
    for (std::map<std::string, GLuint>::const_iterator iter = _shaderObjects.begin();
         iter != _shaderObjects.end(); ++iter) {
        std::cout << iter->first << "\n\t";
    }
    std::cout << std::endl;
}
