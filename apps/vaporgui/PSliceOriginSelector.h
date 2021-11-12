#pragma once

#include "PSection.h"
#include "PLineItem.h"

class PDoubleSliderEdit;

//! \class PSliceOriginSelector
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
