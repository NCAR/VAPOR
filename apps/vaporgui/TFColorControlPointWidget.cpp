#include "TFControlPointWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleValidator>
#include <vapor/RenderParams.h>

TFControlPointWidget::TFControlPointWidget()
{
    QBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    this->setLayout(layout);
    layout->addWidget(_valueEdit = new QLineEdit, 30);
    layout->addWidget(_valueEditType = new QComboBox, 20);
    layout->addStretch(20);
    layout->addWidget(_opacityEdit = new QLineEdit, 30);
    
    _valueEdit->setValidator(new QDoubleValidator);
    _opacityEdit->setValidator(new QDoubleValidator(0, 1, 10));
    
    _valueEditType->blockSignals(true);
    _valueEditType->setMinimumSize(30, 10);
    _valueEditType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _valueEditType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _valueEditType->addItem("data");
    _valueEditType->addItem("%");
    _valueEditType->blockSignals(false);
    
    connect(_valueEditType, SIGNAL(currentIndexChanged(int)), this, SLOT(valueEditTypeChanged(int)));
    connect(_valueEdit, SIGNAL(returnPressed()), this, SLOT(valueEditChanged()));
    connect(_opacityEdit, SIGNAL(returnPressed()), this, SLOT(opacityEditChanged()));
    
    this->setDisabled(true);
}

void TFControlPointWidget::Update(VAPoR::RenderParams *rParams)
{
    if (!rParams)
        return;
    
    _min = rParams->GetMapperFunc(rParams->GetVariableName())->getMinMapValue();
    _max = rParams->GetMapperFunc(rParams->GetVariableName())->getMaxMapValue();
    
    ((QDoubleValidator*)_valueEdit->validator())->setRange(_min, _max);
}

//void TFControlPointWidget::SelectOpacityControlPoint(int index)
//{
//    this->setEnabled(true);
//    _opacityId = index;
//    _colorId = -1;
//    Update(_params);
//}

//void TFControlPointWidget::SelectColorControlPoint(int index)
//{
//    this->setEnabled(true);
//    _opacityId = -1;
//    _colorId = index;
//    Update(_params);
//}

void TFControlPointWidget::DeselectControlPoint()
{
    this->setDisabled(true);
//    _opacityId = -1;
//    _colorId = -1;
    _valueEdit->clear();
    _opacityEdit->clear();
}

void TFControlPointWidget::SetNormalizedValue(float value)
{
    _value = value;
    updateValue();
}

void TFControlPointWidget::SetOpacity(float opacity)
{
    _opacity = opacity;
    updateOpacity();
}

void TFControlPointWidget::SetControlPoint(float value, float opacity)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
    SetOpacity(opacity);
}

void TFControlPointWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    QWidget::paintEvent(event);
}

void TFControlPointWidget::updateValue()
{
    if (!isEnabled())
        return;
    
    float value = _value;
    if (isUsingMappedValue())
        value = toMappedValue(value);
    else
        value *= 100;
    _valueEdit->setText(QString::number(value));
}

void TFControlPointWidget::updateOpacity()
{
    if (!isEnabled())
        return;
    
    _opacityEdit->setText(QString::number(_opacity));
}

bool TFControlPointWidget::isUsingNormalizedValue() const
{
    return _valueEditType->currentIndex() == ValueFormat::Percent;
}

bool TFControlPointWidget::isUsingMappedValue() const
{
    return _valueEditType->currentIndex() == ValueFormat::Mapped;
}

float TFControlPointWidget::toMappedValue(float normalized) const
{
    return normalized * (_max - _min) + _min;
}

float TFControlPointWidget::toNormalizedValue(float mapped) const
{
    if (_max == _min)
        return 0;
    
    return (mapped - _min) / (_max - _min);
}

float TFControlPointWidget::getValueFromEdit() const
{
    float value = _valueEdit->text().toFloat();
    if (isUsingMappedValue())
        return toNormalizedValue(value);
    else
        return value / 100.f;
}

float TFControlPointWidget::getOpacityFromEdit() const
{
    return _opacityEdit->text().toFloat();
}

void TFControlPointWidget::valueEditTypeChanged(int)
{
    updateValue();
}

void TFControlPointWidget::valueEditChanged()
{
    _value = getValueFromEdit();
    ControlPointChanged(_value, _opacity);
}

void TFControlPointWidget::opacityEditChanged()
{
    _opacity = getOpacityFromEdit();
    ControlPointChanged(_value, _opacity);
}
