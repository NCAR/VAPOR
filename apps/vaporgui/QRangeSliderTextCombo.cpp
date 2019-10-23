#include "QRangeSliderTextCombo.h"
#include <QBoxLayout>
#include <QDoubleValidator>
#include <vapor/VAssert.h>

QRangeSliderTextCombo::QRangeSliderTextCombo()
{
    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(_leftText = new QLineEdit, 20);
    layout->addWidget(_slider = new QRangeSlider, 60);
    layout->addWidget(_rightText = new QLineEdit, 20);

    _min = 0;
    _max = 1;
    SetValue(0, 1);

    connect(_slider, SIGNAL(ValueChanged(float, float)), this, SLOT(sliderChanged(float, float)));
    connect(_slider, SIGNAL(ValueChangedIntermediate(float, float)), this, SLOT(sliderChangedIntermediate(float, float)));
    connect(_slider, SIGNAL(ValueChangedBegin()), this, SIGNAL(ValueChangedBegin()));
    connect(_leftText, SIGNAL(returnPressed()), this, SLOT(leftTextChanged()));
    connect(_rightText, SIGNAL(returnPressed()), this, SLOT(rightTextChanged()));
}

void QRangeSliderTextCombo::SetRange(float min, float max)
{
    VAssert(_max - _min != 0);
    _min = min;
    _max = max;
    setValidator(_leftText, new QDoubleValidator(min, max, 100));
    setValidator(_rightText, new QDoubleValidator(min, max, 100));
    SetValue(_left, _right);
}

void QRangeSliderTextCombo::SetValue(float left, float right)
{
    _left = left;
    _right = right;
    setTextboxes(left, right);
    _slider->SetValue((left - _min) / (_max - _min), (right - _min) / (_max - _min));
}

void QRangeSliderTextCombo::setValidator(QLineEdit *edit, QValidator *validator)
{
    const QValidator *toDelete = edit->validator();
    edit->setValidator(validator);
    if (toDelete) delete toDelete;
}

void QRangeSliderTextCombo::setTextboxes(float left, float right)
{
    _leftText->setText(QString::number(left));
    _rightText->setText(QString::number(right));
}

void QRangeSliderTextCombo::sliderChangedIntermediate(float leftNorm, float rightNorm)
{
    float left = (_max - _min) * leftNorm + _min;
    float right = (_max - _min) * rightNorm + _min;
    setTextboxes(left, right);
    emit ValueChangedIntermediate(left, right);
}

void QRangeSliderTextCombo::sliderChanged(float leftNorm, float rightNorm)
{
    float left = (_max - _min) * leftNorm + _min;
    float right = (_max - _min) * rightNorm + _min;
    SetValue(left, right);
    emit ValueChanged(_left, _right);
}

void QRangeSliderTextCombo::leftTextChanged()
{
    float left = _leftText->text().toDouble();
    SetValue(left, _right);
    emit ValueChanged(_left, _right);
}

void QRangeSliderTextCombo::rightTextChanged()
{
    float right = _rightText->text().toDouble();
    SetValue(_left, right);
    emit ValueChanged(_left, _right);
}
