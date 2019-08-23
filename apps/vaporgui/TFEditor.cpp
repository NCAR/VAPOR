#include "TFEditor.h"
#include <QBoxLayout>
#include <QLabel>
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "TFOpacityInfoWidget.h"
#include "TFColorInfoWidget.h"
#include "QRangeSlider.h"
#include <vapor/ColorMap.h>
//#include "QColorWidget.h"

static ParamsWidgetColor *c;

TFEditor::TFEditor()
{
    addTab(new QWidget(this), "Transfer Function");
    
    QVBoxLayout *layout = new QVBoxLayout;
//    layout->setSpacing(0);
    layout->setMargin(12);
    _tab()->setLayout(layout);
    layout->addWidget(tff = new TFOpacityWidget);
    layout->addWidget(tfh = new TFHistogramWidget);
    layout->addWidget(colorWidget = new TFColorWidget);
    layout->addWidget(controlPointWidget = tff->CreateInfoWidget());
    layout->addWidget(colorInfo = colorWidget->CreateInfoWidget());
    layout->addWidget(_maps = new TFMapsGroup);
    layout->addWidget(range = new QRangeSlider);
    layout->addWidget(colorMapTypeDropdown = new ParamsWidgetDropdown(VAPoR::ColorMap::_interpTypeTag, {"Linear", "Discrete", "Diverging"}, "Color Interpolation"));
    layout->addWidget(c = new ParamsWidgetColor("test"));
    
//    this->setStyleSheet(R"(QWidget:hover:!pressed {border: 1px solid red;})");
    
}

void TFEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    tff->Update(dataMgr, paramsMgr, rParams);
    tfh->Update(dataMgr, paramsMgr, rParams);
    colorWidget->Update(dataMgr, paramsMgr, rParams);
    colorMapTypeDropdown->Update(rParams->GetMapperFunc(rParams->GetVariableName())->GetColorMap());
    controlPointWidget->Update(rParams);
    colorInfo->Update(rParams);
    _maps->Update(dataMgr, paramsMgr, rParams);
    c->Update(rParams);
}

QWidget *TFEditor::_tab() const
{
    return this->widget(0);
}




TFMapsGroup::TFMapsGroup()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    
    _widgets.push_back(new TFOpacityWidget);
    _widgets.push_back(new TFHistogramWidget);
    _widgets.push_back(new TFColorWidget);
    
    for (auto map : _widgets)
        layout->addWidget(map);
}

void TFMapsGroup::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto &map : _widgets)
        map->Update(dataMgr, paramsMgr, rParams);
}
