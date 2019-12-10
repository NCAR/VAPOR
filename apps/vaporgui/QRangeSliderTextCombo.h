#pragma once

#include "QRangeSlider.h"
#include <QLineEdit>
#include <QWidget>
#include <QFrame>

//! \class QRangeSliderTextCombo
//! Combines a QRangeSlider with two text inputs that represent the values of
//! the min/max sliders. The two are automatically synced.

class QRangeSliderTextCombo : public QFrame {
    Q_OBJECT

    QRangeSlider *_slider;
    QLineEdit *   _leftText;
    QLineEdit *   _rightText;
    float         _min, _max;
    float         _left, _right;
    bool          _allowCustomRange = false;

public:
    QRangeSliderTextCombo();

    void SetRange(float min, float max);
    void SetValue(float left, float right);

    //! Allows the user to input values outside of the range and allows them to change the range.
    void AllowCustomRange();

private:
    void  setValidator(QLineEdit *edit, QValidator *validator);
    void  setTextboxes(float left, float right);
    float getRange() const;

private slots:
    void sliderChangedIntermediate(float min, float max);
    void sliderChanged(float min, float max);
    void leftTextChanged();
    void rightTextChanged();
    void makeLeftValueNewMin();
    void makeRightValueNewMax();

signals:
    //! User began to change the value.
    void ValueChangedBegin();
    //! User finalized changing the value.
    void ValueChanged(float min, float max);
    //! User changed the value but they have not finalized it.
    void ValueChangedIntermediate(float min, float max);

    void RangeChanged(float min, float max);
    void RangeDefaultRequested();
};
