#pragma once

#include "PSection.h"

class PDoubleSliderEdit;

class PSliceOriginSelector : public PSection {
public:
    PSliceOriginSelector();

protected:
    PDoubleSliderEdit *_xSlider;
    PDoubleSliderEdit *_ySlider;
    PDoubleSliderEdit *_zSlider;

    virtual void updateGUI() const override;
    virtual bool requireDataMgr() const override { return true; }
};
