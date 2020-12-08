#include <vapor/VolumeAlgorithm.h>
#include <vapor/VolumeParams.h>
#include <vapor/VolumeRenderer.h>
#include <glm/glm.hpp>

using namespace VAPoR;
using std::vector;
using std::string;

VolumeAlgorithm::VolumeAlgorithm(GLManager *gl, VolumeRenderer *renderer)
: _glManager(gl), _renderer(renderer) {}

VolumeAlgorithm *VolumeAlgorithm::NewAlgorithm(const std::string &name, GLManager *gl, VolumeRenderer *renderer)
{
    if (factories.count(name)) {
        return factories[name]->Create(gl, renderer);
    }
    Wasp::MyBase::SetErrMsg("Invalid volume rendering algorithm: \"%s\"\n", name.c_str());
    return nullptr;
}

VolumeParams *VolumeAlgorithm::GetParams() const
{
    return (VolumeParams *)_renderer->GetActiveParams();
}

ViewpointParams *VolumeAlgorithm::GetViewpointParams() const
{
    return _renderer->GetViewpointParams();
}

AnnotationParams *VolumeAlgorithm::GetAnnotationParams() const
{
    return _renderer->GetAnnotationParams();
}

Transform *VolumeAlgorithm::GetDatasetTransform() const
{
    return _renderer->GetDatasetTransform();
}

void VolumeAlgorithm::GetExtents(glm::vec3 *dataMin_, glm::vec3 *dataMax_, glm::vec3 *userMin_, glm::vec3 *userMax_) const
{
    vector<double> minRendererExtents, maxRendererExtents;
    GetParams()->GetBox()->GetExtents(minRendererExtents, maxRendererExtents);
    
    vector<double> minDataExtents, maxDataExtents;
    auto p = GetParams();
    _renderer->_dataMgr->GetVariableExtents(p->GetCurrentTimestep(), p->GetVariableName(), p->GetRefinementLevel(), p->GetCompressionLevel(), minDataExtents, maxDataExtents);
    
    glm::vec3 userMin = glm::vec3(minRendererExtents[0], minRendererExtents[1], minRendererExtents[2]);
    glm::vec3 userMax = glm::vec3(maxRendererExtents[0], maxRendererExtents[1], maxRendererExtents[2]);
    glm::vec3 dataMin = glm::vec3(minDataExtents[0], minDataExtents[1], minDataExtents[2]);
    glm::vec3 dataMax = glm::vec3(maxDataExtents[0], maxDataExtents[1], maxDataExtents[2]);
    
    if (dataMin_) *dataMin_ = dataMin;
    if (dataMax_) *dataMax_ = dataMax;
    
    // Moving domain allows area outside of data to be selected
    if (userMin_) *userMin_ = glm::max(userMin, dataMin);
    if (userMax_) *userMax_ = glm::min(userMax, dataMax);
}

std::map<std::string, VolumeAlgorithmFactory*> VolumeAlgorithm::factories;
void VolumeAlgorithm::Register(VolumeAlgorithmFactory *f)
{
    VolumeParams::Type type = VolumeParams::Type::Any;
    if      (f->type == Type::Iso) type = VolumeParams::Type::Iso;
    else if (f->type == Type::DVR) type = VolumeParams::Type::DVR;
 
    factories[f->name] = f;
    VolumeParams::Register(f->name, type);
}

static VolumeAlgorithmRegistrar<VolumeAlgorithmNull> registration;
