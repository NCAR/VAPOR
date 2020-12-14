#include "TFIsoValueInfoWidget.h"
#include <vapor/RenderParams.h>
#include <QLabel>
#include <QBoxLayout>

void TFIsoValueInfoWidget::controlPointChanged() { emit ControlPointChanged(this->getValueFromEdit()); }

void TFIsoValueInfoWidget::SetControlPoint(float value)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
}

void TFIsoValueInfoWidget::Deselect()
{
    this->setEnabled(false);
    _valueEdit->Clear();
}
