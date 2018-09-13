
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

const string SliceParams::_samplingRateTag = "SamplingRate";

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
    cout << "Default sample rate " << sampleRate << endl;
    std::vector<int> sampleRateVec(3, sampleRate);
    SetSampleRates(sampleRateVec);
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

std::vector<int> SliceParams::GetSampleRates() const
{
    std::vector<double> defaultVec(3, 1);
    std::vector<double> tmpVec = GetValueDoubleVec(_samplingRateTag, defaultVec);

    std::vector<int> returnVec;
    for (int i = 0; i < tmpVec.size(); i++) returnVec.push_back((int)tmpVec[i]);

    return returnVec;
}

void SliceParams::SetSampleRates(std::vector<int> rates)
{
    std::vector<double> doubleVec;
    for (int i = 0; i < rates.size(); i++) {
        doubleVec.push_back(rates[0]);
        // doubleVec.push_back((double)rates[i]);
    }
    SetValueDoubleVec(_samplingRateTag, "Set sampling rate", doubleVec);
}
