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

    static std::vector<std::string> GetAlgorithmNames();
    static VolumeAlgorithm *        NewAlgorithm(const std::string &name);
};

}    // namespace VAPoR
