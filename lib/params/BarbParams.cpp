
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/BarbParams.h>

using namespace Wasp;
using namespace VAPoR;

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

BarbParams::BarbParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node)
{
    SetDiagMsg("BarbParams::BarbParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != BarbParams::GetClassType()) {
        node->SetTag(BarbParams::GetClassType());
        _init();
    }
}

BarbParams::~BarbParams() { SetDiagMsg("BarbParams::~BarbParams() this=%p", this); }

void BarbParams::SetNeedToRecalculateScales(bool val)
{
    double dval = val ? 1.0 : 0.0;
    SetValueDouble(_needToRecalculateScalesTag, "Whether or not scales need to be recalculated", dval);
}

bool BarbParams::IsOpaque() const { return true; }

bool BarbParams::usingVariable(const std::string &varname)
{
    if ((varname.compare(GetHeightVariableName()) == 0)) { return (true); }

    return (varname.compare(GetVariableName()) == 0);
}

// Set everything to default values
void BarbParams::_init()
{
    SetDiagMsg("BarbParams::_init()");

    int defaultTS = GetCurrentTimestep();
    int defaultLOD = GetRefinementLevel();
    int defaultCRatio = GetCompressionLevel();
    int defaultDimension = 3;

    vector<string> varnames = _dataMgr->GetDataVarNames(defaultDimension);
    if (varnames.empty()) varnames = _dataMgr->GetDataVarNames(defaultDimension - 1);

    string varname;
    if (!varnames.empty())
        varname = varnames[0];
    else
        return;

    SetVariableName(varname);
    SetColorMapVariableName(varname);

    bool variableExists = _dataMgr->VariableExists(defaultTS, varname, defaultCRatio, defaultLOD);
    if (!variableExists) return;

    vector<double> minExt, maxExt;
    int            rc = _dataMgr->GetVariableExtents(defaultTS, varname, defaultLOD, minExt, maxExt);
    assert(rc >= 0);
    assert(minExt.size() == maxExt.size() && minExt.size() >= 2);

    GetBox()->SetExtents(minExt, maxExt);

    SetUseSingleColor(true);
    float rgb[] = {1.f, 1.f, 1.f};
    SetConstantColor(rgb);
}
