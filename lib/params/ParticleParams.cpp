
#include <vapor/ParticleParams.h>
#include <vapor/STLUtils.h>
#include <string>
#include <cassert>

using namespace Wasp;
using namespace VAPoR;


//
// Register class with object factory!!!
//
static RenParamsRegistrar<ParticleParams> registrar(ParticleParams::GetClassType());

std::vector<ParticleParams::AlgorithmEntry> ParticleParams::_algorithms;

const std::string ParticleParams::_algorithmTag = "AlgorithmTag";
const std::string ParticleParams::_algorithmWasManuallySetByUserTag = "AlgorithmWasManuallySetByUserTag";
const std::string ParticleParams::_isoValuesTag = "IsoValuesTag";
const std::string ParticleParams::_enabledIsoValuesTag = "EnabledIsoValuesTag";
const std::string ParticleParams::LightingEnabledTag = "LightingEnabledTag";
const std::string ParticleParams::PhongAmbientTag = "PhongAmbientTag";
const std::string ParticleParams::PhongDiffuseTag = "PhongDiffuseTag";
const std::string ParticleParams::PhongSpecularTag = "PhongSpecularTag";
const std::string ParticleParams::PhongShininessTag = "PhongShininessTag";
const std::string ParticleParams::UseColormapVariableTag = "UseColormapVariable";
const std::string ParticleParams::SamplingRateMultiplierTag = "SamplingRateMultiplierTag";
const std::string ParticleParams::ParticleDensityTag = "ParticleDensityTag";

ParticleParams::ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave)
: RenderParams(dataMgr, ssave, ParticleParams::GetClassType(), 3)
{
    SetDiagMsg("ParticleParams::ParticleParams() this=%p", this);
    _init();
}

ParticleParams::ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType)
: RenderParams(dataMgr, ssave, classType, 3)
{
    SetDiagMsg("ParticleParams::ParticleParams() this=%p", this);
    _init();
}

ParticleParams::ParticleParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node
) : RenderParams(dataMgr, ssave, node, 3) {}


ParticleParams::~ParticleParams() {
	SetDiagMsg("ParticleParams::~ParticleParams() this=%p", this);

}

string ParticleParams::GetDefaultAlgorithmName() const
{
    return "Regular";
}

std::string ParticleParams::GetAlgorithm() const
{
    return GetValueString(_algorithmTag, GetDefaultAlgorithmName());
}

void ParticleParams::SetAlgorithm(std::string algorithm)
{
    if (!STLUtils::Contains(GetAlgorithmNames(), algorithm))
        MyBase::SetErrMsg("Invalid Particle rendering algorithm \"%s\"", algorithm.c_str());
    SetValueString(_algorithmTag, "Particle rendering algorithm", algorithm);
}

void ParticleParams::SetAlgorithmByUser(std::string algorithm)
{
    BeginGroup("Set Algorithm and set by user");
    SetAlgorithm(algorithm);
    SetAlgorithmWasManuallySetByUser(true);
    EndGroup();
}

bool ParticleParams::GetAlgorithmWasManuallySetByUser() const
{
    return GetValueLong(_algorithmWasManuallySetByUserTag, false);
}

void ParticleParams::SetAlgorithmWasManuallySetByUser(bool v)
{
    SetValueLong(_algorithmWasManuallySetByUserTag, "User manually changed the algorithm", v);
}

std::vector<float> ParticleParams::GetSamplingRateMultiples()
{
    return {1, 2, 4, 8, 16};
}

long ParticleParams::GetSamplingMultiplier() const
{
    return GetValueLong(SamplingRateMultiplierTag, 1);
}

void ParticleParams::SetSamplingMultiplier(long d)
{
    SetValueLong(SamplingRateMultiplierTag, "Sampling Rate Multiplier", d);
}

vector<double> ParticleParams::GetIsoValues(const string &variable)
{
    const string tag = "IsoValues_"+variable;
    
    if (!GetNode()->HasElementDouble(tag)) {
        const MapperFunction* mf = GetMapperFunc(variable);
        const float min = mf->getMinMapValue();
        const float max = mf->getMaxMapValue();
        const float middle = (min + max)/2.f;
        SetIsoValues(variable, {middle});
    }
    
    return GetValueDoubleVec(tag);
}

void ParticleParams::SetIsoValues(const string &variable, const vector<double> &values)
{
    assert(values.size() <= 4);
    const string tag = "IsoValues_"+variable;
    SetValueDoubleVec(tag, tag, values);
}

void   ParticleParams::SetLightingEnabled(bool v) {        SetValueLong(LightingEnabledTag, "Lighting enabled", v); }
bool   ParticleParams::GetLightingEnabled() const { return GetValueLong(LightingEnabledTag, GetDefaultLightingEnabled());  }
void   ParticleParams::SetPhongAmbient(float v)   {        SetValueDouble(PhongAmbientTag,    "Phong ambient", v); };
float ParticleParams::GetPhongAmbient() const     { return GetValueDouble(PhongAmbientTag,    GetDefaultPhongAmbient()); };
void   ParticleParams::SetPhongDiffuse(float v)   {        SetValueDouble(PhongDiffuseTag,    "Phong diffuse", v); };
float ParticleParams::GetPhongDiffuse() const     { return GetValueDouble(PhongDiffuseTag,    GetDefaultPhongDiffuse()); };
void   ParticleParams::SetPhongSpecular(float v)  {        SetValueDouble(PhongSpecularTag,   "Phong specular", v); };
float ParticleParams::GetPhongSpecular() const    { return GetValueDouble(PhongSpecularTag,   GetDefaultPhongSpecular()); };
void   ParticleParams::SetPhongShininess(float v) {        SetValueDouble(PhongShininessTag,  "Phong shininess", v); };
float ParticleParams::GetPhongShininess() const   { return GetValueDouble(PhongShininessTag,  GetDefaultPhongShininess()); };

const std::vector<std::string> ParticleParams::GetAlgorithmNames(Type type)
{
    vector<string> names;
    for (const AlgorithmEntry &entry : _algorithms) {
        if (entry.type == type || type == Type::Any)
            names.push_back(entry.name);
    }
    return names;
}

void ParticleParams::Register(const std::string &name, Type type)
{
    AlgorithmEntry entry = {name, type};
    
    for (const AlgorithmEntry &existing : _algorithms)
        if (entry == existing)
            VAssert(!"Algorithm already registered");
    
    _algorithms.push_back(entry);
}

//Set everything to default values
void ParticleParams::_init() {
	SetDiagMsg("ParticleParams::_init()");

	SetFieldVariableNames(vector <string> ());
    SetValueDouble(ParticleDensityTag, "", 1);
    
    SetValueLong(LightingEnabledTag, "", GetDefaultLightingEnabled());
    SetValueDouble(PhongAmbientTag, "", GetDefaultPhongAmbient());
    SetValueDouble(PhongDiffuseTag, "", GetDefaultPhongDiffuse());
    SetValueDouble(PhongSpecularTag, "", GetDefaultPhongSpecular());
    SetValueDouble(PhongShininessTag, "", GetDefaultPhongShininess());
}

