//**************************************************************
//
// Copyright (C) 2017
// University Corporation for Atmospheric Research
// All Rights Reserved
//
// *************************************************************

#ifndef VAPOR_BARBRENDERER_H
#define VAPOR_BARBRENDERER_H

#include <vapor/glutil.h> // Must be included first!!!

#ifdef Darwin
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vapor/DataMgr.h>
#include <vapor/utils.h>
#include <vapor/Renderer.h>
#include <vapor/StructuredGrid.h>
#include <vapor/BarbParams.h>

namespace VAPoR {

//! \class BarbRenderer
//! \brief Class that draws the barbs as specified by BarbParams
//! \author Scott Pearse and Alan Norton
//! \version 3.0
//! \date June 2017

class RENDER_API BarbRenderer : public Renderer {

  public:
    BarbRenderer(
        const ParamsMgr *pm, string winName, string dataSetName,
        string instName, DataStatus *ds);

    virtual ~BarbRenderer();

    static string GetClassType() {
        return ("Barb");
    }

  private:
    // Copied from TwoDDataRender.h
    //
    size_t _currentTimestep;        // new!
    int _currentRefLevel;           // new!
    int _currentLod;                // new!
    vector<string> _fieldVariables; // old, used instead of _currentVarname
    double _vectorScaleFactor;      // old

    vector<double> _currentBoxMinExts; // new!
    vector<double> _currentBoxMaxExts; // new!

    size_t _currentTimestepTex;           // new, do we need this?
    string _currentHgtVar;                // new!
    vector<double> _currentBoxMinExtsTex; // new, do we need this?
    vector<double> _currentBoxMaxExtsTex; // new, do we need this?

    GLuint _cMapTexID;    // new, do we need this?
    GLfloat *_colormap;   // new, do we need this?
    size_t _colormapsize; // new, do we need this?

    // Copied from TwoDRenderer.h
    //
    // ...TBD...

    double _calcDefaultScale(const std::vector<string> &varnames,
                             const BarbParams *bParams);

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();

    //! \copydoc Renderer::_paintGL()
    virtual int _paintGL();

    //! Protected method that performs rendering of all barbs.
    //! \param[in] DataMgr* current DataMgr
    //! \param[in] const RenderParams* associated RenderParams
    //! \param[in] int actualRefLevel refinement level to be rendered.
    //! \param[in] float vectorScale Scale factor to be applied to barbs.
    //! \param[in] float barbRadius Radius of barbs in voxel diameters.
    //! \param[in] StructuredGrid*[5] StructuredGrid used in rendering.
    //! The first three are the vector field, StructuredGrid[3] is the Height variable, StructuredGrid[4] is the color variable.
    //! \retval int zero if successful
    int performRendering(const RenderParams *rParams,
                         int actualRefLevel, float vectorScale,
                         StructuredGrid *variableData[5]);
    //! Protected method that performs rendering if grid is not aligned to data
    //! \param[in] int rakeGrid[3] Dimensions of rake
    //! \param[in] double rakeExts[6] Extents of rake
    //! \param[in] StructuredGrid* variableData[5] StructuredGrid containing field, color, and height variables
    //! \param[in] float vectorLengthScale:  Scale factor used in setting barb length
    //! \param[in] int timestep:  current time step.
    //! \param[in] float rad Radius of barbs in voxel diameters.
    void renderUnaligned(int rakeGrid[3], double rakeExts[6],
                         StructuredGrid *variableData[5], int timestep,
                         float vectorLengthScale, float rad, const RenderParams *params);

    float getHeightOffset(StructuredGrid *heightVar, float xCoord,
                          float yCoord, bool &missing);

    void renderScottsGrid(int rakeGrid[3], double rakeExts[6],
                          StructuredGrid *variableData[5], int timestep,
                          float vectorLengthScale, float rad, const RenderParams *params);

    //! Protected method that performs rendering if grid is aligned to data
    //! \param[in] int rakeGrid[3] Dimensions of rake
    //! \param[in] double rakeExts[6] Extents of rake
    //! \param[in] StructuredGrid* variableData[5] StructuredGrid containing field, color, and height variables
    //! \param[in] float vectorLengthScale:  Scale factor used in setting barb length
    //! \param[in] int timestep:  current time step.
    //! \param[in] float rad Radius of barbs in voxel diameters.
    void renderAligned(int rakeGrid[3], double rakeExts[6],
                       StructuredGrid *variableData[5], int timestep,
                       float vectorLengthScale, float rad, const RenderParams *params);

    //! Protected method to draw one barb (a hexagonal tube with a cone barbhead)
    //! \param[in] const float startPoint[3] beginning position of barb
    //! \param[in] const float endPoint[3] ending position of barb
    //! \param[in] float radius Radius of barb in voxels
    void drawBarb(const float startPoint[3], const float endPoint[3], float radius);
};
}; // namespace VAPoR

#endif //VAPOR_BARBRENDERER_H
