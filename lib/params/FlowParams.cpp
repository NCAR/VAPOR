#include "vapor/FlowParams.h"
#include <vapor/FileUtils.h>
#include <vapor/ResourcePath.h>

using namespace VAPoR;

const std::string FlowParams::RenderTypeTag = "RenderTypeTag";
const std::string FlowParams::RenderRadiusBaseTag = "RenderRadiusBaseTag";
const std::string FlowParams::RenderRadiusScalarTag = "RenderRadiusScalarTag";
const std::string FlowParams::RenderGeom3DTag = "RenderGeom3DTag";
const std::string FlowParams::RenderLightAtCameraTag = "RenderLightAtCameraTag";
const std::string FlowParams::RenderShowStreamDirTag = "RenderShowStreamDirTag";
const std::string FlowParams::RenderGlyphTypeTag = "RenderGlyphTypeTag";
const std::string FlowParams::RenderGlyphStrideTag = "RenderGlyphStrideTag";
const std::string FlowParams::RenderGlyphOnlyLeadingTag = "RenderGlyphOnlyLeadingTag";
const std::string FlowParams::RenderDensityFalloffTag = "RenderDensityFalloffTag";
const std::string FlowParams::RenderDensityToneMappingTag = "RenderDensityToneMappingTag";
const std::string FlowParams::RenderFadeTailTag = "RenderFadeTailTag";
const std::string FlowParams::RenderFadeTailStartTag = "RenderFadeTailStartTag";
const std::string FlowParams::RenderFadeTailStopTag = "RenderFadeTailStopTag";
const std::string FlowParams::RenderFadeTailLengthTag = "RenderFadeTailLengthTag";
const std::string FlowParams::PhongAmbientTag = "PhongAmbientTag";
const std::string FlowParams::PhongDiffuseTag = "PhongDiffuseTag";
const std::string FlowParams::PhongSpecularTag = "PhongSpecularTag";
const std::string FlowParams::PhongShininessTag = "PhongShininessTag";

const std::string FlowParams::_isSteadyTag = "IsSteadyTag";
const std::string FlowParams::_velocityMultiplierTag = "VelocityMultiplierTag";
const std::string FlowParams::_steadyNumOfStepsTag = "SteadyNumOfStepsTag";
const std::string FlowParams::_seedGenModeTag = "SeedGenModeTag";
const std::string FlowParams::_seedInputFilenameTag = "SeedInputFilenameTag";
const std::string FlowParams::_flowlineOutputFilenameTag = "FlowlineOutputFilenameTag";
const std::string FlowParams::_flowOutputMoreVariablesTag = "FlowOutputMoreVariablesTag";
const std::string FlowParams::_flowDirectionTag = "FlowDirectionTag";
const std::string FlowParams::_needFlowlineOutputTag = "NeedFlowlineOutputTag";
const std::string FlowParams::_xPeriodicTag = "PeriodicTag_X";
const std::string FlowParams::_yPeriodicTag = "PeriodicTag_Y";
const std::string FlowParams::_zPeriodicTag = "PeriodicTag_Z";
const std::string FlowParams::_rakeTag = "RakeTag";
const std::string FlowParams::_rakeBiasVariable = "RakeBiasVariable";
const std::string FlowParams::_rakeBiasStrength = "RakeBiasStrength";
const std::string FlowParams::_pastNumOfTimeSteps = "PastNumOfTimeSteps";
const std::string FlowParams::_seedInjInterval = "SeedInjInterval";
const std::string FlowParams::_xGridNumOfSeedsTag = "GridNumOfSeeds_X";
const std::string FlowParams::_yGridNumOfSeedsTag = "GridNumOfSeeds_Y";
const std::string FlowParams::_zGridNumOfSeedsTag = "GridNumOfSeeds_Z";
const std::string FlowParams::_randomNumOfSeedsTag = "RandomNumOfSeeds";

static RenParamsRegistrar<FlowParams> registrar(FlowParams::GetClassType());

