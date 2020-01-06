#pragma once

#include <QLabel>
#include <QWidget>
#include "QRangeSliderTextCombo.h"

//! class VGeometry2
//!
//! Wraps three QRangeSliderCombo objects and provides vaporgui's standard
//! setter/getter functions and signals.  This is used to select a 2D or 3D
//! region within Vapor's scene.

class VGeometry2 : public QWidget {
    Q_OBJECT

public:
    VGeometry2();

    void SetRange(const std::vector<float> &range);

    void SetValue(const std::vector<float> &vals);
    void GetValue(std::vector<float> &vals) const;

signals:
    void ValueChanged(const std::vector<float> &);

private slots:
    void _xRangeChanged(float min, float max);
    void _yRangeChanged(float min, float max);
    void _zRangeChanged(float min, float max);

private:
    std::vector<float>     _values;
    std::vector<float>     _range;
    QRangeSliderTextCombo *_xRange, *_yRange, *_zRange;
    QLabel *               _zLabel;
};
