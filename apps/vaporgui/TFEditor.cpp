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
    layout->addWidget(_maps = new TFMapsGroup);
    layout->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    layout->addWidget(range = new QRangeSlider);
    layout->addWidget(colorMapTypeDropdown = new ParamsWidgetDropdown(VAPoR::ColorMap::_interpTypeTag, {"Linear", "Discrete", "Diverging"}, "Color Interpolation"));
    layout->addWidget(c = new ParamsWidgetColor("test"));
    
//    this->setStyleSheet(R"(QWidget:hover:!pressed {border: 1px solid red;})");
    
}

void TFEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    colorMapTypeDropdown->Update(rParams->GetMapperFunc(rParams->GetVariableName())->GetColorMap());
    _maps->Update(dataMgr, paramsMgr, rParams);
    _mapsInfo->Update(rParams);
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
    
    _maps.push_back(new TFOpacityWidget);
    _maps.push_back(new TFHistogramWidget);
    _maps.push_back(new TFColorWidget);
    
    for (auto map : _maps)
        layout->addWidget(map);
}

void TFMapsGroup::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto &map : _maps)
        map->Update(dataMgr, paramsMgr, rParams);
}

TFMapsInfoGroup *TFMapsGroup::CreateInfoGroup()
{
    TFMapsInfoGroup *infos = new TFMapsInfoGroup;
    
    for (auto &map : _maps) {
        infos->addWidget(map->CreateInfoWidget());
    }
    
    return infos;
}

TFMapsInfoGroup::TFMapsInfoGroup()
{
    
}

void TFMapsInfoGroup::Update(VAPoR::RenderParams *rParams)
{
    
}
