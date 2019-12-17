#include "TFEditor.h"
#include "TFMapGroupWidget.h"
#include "TFMappingRangeSelector.h"
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "TFIsoValueWidget.h"

TFEditor::TFEditor(bool usingColormapVariable) : VSection("Transfer Function")
{
    _maps = new TFMapGroupWidget;
    _opacityMap = new TFOpacityMap;
    _histogramMap = new TFHistogramMap;
    _colorMap = new TFColorMap;
    _isoMap = new TFIsoValueMap;
    _range = new TFMappingRangeSelector;

    _opacityMap->UsingColormapVariable = usingColormapVariable;
    _histogramMap->UsingColormapVariable = usingColormapVariable;
    _colorMap->UsingColormapVariable = usingColormapVariable;
    _isoMap->UsingColormapVariable = usingColormapVariable;
    _range->UsingColormapVariable = usingColormapVariable;

    _maps->Add({_opacityMap, _histogramMap});
    _maps->Add(_isoMap);
    _maps->Add(_colorMap);

    layout()->addWidget(_maps);
    layout()->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    layout()->addWidget(_range);
    connect(_range, SIGNAL(ValueChangedIntermediate(float, float)), _histogramMap, SLOT(update()));

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
    _range->Update(dataMgr, paramsMgr, rParams);
}

void TFEditor::SetShowOpacityMap(bool b)
{
    if (b)
        _opacityMap->show();
    else
        _opacityMap->hide();
}
