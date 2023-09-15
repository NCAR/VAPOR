#pragma once

#include <QWidget>
#include <vector>

class VIntLineEdit;

//! \class V3DIntInput
//! \brief Widget that allows the user to specify a 3D integer vector
//! \author Scott Pearse

class V3DIntInput : public QWidget {
    Q_OBJECT

    VIntLineEdit *_x, *_y, *_z;

public:
    V3DIntInput();
    void SetValue(int x, int y, int z);
    void SetValue(const std::vector<int> &xyz);

signals:
    void ValueChanged(int x, int y, int z);
    void ValueChangedVec(const std::vector<int> &xyz);

private slots:
    void axisValueChanged(int);
};
