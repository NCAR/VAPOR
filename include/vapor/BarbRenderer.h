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

protected:
    virtual std::string _getColorbarVariableName() const;

private:
    vector<string> _fieldVariables;    // old, used instead of _currentVarname
    double         _vectorScaleFactor;
    double         _maxThickness;

    vector<double> _currentBoxMinExts;
    vector<double> _currentBoxMaxExts;

    string         _currentHgtVar;
    vector<double> _currentBoxMinExtsTex;
    vector<double> _currentBoxMaxExtsTex;

    double _maxValue;

    void _getMagnitudeAtPoint(std::vector<VAPoR::Grid *> variables, float point[3]);

    void _recalculateScales(std::vector<VAPoR::Grid *> &varData, int ts);

    double _getDomainHypotenuse(size_t ts) const;

    void _setDefaultLengthAndThicknessScales(size_t ts, const std::vector<VAPoR::Grid *> &varData, const BarbParams *bParams);

    void _getGridRequirements(int &ts, int &refLevel, int &lod, std::vector<double> &minExts, std::vector<double> &maxExts) const;

    //! \copydoc Renderer::_initializeGL()
    virtual int _initializeGL();

    //! \copydoc Renderer::_paintGL()
    virtual int _paintGL(bool fast);

    int _generateBarbs();

    int _getVectorVarGrids(int ts, int refLevel, int lod, std::vector<double> minExts, std::vector<double> maxExts, std::vector<VAPoR::Grid *> &varData);

    int _getVarGrid(int ts, int refLevel, int lod, string varName, std::vector<double> minExts, std::vector<double> maxExts, std::vector<VAPoR::Grid *> &varData);

    void _setUpLightingAndColor();

    void _reFormatExtents(vector<float> &rakeExts) const;

    void _makeRakeGrid(vector<int> &rakeGrid) const;

    //! Protected method that performs rendering of all barbs.
    //! \param[in] DataMgr* current DataMgr
    //! \param[in] const BarbParams* associated BarbParams
    //! \param[in] int actualRefLevel refinement level to be rendered.
    //! \param[in] float vectorScale Scale factor to be applied to barbs.
    //! \param[in] float barbRadius Radius of barbs in voxel diameters.
    //! \param[in] Grid Grid used in rendering.
    //! The first three are the vector field, Grid[3] is the Height variable, Grid[4] is the color variable.
    //! \retval int zero if successful
    //	int performRendering(BarbParams* rParams,
    //		int actualRefLevel,
    //		vector <Grid *> variableData
    //	);

    float _getHeightOffset(Grid *heightVar, float xCoord, float yCoord, bool &missing) const;

    bool _makeCLUT(float clut[1024]) const;

    void _getDirection(float direction[3], std::vector<Grid *> varData, float xCoord, float yCoord, float zCoord, bool &missing) const;

    vector<double> _getScales();

    float _calculateLength(float start[3], float end[3]) const;

    void _makeStartAndEndPoint(float start[3], float end[3], float direction[3]);

    void _getStrides(vector<float> &strides, vector<int> &rakeGrid, vector<float> &rakeExts) const;

    bool _defineBarb(const std::vector<Grid *>, float start[3], float end[3], float *value, bool doColorMapping, const float clut[1024]);

    void _operateOnGrid(vector<Grid *> variableData, bool drawBarb = true);

    bool _getColorMapping(float val, const float clut[256 * 4]);

    float _calculateDirVec(const float start[3], const float end[3], float dirVec[3]);

    void _drawBackOfBarb(const float dirVec[3], const float startVertex[3]) const;

    void _drawCylinderSides(const float nextNormal[3], const float nextVertex[3], const float startNormal[3], const float startVertex[3]) const;

    void _drawBarbHead(const float dirVec[3], const float vertexPoint[3], const float startNormal[3], const float startVertex[3]) const;

    //! Protected method to draw one barb (a hexagonal tube with a cone barbhead)
    //! \param[in] const float startPoint[3] beginning position of barb
    //! \param[in] const float endPoint[3] ending position of barb
    // void drawBarb(const float startPoint[3], const float endPoint[3]);
    void _drawBarb(const std::vector<Grid *> variableData, const float startPoint[3], bool doColorMapping, const float clut[1024]);

    void _setBarbColor(float value, const float clut[1024], double crange[2]) const;

    struct Barb;
    void _drawBarb(Barb b, bool doColorMapping, const float clut[1024], double crange[2]);

#ifdef DEBUG
    _printBackDiameter(const float startVertex[18]) const;
#endif

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
        bool           needToRecalc;
    } _cacheParams;

    bool _isCacheDirty() const;
    void _saveCacheParams();

    void _clearCache() { _cacheParams.fieldVarNames.clear(); }

    struct Barb {
        float startPoint[3];
        float endPoint[3];
        float value;
        float lengthScalar;
    };

    vector<Barb> _barbCache;
};

};    // namespace VAPoR

#endif    // VAPOR_BARBRENDERER_H
