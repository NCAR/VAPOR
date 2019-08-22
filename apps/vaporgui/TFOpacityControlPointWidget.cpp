#include "TFOpacityControlPointWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleValidator>
#include <vapor/RenderParams.h>

TFOpacityControlPointWidget::TFOpacityControlPointWidget()
{
    ((QBoxLayout*)layout())->addWidget(_opacityEdit = new QLineEdit, 30);
    
    _opacityEdit->setValidator(new QDoubleValidator(0, 1, 6));
    connect(_opacityEdit, SIGNAL(returnPressed()), this, SLOT(opacityEditChanged()));
}

void TFOpacityControlPointWidget::Update(VAPoR::RenderParams *rParams)
{
    TFInfoWidget::Update(rParams);
    if (!rParams)
        return;
}

void TFOpacityControlPointWidget::DeselectControlPoint()
{
    TFInfoWidget::DeselectControlPoint();
    _opacityEdit->clear();
}

void TFOpacityControlPointWidget::SetOpacity(float opacity)
{
    _opacity = opacity;
    updateOpacity();
}

void TFOpacityControlPointWidget::SetControlPoint(float value, float opacity)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
    SetOpacity(opacity);
}

void TFOpacityControlPointWidget::updateOpacity()
{
    if (!isEnabled())
        return;
    
    _opacityEdit->setText(QString::number(_opacity));
}

float TFOpacityControlPointWidget::getOpacityFromEdit() const
{
    return _opacityEdit->text().toFloat();
}

void TFOpacityControlPointWidget::controlPointChanged()
{
    emit ControlPointChanged(_value, _opacity);
}

void TFOpacityControlPointWidget::opacityEditChanged()
{
    _opacity = getOpacityFromEdit();
    controlPointChanged();
}
