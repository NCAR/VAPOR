
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

ModelParams::ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 3) {}

ModelParams::~ModelParams() { SetDiagMsg("ModelParams::~ModelParams() this=%p", this); }

// Set everything to default values
void ModelParams::_init() { SetDiagMsg("ModelParams::_init()"); }
