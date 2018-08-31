#include "vapor/IsoSurfaceParams.h"

using namespace VAPoR;

const std::string IsoSurfaceParams::_IsoValuesTag = "IsoValuesTag";

//
// Register class with object factory
//
static RenParamsRegistrar<IsoSurfaceParams> registrar(IsoSurfaceParams::GetClassType());

IsoSurfaceParams::IsoSurfaceParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave) : RayCasterParams(dataManager, stateSave, IsoSurfaceParams::GetClassType()) {}

IsoSurfaceParams::IsoSurfaceParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode) : RayCasterParams(dataManager, stateSave, xmlNode)
{
    if (xmlNode->GetTag() != IsoSurfaceParams::GetClassType()) { xmlNode->SetTag(IsoSurfaceParams::GetClassType()); }
}

std::vector<double> IsoSurfaceParams::GetIsoValues() const
{
    std::vector<double> defaultVec(1, 0.0);
    return GetValueDoubleVec(_IsoValuesTag, defaultVec);
}

void IsoSurfaceParams::SetIsoValues(const std::vector<double> &vals) { SetValueDoubleVec(_IsoValuesTag, "Iso Surface Iso Values", vals); }
