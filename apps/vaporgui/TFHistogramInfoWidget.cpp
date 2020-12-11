#include "TFHistogramInfoWidget.h"
#include <vapor/RenderParams.h>
#include <QLabel>
#include <QBoxLayout>

TFHistogramInfoWidget::TFHistogramInfoWidget(const std::string &variableNameTag)
: TFInfoWidget(variableNameTag)
{
    _valueEdit->SetReadOnly(true);
}

void TFHistogramInfoWidget::SetControlPoint(float value)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
}

void TFHistogramInfoWidget::Deselect()
{
    this->setEnabled(false);
    _valueEdit->Clear();
}
