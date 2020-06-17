#include <vapor/VolumeAlgorithm.h>
#include <vapor/VolumeParams.h>

using namespace VAPoR;
using std::vector;
using std::string;

VolumeAlgorithm::VolumeAlgorithm(GLManager *gl)
: _glManager(gl) {}

VolumeAlgorithm *VolumeAlgorithm::NewAlgorithm(const std::string &name, GLManager *gl)
{
    if (factories.count(name)) {
        return factories[name]->Create(gl);
    }
    Wasp::MyBase::SetErrMsg("Invalid volume rendering algorithm: \"%s\"\n", name.c_str());
    VAssert(0);
    return nullptr;
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

#include <vapor/GLManager.h>

static VolumeAlgorithmRegistrar<VolumeAlgorithmNull> registration;

ShaderProgram *VolumeAlgorithmNull::GetShader() const {return _glManager->shaderManager->GetShader("VolumeDVR");}
