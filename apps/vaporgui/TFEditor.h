#pragma once

#include "VSection.h"

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
}

class TFEditor : public VSection {
    Q_OBJECT
    
public:
    TFEditor(bool usingColormapVariable = false);
    TFEditor(const std::string &tag);
    
    virtual void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    void SetShowOpacityMap(bool b);
    
protected:
    TFMapGroupWidget *_maps;
    TFMapInfoGroupWidget *_mapsInfo;
    TFMappingRangeSelector *_range;
    
    TFOpacityMap *_opacityMap;
    TFHistogramMap *_histogramMap;
    TFColorMap *_colorMap;
    TFIsoValueMap *_isoMap;
};
