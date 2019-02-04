#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeCellTraversal : public VolumeRegular {
  public:
    VolumeCellTraversal(GLManager *gl);
    ~VolumeCellTraversal();
    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader(ShaderManager *sm);

  private:
    unsigned int minTexture;
    unsigned int maxTexture;
    unsigned int BBLevelDimTexture;
    unsigned int coordTexture;

    unsigned int VAO;
    unsigned int VBO;

    int coordDims[3];
    int BBLevels;
};

} // namespace VAPoR
