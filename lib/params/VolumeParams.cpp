
#include <vapor/VolumeParams.h>
#include <vapor/STLUtils.h>
#include <string>
#include <cassert>

using namespace Wasp;
using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenParamsRegistrar<VolumeParams> registrar(VolumeParams::GetClassType());

std::vector<VolumeParams::AlgorithmEntry> VolumeParams::_algorithms;

const std::string VolumeParams::_algorithmTag = "AlgorithmTag";
const std::string VolumeParams::_algorithmWasManuallySetByUserTag = "AlgorithmWasManuallySetByUserTag";
const std::string VolumeParams::_isoValuesTag = "IsoValuesTag";
const std::string VolumeParams::_enabledIsoValuesTag = "EnabledIsoValuesTag";
const std::string VolumeParams::LightingEnabledTag = "LightingEnabledTag";
const std::string VolumeParams::PhongAmbientTag = "PhongAmbientTag";
const std::string VolumeParams::PhongDiffuseTag = "PhongDiffuseTag";
const std::string VolumeParams::PhongSpecularTag = "PhongSpecularTag";
const std::string VolumeParams::PhongShininessTag = "PhongShininessTag";
const std::string VolumeParams::UseColormapVariableTag = "UseColormapVariable";
const std::string VolumeParams::SamplingRateMultiplierTag = "SamplingRateMultiplierTag";
const std::string VolumeParams::VolumeDensityTag = "VolumeDensityTag";

VolumeParams::VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, VolumeParams::GetClassType(), 3)
{
    SetDiagMsg("VolumeParams::VolumeParams() this=%p", this);
    _init();
}

VolumeParams::VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType) : RenderParams(dataMgr, ssave, classType, 3)
{
    SetDiagMsg("VolumeParams::VolumeParams() this=%p", this);
    _init();
}

VolumeParams::VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 3) {}

VolumeParams::~VolumeParams() { SetDiagMsg("VolumeParams::~VolumeParams() this=%p", this); }

string VolumeParams::GetDefaultAlgorithmName() const { return "Regular"; }

std::string VolumeParams::GetAlgorithm() const { return GetValueString(_algorithmTag, GetDefaultAlgorithmName()); }

void VolumeParams::SetAlgorithm(std::string algorithm)
{
    if (!STLUtils::Contains(GetAlgorithmNames(), algorithm)) MyBase::SetErrMsg("Invalid volume rendering algorithm \"%s\"", algorithm.c_str());
    SetValueString(_algorithmTag, "Volume rendering algorithm", algorithm);
}

void VolumeParams::SetAlgorithmByUser(std::string algorithm)
{
    BeginGroup("Set Algorithm and set by user");
    SetAlgorithm(algorithm);
    SetAlgorithmWasManuallySetByUser(true);
    EndGroup();
}

bool VolumeParams::GetAlgorithmWasManuallySetByUser() const { return GetValueLong(_algorithmWasManuallySetByUserTag, false); }

void VolumeParams::SetAlgorithmWasManuallySetByUser(bool v) { SetValueLong(_algorithmWasManuallySetByUserTag, "User manually changed the algorithm", v); }

std::vector<float> VolumeParams::GetSamplingRateMultiples() { return {1, 2, 4, 8, 16}; }

long VolumeParams::GetSamplingMultiplier() const { return GetValueLong(SamplingRateMultiplierTag, 1); }

void VolumeParams::SetSamplingMultiplier(long d) { SetValueLong(SamplingRateMultiplierTag, "Sampling Rate Multiplier", d); }

vector<double> VolumeParams::GetIsoValues(const string &variable)
{
    const string tag = "IsoValues_" + variable;

    if (!GetNode()->HasElementDouble(tag)) {
        const MapperFunction *mf = GetMapperFunc(variable);
        const float           min = mf->getMinMapValue();
        const float           max = mf->getMaxMapValue();
        const float           middle = (min + max) / 2.f;
        SetIsoValues(variable, {middle});
    }

    return GetValueDoubleVec(tag);
}

void VolumeParams::SetIsoValues(const string &variable, const vector<double> &values)
{
    assert(values.size() <= 4);
    const string tag = "IsoValues_" + variable;
    SetValueDoubleVec(tag, tag, values);
}

void  VolumeParams::SetLightingEnabled(bool v) { SetValueLong(LightingEnabledTag, "Lighting enabled", v); }
bool  VolumeParams::GetLightingEnabled() const { return GetValueLong(LightingEnabledTag, GetDefaultLightingEnabled()); }
void  VolumeParams::SetPhongAmbient(float v) { SetValueDouble(PhongAmbientTag, "Phong ambient", v); };
float VolumeParams::GetPhongAmbient() const { return GetValueDouble(PhongAmbientTag, GetDefaultPhongAmbient()); };
void  VolumeParams::SetPhongDiffuse(float v) { SetValueDouble(PhongDiffuseTag, "Phong diffuse", v); };
float VolumeParams::GetPhongDiffuse() const { return GetValueDouble(PhongDiffuseTag, GetDefaultPhongDiffuse()); };
void  VolumeParams::SetPhongSpecular(float v) { SetValueDouble(PhongSpecularTag, "Phong specular", v); };
float VolumeParams::GetPhongSpecular() const { return GetValueDouble(PhongSpecularTag, GetDefaultPhongSpecular()); };
void  VolumeParams::SetPhongShininess(float v) { SetValueDouble(PhongShininessTag, "Phong shininess", v); };
float VolumeParams::GetPhongShininess() const { return GetValueDouble(PhongShininessTag, GetDefaultPhongShininess()); };

const std::vector<std::string> VolumeParams::GetAlgorithmNames(Type type)
{
    vector<string> names;
    for (const AlgorithmEntry &entry : _algorithms) {
        if (entry.type == type || type == Type::Any) names.push_back(entry.name);
    }
    return names;
}

void VolumeParams::Register(const std::string &name, Type type)
{
    AlgorithmEntry entry = {name, type};

    for (const AlgorithmEntry &existing : _algorithms)
        if (entry == existing) VAssert(!"Algorithm already registered");

    _algorithms.push_back(entry);
}

// Set everything to default values
void VolumeParams::_init()
{
    SetDiagMsg("VolumeParams::_init()");

    SetFieldVariableNames(vector<string>());
    SetValueDouble(VolumeDensityTag, "", 1);

    SetValueLong(LightingEnabledTag, "", GetDefaultLightingEnabled());
    SetValueDouble(PhongAmbientTag, "", GetDefaultPhongAmbient());
    SetValueDouble(PhongDiffuseTag, "", GetDefaultPhongDiffuse());
    SetValueDouble(PhongSpecularTag, "", GetDefaultPhongSpecular());
    SetValueDouble(PhongShininessTag, "", GetDefaultPhongShininess());
}
