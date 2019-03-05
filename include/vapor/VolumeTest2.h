#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeTest2 : public VolumeRegular {
  public:
    VolumeTest2(GLManager *gl);
    ~VolumeTest2();

    static std::string GetName() { return "Test2"; }

    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader() const;

  private:
    unsigned int xyCoordTexture;
    unsigned int zCoordTexture;
};

} // namespace VAPoR
