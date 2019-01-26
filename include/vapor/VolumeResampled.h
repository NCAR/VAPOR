#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeResampled : public VolumeRegular {
  public:
    virtual int LoadData(const Grid *grid);
};

} // namespace VAPoR
