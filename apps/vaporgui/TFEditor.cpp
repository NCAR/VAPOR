#include "TFEditor.h"
#include "TFMapGroupWidget.h"
#include "TFMappingRangeSelector.h"
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "TFIsoValueWidget.h"

TFEditor::TFEditor() : VSection("Transfer Function")
{
    _maps = new TFMapGroupWidget;
    _opacityMap = new TFOpacityMap;
    _histogramMap = new TFHistogramMap;
    _colorMap = new TFColorMap;
    _isoMap = new TFIsoValueMap;

    _maps->Add({_opacityMap, _histogramMap});
    _maps->Add(_isoMap);
    _maps->Add(_colorMap);

    layout()->addWidget(_maps);
    layout()->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    layout()->addWidget(range = new TFMappingRangeSelector);
    connect(range, SIGNAL(ValueChangedIntermediate(float, float)), _histogramMap, SLOT(update()));

    QMenu *menu = new QMenu;
    _colorMap->PopulateSettingsMenu(menu);
    menu->addSeparator();
    _opacityMap->PopulateSettingsMenu(menu);
    menu->addSeparator();
    _histogramMap->PopulateSettingsMenu(menu);
    setMenu(menu);

    //    this->setStyleSheet(R"(QWidget:hover:!pressed {border: 1px solid red;})");
}

void TFEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _maps->Update(dataMgr, paramsMgr, rParams);
    _mapsInfo->Update(rParams);
    range->Update(dataMgr, paramsMgr, rParams);
}

void TFEditor::SetShowOpacityMap(bool b)
{
    if (b)
        _opacityMap->show();
    else
        _opacityMap->hide();
}
