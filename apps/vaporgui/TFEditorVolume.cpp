#include "TFEditorVolume.h"
#include "TFMapGroupWidget.h"
#include "TFMappingRangeSelector.h"
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "TFIsoValueWidget.h"
#include "ParamsWidgets.h"
#include <vapor/VolumeParams.h>

using VAPoR::RenderParams;

TFEditorVolume::TFEditorVolume() : TFEditor()
{
    layout()->addWidget(_colormappedVariableCheckbox = new ParamsWidgetCheckbox(VAPoR::VolumeParams::UseColormapVariableTag, "Color by second variable"));

    _maps2 = new TFMapGroupWidget;
    _histogramMap2 = new TFHistogramMap(RenderParams::_colorMapVariableNameTag);
    _colorMap2 = new TFColorMap(RenderParams::_colorMapVariableNameTag);

    _maps2->Add(_histogramMap2);
    _maps2->Add(_colorMap2);

    layout()->addWidget(_maps2);
    layout()->addWidget(_mapsInfo2 = _maps2->CreateInfoGroup());
    layout()->addWidget(range2 = new TFMappingRangeSelector(RenderParams::_colorMapVariableNameTag));
    connect(range2, SIGNAL(ValueChangedIntermediate(float, float)), _histogramMap2, SLOT(update()));
}

void TFEditorVolume::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _maps->Update(dataMgr, paramsMgr, rParams);
    _mapsInfo->Update(rParams);
    _range->Update(dataMgr, paramsMgr, rParams);
    _colormappedVariableCheckbox->Update(rParams);

    bool useColormappedVar = rParams->GetValueLong(VAPoR::VolumeParams::UseColormapVariableTag, false);

    if (useColormappedVar) {
        _maps2->show();
        _mapsInfo2->show();
        range2->show();
        _colorMap->hide();

        _maps2->Update(dataMgr, paramsMgr, rParams);
        _mapsInfo2->Update(rParams);
        range2->Update(dataMgr, paramsMgr, rParams);
    } else {
        _maps2->hide();
        _mapsInfo2->hide();
        range2->hide();
        _colorMap->show();
    }
}
