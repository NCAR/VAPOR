
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/BarbParams.h>

using namespace Wasp;
using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenParamsRegistrar<BarbParams> registrar(BarbParams::GetClassType());

const string BarbParams::_lineThicknessTag = "LineThickness";
const string BarbParams::_vectorScaleTag = "VectorScale";
const string BarbParams::_gridTag = "GridDimensions";
const string BarbParams::_alignGridTag = "GridAlignedToData";
const string BarbParams::_alignGridStridesTag = "GridAlignedStrides";

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

#ifdef DEAD
// Initialize for new metadata.
//
//
void BarbParams::Validate(int type)
{
    // Command capturing should be disabled
    assert(!Command::isRecording());

    DataMgr *dataMgr = GetDataMgr();
    if (!dataMgr) return;
    if (dataMgr->GetDataVarNames().size() == 0) return;

    (void)validateHgtVar(type, dataMgr);
    (void)validatePrimaryVar(type, dataMgr);
    validateExtents(type);
    _validateTF(type, dataMgr);

    // Perform validations unique to BarbParams:

    // Check the rake grid.

    return;
}
#endif

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

    // Only 2D variables supported. Override base class
    //
    vector<string> varnames = _dataMgr->GetDataVarNames(3, true);
    string         varname;

    if (!varnames.empty()) varname = varnames[0];
    SetVariableName(varname);

    // Initialize 2D box
    //
    if (varname.empty()) return;

    vector<double> minExt, maxExt;
    int            rc = _dataMgr->GetVariableExtents(0, varname, -1, minExt, maxExt);

    SetUseSingleColor(true);

    // Crap. No error handling from constructor. Need Initialization()
    // method.
    //
    assert(rc >= 0);
    assert(minExt.size() == maxExt.size() && minExt.size() == 3);

    GetBox()->SetExtents(minExt, maxExt);
    GetBox()->SetPlanar(true);
    GetBox()->SetOrientation(2);
}

#ifdef DEAD
void BarbParams::_validateTF(int type, DataMgr *dataMgr)
{
    if (type == 0) {
        _TFs->Clear();
        return;
    } else if (type == 1) {
        // For now simply remove any IsoControls for variables that don't
        // exist in the data set
        //
        vector<string> dvars = dataMgr->GetDataVarNames();
        vector<string> ivars = _TFs->GetNames();
        for (int i = 0; i < ivars.size(); i++) {
            if (find(dvars.begin(), dvars.end(), ivars[i]) == dvars.end()) { _TFs->Erase(ivars[i]); }
        }
    } else if (type == 2) {
    }
}
#endif
