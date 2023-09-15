#include "vapor/RayCasterParams.h"

using namespace VAPoR;

const std::string RayCasterParams::_lightingTag = "LightingTag";
const std::string RayCasterParams::_lightingCoeffsTag = "LightingCoeffTag";
const std::string RayCasterParams::_castingModeTag = "CastingModeTag";
const std::string RayCasterParams::_sampleMultiplierTag = "SampleMultiplierTag";

RayCasterParams::RayCasterParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, std::string classType) : RenderParams(dataManager, stateSave, classType, 3 /* max dim */)
{
    SetFieldVariableNames(vector<string>());
    SetDiagMsg("RayCasterParams::RayCasterParams() this=%p", this);
}

// Constructor
RayCasterParams::RayCasterParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *node) : RenderParams(dataManager, stateSave, node, 3 /* max dim */)
{
    SetDiagMsg("RayCasterParams::RayCasterParams() this=%p", this);
}

// Destructor
RayCasterParams::~RayCasterParams() { SetDiagMsg("RayCasterParams::~RayCasterParams() this=%p", this); }

MapperFunction *RayCasterParams::GetMapperFunc() { return RenderParams::GetMapperFunc(GetVariableName()); }

void RayCasterParams::SetLighting(bool lightingOn) { SetValueLong(_lightingTag, "Apply lighting or not", (long int)lightingOn); }

bool RayCasterParams::GetLighting() const
{
    long l = GetValueLong(_lightingTag, 1);

    return (bool)l;
}

std::vector<double> RayCasterParams::GetLightingCoeffs() const
{
    std::vector<double> defaultVec(4);
    defaultVec[0] = 0.2;
    defaultVec[1] = 0.5;
    defaultVec[2] = 0.25;
    defaultVec[3] = 8.0;
    return GetValueDoubleVec(_lightingCoeffsTag, defaultVec);
}

void RayCasterParams::SetLightingCoeffs(const std::vector<double> &coeffs) { SetValueDoubleVec(_lightingCoeffsTag, "Coefficients for lighting effects", coeffs); }

long RayCasterParams::GetCastingMode() const
{
    return GetValueLong(_castingModeTag, 0);    // 0 means it's not set by the user yet.
}

void RayCasterParams::SetCastingMode(long mode)
{
    if (mode == 1 || mode == 2)    // currently supported casting modes
        SetValueLong(_castingModeTag, "Which ray casting mode", mode);
    else    // put mode 1 here
        SetValueLong(_castingModeTag, "Which ray casting mode", 1);
}

long RayCasterParams::GetSampleRateMultiplier() const { return GetValueLong(_sampleMultiplierTag, 0); }

void RayCasterParams::SetSampleRateMultiplier(long val)
{
    if (val >= 0 && val < 7)
        SetValueLong(_sampleMultiplierTag, "How to adjust the sample rate", val);
    else
        SetValueLong(_sampleMultiplierTag, "How to adjust the sample rate", 0);
}
