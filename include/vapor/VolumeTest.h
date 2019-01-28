#pragma once

#include <vapor/VolumeAlgorithm.h>

namespace VAPoR {

class VolumeTest : public VolumeAlgorithm {
  public:
    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader(ShaderManager *sm);
};

} // namespace VAPoR
