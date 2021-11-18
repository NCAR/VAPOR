
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/SliceParams.h>

using namespace Wasp;
using namespace VAPoR;

#define THREED 3

#define X 0
#define Y 1
#define Z 2

#define XY 0
#define XZ 1
#define YZ 2

#define DEFAULT_SAMPLERATE 200

//
// Register class with object factory!!!
//
static RenParamsRegistrar<SliceParams> registrar(SliceParams::GetClassType());

SliceParams::SliceParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, SliceParams::GetClassType(), THREED)
{
    SetDiagMsg("SliceParams::SliceParams() this=%p", this);

    _cachedValues.clear();
    _init();
}

SliceParams::SliceParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, THREED) { _initialized = true; }

SliceParams::~SliceParams() { SetDiagMsg("SliceParams::~SliceParams() this=%p", this); }

void SliceParams::_init()
{
    SetDiagMsg("SliceParams::_init()");

    SetFieldVariableNames(vector<string>());
}

int SliceParams::Initialize()
{
    int rc = RenderParams::Initialize();
    if (rc < 0) return (rc);
    if (_initialized) return 0;
    _initialized = true;

    Box *box = GetBox();
    box->SetOrientation(Box::XYZ);

    std::vector<double> minExt, maxExt;
    box->GetExtents(minExt, maxExt);

    std::vector<double> sampleLocation(3);
    for (int i = 0; i < 3; i++) sampleLocation[i] = (minExt[i] + maxExt[i]) / 2.0;

    SetValueDouble(RenderParams::XSlicePlaneOriginTag, "", sampleLocation[0]);
    SetValueDouble(RenderParams::YSlicePlaneOriginTag, "", sampleLocation[1]);
    SetValueDouble(RenderParams::ZSlicePlaneOriginTag, "", sampleLocation[2]);
    SetValueDouble(RenderParams::SampleRateTag, "", DEFAULT_SAMPLERATE);

    return (0);
}

void SliceParams::SetCachedValues(std::vector<double> values)
{
    _cachedValues.clear();
    _cachedValues = values;
}

std::vector<double> SliceParams::GetCachedValues() const { return _cachedValues; }

bool SliceParams::GetOrientable() const { return true; }
