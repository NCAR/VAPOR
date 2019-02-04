#pragma once

#include <vapor/VolumeAlgorithm.h>

namespace VAPoR {

class VolumeRegular : public VolumeAlgorithm {
  public:
    VolumeRegular(GLManager *gl) : VolumeAlgorithm(gl) {}
    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader(ShaderManager *sm);
};

} // namespace VAPoR
