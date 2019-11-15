
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/WireFrameParams.h>

using namespace Wasp;
using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenParamsRegistrar<WireFrameParams> registrar(WireFrameParams::GetClassType());

WireFrameParams::WireFrameParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, WireFrameParams::GetClassType(), 3)
{
    SetDiagMsg("WireFrameParams::WireFrameParams() this=%p", this);

    _init();
}

WireFrameParams::WireFrameParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 3)
{
    SetDiagMsg("WireFrameParams::WireFrameParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != WireFrameParams::GetClassType()) {
        node->SetTag(WireFrameParams::GetClassType());
        _init();
    }
}

WireFrameParams::~WireFrameParams() { SetDiagMsg("WireFrameParams::~WireFrameParams() this=%p", this); }

bool WireFrameParams::IsOpaque() const { return true; }

bool WireFrameParams::usingVariable(const std::string &varname)
{
    if ((varname.compare(GetHeightVariableName()) == 0)) { return (true); }

    return (varname.compare(GetVariableName()) == 0);
}

// Set everything to default values
void WireFrameParams::_init()
{
    SetDiagMsg("WireFrameParams::_init()");

    SetFieldVariableNames(vector<string>());
}
