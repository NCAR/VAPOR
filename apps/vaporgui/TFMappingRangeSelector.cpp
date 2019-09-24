#include "TFMappingRangeSelector.h"
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>

TFMappingRangeSelector::TFMappingRangeSelector()
{
    connect(this, SIGNAL(ValueChanged(float, float)), this, SLOT(_rangeChanged(float, float)));
    connect(this, SIGNAL(ValueChangedBegin()), this, SLOT(_rangeChangedBegin()));
    connect(this, SIGNAL(ValueChangedIntermediate(float, float)), this, SLOT(_rangeChangedIntermediate(float, float)));
}

void TFMappingRangeSelector::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _rParams = rParams;
    _paramsMgr = paramsMgr;
    
    float min, max;
    _getDataRange(dataMgr, rParams, &min, &max);
    SetRange(min, max);
    
    vector<double> mapperRange = rParams->GetMapperFunc(rParams->GetVariableName())->getMinMaxMapValue();
    SetValue(mapperRange[0], mapperRange[1]);
}

void TFMappingRangeSelector::_getDataRange(VAPoR::DataMgr *d, VAPoR::RenderParams *r, float *min, float *max) const
{
    std::vector<double> range;
    d->GetDataRange(r->GetCurrentTimestep(), r->GetVariableName(), r->GetRefinementLevel(), r->GetCompressionLevel(), d->GetDefaultMetaInfoStride(r->GetVariableName(), r->GetRefinementLevel()), range);
    *min = range[0];
    *max = range[1];
}

void TFMappingRangeSelector::_rangeChangedBegin()
{
    if (!_rParams || !_paramsMgr) return;
    
    _paramsMgr->BeginSaveStateGroup("Change tf mapping range");
}

void TFMappingRangeSelector::_rangeChangedIntermediate(float left, float right)
{
    if (!_rParams || !_paramsMgr) return;
    
    _rParams->GetMapperFunc(_rParams->GetVariableName())->setMinMaxMapValue(left, right);
    
//    _maps->histo->update();
    
    _paramsMgr->IntermediateChange();
}

void TFMappingRangeSelector::_rangeChanged(float left, float right)
{
    if (!_rParams || !_paramsMgr) return;
    
    _rParams->GetMapperFunc(_rParams->GetVariableName())->setMinMaxMapValue(left, right);
    _paramsMgr->EndSaveStateGroup();
}
