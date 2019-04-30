#pragma once

#include <vapor/Grid.h>
#include <vapor/ShaderManager.h>
#include <vector>
#include <map>
#include <string>
#include <vapor/NonCopyableMixin.h>

namespace VAPoR {

struct GLManager;
class VolumeAlgorithmFactory;

class VolumeAlgorithm : private NonCopyableMixin {
public:
    enum class Type { Any, DVR, Iso };

    VolumeAlgorithm(GLManager *gl);
    virtual ~VolumeAlgorithm() {}
    virtual int            LoadData(const Grid *grid) = 0;
    virtual int            LoadSecondaryData(const Grid *grid) = 0;
    virtual void           DeleteSecondaryData() = 0;
    virtual ShaderProgram *GetShader() const = 0;
    virtual void           SetUniforms(const ShaderProgram *shader) const = 0;
    virtual bool           IsSlow() = 0;
    virtual float          GuestimateFastModeSpeedupFactor() const { return 1; }

    static VolumeAlgorithm *NewAlgorithm(const std::string &name, GLManager *gl);

    static void Register(VolumeAlgorithmFactory *f);

private:
    static std::map<std::string, VolumeAlgorithmFactory *> factories;

protected:
    GLManager *_glManager;
};

class VolumeAlgorithmFactory {
public:
    std::string              name;
    VolumeAlgorithm::Type    type;
    virtual VolumeAlgorithm *Create(GLManager *gl) = 0;
};

template<class T> class VolumeAlgorithmRegistrar : public VolumeAlgorithmFactory {
public:
    VolumeAlgorithmRegistrar()
    {
        static_assert(std::is_base_of<VolumeAlgorithm, T>::value, "Register is not derived from VolumeAlgorithm");
        name = T::GetName();
        type = T::GetType();
        VolumeAlgorithm::Register(this);
    }
    VolumeAlgorithm *Create(GLManager *gl) { return new T(gl); }
};

}    // namespace VAPoR
