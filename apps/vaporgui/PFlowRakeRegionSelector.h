#pragma once

#include "PRegionSelector.h"

class PFlowRakeRegionSelector1D : public PRegionSelector1D {
public:
    using PRegionSelector1D::PRegionSelector1D;

protected:
    VAPoR::Box *getBox() const override;
};
