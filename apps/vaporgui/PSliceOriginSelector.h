#pragma once

#include "PGroup.h"
#include "PLineItem.h"

class PDoubleSliderEdit;

//! \class PSliceOriginSelector
class PSliceOriginSelector : public PGroup {
public:
    PSliceOriginSelector();

protected:
    PDoubleSliderEdit *_xSlider;
    PDoubleSliderEdit *_ySlider;
    PDoubleSliderEdit *_zSlider;

    virtual void updateGUI() const override;
    virtual bool requireDataMgr() const override { return true; }

private:
    //void _xSliderValueChanged(float v0);
    //void _ySliderValueChanged(float v0);
    //void _zSliderValueChanged(float v0);
};

/*template<int dim> class __PSliceOriginSelector1D : public PSliceOriginSelector1D {
public:
    __PSliceOriginSelector1D() : PSliceOriginSelector1D(dim) {}
};
typedef __PSliceOriginSelector1D<0> PSliceOriginSelectorX;
typedef __PSliceOriginSelector1D<1> PSliceOriginSelectorY;
typedef __PSliceOriginSelector1D<2> PSliceOriginSelectorZ;*/
