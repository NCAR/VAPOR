
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/BarbParams.h>
#include <vapor/DataMgrUtils.h>

using namespace Wasp;
using namespace VAPoR;

#define X 0
#define Y 1
#define Z 2

//
// Register class with object factory!!!
//
static RenParamsRegistrar<BarbParams> registrar(BarbParams::GetClassType());

const string BarbParams::_needToRecalculateScalesTag = "NeedToRecalc";
const string BarbParams::_thicknessScaleTag = "LineThickness";
const string BarbParams::_lengthScaleTag = "VectorScale";
const string BarbParams::_gridTag = "GridDimensions";
const string BarbParams::_alignGridTag = "GridAlignedToData";
const string BarbParams::_alignGridStridesTag = "GridAlignedStrides";
const string BarbParams::_varsAre3dTag = "VarsAre3D";

BarbParams::BarbParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, BarbParams::GetClassType())
{
    SetDiagMsg("BarbParams::BarbParams() this=%p", this);

    _init();
}

BarbParams::BarbParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node) {}

BarbParams::~BarbParams() { SetDiagMsg("BarbParams::~BarbParams() this=%p", this); }

void BarbParams::SetNeedToRecalculateScales(bool val)
{
    double dval = val ? 1.0 : 0.0;
    SetValueDouble(_needToRecalculateScalesTag, "Whether or not scales need to be recalculated", dval);
}

void BarbParams::_init()
{
    SetDiagMsg("BarbParams::_init()");

    SetVariableName("");
    SetUseSingleColor(true);
    float rgb[] = {1.f, 1.f, 1.f};
    SetConstantColor(rgb);
}
