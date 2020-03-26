
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

#define MIN_DEFAULT_SAMPLERATE 200

const string SliceParams::_sampleRateTag   = "SampleRate";

//
// Register class with object factory!!!
//
static RenParamsRegistrar<SliceParams> registrar(SliceParams::GetClassType());


SliceParams::SliceParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave
) : RenderParams(dataMgr, ssave, SliceParams::GetClassType(), THREED) {
	SetDiagMsg("SliceParams::SliceParams() this=%p", this);

    _cachedValues.clear();
	_init();
}

SliceParams::SliceParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node
) : RenderParams(dataMgr, ssave, node, THREED) {}

SliceParams::~SliceParams() {
	SetDiagMsg("SliceParams::~SliceParams() this=%p", this);
}

void SliceParams::_init() {
	SetDiagMsg("SliceParams::_init()");

	SetFieldVariableNames(vector <string>());


    SetSampleRate(MIN_DEFAULT_SAMPLERATE);
}

int SliceParams::Initialize() {
	int rc = RenderParams::Initialize();
	if (rc<0) return(rc);

    Box* box = GetBox();
    box->SetOrientation(XY);

    std::vector<double> minExt, maxExt;
    box->GetExtents(minExt, maxExt);
    double average = (minExt[Z] + maxExt[Z])/2.f;
    minExt[Z] = average;
    maxExt[Z] = average;
    box->SetExtents(minExt, maxExt);

	return(0);
}


int SliceParams::GetDefaultSampleRate() const {
    string varName = GetVariableName();
    int refLevel = GetRefinementLevel();
    vector<size_t> dimsAtLevel;
    _dataMgr->GetDimLensAtLevel(varName, refLevel, dimsAtLevel);
    int sampleRate = *max_element(dimsAtLevel.begin(), dimsAtLevel.end());
    
    if (sampleRate < MIN_DEFAULT_SAMPLERATE)
        sampleRate = MIN_DEFAULT_SAMPLERATE;

    return sampleRate;
}

int SliceParams::GetSampleRate() const {
    int rate = (int)GetValueDouble(_sampleRateTag, MIN_DEFAULT_SAMPLERATE);
    return rate;
}

void SliceParams::SetSampleRate(int rate) {
    SetValueDouble(_sampleRateTag, "Set sample rate",  (double)rate);
}

void SliceParams::SetCachedValues(std::vector<double> values) {
    _cachedValues.clear();
    _cachedValues = values;
}

std::vector<double> SliceParams::GetCachedValues() const {
    return _cachedValues;
}
