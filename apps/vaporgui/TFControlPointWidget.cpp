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
    layout->addWidget(_locationEdit = new QLineEdit, 30);
    layout->addWidget(_locationEditType = new QComboBox, 20);
    layout->addStretch(20);
    layout->addWidget(_valueEdit = new QLineEdit, 30);
    
    _locationEdit->setValidator(new QDoubleValidator);
    
    _locationEditType->blockSignals(true);
    _locationEditType->setMinimumSize(30, 10);
    _locationEditType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _locationEditType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _locationEditType->addItem("%");
    _locationEditType->addItem("data");
    _locationEditType->blockSignals(false);
    
//    this->setStyleSheet(R"(QWidget:hover:!pressed {border: 1px solid red;})");
    this->setDisabled(true);
}

void TFControlPointWidget::Update(VAPoR::RenderParams *rParams)
{
    _min = rParams->GetMapperFunc(rParams->GetVariableName())->getMinMapValue();
    _max = rParams->GetMapperFunc(rParams->GetVariableName())->getMaxMapValue();
    
    ((QDoubleValidator*)_locationEdit->validator())->setRange(_min, _max);
//    ((QDoubleValidator*)_locationEdit->validator())->setDecimals(2);
    
    if (_opacityId >= 0) {
        float value;

        value = rParams->GetMapperFunc(rParams->GetVariableName())->GetOpacityMap(0)->controlPointValueNormalized(_opacityId);
        if (isUsingMappedValue())
            value = toMappedValue(value);
        _locationEdit->setText(QString::number(value * 100));
        _valueEdit->setText(QString::number(rParams->GetMapperFunc(rParams->GetVariableName())->GetOpacityMap(0)->controlPointOpacity(_opacityId)));
    }
}

void TFControlPointWidget::SelectOpacityControlPoint(int index)
{
    this->setEnabled(true);
    _opacityId = index;
    _colorId = -1;
}

void TFControlPointWidget::SelectColorControlPoint(int index)
{
    this->setEnabled(true);
    _opacityId = -1;
    _colorId = index;
}

void TFControlPointWidget::DeselectControlPoint()
{
    this->setDisabled(true);
    _opacityId = -1;
    _colorId = -1;
}

void TFControlPointWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    QWidget::paintEvent(event);
}

bool TFControlPointWidget::isUsingNormalizedValue() const
{
    return _locationEditType->currentIndex() == 0;
}

bool TFControlPointWidget::isUsingMappedValue() const
{
    return _locationEditType->currentIndex() == 1;
}

float TFControlPointWidget::toMappedValue(float normalized) const
{
    printf("%f -> %f\n", _min, _max);
    return normalized * (_max - _min) + _min;
}

float TFControlPointWidget::toNormalizedValue(float mapped) const
{
    if (_max == _min)
        return 0;
    
    return (mapped - _min) / (_max - _min);
}
