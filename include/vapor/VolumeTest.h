#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeTest : public VolumeRegular {
  public:
    VolumeTest(GLManager *gl);
    ~VolumeTest();

    static std::string GetName() { return "Test"; }

    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader(ShaderManager *sm);

  private:
    unsigned int xyCoordTexture;
    unsigned int zCoordTexture;
};

} // namespace VAPoR
