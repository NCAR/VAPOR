#pragma once

#include "PGroup.h"
#include "PLineItem.h"

class VDoubleSliderEdit;

//! \class PSliceSampleLocationSelector
class PSliceSampleLocationSelector : public PGroup {
public:
    PSliceSampleLocationSelector();
};

class PSliceSampleLocationSelector1D : public PLineItem {
    VDoubleSliderEdit *_slider;
    const int          _dim;

public:
    PSliceSampleLocationSelector1D(int dim);

protected:
    virtual void updateGUI() const override;
    virtual bool isShown() const override;
    virtual bool requireDataMgr() const override { return true; }

private:
    void sliderValueChanged(float v0);
};

template<int dim> class __PSliceSampleLocationSelector1D : public PSliceSampleLocationSelector1D {
public:
    __PSliceSampleLocationSelector1D() : PSliceSampleLocationSelector1D(dim) {}
};
typedef __PSliceSampleLocationSelector1D<0> PSliceSampleLocationSelectorX;
typedef __PSliceSampleLocationSelector1D<1> PSliceSampleLocationSelectorY;
typedef __PSliceSampleLocationSelector1D<2> PSliceSampleLocationSelectorZ;
