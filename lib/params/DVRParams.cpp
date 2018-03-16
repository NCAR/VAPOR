#include <vapor/DVRParams.h>

using namespace VAPoR;

// const std::string   DVRParams::_fileNameTag           = "DVRParams filename tag";
// const std::string   DVRParams::_isGeoRefTag          = "DVRParams isgeotiff tag";

//
// Register class with object factory
//
static RenParamsRegistrar<DVRParams> registrar(DVRParams::GetClassType());

DVRParams::DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave) : RenderParams(dataManager, stateSave, DVRParams::GetClassType(), 3 /* max dim */)
{
    SetDiagMsg("DVRParams::DVRParams() this=%p", this);
}

DVRParams::DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *node) : RenderParams(dataManager, stateSave, node, 3 /* max dim */)
{
    SetDiagMsg("DVRParams::DVRParams() this=%p", this);
    if (node->GetTag() != DVRParams::GetClassType()) { node->SetTag(DVRParams::GetClassType()); }
}

DVRParams::~DVRParams() { SetDiagMsg("DVRParams::~DVRParams() this=%p", this); }
