#include "QRangeSliderTextCombo.h"
#include <QBoxLayout>
#include "VDoubleValidator.h"
#include <vapor/VAssert.h>
#include <cfloat>
#include <QAction>
#include "VDoubleLineEdit.h"
#include "VDoubleRangeMenu.h"

QRangeSliderTextCombo::QRangeSliderTextCombo()
{
    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(_leftText = new VDoubleLineEdit, 20);
    layout->addWidget(_slider = new QRangeSlider, 60);
    layout->addWidget(_rightText = new VDoubleLineEdit, 20);

    _min = 0;
    _max = 1;
    SetValue(0, 1);

    connect(_slider, SIGNAL(ValueChanged(float, float)), this, SLOT(sliderChanged(float, float)));
    connect(_slider, SIGNAL(ValueChangedIntermediate(float, float)), this, SLOT(sliderChangedIntermediate(float, float)));
    connect(_slider, SIGNAL(ValueChangedBegin()), this, SIGNAL(ValueChangedBegin()));
    connect(_leftText, SIGNAL(ValueChanged(double)), this, SLOT(leftTextChanged(double)));
    connect(_rightText, SIGNAL(ValueChanged(double)), this, SLOT(rightTextChanged(double)));

    _leftText->RemoveContextMenu();
    _rightText->RemoveContextMenu();

    _leftText->SetAutoTooltip(false);
    _rightText->SetAutoTooltip(false);

    _menu = new VDoubleRangeMenu(this, _leftText->GetSciNotation(), _leftText->GetNumDigits(), _min, _max, _allowCustomRange);
    connect(_menu, &VDoubleRangeMenu::MinChanged, this, &QRangeSliderTextCombo::minChanged);
    connect(_menu, &VDoubleRangeMenu::MaxChanged, this, &QRangeSliderTextCombo::maxChanged);
    connect(_menu, &VDoubleRangeMenu::SciNotationChanged, _leftText, &VDoubleLineEdit::SetSciNotation);
    connect(_menu, &VDoubleRangeMenu::SciNotationChanged, _rightText, &VDoubleLineEdit::SetSciNotation);
    connect(_menu, &VDoubleRangeMenu::DecimalDigitsChanged, _leftText, &VDoubleLineEdit::SetNumDigits);
    connect(_menu, &VDoubleRangeMenu::DecimalDigitsChanged, _rightText, &VDoubleLineEdit::SetNumDigits);

    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
}

void QRangeSliderTextCombo::SetRange(float min, float max)
{
    if (min <= max) {
        _min = min;
        _max = max;
    }

    SetValue(_left, _right);
    _menu->SetMinimum(_min);
    _menu->SetMaximum(_max);

    setToolTip(QString::fromStdString("Min: " + std::to_string(_min) + "\n" + "Max: " + std::to_string(_max)));
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
    if (abs(_max - _min) < FLT_EPSILON)
        _slider->SetValue(0, 1);
    else
        _slider->SetValue((left - _min) / (_max - _min), (right - _min) / (_max - _min));
}

void QRangeSliderTextCombo::AllowCustomRange()
{
    if (_allowCustomRange) return;

    _allowCustomRange = true;
    _menu->AllowUserRange();
}

void QRangeSliderTextCombo::setTextboxes(float left, float right)
{
    _leftText->SetValueDouble(left);
    _rightText->SetValueDouble(right);
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

void QRangeSliderTextCombo::leftTextChanged(double left)
{
    SetValue(left, _right);
    emit ValueChanged(_left, _right);
}

void QRangeSliderTextCombo::rightTextChanged(double right)
{
    SetValue(_left, right);
    emit ValueChanged(_left, _right);
}

void QRangeSliderTextCombo::minChanged(double min)
{
    SetRange(min, _max);
    emit RangeChanged(_min, _max);
}

void QRangeSliderTextCombo::maxChanged(double max)
{
    SetRange(_min, max);
    emit RangeChanged(_min, _max);
}

void QRangeSliderTextCombo::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
}
