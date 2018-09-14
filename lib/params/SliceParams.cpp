
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/SliceParams.h>

using namespace Wasp;
using namespace VAPoR;

#define THREED 3

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

const string SliceParams::_sampleRateTag = "SampleRate";

//
// Register class with object factory!!!
//
static RenParamsRegistrar<SliceParams> registrar(SliceParams::GetClassType());

SliceParams::SliceParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, SliceParams::GetClassType(), THREED)
{
    SetDiagMsg("SliceParams::SliceParams() this=%p", this);

    _init();
}

SliceParams::SliceParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, THREED)
{
    SetDiagMsg("SliceParams::SliceParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != SliceParams::GetClassType()) {
        node->SetTag(SliceParams::GetClassType());
        _init();
    }
}

SliceParams::~SliceParams() { SetDiagMsg("SliceParams::~SliceParams() this=%p", this); }

void SliceParams::_init()
{
    SetDiagMsg("SliceParams::_init()");

    Box *box = GetBox();
    box->SetOrientation(XY);

    std::vector<double> minExt, maxExt;
    box->GetExtents(minExt, maxExt);
    double average = (minExt[Z] + maxExt[Z]) / 2.f;
    minExt[Z] = average;
    maxExt[Z] = average;
    box->SetExtents(minExt, maxExt);

    int sampleRate = GetDefaultSampleRate();
    SetSampleRate(sampleRate);
}

int SliceParams::GetDefaultSampleRate() const
{
    string         varName = GetVariableName();
    int            refLevel = GetRefinementLevel();
    vector<size_t> dimsAtLevel, bsAtLevel;
    _dataMgr->GetDimLensAtLevel(varName, refLevel, dimsAtLevel, bsAtLevel);
    int sampleRate = *max_element(dimsAtLevel.begin(), dimsAtLevel.end());
    return sampleRate;
}

bool SliceParams::IsOpaque() const { return true; }

bool SliceParams::usingVariable(const std::string &varname)
{
    if ((varname.compare(GetHeightVariableName()) == 0)) { return (true); }

    return (varname.compare(GetVariableName()) == 0);
}

int SliceParams::GetSampleRate() const
{
    int rate = (int)GetValueDouble(_sampleRateTag, 50);
    return rate;
}

void SliceParams::SetSampleRate(int rate) { SetValueDouble(_sampleRateTag, "Set sample rate", (double)rate); }

void SliceParams::SetCachedValues(std::vector<double> values)
{
    _cachedValues.clear();
    _cachedValues = values;
}

std::vector<double> SliceParams::GetCachedValues() const { return _cachedValues; }
