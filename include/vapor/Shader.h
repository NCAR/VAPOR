#pragma once

#include <string>

namespace VAPoR {

class Shader {
    unsigned int _id;
    int          _successStatus;
    bool         _compiled;
    unsigned int _type;
    std::string  _name;

public:
    Shader(unsigned int type);
    ~Shader();

    bool CompileFromSource(const char *source);
    bool CompileFromFile(const std::string path);

    std::string  GetLog() const;
    unsigned int GetID() const;
    unsigned int GetType() const;
    bool         WasCompilationSuccessful() const;
};

}    // namespace VAPoR
