#pragma once

#include <vapor/MyBase.h>
#include <string>

namespace VAPoR {

//! \class Shader
//! \ingroup Public_Render
//!
//! \brief Provides a C++ interface to the OpenGL shader construct
//!
//! \author Stanislaw Jaroszynski
//! \date    August, 2018

class RENDER_API Shader : public Wasp::MyBase {
    unsigned int _id;
    int          _successStatus;
    bool         _compiled;
    unsigned int _type;
    std::string  _name;

public:
    //! Allocate a new C++ shader and specify the type. An OpenGL shader
    //! is not allocated until it is compiled
    //!
    //! \param[in] type OpenGL shader type enum
    //!
    Shader(unsigned int type);
    ~Shader();

    //! \param[in] source GLSL source code
    //!
    //! \retval 1 is returned on success
    //! \retval -1 is returned on failure
    //!
    int CompileFromSource(const std::string &source);

    //! Calls glGetShaderInfoLog
    std::string GetLog() const;

    unsigned int GetID() const;
    unsigned int GetType() const;
    bool         WasCompilationSuccessful() const;
};

}    // namespace VAPoR
