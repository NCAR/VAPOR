#pragma once

#include "QRangeSliderTextCombo.h"
#include <string>

namespace VAPoR {
class DataMgr;
class ParamsMgr;
class RenderParams;
}    // namespace VAPoR

class TFMappingRangeSelector : public QRangeSliderTextCombo {
    Q_OBJECT

public:
    bool UsingColormapVariable;

    TFMappingRangeSelector();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    VAPoR::RenderParams *_rParams = nullptr;
    VAPoR::ParamsMgr *   _paramsMgr = nullptr;

    void        _getDataRange(VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams, float *min, float *max) const;
    std::string _getVariableName() const;

private slots:
    void _rangeChangedBegin();
    void _rangeChangedIntermediate(float left, float right);
    void _rangeChanged(float left, float right);
};
