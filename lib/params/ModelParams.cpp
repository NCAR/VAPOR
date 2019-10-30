
#include <vapor/ModelParams.h>
#include <string>
#include <cassert>

using namespace Wasp;
using namespace VAPoR;

const std::string ModelParams::FileTag = "FileTag";

//
// Register class with object factory!!!
//
static RenParamsRegistrar<ModelParams> registrar(ModelParams::GetClassType());

ModelParams::ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, ModelParams::GetClassType(), 3)
{
    SetDiagMsg("ModelParams::ModelParams() this=%p", this);
    _init();
}

ModelParams::ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType) : RenderParams(dataMgr, ssave, classType, 3)
{
    SetDiagMsg("ModelParams::ModelParams() this=%p", this);
    _init();
}

ModelParams::ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 3)
{
    SetDiagMsg("ModelParams::ModelParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != ModelParams::GetClassType()) {
        node->SetTag(ModelParams::GetClassType());
        _init();
    }
}

ModelParams::~ModelParams() { SetDiagMsg("ModelParams::~ModelParams() this=%p", this); }

bool ModelParams::IsOpaque() const { return true; }

bool ModelParams::usingVariable(const std::string &varname) { return false; }

// Set everything to default values
void ModelParams::_init() { SetDiagMsg("ModelParams::_init()"); }
