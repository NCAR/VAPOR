#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/STLUtils.h>

namespace VAPoR {

class PARAMS_API VolumeParams : public RenderParams {
public:
    enum class Type { Any, DVR, Iso };

    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType);
    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~VolumeParams();

    virtual string GetDefaultAlgorithmName() const;

    virtual bool   GetDefaultLightingEnabled() const { return true; }
    virtual double GetDefaultPhongAmbient() const { return 0.6; }
    virtual double GetDefaultPhongDiffuse() const { return 0.5; }
    virtual double GetDefaultPhongSpecular() const { return 0.1; }
    virtual double GetDefaultPhongShininess() const { return 0.2; }

    static string GetClassType() { return ("VolumeParams"); }

    //! Get the current raytracing algorithm
    //! \retval string - Current raytracing algorithm (Regular, Curvilinear, or Ospray) 
    std::string GetAlgorithm() const;

    //! Set the current raytracing algorithm
    //! \param[in] string - Raytracing algorithm (Regular, Curvilinear, or Ospray)
    void        SetAlgorithm(std::string algorithm);

    void        SetAlgorithmByUser(std::string algorithm);
    bool        GetAlgorithmWasManuallySetByUser() const;
    void        SetAlgorithmWasManuallySetByUser(bool v);
    static std::vector<float> GetSamplingRateMultiples();

    //! Get the sampling rate multiplier used with the current raytracing algorithm
    //! \retval long - Sampling rate multiplier
    long                      GetSamplingMultiplier() const;
    
    //! Set the sampling rate multiplier used with the current raytracing algorithm
    //! \param[in] long - Sampling rate multiplier
    void                      SetSamplingMultiplier(long d);

    using RenderParams::GetIsoValues;
    using RenderParams::SetIsoValues;
    vector<double> GetIsoValues(const string &variable) override;
    void           SetIsoValues(const string &variable, const vector<double> &values) override;

    //! Enable or disable lighting from the position of the camera
    //! \param[in] bool - Enable lighting (1/true) or disable lighting (0/false)
    void  SetLightingEnabled(bool v);
    
    //! Get the state for whether lighting is enabled or disabled
    //! \retval bool - State for enabled lighting (1/true) or disabled lighting (0/false)
    bool  GetLightingEnabled() const;
    
    //! Set the Phong Ambient lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong ambient lighting coefficient
    void  SetPhongAmbient(float v);
    
    //! Get the Phong Ambient lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong ambient lighting coefficient
    float GetPhongAmbient() const;

    //! Set the Phong Diffuse lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong diffuse lighting coefficient
    void  SetPhongDiffuse(float v);
    
    //! Get the Phong Diffuse lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong diffuse lighting coefficient
    float GetPhongDiffuse() const;

    //! Set the Phong Specular lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong specular lighting coefficient
    void  SetPhongSpecular(float v);
    
    //! Get the Phong Specular lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong specular lighting coefficient
    float GetPhongSpecular() const;
    
    //! Set the Phong Shininess lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong shininess lighting coefficient
    void  SetPhongShininess(float v);
    
    //! Get the Phong Diffuse lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model)
    //! \param[in] float - Phong shininess lighting coefficient
    float GetPhongShininess() const;

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override { return (3); }

    //! \copydoc RenderParams::GetActualColorMapVariableName()
    virtual string GetActualColorMapVariableName() const override
    {
        if (GetAlgorithm() == OSPVolmeAlgorithmName)
            return GetVariableName();
        else if (GetValueLong(UseColormapVariableTag, 0))
            return GetColorMapVariableName();
        else
            return GetVariableName();
    }

    static const std::vector<std::string> GetAlgorithmNames(Type type = Type::Any);
    static void                           Register(const std::string &name, Type type = Type::Any);

private:
    void _init();

    struct AlgorithmEntry {
        const std::string name;
        const Type        type;
        bool              operator==(const VolumeParams::AlgorithmEntry &b) { return std::tie(name, type) == std::tie(b.name, b.type); }
    };
    static std::vector<AlgorithmEntry> _algorithms;

    static const std::string _algorithmWasManuallySetByUserTag;
    static const std::string _isoValuesTag;
    static const std::string _enabledIsoValuesTag;

public:
    static const std::string _algorithmTag;

    //! If this is enabled, the volume opacity will be controlled by the main variable while the colormapping will be determined by the colormap variable
    static const std::string UseColormapVariableTag;
    static const std::string SamplingRateMultiplierTag;

    //! The VolumeDensityTag applies an opacity factor to the entirety of the volume rendering
    //! in addition to the opacity applied in the Transfer Function.
    //! Values range between 0.0 (completely transparent) and 1.0 (completely opaque).
    static const std::string VolumeDensityTag;

    static const std::string LightingEnabledTag;
    static const std::string PhongAmbientTag;
    static const std::string PhongDiffuseTag;
    static const std::string PhongSpecularTag;
    static const std::string PhongShininessTag;

    static const std::string OSPDensity;
    static const std::string OSPSampleRateScalar;
    static const std::string OSPAmbientLightIntensity;
    static const std::string OSPDirectionalLightIntensity;

    static const std::string OSPVolmeAlgorithmName;
};

};    // namespace VAPoR
