#include "TFOpacityInfoWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleValidator>
#include <vapor/RenderParams.h>
#include "VLineItem.h"

TFOpacityInfoWidget::TFOpacityInfoWidget(const std::string &variableNameTag)
: TFInfoWidget(variableNameTag)
{
    ((QBoxLayout*)layout())->addWidget(new VLineItem("Opacity", _opacityEdit = new VDoubleLineEdit));
    
    _opacityEdit->SetRange(0, 1);
    connect(_opacityEdit, &VDoubleLineEdit::ValueChanged, this, &TFOpacityInfoWidget::opacityEditChanged);
}

void TFOpacityInfoWidget::DeselectControlPoint()
{
    TFInfoWidget::DeselectControlPoint();
    _opacityEdit->Clear();
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
    
    _opacityEdit->SetValueDouble(_opacity);
}

float TFOpacityInfoWidget::getOpacityFromEdit() const
{
    return _opacityEdit->GetValueDouble();
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
