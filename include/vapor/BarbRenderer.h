//**************************************************************
//
// Copyright (C) 2017
// University Corporation for Atmospheric Research
// All Rights Reserved
//
// *************************************************************

#ifndef VAPOR_BARBRENDERER_H
#define VAPOR_BARBRENDERER_H

#include <vapor/glutil.h>    // Must be included first!!!

#ifdef Darwin
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#include <vapor/DataMgr.h>
#include <vapor/utils.h>
#include <vapor/Renderer.h>
#include <vapor/Grid.h>
#include <vapor/BarbParams.h>

namespace VAPoR {

//! \class BarbRenderer
//! \brief Class that draws the barbs as specified by BarbParams
//! \author Scott Pearse and Alan Norton
//! \version 3.0
//! \date June 2017

class RENDER_API BarbRenderer : public Renderer {
public:
    BarbRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    virtual ~BarbRenderer();

    static string GetClassType() { return ("Barb"); }

private:
    vector<string> _fieldVariables;       // old, used instead of _currentVarname
    double         _vectorScaleFactor;    // old

    vector<double> _currentBoxMinExts;    // new!
    vector<double> _currentBoxMaxExts;    // new!

    string         _currentHgtVar;           // new!
    vector<double> _currentBoxMinExtsTex;    // new, do we need this?
    vector<double> _currentBoxMaxExtsTex;    // new, do we need this?

    GLuint _drawList;

    // Copied from TwoDRenderer.h
    //
    // ...TBD...

    double _getMaxAtBarbLocations(VAPoR::Grid *grid) const;

    std::vector<double> _getMaximumValues(size_t ts, const std::vector<string> &varnames) const;

    double _getDomainHypotenuse(size_t ts, const std::vector<string> varnames) const;

    double _calcDefaultScale(size_t ts, const std::vector<string> &varnames, const BarbParams *bParams);

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();

    //! \copydoc Renderer::_paintGL()
    virtual int _paintGL();

    void _setUpLightingAndColor();

    void _reFormatExtents(double rakeExts[6]) const;

    void _makeRakeGrid(int rakeGrid[3]) const;

    //! Protected method that performs rendering of all barbs.
    //! \param[in] DataMgr* current DataMgr
    //! \param[in] const BarbParams* associated BarbParams
    //! \param[in] int actualRefLevel refinement level to be rendered.
    //! \param[in] float vectorScale Scale factor to be applied to barbs.
    //! \param[in] float barbRadius Radius of barbs in voxel diameters.
    //! \param[in] Grid Grid used in rendering.
    //! The first three are the vector field, Grid[3] is the Height variable, Grid[4] is the color variable.
    //! \retval int zero if successful
    int performRendering(BarbParams *rParams, int actualRefLevel, float vectorScale, vector<Grid *> variableData);

    float getHeightOffset(Grid *heightVar, float xCoord, float yCoord, bool &missing);

    bool _makeCLUT(float clut[1024]) const;

    void _getDirection(float direction[3], vector<Grid *> varData, float xCoord, float yCoord, float zCoord, bool &missing) const;

    vector<double> _getScales();

    void renderGrid(int rakeGrid[3], double rakeExts[6], vector<Grid *> variableData, int timestep, float vectorLengthScale, float rad, BarbParams *params);

    bool GetColorMapping(MapperFunction *tf, float val, float clut[256 * 4]);

    //! Protected method to draw one barb (a hexagonal tube with a cone barbhead)
    //! \param[in] const float startPoint[3] beginning position of barb
    //! \param[in] const float endPoint[3] ending position of barb
    //! \param[in] float radius Radius of barb in voxels
    void drawBarb(const float startPoint[3], const float endPoint[3], float radius);

    struct {
        vector<string> fieldVarNames;
        string         heightVarName;
        string         colorVarName;
        size_t         ts;
        int            level;
        int            lod;
        bool           useSingleColor;
        float          constantColor[3];
        double         lineThickness;
        double         opacity;
        double         lengthScale;
        vector<long>   grid;
        vector<double> boxMin, boxMax;
        float          minMapValue;
        float          maxMapValue;
        float          colorSamples[10][3];
        float          alphaSamples[10];
    } _cacheParams;

    bool _isCacheDirty() const;
    void _saveCacheParams();
};
};    // namespace VAPoR

#endif    // VAPOR_BARBRENDERER_H
