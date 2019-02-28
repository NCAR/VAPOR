#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeResampled : public VolumeRegular {
  public:
    VolumeResampled(GLManager *gl) : VolumeRegular(gl) {}

    static std::string GetName() { return "Resampled"; }

    virtual int LoadData(const Grid *grid);
};

} // namespace VAPoR