// Constructor
FlowParams::FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave) : RenderParams(dataManager, stateSave, FlowParams::GetClassType(), 3 /* max dim */)
{
    SetVariableName("");
    SetDiagMsg("FlowParams::FlowParams() this=%p", this);

    SetValueLong(RenderTypeTag, "", RenderTypeStream);
    SetValueDouble(RenderRadiusBaseTag, "", -1);
    SetValueDouble(RenderRadiusScalarTag, "", 1);
    SetValueLong(RenderGeom3DTag, "", false);
    SetValueLong(RenderLightAtCameraTag, "", true);
    SetValueLong(RenderShowStreamDirTag, "", false);

    SetValueLong(RenderGlyphTypeTag, "", GlpyhTypeSphere);
    SetValueLong(RenderGlyphStrideTag, "", 5);
    SetValueLong(RenderGlyphOnlyLeadingTag, "", false);

    SetValueDouble(RenderDensityFalloffTag, "", 1);
    SetValueDouble(RenderDensityToneMappingTag, "", 1);

    SetValueLong(RenderFadeTailTag, "", false);
    SetValueLong(RenderFadeTailStartTag, "", 10);
    SetValueLong(RenderFadeTailLengthTag, "", 10);
    SetValueLong(RenderFadeTailStopTag, "", 0);

    SetValueDouble(PhongAmbientTag, "", 0.4);
    SetValueDouble(PhongDiffuseTag, "", 0.8);
    SetValueDouble(PhongSpecularTag, "", 0);
    SetValueDouble(PhongShininessTag, "", 2);

    // Give the random bias variable the same as color mapping variable.
    auto colorvar = GetColorMapVariableName();
    SetValueString(_rakeBiasVariable, "which variable to bias with", colorvar);
}

FlowParams::FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *node) : RenderParams(dataManager, stateSave, node, 3 /* max dim */)
{
    _initialized = true;
    SetDiagMsg("FlowParams::FlowParams() this=%p", this);
}

// Destructor
FlowParams::~FlowParams()
{
    SetDiagMsg("FlowParams::~FlowParams() this=%p", this);
    if (_fakeRakeBox) delete _fakeRakeBox;
}

int FlowParams::Initialize()
{
    int rc = RenderParams::Initialize();
    if (rc < 0) return (rc);
    if (_initialized) return 0;

    // At this point the base class is initialized, and the _Box is
    // properly initialized
    // to be the extents of the domain. Let's use that information to
    // initialize the rake!
    //
    std::vector<double> minext, maxext;
    auto                box = RenderParams::GetBox();
    box->GetExtents(minext, maxext);
    std::vector<float> floats(minext.size() * 2);
    for (int i = 0; i < minext.size(); i++) {
        floats[i * 2] = minext[i];
        floats[i * 2 + 1] = maxext[i];
    }
    this->SetRake(floats);

    vector<float> rake;
    rake.push_back(minext[0]);
    rake.push_back(maxext[0]);
    rake.push_back(minext[1]);
    rake.push_back(maxext[1]);
    if (minext.size() == 3) {
        rake.push_back(minext[2]);
        rake.push_back(maxext[2]);
    }
    SetRake(rake);

    SetFlowDirection((int)FlowDir::FORWARD);
    SetSteadyNumOfSteps(100);
    SetVelocityMultiplier(1.0);
    SetPeriodic(vector<bool>(3, false));
    SetGridNumOfSeeds({5, 5, 1});
    SetRandomNumOfSeeds(50);
    SetFlowlineOutputFilename(Wasp::FileUtils::HomeDir() + "/VaporFlow.txt");
    SetSeedInputFilename(Wasp::GetSharePath("examples/listOfSeeds.txt"));
    SetIsSteady(true);
    SetPastNumOfTimeSteps(std::max(_dataMgr->GetNumTimeSteps() - 1, 1));

    return (0);
}

void FlowParams::SetIsSteady(bool steady) { SetValueLong(_isSteadyTag, "are we using steady advection", long(steady)); }

