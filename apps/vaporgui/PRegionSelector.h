#pragma once

#include "PSection.h"
#include "PLineItem.h"
#include <vapor/ControlExecutive.h>

class QRangeSliderTextCombo;

namespace VAPoR {
class Box;
}

//! \class PRegionSelector
class PRegionSelector : public PSection {
public:
    PRegionSelector(VAPoR::ControlExec* ce= nullptr, const std::string &label = "Region");
    //PRegionSelector(VAPoR::ControlExec* ce= nullptr, const std::string &label = "Region", VAPoR::ControlExec* ce = nullptr);
};

class PRegionSelector1D : public PLineItem {
    //VAPoR::ControlExec    *_ce;
    QRangeSliderTextCombo *_slider;
    const int              _dim;

public:
    PRegionSelector1D(int dim, VAPoR::ControlExec* ce);

protected:
    VAPoR::ControlExec    *_ce;
    virtual void        updateGUI() const override;
    virtual bool        isShown() const override;
    virtual bool        requireDataMgr() const override { return true; }
    virtual VAPoR::Box *getBox() const;

private:
    void sliderValueChanged(float v0, float v1);
};

template<int dim> class __PRegionSelector1D : public PRegionSelector1D {
public:
    __PRegionSelector1D(VAPoR::ControlExec* ce) : PRegionSelector1D(dim, ce) {}
//private:
//    VAPoR::ControlExec* _ce;
};
typedef __PRegionSelector1D<0> PRegionSelectorX;
typedef __PRegionSelector1D<1> PRegionSelectorY;
typedef __PRegionSelector1D<2> PRegionSelectorZ;
