
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/TwoDDataParams.h>
#include <vapor/DataMgrUtils.h>

using namespace Wasp;
using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenParamsRegistrar<TwoDDataParams> registrar(TwoDDataParams::GetClassType());

TwoDDataParams::TwoDDataParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, TwoDDataParams::GetClassType(), 2)
{
    SetDiagMsg("TwoDDataParams::TwoDDataParams() this=%p", this);

    _init();
}

TwoDDataParams::TwoDDataParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 2)
{
    SetDiagMsg("TwoDDataParams::TwoDDataParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != TwoDDataParams::GetClassType()) {
        node->SetTag(TwoDDataParams::GetClassType());
        _init();
    }
}

TwoDDataParams::~TwoDDataParams() { SetDiagMsg("TwoDDataParams::~TwoDDataParams() this=%p", this); }

bool TwoDDataParams::IsOpaque() const { return true; }

bool TwoDDataParams::usingVariable(const std::string &varname)
{
    if ((varname.compare(GetHeightVariableName()) == 0)) { return (true); }

    return (varname.compare(GetVariableName()) == 0);
}

// Set everything to default values
void TwoDDataParams::_init()
{
    SetDiagMsg("TwoDDataParams::_init()");

    // Necessary?
    //
    SetFieldVariableNames(vector<string>());
}
