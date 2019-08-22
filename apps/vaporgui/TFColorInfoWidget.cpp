#include "TFColorInfoWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleValidator>
#include <vapor/RenderParams.h>

TFColorInfoWidget::TFColorInfoWidget()
{
    ((QBoxLayout*)layout())->addWidget(_colorEdit = new QColorWidget, 30);
    
    connect(_colorEdit, SIGNAL(returnPressed()), this, SLOT(opacityEditChanged()));
}

void TFColorInfoWidget::Update(VAPoR::RenderParams *rParams)
{
    TFInfoWidget::Update(rParams);
    if (!rParams)
        return;
}

void TFColorInfoWidget::DeselectControlPoint()
{
    TFInfoWidget::DeselectControlPoint();
    _colorEdit->setColor(Qt::gray);
}

void TFColorInfoWidget::SetColor(const QColor &color)
{
    _colorEdit->setColor(color);
}

void TFColorInfoWidget::SetControlPoint(float value, const QColor &color)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
    SetColor(color);
}

void TFColorInfoWidget::controlPointChanged()
{
    emit ControlPointChanged(_value, _colorEdit->getColor());
}

void TFColorInfoWidget::opacityEditChanged()
{
    controlPointChanged();
}
