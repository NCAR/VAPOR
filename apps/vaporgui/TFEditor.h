#pragma once

#include <QTabWidget>
#include <QStackedWidget>
#include "ParamsWidgets.h"
#include <vector>
#include "VSection.h"

#include <QToolButton>

class TFOpacityWidget;
class TFColorWidget;
class TFOpacityMap;
class TFColorMap;
class TFHistogramMap;
class TFHistogramWidget;
class QRangeSlider;
class QRangeSliderTextCombo;
class TFInfoWidget;
class TFMapGroupWidget;
class TFMapWidget;
class TFMap;
class TFMapInfoGroupWidget;
class TFIsoValueWidget;
class TFMappingRangeSelector;

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

class TFEditor : public VSection {
    Q_OBJECT
    
public:
    TFEditor();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
private:
    VAPoR::RenderParams *_rParams = nullptr;
    VAPoR::ParamsMgr *_paramsMgr = nullptr;
    TFMappingRangeSelector *range;
    TFMapGroupWidget *_maps;
    TFMapInfoGroupWidget *_mapsInfo;
    
    TFOpacityMap *_opacityMap;
    TFHistogramMap *_histogramMap;
    TFColorMap *_colorMap;
};
