#pragma once

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

class PARAMS_API ModelParams : public RenderParams {
public:
    //! Path to a 3D model file or Vapor vms scene file.
    //! 3D models can be in formats supported by the ASSIMP library.
    //! A description of the Vapor vms scene format can be found at
    //! https://ncar.github.io/VaporDocumentationWebsite/vaporApplicationReference/modelRenderer.html
    //! Applies to data of type: string
    static const std::string FileTag;

    ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);
    ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType);
    ModelParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);
    virtual ~ModelParams();

    static string GetClassType() { return ("ModelParams"); }

    //! \copydoc RenderParams::GetRenderDim()
    //
    virtual size_t GetRenderDim() const override { return (0); }

    //! \copydoc RenderParams::GetActualColorMapVariableName()
    virtual string GetActualColorMapVariableName() const override { return ""; }

private:
    void _init();
};

};    // namespace VAPoR
