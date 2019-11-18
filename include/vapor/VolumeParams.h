#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API VolumeParams : public RenderParams {
public:
    enum class Type { Any, DVR, Iso };

    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType);
    VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~VolumeParams();

    virtual bool IsOpaque() const;

    virtual bool   usingVariable(const std::string &varname);
    virtual string GetDefaultAlgorithmName() const;

    virtual bool   GetDefaultLightingEnabled() const { return true; }
    virtual double GetDefaultPhongAmbient() const { return 0.6; }
    virtual double GetDefaultPhongDiffuse() const { return 0.5; }
    virtual double GetDefaultPhongSpecular() const { return 0.1; }
    virtual double GetDefaultPhongShininess() const { return 4; }

    static string GetClassType() { return ("VolumeParams"); }

    std::string GetAlgorithm() const;
    void        SetAlgorithm(std::string algorithm);
    bool        GetAlgorithmWasManuallySetByUser() const;
    void        SetAlgorithmWasManuallySetByUser(bool v);

    static std::vector<float> GetSamplingRateMultiples();
    float                     GetSamplingMultiplier() const;
    void                      SetSamplingMultiplier(float d);

    using RenderParams::GetIsoValues;
    using RenderParams::SetIsoValues;
    vector<double> GetIsoValues(const string &variable);
    void           SetIsoValues(const string &variable, const vector<double> &values);

    void  SetLightingEnabled(bool v);
    bool  GetLightingEnabled() const;
    void  SetPhongAmbient(float v);
    float GetPhongAmbient() const;
    void  SetPhongDiffuse(float v);
    float GetPhongDiffuse() const;
    void  SetPhongSpecular(float v);
    float GetPhongSpecular() const;
    void  SetPhongShininess(float v);
    float GetPhongShininess() const;

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

    static const std::string _algorithmTag;
    static const std::string _algorithmWasManuallySetByUserTag;
    static const std::string _samplingRateMultiplierTag;
    static const std::string _isoValuesTag;
    static const std::string _enabledIsoValuesTag;
    static const std::string _lightingEnabledTag;
    static const std::string _phongAmbientTag;
    static const std::string _phongDiffuseTag;
    static const std::string _phongSpecularTag;
    static const std::string _phongShininessTag;

public:
    static const std::string UseColormapVariableTag;
};

};    // namespace VAPoR
