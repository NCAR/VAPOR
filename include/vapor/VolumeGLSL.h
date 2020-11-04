#pragma once

#include <vapor/VolumeAlgorithm.h>
#include <vapor/Texture.h>
#include <vapor/MapperFunction.h>

namespace VAPoR {

//! \class VolumeGLSL
//! \ingroup Public_Render
//!
//! \brief Volume rendering algorithms using GLSL
//!
//! \author Stanislaw Jaroszynski
//! \date July, 2020

class VolumeGLSL : public VolumeAlgorithm {
public:
    VolumeGLSL(GLManager *gl, VolumeRenderer *renderer);
    ~VolumeGLSL();

    virtual void           SaveDepthBuffer(bool fast);
    virtual int            Render(bool fast);
    virtual int            LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader() const = 0;
    virtual void           SetUniforms(const ShaderProgram *shader) const = 0;
    virtual void           GetFinalBlendingMode(int *src, int *dst);

private:
    Texture1D       _LUTTexture;
    Texture1D       _LUT2Texture;
    Texture2D       _depthTexture;
    MapperFunction *_tf = 0, *_tf2 = 0;
    vector<double>  _minDataExtents, _maxDataExtents;
    vector<float>   _constantColor;

    void _loadTF();
    void _loadTF(Texture1D *texture, MapperFunction *tf, MapperFunction **cacheTF);
    void _getLUTFromTF(const MapperFunction *tf, float *LUT) const;

    void      _setShaderUniforms(const ShaderProgram *shader, const bool fast) const;
    glm::vec3 _getVolumeScales() const;
    void      _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;
    bool      _usingColorMapData() const;
};
}    // namespace VAPoR
