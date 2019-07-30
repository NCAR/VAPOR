#include "vapor/VolumeIsoRenderer.h"
#include <vapor/VolumeIsoParams.h>

#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

#include <vapor/VolumeRegular.h>
#include <vapor/VolumeCellTraversal.h>


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
        Grid *grid = _dataMgr->GetVariable(vp->GetCurrentTimestep(), vp->GetVariableName(), vp->GetRefinementLevel(), vp->GetCompressionLevel());
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
    return !GetActiveParams()->UseSingleColor();
}

void VolumeIsoRenderer::_setShaderUniforms(const ShaderProgram *shader, const bool fast) const
{
    VolumeRenderer::_setShaderUniforms(shader, fast);
    
    VolumeIsoParams *vp = (VolumeIsoParams *)GetActiveParams();
    
    vector<double> isoValuesD = vp->GetIsoValues();
    vector<float> isoValues(isoValuesD.begin(), isoValuesD.end());
    vector<bool> enabledIsoValues = vp->GetEnabledIsoValues();
    shader->SetUniformArray("isoValue", 4, isoValues.data());
    shader->SetUniform("isoEnabled[0]", (bool)enabledIsoValues[0]);
    shader->SetUniform("isoEnabled[1]", (bool)enabledIsoValues[1]);
    shader->SetUniform("isoEnabled[2]", (bool)enabledIsoValues[2]);
    shader->SetUniform("isoEnabled[3]", (bool)enabledIsoValues[3]);
    if (_cache.constantColor.size() == 4)
        shader->SetUniform("constantColor", *(vec4*)_cache.constantColor.data());
}

std::string VolumeIsoRenderer::_getDefaultAlgorithmForGrid(const Grid *grid) const
{
    if (GLManager::GetVendor() == GLManager::Vendor::Intel)
        return VolumeRegularIso::GetName();
    
    if (dynamic_cast<const RegularGrid *>   (grid)) return VolumeRegularIso      ::GetName();
    if (dynamic_cast<const StructuredGrid *>(grid)) return VolumeCellTraversalIso::GetName();
    MyBase::SetErrMsg("Unsupported grid type: %s", grid->GetType().c_str());
    return "";
}

void VolumeIsoRenderer::_getLUTFromTF(const MapperFunction *tf, float *LUT) const
{
    tf->makeLut(LUT);
}

int VolumeIsoRenderer::OSPRayUpdate(OSPModel world)
{
    int ret = VolumeRenderer::OSPRayUpdate(world);
    
    VolumeIsoParams *vp = (VolumeIsoParams *)GetActiveParams();
    vector<double> allIsoValues = vp->GetIsoValues();
    vector<bool> enabledIsoValueFlags = vp->GetEnabledIsoValues();
    vector<float> enabledIsoValues;
    for (int i = 0; i < allIsoValues.size(); i++) {
        if (enabledIsoValueFlags[i])
            enabledIsoValues.push_back(allIsoValues[i]);
    }
    
    OSPData ospData = ospNewData(enabledIsoValues.size(), OSP_FLOAT, enabledIsoValues.data());
    ospCommit(ospData);
    ospSetData(_ospIsoSurfaces, "isovalues", ospData);
    ospRelease(ospData);
    
    if (!_ospMaterial) {
        _ospMaterial = ospNewMaterial2("scivis", "OBJMaterial");
        ospSetMaterial(_ospIsoSurfaces, _ospMaterial);
    }
    
    vec3 diffuse = glm::make_vec3(vp->GetConstantColor().data());
    diffuse *= vp->GetPhongDiffuse();
    vec3 specular = vec3(1.f) * vp->GetPhongSpecular();
    ospSet3fv(_ospMaterial, "Kd", (float*)&diffuse);
    ospSet3fv(_ospMaterial, "Ks", (float*)&specular);
    ospSet1f(_ospMaterial, "Ns", vp->GetPhongShininess());
    ospSet1f(_ospMaterial, "d", _cache.tf->getOpacityScale());
    
    ospCommit(_ospMaterial);
    ospCommit(_ospIsoSurfaces);
    
    return ret;
}

void VolumeIsoRenderer::OSPRayDelete(OSPModel world)
{
    VolumeRenderer::OSPRayDelete(world);
    
    ospRelease(_ospMaterial); _ospMaterial = nullptr;
    ospRelease(_ospIsoSurfaces); _ospIsoSurfaces = nullptr;
}

int VolumeIsoRenderer::OSPRayLoadTF()
{
    RenderParams *rp = GetActiveParams();
    if (!rp->UseSingleColor())
        return VolumeRenderer::OSPRayLoadTF();
    
    MapperFunction *tf = _needToLoadTF();
    if (!tf)
        return 0;
    
    if (_cache.tf) delete _cache.tf;
    _cache.tf = new MapperFunction(*tf);
    _cache.mapRange = tf->getMinMaxMapValue();
    
    float *LUT = new float[4 * 256];
    tf->makeLut(LUT);
    
    if (!_tf) {
        _tf = ospNewTransferFunction("piecewise_linear");
        ospSetObject(_volume, "transferFunction", _tf);
    }
    
    float colors[3];
    float opacities[1];
    
    rp->GetConstantColor(colors);
    opacities[0] = rp->GetConstantOpacity();
    
    OSPData colorData = ospNewData(1, OSP_FLOAT3, colors);
    OSPData opacityData = ospNewData(1, OSP_FLOAT, opacities);
    ospCommit(colorData);
    ospCommit(opacityData);
    ospSetData(_tf, "colors", colorData);
    ospSetData(_tf, "opacities", opacityData);
    ospRelease(colorData);
    ospRelease(opacityData);
    
    osp::vec2f valueRange = {(float)_cache.mapRange[0], (float)_cache.mapRange[1]};
    ospSetVec2f(_tf, "valueRange", valueRange);
    
    ospCommit(_tf);
    
    return 0;
}

void VolumeIsoRenderer::OSPRayAddObjectToWorld(OSPModel world)
{
    if (!_ospIsoSurfaces) {
        _ospIsoSurfaces = ospNewGeometry("isosurfaces");
        ospAddGeometry(world, _ospIsoSurfaces);
    }
    
    ospSetObject(_ospIsoSurfaces, "volume", _volume);
}

void VolumeIsoRenderer::OSPRayRemoveObjectFromWorld(OSPModel world)
{
    ospRemoveGeometry(world, _ospIsoSurfaces);
}
