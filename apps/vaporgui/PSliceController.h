#pragma once

#include "PSection.h"

//! \class PSliceController
//! \brief Provides controls for rotating slices, offsetting them, and positioning their origin

class PDoubleSliderEdit;

class PSliceController : public PGroup {
public:
    PSliceController();
};

class PSliceOrientationSelector : public PSection {
public:
    PSliceOrientationSelector();
};

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



class PSliceOffsetSelector : public PSection {
public:
    PSliceOffsetSelector();

protected:
    PDoubleSliderEdit *_offsetSlider;

    virtual void updateGUI() const override;
    virtual bool requireDataMgr() const override { return true; }
};
