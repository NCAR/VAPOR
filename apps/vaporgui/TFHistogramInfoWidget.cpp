#include "TFHistogramInfoWidget.h"
#include <vapor/RenderParams.h>
#include <QLabel>
#include <QBoxLayout>

TFHistogramInfoWidget::TFHistogramInfoWidget()
{
    ((QBoxLayout*)layout())->addWidget(new QLabel, 30, Qt::AlignRight);
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
