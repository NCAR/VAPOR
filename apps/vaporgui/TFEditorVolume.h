#pragma once

#include "VSection.h"
#include "ParamsWidgets.h"
#include "TFEditor.h"

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

// TFEditor was not designed to be inherited from.
// It is here because it simplifies the solution to a requirement
// that was added after the fact and the downsides are only minor
// inconveniences that manifest in an uncommon use case.
class TFEditorVolume : public TFEditor {
    Q_OBJECT

public:
    TFEditorVolume();

    virtual void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    ParamsWidgetCheckbox *_colormappedVariableCheckbox;

    TFMapGroupWidget *      _maps2;
    TFMapInfoGroupWidget *  _mapsInfo2;
    TFMappingRangeSelector *range2;

    TFHistogramMap *_histogramMap2;
    TFColorMap *    _colorMap2;

    QList<QAction *> _colormappedMenuSettings;
};
