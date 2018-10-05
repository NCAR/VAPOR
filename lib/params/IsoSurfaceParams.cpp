#include "vapor/IsoSurfaceParams.h"

using namespace VAPoR;

const std::string IsoSurfaceParams::_isoValuesTag = "IsoValuesTag";
const std::string IsoSurfaceParams::_enabledIsoValuesTag = "EnabledIsoValuesTag";

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
    // std::vector<double> defaultVec( 4, 0.0 );
    // return GetValueDoubleVec( _isoValuesTag, defaultVec );

    std::vector<double> defaultVec(4, 0.0);
    std::vector<double> isovals = GetValueDoubleVec(_isoValuesTag, defaultVec);
    std::vector<bool>   enabled = GetEnabledIsoValueFlags();

    // Remove disabled isovalues
    cout << "Before: " << isovals.size() << endl;
    int size = enabled.size();
    for (int i = size - 1; i >= 0; i--) {
        if (!enabled[i]) isovals.erase(isovals.begin() + i);
    }
    cout << "After: " << isovals.size() << endl;
    return isovals;
}

void IsoSurfaceParams::SetIsoValues(std::vector<double> vals)
{
    int expectedSize = 4;
    if (vals.size() != expectedSize)    // make sure vals has the expected size.
        vals.resize(expectedSize, 0.0);
    SetValueDoubleVec(_isoValuesTag, "Iso Surface Iso Values", vals);
}

std::vector<bool> IsoSurfaceParams::GetEnabledIsoValueFlags() const
{
    std::vector<long> defaultVal(4, 0);
    defaultVal[0] = 1;
    std::vector<long> enabled = GetValueLongVec(_enabledIsoValuesTag, defaultVal);
    std::vector<bool> retVal;
    for (int i = 0; i < enabled.size(); i++) retVal.push_back((bool)enabled[i]);
    return retVal;
}

void IsoSurfaceParams::SetEnabledIsoValueFlags(const std::vector<bool> &enabled)
{
    std::vector<long> in;
    for (int i = 0; i < enabled.size(); i++) in.push_back((long)enabled[i]);
    int expectedSize = 4;
    if (in.size() != expectedSize) in.resize(expectedSize, false);
    SetValueLongVec(_enabledIsoValuesTag, "Iso Surface Enabled Flags", in);
}
