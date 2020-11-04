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
class VolumeRenderer;
class VolumeParams;
class ViewpointParams;
class AnnotationParams;
class Transform;

//! \class VolumeAlgorithm
//! \ingroup Public_Render
//!
//! \brief Strategy pattern for volume rendering algorithms
//!
//! \author Stanislaw Jaroszynski
//! \date Feburary, 2019
//!
//! Instances are created with a Factory pattern

class VolumeAlgorithm : private NonCopyableMixin {
public:
    enum class Type { Any, DVR, Iso };

    VolumeAlgorithm(GLManager *gl, VolumeRenderer *renderer);
    virtual ~VolumeAlgorithm() {}
    virtual void SaveDepthBuffer(bool fast){};
    virtual int  Render(bool fast) = 0;
    virtual int  LoadData(const Grid *grid) = 0;
    virtual int  LoadSecondaryData(const Grid *grid) = 0;
    virtual void DeleteSecondaryData() = 0;
    virtual void GetFinalBlendingMode(int *src, int *dst) = 0;

    //! On OSX, some shaders can run for a long time without problems
    //! while others will crash if the run too long. It seems to correlate
    //! with complexity. Chunked rendering splits the rendering into smaller
    //! tasks so they won't crash
    virtual bool  RequiresChunkedRendering() = 0;
    virtual float GuestimateFastModeSpeedupFactor() const { return 1; }

    static VolumeAlgorithm *NewAlgorithm(const std::string &name, GLManager *gl, VolumeRenderer *renderer);

    static void Register(VolumeAlgorithmFactory *f);

protected:
    GLManager *_glManager;

    VolumeParams *    GetParams() const;
    ViewpointParams * GetViewpointParams() const;
    AnnotationParams *GetAnnotationParams() const;
    Transform *       GetDatasetTransform() const;
    void              GetExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;

private:
    static std::map<std::string, VolumeAlgorithmFactory *> factories;
    VolumeRenderer *                                       _renderer;
};

class VolumeAlgorithmNull : public VolumeAlgorithm {
public:
    VolumeAlgorithmNull(GLManager *gl, VolumeRenderer *renderer) : VolumeAlgorithm(gl, renderer) {}
    static std::string GetName() { return "NULL"; }
    static Type        GetType() { return Type::Any; }
    int                Render(bool fast) { return 0; }
    int                LoadData(const Grid *grid) { return 0; }
    int                LoadSecondaryData(const Grid *grid) { return 0; }
    void               DeleteSecondaryData() {}
    bool               RequiresChunkedRendering() { return false; }
    void               GetFinalBlendingMode(int *src, int *dst) {}
};

class VolumeAlgorithmFactory {
public:
    std::string              name;
    VolumeAlgorithm::Type    type;
    virtual VolumeAlgorithm *Create(GLManager *gl, VolumeRenderer *renderer) = 0;
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
    VolumeAlgorithm *Create(GLManager *gl, VolumeRenderer *renderer) { return new T(gl, renderer); }
};

}    // namespace VAPoR
