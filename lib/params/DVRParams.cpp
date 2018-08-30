#include "vapor/DVRParams.h"

using namespace VAPoR;

//
// Register class with object factory
//
static RenParamsRegistrar<DVRParams> registrar(DVRParams::GetClassType());

DVRParams::DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave) : RayCasterParams(dataManager, stateSave, DVRParams::GetClassType()) {}

DVRParams::DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode) : RayCasterParams(dataManager, stateSave, xmlNode)
{
    if (xmlNode->GetTag() != DVRParams::GetClassType()) { xmlNode->SetTag(DVRParams::GetClassType()); }
}
