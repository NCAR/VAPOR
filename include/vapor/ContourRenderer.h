//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   ContourRenderer.cpp
//
//  Author: Stas Jaroszynski
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   March 2018
//
//  Description:
//          Definition of ContourRenderer
//
#ifndef CONTOURRENDERER_H
#define CONTOURRENDERER_H

#include <GL/glew.h>
#ifdef Darwin
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <cassert>

#include <vapor/Renderer.h>
#include <vapor/ContourParams.h>

namespace VAPoR {

class DataMgr;

//! \class ContourRenderer
//! \brief Class that draws the contours (contours) as specified by IsolineParams
//! \author Stas Jaroszynski
//! \version 1.0
//! \date March 2018
class RENDER_API ContourRenderer : public Renderer {
public:
    ContourRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    virtual ~ContourRenderer();

    static string GetClassType() { return ("Contour"); }

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();
    //! \copydoc Renderer::_paintGL()
    virtual int _paintGL();

private:
    GLuint _drawList;

    struct {
        string         varName;
        string         heightVarName;
        int            ts;
        int            level;
        int            lod;
        bool           useSingleColor;
        float          constantColor[3];
        float          lineThickness;
        float          opacity;
        vector<double> boxMin, boxMax;
        vector<double> contourValues;
        vector<float>  contourColors;
    } _cacheParams;

    void _buildCache();
    bool _isCacheDirty() const;
    void _saveCacheParams();
};

};    // namespace VAPoR

#endif    // CONTOURRENDERER_H
