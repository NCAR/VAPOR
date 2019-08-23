#include "TFHistogramInfoWidget.h"
#include <vapor/RenderParams.h>

TFHistogramInfoWidget::TFHistogramInfoWidget()
{
    _valueEdit->setReadOnly(true);
}

void TFHistogramInfoWidget::Update(VAPoR::RenderParams *rParams)
{
    TFInfoWidget::Update(rParams);
    if (!rParams)
        return;
}

void TFHistogramInfoWidget::SetControlPoint(float value)
{
    this->setEnabled(true);
    SetNormalizedValue(value);
}
