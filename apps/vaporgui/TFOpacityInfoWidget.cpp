#include "TFOpacityInfoWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleValidator>
#include <vapor/RenderParams.h>

TFOpacityInfoWidget::TFOpacityInfoWidget()
{
    ((QBoxLayout*)layout())->addWidget(_opacityEdit = new QLineEdit, 30, Qt::AlignRight);
    
    _opacityEdit->setValidator(new QDoubleValidator(0, 1, 6));
    connect(_opacityEdit, SIGNAL(returnPressed()), this, SLOT(opacityEditChanged()));
}

void TFOpacityInfoWidget::Update(VAPoR::RenderParams *rParams)
{
    TFInfoWidget::Update(rParams);
    if (!rParams)
        return;
}

void TFOpacityInfoWidget::DeselectControlPoint()
{
    TFInfoWidget::DeselectControlPoint();
    _opacityEdit->clear();
}

void TFOpacityInfoWidget::SetOpacity(float opacity)
{
    _opacity = opacity;
    updateOpacity();
}

void TFOpacityInfoWidget::SetControlPoint(float value, float opacity)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
    SetOpacity(opacity);
}

void TFOpacityInfoWidget::updateOpacity()
{
    if (!isEnabled())
        return;
    
    _opacityEdit->setText(QString::number(_opacity));
}

float TFOpacityInfoWidget::getOpacityFromEdit() const
{
    return _opacityEdit->text().toFloat();
}

void TFOpacityInfoWidget::controlPointChanged()
{
    emit ControlPointChanged(_value, _opacity);
}

void TFOpacityInfoWidget::opacityEditChanged()
{
    _opacity = getOpacityFromEdit();
    controlPointChanged();
}
