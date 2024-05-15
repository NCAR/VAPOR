//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   ParticleRenderer.cpp
//
//  Author: Stas Jaroszynski
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   March 2018
//
//  Description:
//          Definition of ParticleRenderer
//
#ifndef ParticleRENDERER_H
#define ParticleRENDERER_H

#include <glm/glm.hpp>
#include <vapor/glutil.h>
#include "vapor/VAssert.h"

#include <vapor/Renderer.h>
#include <vapor/ParticleParams.h>
#include <vapor/ShaderProgram.h>
#include <vapor/Texture.h>

namespace VAPoR {

class DataMgr;

//! \class ParticleRenderer
//! \brief Class that draws the Particles (Particles) as specified by IsolineParams
//! \author Stas Jaroszynski, Scott Pearse
//! \version 1.0
//! \date March 2018
class RENDER_API ParticleRenderer : public Renderer {
public:
    ParticleRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    virtual ~ParticleRenderer();

    static string GetClassType() { return ("Particle"); }

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();
    //! \copydoc Renderer::_paintGL()
    virtual int _paintGL(bool fast);

private:
    struct {
        size_t              ts;
        int                 rLevel;
        int                 cLevel;
        std::vector<float>  tf_lut;
        std::vector<double> tf_minMax;
        VAPoR::CoordType    boxMin, boxMax;
        float               radius;
        bool                direction;
        float               directionScale;
        size_t              stride;
        string              varName;
        string              radiusVarName;
        std::vector<std::string> fieldVars;
    } _cacheParams;

    struct Vertex {
        glm::vec3  point;
        float      value;
    };

    size_t _particlesCount;

    unsigned int _VAO = 0;
    unsigned int _VBO = 0;

    GLuint              _colorMapTexId = 0;
    GLuint              _vertexArrayId = 0;
    GLuint              _vertexBufferId = 0;
    const GLint         _colorMapTexOffset;
    float               _colorMapRange[3];
    std::vector<float>  _colorMap;

    void _clearCache() {}

    bool _particleBaseSizeIsDirty() const;
    bool _particleCacheIsDirty() const;
    bool _colormapCacheIsDirty() const;
    void _resetParticleCache();
    void _resetColormapCache();
    int  _generateParticlesLegacy(Grid*& grid, std::vector<Grid*>& vecGrids);
    int  _getGrids(Grid*& grid, std::vector<Grid*>& vecGrids) const;
    void _generateTextureData(const Grid* grid, const std::vector<Grid*>& vecGrids);
    void _computeBaseRadius();
    void _renderParticlesLegacy(const Grid* grid, const std::vector<Grid*>& vecGrids) const;
    int  _renderParticlesHelper();
    void _prepareColormap();
    glm::vec3 _getScales();
};

};    // namespace VAPoR

#endif    // ParticleRENDERER_H
