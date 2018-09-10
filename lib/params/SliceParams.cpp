
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/SliceParams.h>

using namespace Wasp;
using namespace VAPoR;

#define THREED 3

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

void SliceParams::_init() { SetDiagMsg("SliceParams::_init()"); }

bool SliceParams::IsOpaque() const { return true; }

std::vector<int> SliceParams::GetSamplingRates() const
{
    std::vector<double> defaultVec(3, 1);
    std::vector<double> tmpVec = GetValueDoubleVec(_samplingRateTag, defaultVec);

    std::vector<int> returnVec;
    for (int i = 0; i < tmpVec.size(); i++) returnVec[i] = (int)tmpVec[i];

    return returnVec;
}

bool SliceParams::usingVariable(const std::string &varname)
{
    if ((varname.compare(GetHeightVariableName()) == 0)) { return (true); }

    return (varname.compare(GetVariableName()) == 0);
}

void SliceParams::SetSamplingRates(std::vector<int> rates)
{
    std::vector<double> doubleVec;
    for (int i = 0; i < rates.size(); i++) { doubleVec.push_back((double)rates[i]); }
    SetValueDoubleVec(_samplingRateTag, "Set sampling rate", doubleVec);
}
