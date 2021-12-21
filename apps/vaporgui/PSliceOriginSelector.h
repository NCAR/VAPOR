#pragma once

#include "PSection.h"

//! \class PSliceOriginSelector
//! \brief POrovides three sliders that control the origin of a slice.  Minimum and maximum values along each axis are recalculated on each updateGUI() call.
//! \author Scott Pearse

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
