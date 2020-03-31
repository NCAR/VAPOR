#pragma once

#include "PLineItem.h"

class QRangeSliderTextCombo;

//! \class PRegionSelector
//! Placeholder. Not in use
//! Cannot ifdef out otherwise Qt MOC will fail.
class PRegionSelector : public PWidget {
    Q_OBJECT

    QWidget *              _container;
    QRangeSliderTextCombo *_sx;
    QRangeSliderTextCombo *_sy;
    QRangeSliderTextCombo *_sz;

public:
    PRegionSelector(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    void checkboxStateChanged(bool on);
};
