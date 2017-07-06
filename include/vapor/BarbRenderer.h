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
        string instName, DataMgr *dataMgr);

    virtual ~BarbRenderer();

    static string GetClassType() {
        return ("Barb");
    }

  private:
    vector<string> _fieldVariables; // old, used instead of _currentVarname
    double _vectorScaleFactor;      // old

    vector<double> _currentBoxMinExts; // new!
    vector<double> _currentBoxMaxExts; // new!

    string _currentHgtVar;                // new!
    vector<double> _currentBoxMinExtsTex; // new, do we need this?
    vector<double> _currentBoxMaxExtsTex; // new, do we need this?

    // Copied from TwoDRenderer.h
    //
    // ...TBD...

    double _calcDefaultScale(
        size_t ts, const std::vector<string> &varnames,
        const BarbParams *bParams);

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();

    //! \copydoc Renderer::_paintGL()
    virtual int _paintGL();

    //! Protected method that performs rendering of all barbs.
    //! \param[in] DataMgr* current DataMgr
    //! \param[in] const BarbParams* associated BarbParams
    //! \param[in] int actualRefLevel refinement level to be rendered.
    //! \param[in] float vectorScale Scale factor to be applied to barbs.
    //! \param[in] float barbRadius Radius of barbs in voxel diameters.
    //! \param[in] StructuredGrid StructuredGrid used in rendering.
    //! The first three are the vector field, StructuredGrid[3] is the Height variable, StructuredGrid[4] is the color variable.
    //! \retval int zero if successful
    int performRendering(const BarbParams *rParams,
                         int actualRefLevel, float vectorScale,
                         vector<StructuredGrid *> variableData);

    float getHeightOffset(StructuredGrid *heightVar, float xCoord,
                          float yCoord, bool &missing);

    void renderScottsGrid(int rakeGrid[3], double rakeExts[6],
                          vector<StructuredGrid *> variableData, int timestep,
                          float vectorLengthScale, float rad, const BarbParams *params);

    bool GetColorMapping(TransferFunction *tf, float val);

    //! Protected method to draw one barb (a hexagonal tube with a cone barbhead)
    //! \param[in] const float startPoint[3] beginning position of barb
    //! \param[in] const float endPoint[3] ending position of barb
    //! \param[in] float radius Radius of barb in voxels
    void drawBarb(const float startPoint[3], const float endPoint[3], float radius);
};
}; // namespace VAPoR

#endif //VAPOR_BARBRENDERER_H
