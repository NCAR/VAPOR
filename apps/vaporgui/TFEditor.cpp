#include "TFEditor.h"
#include <QBoxLayout>
#include <QLabel>
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "QRangeSlider.h"
#include <vapor/ColorMap.h>

TFEditor::TFEditor()
{
    addTab(new QWidget(this), "Transfer Function");
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    _tab()->setLayout(layout);
    layout->addWidget(tff = new TFOpacityWidget);
    layout->addWidget(tfh = new TFHistogramWidget);
    layout->addWidget(colorWidget = new TFColorWidget);
    layout->addWidget(range = new QRangeSlider);
    layout->addWidget(colorMapTypeDropdown = new ParamsWidgetDropdown(VAPoR::ColorMap::_interpTypeTag, {"Linear", "Discrete", "Diverging"}, "Color Interpolation"));
}

void TFEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    tff->Update(dataMgr, paramsMgr, rParams);
    tfh->Update(dataMgr, paramsMgr, rParams);
    colorWidget->Update(dataMgr, paramsMgr, rParams);
    colorMapTypeDropdown->Update(rParams->GetMapperFunc(rParams->GetVariableName())->GetColorMap());
}

QWidget *TFEditor::_tab() const
{
    return this->widget(0);
}

