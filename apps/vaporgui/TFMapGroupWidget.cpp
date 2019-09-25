#include "TFMapGroupWidget.h"
#include "TFEditor.h"
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QFileDialog>
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "TFOpacityInfoWidget.h"
#include "TFColorInfoWidget.h"
#include "TFMappingRangeSelector.h"
#include <vapor/ColorMap.h>
#include <vapor/ResourcePath.h>
#include <vapor/FileUtils.h>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "SettingsParams.h"
#include "ErrorReporter.h"
#include "TFIsoValueWidget.h"

using namespace Wasp;
using namespace VAPoR;

TFMapGroupWidget::TFMapGroupWidget()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    
    TFMapWidget *o;
    add(o= new TFOpacityWidget);
    histo = new TFHistogramMap(o);
    o->AddMap(histo);
    add(new TFIsoValueWidget);
    add(new TFColorWidget);
}

void TFMapGroupWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto map : _maps)
        map->Update(dataMgr, paramsMgr, rParams);
}

TFMapInfoGroupWidget *TFMapGroupWidget::CreateInfoGroup()
{
    TFMapInfoGroupWidget *infos = new TFMapInfoGroupWidget;
    
    for (auto map : _maps) {
        infos->add(map);
    }
    
    return infos;
}

void TFMapGroupWidget::add(TFMapWidget *map)
{
    _maps.push_back(map);
    layout()->addWidget(map);
    connect(map, SIGNAL(Activated(TFMapWidget*)), this, SLOT(mapActivated(TFMapWidget*)));
}

void TFMapGroupWidget::mapActivated(TFMapWidget *activatedMap)
{
    for (auto map : _maps)
        if (map != activatedMap)
            map->Deactivate();
}




TFMapInfoGroupWidget::TFMapInfoGroupWidget()
{
    
}

void TFMapInfoGroupWidget::Update(VAPoR::RenderParams *rParams)
{
    for (auto info : _infos)
        info->Update(rParams);
}

void TFMapInfoGroupWidget::add(TFMapWidget *mapWidget)
{
    for (TFMap *map : mapWidget->GetMaps()) {
        TFInfoWidget *info = map->GetInfoWidget();
        if (!info) continue;
        _infos.push_back(info);
        addWidget(info);
        connect(map, SIGNAL(Activated(TFMap*)), this, SLOT(mapActivated(TFMap*)));
    }
}

void TFMapInfoGroupWidget::mapActivated(TFMap *activatedMap)
{
    setCurrentWidget(activatedMap->GetInfoWidget());
}