bool FlowParams::GetIsSteady() const
{
    long rv = GetValueLong(_isSteadyTag, long(true));
    return bool(rv);
}

void FlowParams::SetNeedFlowlineOutput(bool need) { SetValueLong(_needFlowlineOutputTag, "need to do an output of the flow lines", long(need)); }

bool FlowParams::GetNeedFlowlineOutput() const
{
    long rv = GetValueLong(_needFlowlineOutputTag, long(false));
    return bool(rv);
}

double FlowParams::GetVelocityMultiplier() const { return GetValueDouble(_velocityMultiplierTag, 1.0); }

void FlowParams::SetVelocityMultiplier(double coeff) { SetValueDouble(_velocityMultiplierTag, "Field Scale Factor", coeff); }

long FlowParams::GetSteadyNumOfSteps() const { return GetValueLong(_steadyNumOfStepsTag, 100); }

void FlowParams::SetSteadyNumOfSteps(long i) { SetValueLong(_steadyNumOfStepsTag, "num of steps for a steady integration", i); }

int FlowParams::GetSeedGenMode() const { return GetValueLong(_seedGenModeTag, (int)FlowSeedMode::UNIFORM); }

void FlowParams::SetSeedGenMode(int i) { SetValueLong(_seedGenModeTag, "", i); }

std::string FlowParams::GetSeedInputFilename() const { return GetValueString(_seedInputFilenameTag, ""); }

void FlowParams::SetSeedInputFilename(const std::string &name) { SetValueString(_seedInputFilenameTag, "filename for input seeding list", name); }

std::string FlowParams::GetFlowlineOutputFilename() const { return GetValueString(_flowlineOutputFilenameTag, ""); }
void        FlowParams::SetFlowlineOutputFilename(const std::string &name) { SetValueString(_flowlineOutputFilenameTag, "filename for output flow lines", name); }

std::vector<std::string> FlowParams::GetFlowOutputMoreVariables() const { return GetValueStringVec(_flowOutputMoreVariablesTag); }

int FlowParams::GetFlowDirection() const { return GetValueLong(_flowDirectionTag, (int)FlowDir::FORWARD); }

void FlowParams::SetFlowDirection(int i)
{
    VAssert(i == (int)FlowDir::FORWARD || i == (int)FlowDir::BACKWARD || i == (int)FlowDir::BI_DIR);
    SetValueLong(_flowDirectionTag, "", i);
}

std::vector<bool> FlowParams::GetPeriodic() const
{
    vector<bool> sav;
    sav.push_back(GetValueLong(_xPeriodicTag, false));
    sav.push_back(GetValueLong(_yPeriodicTag, false));
    if (GetRenderDim() == 3) sav.push_back(GetValueLong(_zPeriodicTag, false));

    return sav;
}

void FlowParams::SetPeriodic(const std::vector<bool> &bools)
{
    VAssert(bools.size() == 3 || bools.size() == 2);
    SetValueLong(_xPeriodicTag, "", bools[0]);
    SetValueLong(_yPeriodicTag, "", bools[1]);
    if (bools.size() == 3) SetValueLong(_zPeriodicTag, "", bools[2]);
}

void FakeRakeBox::SetExtents(const vector<double> &min, const vector<double> &max)
{
    VAssert(parent);

    vector<float> rake;
    rake.push_back(min[0]);
    rake.push_back(max[0]);
    rake.push_back(min[1]);
    rake.push_back(max[1]);
    if (min.size() == 3) {
        rake.push_back(min[2]);
        rake.push_back(max[2]);
    }

    parent->SetRake(rake);
}

