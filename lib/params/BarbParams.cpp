
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/BarbParams.h>

using namespace Wasp;
using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenParamsRegistrar<BarbParams> registrar(BarbParams::GetClassType());

const string BarbParams::_thicknessScaleTag = "LineThickness";
const string BarbParams::_lengthScaleTag = "VectorScale";
const string BarbParams::_gridTag = "GridDimensions";
const string BarbParams::_alignGridTag = "GridAlignedToData";
const string BarbParams::_alignGridStridesTag = "GridAlignedStrides";
const string BarbParams::_varsAre3dTag = "VarsAre3D";

BarbParams::BarbParams(
    DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, BarbParams::GetClassType()) {
    SetDiagMsg("BarbParams::BarbParams() this=%p", this);

    _init();
}

BarbParams::BarbParams(
    DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node) {
    SetDiagMsg("BarbParams::BarbParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != BarbParams::GetClassType()) {
        node->SetTag(BarbParams::GetClassType());
        _init();
    }
}

BarbParams::~BarbParams() {
    SetDiagMsg("BarbParams::~BarbParams() this=%p", this);
}

bool BarbParams::IsOpaque() const {
    return true;
}

bool BarbParams::usingVariable(const std::string &varname) {
    if ((varname.compare(GetHeightVariableName()) == 0)) {
        return (true);
    }

    return (varname.compare(GetVariableName()) == 0);
}

//Set everything to default values
void BarbParams::_init() {
    SetDiagMsg("BarbParams::_init()");

    // Only 2D variables supported. Override base class
    //
    vector<string> varnames = _dataMgr->GetDataVarNames(3, true);
    string varname;

    if (!varnames.empty())
        varname = varnames[0];
    SetVariableName(varname);

    // Initialize 2D box
    //
    if (varname.empty())
        return;

    vector<double> minExt, maxExt;
    int rc = _dataMgr->GetVariableExtents(0, varname, 0, minExt, maxExt);

    SetUseSingleColor(true);
    float rgb[] = {1.f, 1.f, 1.f};
    SetConstantColor(rgb);

    int grid[] = {4, 4, 1};
    SetGrid(grid);

    // Crap. No error handling from constructor. Need Initialization()
    // method.
    //
    assert(rc >= 0);
    assert(minExt.size() == maxExt.size() && minExt.size() == 3);

    GetBox()->SetExtents(minExt, maxExt);
    //GetBox()->SetPlanar(true);
    //GetBox()->SetOrientation(2);
}
