#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

//! \class VolumeRectilinear
//! \ingroup Public_Render
//!
//! \brief Rectilinear grid rendering algorithm
//!
//! \author Stanislaw Jaroszynski
//!

class VolumeRectilinear : public VolumeRegular {
public:
    VolumeRectilinear(GLManager *gl, VolumeRenderer *renderer);
    ~VolumeRectilinear();

    static std::string GetName() { return "Rectilinear"; }
    static Type        GetType() { return Type::DVR; }
    virtual bool       RequiresChunkedRendering() { return true; }

    virtual int            LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader() const;
    virtual void           SetUniforms(const ShaderProgram *shader) const;
    virtual float          GuestimateFastModeSpeedupFactor() const;
    virtual int CheckHardwareSupport(const Grid *grid) const;

private:
    Texture2DArray _minTexture;
    Texture2DArray _maxTexture;
    Texture2D      _BBLevelDimTexture;
    Texture2D      _coordLUTTexture;

    int  _coordDims[3];
    int  _coordSigns[3];
    bool _useHighPrecisionTriangleRoutine;
    bool _gridHasInvertedCoordinateSystemHandiness;

protected:
    virtual std::string _addDefinitionsToShader(std::string shaderName) const;
};

//! \class VolumeRectilinearIso
//! \ingroup Public_Render
//!
//! \brief Rectilinear grid isosurface rendering algorithm
//!
//! \author Stanislaw Jaroszynski
//!
//! Renders isosurfaces by ray tracing. This class is the same as the rectilinear DVR
//! except it renders an isosurface

class VolumeRectilinearIso : public VolumeRectilinear {
public:
    VolumeRectilinearIso(GLManager *gl, VolumeRenderer *renderer) : VolumeRectilinear(gl, renderer) {}
    static std::string     GetName() { return "Iso Rectilinear"; }
    static Type            GetType() { return Type::Iso; }
    virtual ShaderProgram *GetShader() const;
    virtual void           SetUniforms(const ShaderProgram *shader) const;
};

}    // namespace VAPoR
