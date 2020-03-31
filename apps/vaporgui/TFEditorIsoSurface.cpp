#include "TFEditorIsoSurface.h"
#include "TFMapGroupWidget.h"
#include "TFMappingRangeSelector.h"
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "TFIsoValueWidget.h"
#include "ParamsWidgets.h"
#include <vapor/VolumeIsoParams.h>
#include <vapor/RenderParams.h>

using VAPoR::RenderParams;

TFEditorIsoSurface::TFEditorIsoSurface() : VSection("Transfer Function")
{
    _maps = new TFMapGroupWidget;
    _histogramMap = new TFHistogramMap(RenderParams::_variableNameTag);
    _isoMap = new TFIsoValueMap(RenderParams::_variableNameTag);
    _isoMap->SetEquidistantIsoValues(false);
    _isoMap->BottomPadding = true;

    _maps->Add(_histogramMap);
    _maps->Add(_isoMap);

    layout()->addWidget(_maps);
    layout()->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    layout()->addWidget(range = new TFMappingRangeSelector(RenderParams::_variableNameTag));
    connect(range, SIGNAL(ValueChangedIntermediate(float, float)), _histogramMap, SLOT(update()));

    layout()->addWidget(_colormappedVariableCheckbox = new ParamsWidgetCheckbox(VAPoR::VolumeIsoParams::UseColormapVariableTag, "Color by second variable"));
    layout()->addWidget(_constantColorSelector = new ParamsWidgetColor("ConstantColor"));

    _maps2 = new TFMapGroupWidget;
    _opacityMap2 = new TFOpacityMap(RenderParams::_colorMapVariableNameTag);
    _histogramMap2 = new TFHistogramMap(RenderParams::_colorMapVariableNameTag);
    _colorMap2 = new TFColorMap(RenderParams::_colorMapVariableNameTag);

    _maps2->Add({_opacityMap2, _histogramMap2});
    _maps2->Add(_colorMap2);

    layout()->addWidget(_maps2);
    layout()->addWidget(_mapsInfo2 = _maps2->CreateInfoGroup());
    layout()->addWidget(range2 = new TFMappingRangeSelector(RenderParams::_colorMapVariableNameTag));
    connect(range2, SIGNAL(ValueChangedIntermediate(float, float)), _histogramMap2, SLOT(update()));

    QMenu *menu = new QMenu;
    _colorMap2->PopulateSettingsMenu(menu);
    menu->addSeparator();
    _opacityMap2->PopulateSettingsMenu(menu);
    menu->addSeparator();

    _colormappedMenuSettings = menu->actions();

    _histogramMap->PopulateSettingsMenu(menu);
    setMenu(menu);

    //    this->setStyleSheet(R"(QWidget:hover:!pressed {border: 1px solid red;})");
}

void TFEditorIsoSurface::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _maps->Update(dataMgr, paramsMgr, rParams);
    _mapsInfo->Update(rParams);
    range->Update(dataMgr, paramsMgr, rParams);
    _colormappedVariableCheckbox->Update(rParams);

    bool useColormappedVar = rParams->GetValueLong(VAPoR::VolumeIsoParams::UseColormapVariableTag, false);

    if (useColormappedVar) {
        _maps2->show();
        _mapsInfo2->show();
        range2->show();
        _constantColorSelector->hide();

        _maps2->Update(dataMgr, paramsMgr, rParams);
        _mapsInfo2->Update(rParams);
        range2->Update(dataMgr, paramsMgr, rParams);

        for (auto action : _colormappedMenuSettings) action->setEnabled(true);
    } else {
        _maps2->hide();
        _mapsInfo2->hide();
        range2->hide();
        _constantColorSelector->show();
        _constantColorSelector->Update(rParams);

        for (auto action : _colormappedMenuSettings) action->setEnabled(false);
    }
}
