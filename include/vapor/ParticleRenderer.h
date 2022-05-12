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

#include <GL/glew.h>
#ifdef Darwin
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include "vapor/VAssert.h"

#include <vapor/Renderer.h>
#include <vapor/ParticleParams.h>
#include <vapor/ShaderProgram.h>
#include <vapor/Texture.h>

namespace VAPoR {

class DataMgr;

//! \class ParticleRenderer
//! \brief Class that draws the Particles (Particles) as specified by IsolineParams
//! \author Stas Jaroszynski
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
        std::vector<std::string> fieldVars;
    } _cacheParams;

    std::vector<glm::vec4> _particles;

    std::vector<int> _streamSizes;

    unsigned int _VAO = 0;
    unsigned int _VBO = 0;

    GLuint              _colorMapTexId = 0;
    GLuint              _vertexArrayId = 0;
    GLuint              _vertexBufferId = 0;
    const GLint         _colorMapTexOffset;
    float               _colorMapRange[3];
    std::vector<float>  _colorMap;

    void _clearCache() {}

    bool _particleCacheIsDirty() const;
    bool _colormapCacheIsDirty() const;
    void _resetParticleCache();
    void _resetColormapCache();
    int  _generateParticles(bool legacy=false);
    void _renderParticles();
    int  _renderParticlesHelper(bool renderDirection = false);
    void _prepareColormap();
    glm::vec3 _getScales();
};

};    // namespace VAPoR

#endif    // ParticleRENDERER_H
