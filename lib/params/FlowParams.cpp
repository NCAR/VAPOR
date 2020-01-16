#include "vapor/FlowParams.h"

using namespace VAPoR;

const std::string FlowParams::_isSteadyTag = "IsSteadyTag";
const std::string FlowParams::_velocityMultiplierTag = "VelocityMultiplierTag";
const std::string FlowParams::_steadyNumOfStepsTag = "SteadyNumOfStepsTag";
const std::string FlowParams::_seedGenModeTag = "SeedGenModeTag";
const std::string FlowParams::_seedInputFilenameTag = "SeedInputFilenameTag";
const std::string FlowParams::_flowlineOutputFilenameTag = "FlowlineOutputFilenameTag";
const std::string FlowParams::_flowDirectionTag = "FlowDirectionTag";
const std::string FlowParams::_needFlowlineOutputTag = "NeedFlowlineOutputTag";
const std::string FlowParams::_periodicTag = "PeriodicTag";
const std::string FlowParams::_rakeTag = "RakeTag";
const std::string FlowParams::_rakeBiasVariable = "RakeBiasVariable";
const std::string FlowParams::_rakeBiasStrength = "RakeBiasStrength";
const std::string FlowParams::_pastNumOfTimeSteps = "PastNumOfTimeSteps";
const std::string FlowParams::_seedInjInterval = "SeedInjInterval";
const std::string FlowParams::_gridNumOfSeedsTag = "GridNumOfSeeds";
const std::string FlowParams::_randomNumOfSeedsTag = "RandomNumOfSeeds";

static RenParamsRegistrar<FlowParams> registrar(FlowParams::GetClassType());

// Constructor
FlowParams::FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave) : RenderParams(dataManager, stateSave, FlowParams::GetClassType(), 3 /* max dim */)
{
    SetVariableName("");
    SetDiagMsg("FlowParams::FlowParams() this=%p", this);

    // At this point the base class is initialized, and the _Box is properly initialized
    // to be the extents of the domain. Let's use that information to initialize the rake!
    std::vector<double> minext, maxext;
    auto                box = RenderParams::GetBox();
    box->GetExtents(minext, maxext);
    std::vector<float> floats(minext.size() * 2);
    for (int i = 0; i < minext.size(); i++) {
        floats[i * 2] = minext[i];
        floats[i * 2 + 1] = maxext[i];
    }
    this->SetRake(floats);
}

FlowParams::FlowParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *node) : RenderParams(dataManager, stateSave, node, 3 /* max dim */)
{
    SetDiagMsg("FlowParams::FlowParams() this=%p", this);
}

// Destructor
FlowParams::~FlowParams() { SetDiagMsg("FlowParams::~FlowParams() this=%p", this); }

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

int FlowParams::GetSeedGenMode() const
{
    auto val = GetValueString(_seedGenModeTag, "");
    for (const auto &e : _seed2Str) {
        if (val == e.second) return e.first;
    }
    return 0;
}

void FlowParams::SetSeedGenMode(int i)
{
    for (const auto &e : _seed2Str) {
        if (i == e.first) {
            SetValueString(_seedGenModeTag, "which way to generate seeds", e.second);
            return;
        }
    }
    SetValueString(_seedGenModeTag, "which way to generate seeds", "");
}

std::string FlowParams::GetSeedInputFilename() const { return GetValueString(_seedInputFilenameTag, ""); }

void FlowParams::SetSeedInputFilename(const std::string &name) { SetValueString(_seedInputFilenameTag, "filename for input seeding list", name); }

std::string FlowParams::GetFlowlineOutputFilename() const { return GetValueString(_flowlineOutputFilenameTag, ""); }

void FlowParams::SetFlowlineOutputFilename(const std::string &name) { SetValueString(_flowlineOutputFilenameTag, "filename for output flow lines", name); }

int FlowParams::GetFlowDirection() const
{
    auto val = GetValueString(_flowDirectionTag, "");
    for (const auto &e : _dir2Str) {
        if (val == e.second) return e.first;
    }
    return 0;
}

void FlowParams::SetFlowDirection(int i)
{
    for (const auto &e : _dir2Str) {
        if (i == e.first) {
            SetValueString(_flowDirectionTag, "flow direction", e.second);
            return;
        }
    }
    SetValueString(_flowDirectionTag, "flow direction", "");
}

std::vector<bool> FlowParams::GetPeriodic() const
{
    auto longs = GetValueLongVec(_periodicTag);
    if (longs.size() != 3 && longs.size() != 2) {
        std::vector<long> tmp(3, 0);
        longs = tmp;
    }
    std::vector<bool> bools(longs.size(), false);
    for (int i = 0; i < bools.size(); i++)
        if (longs[i] != 0) bools[i] = true;

    return bools;
}

void FlowParams::SetPeriodic(const std::vector<bool> &bools)
{
    VAssert(bools.size() == 3 || bools.size() == 2);
    std::vector<long> longs(bools.size(), 0);
    for (int i = 0; i < bools.size(); i++)
        if (bools[i]) longs[i] = 1;

    SetValueLongVec(_periodicTag, "any axis is periodic", longs);
}

std::vector<float> FlowParams::GetRake() const
{
    auto               doubles = GetValueDoubleVec(_rakeTag);
    std::vector<float> floats(doubles.size());
    std::copy(doubles.cbegin(), doubles.cend(), floats.begin());
    return floats;
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
    auto num = GetValueLongVec(_gridNumOfSeedsTag);
    if (num.size() == 3 || num.size() == 2)
        return num;
    else {
        // Default: 5 along each of the three dimensions.
        const std::vector<long> tmp(3, 5);
        return tmp;
    }
}

void FlowParams::SetGridNumOfSeeds(const std::vector<long> &num)
{
    VAssert(num.size() == 3 || num.size() == 2);
    SetValueLongVec(_gridNumOfSeedsTag, "grid num of seeds", num);
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

float FlowParams::GetRakeBiasStrength() const { return float(GetValueDouble(_rakeBiasStrength, 0.0f)); }

void FlowParams::SetRakeBiasStrength(float strength) { SetValueDouble(_rakeBiasStrength, "bias strength", strength); }

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
