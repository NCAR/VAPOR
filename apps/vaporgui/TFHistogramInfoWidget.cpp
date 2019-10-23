#include "TFHistogramInfoWidget.h"
#include <vapor/RenderParams.h>
#include <QLabel>
#include <QBoxLayout>

TFHistogramInfoWidget::TFHistogramInfoWidget() { _valueEdit->setReadOnly(true); }

void TFHistogramInfoWidget::SetControlPoint(float value)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
}

void TFHistogramInfoWidget::Deselect()
{
    this->setEnabled(false);
    _valueEdit->clear();
}
