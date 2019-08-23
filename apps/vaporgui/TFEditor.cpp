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
    
    add(new TFOpacityWidget);
    add(new TFHistogramWidget);
    add(new TFColorWidget);
}

void TFMapsGroup::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto map : _maps)
        map->Update(dataMgr, paramsMgr, rParams);
}

TFMapsInfoGroup *TFMapsGroup::CreateInfoGroup()
{
    TFMapsInfoGroup *infos = new TFMapsInfoGroup;
    
    for (auto map : _maps) {
        infos->add(map);
    }
    
    return infos;
}

void TFMapsGroup::add(TFMapWidget *map)
{
    _maps.push_back(map);
    layout()->addWidget(map);
    connect(map, SIGNAL(Activated(TFMapWidget*)), this, SLOT(mapActivated(TFMapWidget*)));
}

void TFMapsGroup::mapActivated(TFMapWidget *activatedMap)
{
    for (auto map : _maps)
        if (map != activatedMap)
            map->Deactivate();
}




TFMapsInfoGroup::TFMapsInfoGroup()
{
    
}

void TFMapsInfoGroup::Update(VAPoR::RenderParams *rParams)
{
    
}

void TFMapsInfoGroup::add(TFMapWidget *map)
{
    TFInfoWidget *info = map->GetInfoWidget();
    _infos.push_back(info);
    addWidget(info);
    connect(map, SIGNAL(Activated(TFMapWidget*)), this, SLOT(mapActivated(TFMapWidget*)));
}

void TFMapsInfoGroup::mapActivated(TFMapWidget *activatedMap)
{
    setCurrentWidget(activatedMap->GetInfoWidget());
}
