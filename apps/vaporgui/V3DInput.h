#pragma once

#include <QWidget>
#include <vector>

class VDoubleLineEdit;

//! \class V3DInput
//! \brief Widget that allows the user to specify a 3D point.
//! \author Stas Jaroszynski

class V3DInput : public QWidget {
    Q_OBJECT

    VDoubleLineEdit *_x, *_y, *_z;

public:
    V3DInput();
    void SetValue(double x, double y, double z);
    void SetValue(const std::vector<double> &xyz);

signals:
    void ValueChanged(double x, double y, double z);
    void ValueChangedVec(const std::vector<double> &xyz);

private slots:
    void axisValueChanged(double);
};
