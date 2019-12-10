#include "QRangeSliderTextCombo.h"
#include <QBoxLayout>
#include <QDoubleValidator>
#include <vapor/VAssert.h>
#include <cfloat>
#include <QAction>

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
    VAssert(_max >= _min);
    _min = min;
    _max = max;

    if (_allowCustomRange) {
        min = -FLT_MAX;
        max = FLT_MAX;
    }

    setValidator(_leftText, new QDoubleValidator(min, max, 100));
    setValidator(_rightText, new QDoubleValidator(min, max, 100));
    SetValue(_left, _right);
}

void QRangeSliderTextCombo::SetValue(float left, float right)
{
    if (left > right) std::swap(left, right);
    if (!_allowCustomRange) {
        left = std::max(_min, left);
        right = std::min(_max, right);
    }
    _left = left;
    _right = right;
    setTextboxes(left, right);
    _slider->SetValue((left - _min) / (_max - _min), (right - _min) / (_max - _min));
}

void QRangeSliderTextCombo::AllowCustomRange()
{
    if (_allowCustomRange) return;

    _allowCustomRange = true;

    QAction *newMinAction = new QAction("Set as new min", this);
    QAction *newMaxAction = new QAction("Set as new max", this);
    QAction *resetRangeAction = new QAction("Reset range to default", this);
    QObject::connect(newMinAction, &QAction::triggered, this, &QRangeSliderTextCombo::makeLeftValueNewMin);
    QObject::connect(newMaxAction, &QAction::triggered, this, &QRangeSliderTextCombo::makeRightValueNewMax);
    QObject::connect(resetRangeAction, &QAction::triggered, this, &QRangeSliderTextCombo::RangeDefaultRequested);

    _leftText->setContextMenuPolicy(Qt::ActionsContextMenu);
    _rightText->setContextMenuPolicy(Qt::ActionsContextMenu);
    _leftText->addAction(newMinAction);
    _rightText->addAction(newMaxAction);
    _leftText->addAction(resetRangeAction);
    _rightText->addAction(resetRangeAction);
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

float QRangeSliderTextCombo::getRange() const
{
    float range = _max - _min;
    if (range < FLT_EPSILON)
        return 1;
    else
        return range;
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

void QRangeSliderTextCombo::makeLeftValueNewMin()
{
    SetRange(_left, _max);
    emit RangeChanged(_min, _max);
}

void QRangeSliderTextCombo::makeRightValueNewMax()
{
    SetRange(_min, _right);
    emit RangeChanged(_min, _max);
}
