#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API ParticleParams : public RenderParams {
public:
    ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
    ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType);
    ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~ParticleParams();

    static string GetClassType() { return ("ParticleParams"); }

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override { return (3); }
    virtual string GetActualColorMapVariableName() const override { return GetVariableName(); }

private:
    void _init();

public:
    //! Show direction/velocity of particles.
    //! The field variables must be set to the particles velocity vector components
    //! (this is likely done automatically if using DCP).
    static const std::string ShowDirectionTag;

    //! Scale the length of particles velocity vector
    static const std::string DirectionScaleTag;

    //! Load every nth particle. Useful for improving performance
    static const std::string StrideTag;

    //! Scale the rendered particle size
    static const std::string RenderRadiusScalarTag;

    static const std::string RenderRadiusBaseTag;
    static const std::string RenderLegacyTag;

    static const std::string LightingEnabledTag;

    //! Specifies the Phong Ambient lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Typical values: 0.0 to 1.0.
    static const std::string PhongAmbientTag;

    //! Specifies the Phong Diffuse lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Typical values: 0.0 to 1.0.
    static const std::string PhongDiffuseTag;

    //! Specifies the Phong Specular lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Typical values: 0.0 to 1.0.
    static const std::string PhongSpecularTag;

    //! Specifies the Phong Shininess lighting coefficient (https://en.wikipedia.org/wiki/Phong_reflection_model).
    //! Typical values: 0.0 to 100.0.
    static const std::string PhongShininessTag;
};

};    // namespace VAPoR
