#pragma once

#include "QRangeSliderTextCombo.h"
#include <string>

namespace VAPoR {
class DataMgr;
class ParamsMgr;
class RenderParams;
class MapperFunction;
}    // namespace VAPoR

class TFMappingRangeSelector : public QRangeSliderTextCombo {
    Q_OBJECT

public:
    TFMappingRangeSelector(const std::string &variableNameTag);
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    VAPoR::RenderParams *_rParams = nullptr;
    VAPoR::ParamsMgr *   _paramsMgr = nullptr;
    const std::string &  _variableNameTag;

    void                   _getDataRange(VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams, float *min, float *max) const;
    std::string            _getVariableName() const;
    VAPoR::MapperFunction *_getTF() const;

private slots:
    void _rangeChangedBegin();
    void _rangeChangedIntermediate(float left, float right);
    void _rangeChanged(float left, float right);
    void _sliderRangeChanged(float left, float right);
    void _sliderRangeResetToDefaultRequested();
};
