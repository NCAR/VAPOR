#pragma once

#include "QRangeSliderTextCombo.h"

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

class TFMappingRangeSelector : public QRangeSliderTextCombo {
    Q_OBJECT
    
public:
    TFMappingRangeSelector();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
private:
    VAPoR::RenderParams *_rParams = nullptr;
    VAPoR::ParamsMgr *_paramsMgr = nullptr;
    
    void _getDataRange(VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams, float *min, float *max) const;
    
private slots:
    void _rangeChangedBegin();
    void _rangeChangedIntermediate(float left, float right);
    void _rangeChanged(float left, float right);
};
