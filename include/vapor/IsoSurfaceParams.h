#ifndef ISOSURFACEPARAMS_H
#define ISOSURFACEPARAMS_H

#include "vapor/RayCasterParams.h"

namespace VAPoR {

class PARAMS_API IsoSurfaceParams : public RayCasterParams {
public:
    IsoSurfaceParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave);
    IsoSurfaceParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    static std::string GetClassType() { return ("IsoSurfaceParams"); }

    std::vector<double> GetIsoValues() const;
    void                SetIsoValues(std::vector<double>);
    std::vector<bool>   GetEnabledIsoValueFlags() const;
    void                SetEnabledIsoValueFlags(const std::vector<bool> &);
    bool                UseSingleColor() const;

protected:
    static const std::string _isoValuesTag;
    static const std::string _enabledIsoValuesTag;
};

};    // namespace VAPoR

#endif
