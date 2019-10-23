#pragma once

#include "VSection.h"
#include "ParamsWidgets.h"

class TFMapGroupWidget;
class TFMapInfoGroupWidget;
class TFMappingRangeSelector;
class TFOpacityMap;
class TFHistogramMap;
class TFColorMap;
class TFIsoValueMap;

namespace VAPoR {
class DataMgr;
class ParamsMgr;
class RenderParams;
}    // namespace VAPoR

class TFEditorIsoSurface : public VSection {
    Q_OBJECT

public:
    TFEditorIsoSurface();

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    TFMapGroupWidget *      _maps;
    TFMapInfoGroupWidget *  _mapsInfo;
    TFMappingRangeSelector *range;

    TFHistogramMap *_histogramMap;
    TFIsoValueMap * _isoMap;

    ParamsWidgetCheckbox *_colormappedVariableCheckbox;
    ParamsWidgetColor *   _constantColorSelector;

    TFMapGroupWidget *      _maps2;
    TFMapInfoGroupWidget *  _mapsInfo2;
    TFMappingRangeSelector *range2;

    TFOpacityMap *  _opacityMap2;
    TFHistogramMap *_histogramMap2;
    TFColorMap *    _colorMap2;

    QList<QAction *> _colormappedMenuSettings;
};
