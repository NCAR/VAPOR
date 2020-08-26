#ifndef RAYCASTERPARAMS_H
#define RAYCASTERPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API RayCasterParams : public RenderParams {
public:
    RayCasterParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, std::string classType);
    RayCasterParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    virtual ~RayCasterParams();

    //
    //! Obtain current MapperFunction for the primary variable.
    //
    MapperFunction *GetMapperFunc();

    bool                GetLighting() const;
    void                SetLighting(bool);
    std::vector<double> GetLightingCoeffs() const;
    void                SetLightingCoeffs(const std::vector<double> &coeffs);
    //
    // Different ray casting methods: 1 == fixed step casting
    //                                2 == prism intersection casting
    //
    long GetCastingMode() const;
    void SetCastingMode(long);
    long GetSampleRateMultiplier() const;    // ComboBox index is held here. Need to translate
    void SetSampleRateMultiplier(long);      //   to real multipliers in RayCaster.cpp

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override { return (3); }

protected:
    static const std::string _lightingTag;
    static const std::string _lightingCoeffsTag;
    static const std::string _castingModeTag;
    static const std::string _sampleMultiplierTag;
};

}    // namespace VAPoR

#endif
