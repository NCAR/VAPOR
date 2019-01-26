#pragma once

#include <vapor/Grid.h>
#include <vapor/ShaderManager.h>
#include <vector>
#include <string>

namespace VAPoR {

class VolumeAlgorithm {
public:
    virtual ~VolumeAlgorithm() {}
    virtual int            LoadData(const Grid *grid) = 0;
    virtual ShaderProgram *GetShader(ShaderManager *sm) = 0;

    static const std::vector<std::string> &GetAlgorithmNames();
    static VolumeAlgorithm *               NewAlgorithm(const std::string &name);

private:
    static const std::vector<std::string> _algorithmNames;
};

}    // namespace VAPoR
