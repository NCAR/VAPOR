#include "TFColorInfoWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleValidator>
#include <vapor/RenderParams.h>
#include "VLineItem.h"

TFColorInfoWidget::TFColorInfoWidget()
{
    ((QBoxLayout *)layout())->addWidget(new VLineItem("Color", _colorEdit = new QColorWidget));

    connect(_colorEdit, SIGNAL(colorChanged(QColor)), this, SLOT(colorEditChanged()));
}

void TFColorInfoWidget::DeselectControlPoint()
{
    TFInfoWidget::DeselectControlPoint();
    _colorEdit->setColor(Qt::gray);
}

void TFColorInfoWidget::SetColor(const QColor &color) { _colorEdit->setColor(color); }

void TFColorInfoWidget::SetControlPoint(float value, const QColor &color)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
    SetColor(color);
}

void TFColorInfoWidget::controlPointChanged() { emit ControlPointChanged(_value, _colorEdit->getColor()); }

void TFColorInfoWidget::colorEditChanged() { controlPointChanged(); }
