#ifndef RAYCASTERPARAMS_H
#define RAYCASTERPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/GetAppPath.h>

namespace VAPoR {

class PARAMS_API RayCasterParams : public RenderParams {
public:
    RayCasterParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, std::string classType);
    RayCasterParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    virtual ~RayCasterParams();

    //
    // (Pure virtual methods from RenderParams)
    //
    virtual bool IsOpaque() const override { return false; }
    virtual bool usingVariable(const std::string &varname) override
    {
        return false;    // since this class is for an image, not rendering a variable.
    }

    //
    //! Obtain current MapperFunction for the primary variable.
    //
    MapperFunction *GetMapperFunc();

    bool                GetLighting() const;
    void                SetLighting(bool);
    std::vector<double> GetLightingCoeffs() const;
    void                SetLightingCoeffs(const std::vector<double> &coeffs);

protected:
    static const std::string _lightingTag;
    static const std::string _lightingCoeffsTag;
};

}    // namespace VAPoR

#endif
