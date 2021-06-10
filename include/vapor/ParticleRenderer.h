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
    void _clearCache() {}
};

};    // namespace VAPoR

#endif    // ParticleRENDERER_H
