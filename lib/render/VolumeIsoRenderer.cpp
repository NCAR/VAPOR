#include "vapor/VolumeIsoRenderer.h"
#include <vapor/VolumeIsoParams.h>

#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

#include <vapor/VolumeRegular.h>
#include <vapor/VolumeCellTraversal.h>
#include <vapor/VolumeOSPRay.h>


using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

using namespace VAPoR;

static RendererRegistrar<VolumeIsoRenderer> registrar( VolumeIsoRenderer::GetClassType(),
                                                VolumeIsoParams::GetClassType() );


VolumeIsoRenderer::VolumeIsoRenderer( const ParamsMgr*    pm,
                        std::string&        winName,
                        std::string&        dataSetName,
                        std::string&        instName,
                        DataMgr*            dataMgr )
          : VolumeRenderer(  pm,
                        winName,
                        dataSetName,
                        VolumeIsoParams::GetClassType(),
                        VolumeIsoRenderer::GetClassType(),
                        instName,
                        dataMgr )
{
    // An ugly fix but I don't think we have a mechanism for this
    if (_needToSetDefaultAlgorithm()) {
        VolumeParams *vp = (VolumeParams*)GetActiveParams();
		vector <double> minExt, maxExt;
		vp->GetBox()->GetExtents(minExt, maxExt);

        Grid *grid = _dataMgr->GetVariable(vp->GetCurrentTimestep(), vp->GetVariableName(), vp->GetRefinementLevel(), vp->GetCompressionLevel(), minExt, maxExt);
        if (grid) {
            string algorithmName = _getDefaultAlgorithmForGrid(grid);
            vp->SetAlgorithm(algorithmName);
            delete grid;
        } else {
            vp->SetAlgorithm(VolumeRegularIso::GetName());
        }
    }
}

VolumeIsoRenderer::~VolumeIsoRenderer()
{
}

bool VolumeIsoRenderer::_usingColorMapData() const
{
    return GetActiveParams()->GetValueLong(VolumeIsoParams::UseColormapVariableTag, false);
}

void VolumeIsoRenderer::_setShaderUniforms(const ShaderProgram *shader, const bool fast) const
{
//    VolumeRenderer::_setShaderUniforms(shader, fast);
    
    vector<double> isoValuesD = GetActiveParams()->GetIsoValues();
    vector<float> isoValues(isoValuesD.begin(), isoValuesD.end());
    vector<bool> enabledIsoValues(4, false);
    for (int i = 0; i < isoValues.size(); i++)
        enabledIsoValues[i] = true;
    shader->SetUniformArray("isoValue", isoValues.size(), isoValues.data());
    shader->SetUniform("isoEnabled[0]", (bool)enabledIsoValues[0]);
    shader->SetUniform("isoEnabled[1]", (bool)enabledIsoValues[1]);
    shader->SetUniform("isoEnabled[2]", (bool)enabledIsoValues[2]);
    shader->SetUniform("isoEnabled[3]", (bool)enabledIsoValues[3]);
    if (_cache.constantColor.size() == 4)
        shader->SetUniform("constantColor", *(vec4*)_cache.constantColor.data());
}

std::string VolumeIsoRenderer::_getDefaultAlgorithmForGrid(const Grid *grid) const
{
    bool intel = GLManager::GetVendor() == GLManager::Vendor::Intel;
    
    if (dynamic_cast<const RegularGrid *>     (grid)) return VolumeRegularIso      ::GetName();
    if (dynamic_cast<const StructuredGrid *>  (grid)) return intel ? VolumeOSPRayIso::GetName() : VolumeCellTraversalIso::GetName();
//    if (dynamic_cast<const UnstructuredGrid *>(grid)) return VolumeOSPRayIso       ::GetName();
    MyBase::SetErrMsg("Unsupported grid type: %s", grid->GetType().c_str());
    return "";
}

void VolumeIsoRenderer::_getLUTFromTF(const MapperFunction *tf, float *LUT) const
{
    tf->makeLut(LUT);
}

