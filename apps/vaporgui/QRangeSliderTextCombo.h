#pragma once

#include "QRangeSlider.h"
#include <QLineEdit>
#include <QWidget>

//! \class QRangeSliderTextCombo
//! Combines a QRangeSlider with two text inputs that represent the values of
//! the min/max sliders. The two are automatically synced.

class QRangeSliderTextCombo : public QWidget {
    Q_OBJECT

    QRangeSlider *_slider;
    QLineEdit *   _leftText;
    QLineEdit *   _rightText;
    float         _min, _max;
    float         _left, _right;

public:
    QRangeSliderTextCombo();

    void SetRange(float min, float max);
    void SetValue(float left, float right);

private:
    void  setValidator(QLineEdit *edit, QValidator *validator);
    void  setTextboxes(float left, float right);
    float getRange() const;

private slots:
    void sliderChangedIntermediate(float min, float max);
    void sliderChanged(float min, float max);
    void leftTextChanged();
    void rightTextChanged();

signals:
    //! User began to change the value.
    void ValueChangedBegin();
    //! User finalized changing the value.
    void ValueChanged(float min, float max);
    //! User changed the value but they have not finalized it.
    void ValueChangedIntermediate(float min, float max);
};
