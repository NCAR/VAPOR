#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeCellTraversal : public VolumeRegular {
  public:
    VolumeCellTraversal();
    ~VolumeCellTraversal();
    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader(ShaderManager *sm);

  private:
    unsigned int xyCoordTexture;
    unsigned int coordTexture;

    int coordDims[3];
};

} // namespace VAPoR