Box *FlowParams::GetRakeBox()
{
    if (!_fakeRakeBox) {
        _fakeRakeBox = new FakeRakeBox(&_fakeRakeStateSave);
        _fakeRakeBox->parent = this;
    }

    Box *extBox = GetBox();
    _fakeRakeBox->SetPlanar(extBox->IsPlanar());
    _fakeRakeBox->SetOrientation(extBox->GetOrientation());

    const auto rake = GetRake();
    const auto rakesize = rake.size();
    VAssert(rakesize == 4 || rakesize == 6);
    vector<double> min, max;

    min.push_back(rake[0]);
    max.push_back(rake[1]);
    min.push_back(rake[2]);
    max.push_back(rake[3]);
    if (rakesize == 6) {
        min.push_back(rake[4]);
        max.push_back(rake[5]);
    }

    _fakeRakeBox->Box::SetExtents(min, max);

    return _fakeRakeBox;
}

std::vector<float> FlowParams::GetRake() const
{
    auto          doubles = GetValueDoubleVec(_rakeTag);
    vector<float> ret;

    int dim = GetRenderDim();
    for (int i = 0; i < dim * 2; i++) {
        if (i < doubles.size())
            ret.push_back(doubles[i]);
        else
            ret.push_back(5);
    }
    return ret;
}

void FlowParams::SetRake(const std::vector<float> &rake)
{
    const auto rakesize = rake.size();
    VAssert(rakesize == 4 || rakesize == 6);
    std::vector<double> doubles(rakesize);
    std::copy(rake.cbegin(), rake.cend(), doubles.begin());
    SetValueDoubleVec(_rakeTag, "rake boundaries", doubles);
}

std::vector<long> FlowParams::GetGridNumOfSeeds() const
{
    vector<long> sav(3);
    sav[0] = GetValueLong(_xGridNumOfSeedsTag, 5);
    sav[1] = GetValueLong(_yGridNumOfSeedsTag, 5);
    sav[2] = GetValueLong(_zGridNumOfSeedsTag, 5);

    vector<long> ret;
    int          dims = GetRenderDim();

    for (int i = 0; i < dims; i++) ret.push_back(sav[i]);

    return ret;
}

void FlowParams::SetGridNumOfSeeds(const std::vector<long> &num)
{
    VAssert(num.size() == 3 || num.size() == 2);
    SetValueLong(_xGridNumOfSeedsTag, "", num[0]);
    SetValueLong(_yGridNumOfSeedsTag, "", num[1]);
    if (num.size() == 3) SetValueLong(_zGridNumOfSeedsTag, "", num[2]);
}

long FlowParams::GetRandomNumOfSeeds() const { return GetValueLong(_randomNumOfSeedsTag, 50); }

void FlowParams::SetRandomNumOfSeeds(long num)
{
    VAssert(num >= 0);
    SetValueLong(_randomNumOfSeedsTag, "random num of seeds", num);
}

std::string FlowParams::GetRakeBiasVariable() const
{
    std::string empty;
    return GetValueString(_rakeBiasVariable, empty);
}

void FlowParams::SetRakeBiasVariable(const std::string &varname) { SetValueString(_rakeBiasVariable, "which variable to bias with", varname); }

long FlowParams::GetRakeBiasStrength() const { return (GetValueLong(_rakeBiasStrength, 0)); }

void FlowParams::SetRakeBiasStrength(long strength) { SetValueLong(_rakeBiasStrength, "bias strength", strength); }

int FlowParams::GetPastNumOfTimeSteps() const
{
    // return -1 as an obvious invalid value. Valid values are greater than 0
    return int(GetValueLong(_pastNumOfTimeSteps, -1));
}

void FlowParams::SetPastNumOfTimeSteps(int val) { SetValueLong(_pastNumOfTimeSteps, "how many past time steps to render", val); }

int FlowParams::GetSeedInjInterval() const
{
    // return -1 as an obvious invalid value.
    // 0 means no repeated seed injection
    // 1 means every time step, 2 means every other time step, etc.
    return int(GetValueLong(_seedInjInterval, -1));
}

void FlowParams::SetSeedInjInterval(int val) { SetValueLong(_seedInjInterval, "What's the interval of injecting seeds into an unsteady flow advection", val); }
