//************************************************************************

//		     Copyright (C)  2008										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//																		*
//************************************************************************/
//
//	File:		TwoDDataRenderer.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		March 2016
//
//	Description:	Implementation of the twoDImageRenderer class
//

#include <vapor/glutil.h>    // Must be included first!!!

#include <iostream>
#include <fstream>
#include <numeric>

#include <vapor/Proj4API.h>
#include <vapor/CFuncs.h>
#include <vapor/utils.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/TwoDDataRenderer.h>
#include <vapor/TwoDDataParams.h>
#include "vapor/GLManager.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<TwoDDataRenderer> registrar(TwoDDataRenderer::GetClassType(), TwoDDataParams::GetClassType());

namespace {

// Set to true to force structured grids to be renderered as
// unstructured grids.
//
const bool ForceUnstructured = false;

// GLSL shader constants
//
const string EffectBaseName = "2DData";
const string EffectName = "2DData";
const string EffectNameAttr = "2DDataAttr";
const string VertexDataAttr = "vertexDataAttr";

// Rendering primitives will be aligned with grid points
//
const bool GridAligned = true;

// Texture units. Only use data texture if GridAligned is false
//
// const int dataTexUnit = 0; // GL_TEXTURE0
// const int colormapTexUnit = 1;          // GL_TEXTURE1

// Return name of GLSL shader instance to use
//
string getEffectInstance(bool useVertAttr)
{
    if (useVertAttr)
        return (EffectNameAttr);
    else
        return (EffectName);
}

// Compute surface normal (gradient) for point (x,y) using 1st order
// central differences. 'hgtGrid' is a displacement map for Z coordinate.
//
void computeNormal(const Grid *hgtGrid, float x, float y, float dx, float dy, float mv, float &nx, float &ny, float &nz)
{
    nx = ny = 0.0;
    nz = 1.0;
    if (!hgtGrid) return;

    // Missing value?
    //
    if ((hgtGrid->GetValue(x, y)) == mv) return;

    float z_xpdx = hgtGrid->GetValue(x + dx, y);
    if (z_xpdx == mv) { z_xpdx = x; }

    float z_xmdx = hgtGrid->GetValue(x - dx, y);
    if (z_xmdx == mv) { z_xmdx = x; }

    float z_ypdy = hgtGrid->GetValue(x, y + dy);
    if (z_ypdy == mv) { z_ypdy = y; }

    float z_ymdy = hgtGrid->GetValue(x, y - dy);
    if (z_ymdy == mv) { z_ymdy = x; }

    float dzx = z_xpdx - z_xmdx;
    float dzy = z_ypdy - z_ymdy;

    nx = dy * dzx;
    ny = dx * dzy;
    nz = 1.0;
}
}    // namespace

TwoDDataRenderer::TwoDDataRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: TwoDRenderer(pm, winName, dataSetName, TwoDDataParams::GetClassType(), TwoDDataRenderer::GetClassType(), instName, dataMgr)
{
    _grid_state.clear();
    _tex_state.clear();

    _texWidth = 0;
    _texHeight = 0;
    _texelSize = 8;
    _vertsWidth = 0;
    _vertsHeight = 0;
    _nindices = 0;
    _nverts = 0;
    _colormap = NULL;
    _colormapsize = 0;

    _cMapTexID = 0;

    TwoDDataParams *rp = (TwoDDataParams *)GetActiveParams();
    MapperFunction *tf = rp->GetMapperFunc(rp->GetVariableName());

    _colormapsize = tf->getNumEntries();
    _colormap = new GLfloat[_colormapsize * 4];

    for (int i = 0; i < _colormapsize; i++) {
        _colormap[i * 4 + 0] = (float)i / (float)(_colormapsize - 1);
        _colormap[i * 4 + 1] = (float)i / (float)(_colormapsize - 1);
        _colormap[i * 4 + 2] = (float)i / (float)(_colormapsize - 1);
        _colormap[i * 4 + 3] = 1.0;
    }
}

TwoDDataRenderer::~TwoDDataRenderer()
{
    if (_cMapTexID) glDeleteTextures(1, &_cMapTexID);
    if (_colormap) delete[] _colormap;
}

int TwoDDataRenderer::_initializeGL()
{
    glGenTextures(1, &_cMapTexID);

    //
    // Standard colormap
    //
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _cMapTexID);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, _colormapsize, 0, GL_RGBA, GL_FLOAT, _colormap);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);
    return (TwoDRenderer::_initializeGL());
}

int TwoDDataRenderer::_paintGL(bool fast)
{
    if (CheckGLError() != 0) return (-1);

    TwoDDataParams *rp = (TwoDDataParams *)GetActiveParams();

    MapperFunction *tf = rp->GetMapperFunc(rp->GetVariableName());
    tf->makeLut(_colormap);
    vector<double> crange = tf->getMinMaxMapValue();

    int rc;
#ifndef NOSHADER

    string effect = getEffectInstance(GridAligned);

    // 2D Data LIGHT parameters hard coded
    //
    // _shaderMgr->UploadEffectData(effect, "lightingEnabled", (int) false);
    // _shaderMgr->UploadEffectData(effect, "kd", (float) 0.6);
    // _shaderMgr->UploadEffectData(effect, "ka", (float) 0.3);
    // _shaderMgr->UploadEffectData(effect, "ks", (float) 0.1);
    // _shaderMgr->UploadEffectData(effect, "expS", (float) 16.0);
    // _shaderMgr->UploadEffectData( effect, "lightDirection", (float) 0.0, (float) 0.0, (float) 1.0);

#endif

    ShaderProgram *s = _glManager->shaderManager->GetShader("2DData");
    if (s == nullptr) return -1;
    s->Bind();
    s->SetUniform("minLUTValue", (float)crange[0]);
    s->SetUniform("maxLUTValue", (float)crange[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, _cMapTexID);

    // Really only need to reload colormap texture if it changes
    //
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, _colormapsize, GL_RGBA, GL_FLOAT, _colormap);

    glActiveTexture(GL_TEXTURE0);
    rc = TwoDRenderer::_paintGL(fast);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);

    return (rc);
}

const GLvoid *TwoDDataRenderer::GetTexture(DataMgr *dataMgr, GLsizei &width, GLsizei &height, GLint &internalFormat, GLenum &format, GLenum &type, size_t &texelSize, bool &gridAligned)
{
    internalFormat = GL_RG32F;
    format = GL_RG;
    type = GL_FLOAT;
    texelSize = _texelSize;
    gridAligned = GridAligned;

    GLvoid *texture = (GLvoid *)_getTexture(dataMgr);
    if (!texture) return (NULL);

    width = _texWidth;
    height = _texHeight;
    return (texture);
}

int TwoDDataRenderer::GetMesh(DataMgr *dataMgr, GLfloat **verts, GLfloat **normals, GLsizei &nverts, GLsizei &width, GLsizei &height, GLuint **indices, GLsizei &nindices, bool &structuredMesh)
{
    width = 0;
    height = 0;
    nindices = 0;
    nverts = 0;

    // See if already in cache
    //
    if (!_gridStateDirty() && _sb_verts.GetBuf()) {
        width = _vertsWidth;
        height = _vertsHeight;
        *verts = (GLfloat *)_sb_verts.GetBuf();
        *normals = (GLfloat *)_sb_normals.GetBuf();
        nverts = _nverts;

        nindices = _nindices;
        *indices = (GLuint *)_sb_indices.GetBuf();
        return (0);
    }

    _gridStateClear();

    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();
    int             refLevel = rParams->GetRefinementLevel();
    int             lod = rParams->GetCompressionLevel();

    // Find box extents for ROI
    //
    vector<double> minBoxReq, maxBoxReq;
    size_t         ts = rParams->GetCurrentTimestep();
    rParams->GetBox()->GetExtents(minBoxReq, maxBoxReq);

    string varname = rParams->GetVariableName();
    int    orientation = _getOrientation(dataMgr, varname);
    if (orientation != 2) {
        SetErrMsg("Only XY plane orientations currently supported");
        return (-1);
    }

    Grid *g = NULL;
    int   rc = DataMgrUtils::GetGrids(dataMgr, ts, varname, minBoxReq, maxBoxReq, true, &refLevel, &lod, &g);
    if (rc < 0) return (-1);

    VAssert(g);

    double defaultZ = GetDefaultZ(dataMgr, ts);

    if (dynamic_cast<StructuredGrid *>(g) && !ForceUnstructured) {
        rc = _getMeshStructured(dataMgr, dynamic_cast<StructuredGrid *>(g), defaultZ);
        structuredMesh = true;
    } else {
        rc = _getMeshUnStructured(dataMgr, g, defaultZ);
        structuredMesh = false;
    }

    delete g;

    if (rc < 0) return (-1);

    _gridStateSet();

    *verts = (GLfloat *)_sb_verts.GetBuf();
    *normals = (GLfloat *)_sb_normals.GetBuf();
    nverts = _nverts;
    *indices = (GLuint *)_sb_indices.GetBuf();

    width = _vertsWidth;
    height = _vertsHeight;
    nindices = _nindices;

    return (0);
}

bool TwoDDataRenderer::_gridStateDirty() const
{
    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();

    DC::DataVar dvar;
    _dataMgr->GetDataVarInfo(rParams->GetVariableName(), dvar);

    vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);

    _grid_state_c current_state(_dataMgr->GetNumRefLevels(rParams->GetVariableName()), rParams->GetRefinementLevel(), rParams->GetCompressionLevel(), rParams->GetHeightVariableName(),
                                dvar.GetMeshName(), rParams->GetCurrentTimestep(), minExts, maxExts);

    return (_grid_state != current_state);
}

void TwoDDataRenderer::_gridStateClear() { _grid_state.clear(); }

void TwoDDataRenderer::_gridStateSet()
{
    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();

    DC::DataVar dvar;
    _dataMgr->GetDataVarInfo(rParams->GetVariableName(), dvar);

    vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);
    string meshName;

    _grid_state = _grid_state_c(_dataMgr->GetNumRefLevels(rParams->GetVariableName()), rParams->GetRefinementLevel(), rParams->GetCompressionLevel(), rParams->GetHeightVariableName(),
                                dvar.GetMeshName(), rParams->GetCurrentTimestep(), minExts, maxExts);
}

bool TwoDDataRenderer::_texStateDirty(DataMgr *dataMgr) const
{
    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();

    vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);

    _tex_state_c current_state(rParams->GetRefinementLevel(), rParams->GetCompressionLevel(), rParams->GetVariableName(), rParams->GetCurrentTimestep(), minExts, maxExts);

    return (_tex_state != current_state);
}

void TwoDDataRenderer::_texStateSet(DataMgr *dataMgr)
{
    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();

    vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);

    _tex_state = _tex_state_c(rParams->GetRefinementLevel(), rParams->GetCompressionLevel(), rParams->GetVariableName(), rParams->GetCurrentTimestep(), minExts, maxExts);
}

void TwoDDataRenderer::_texStateClear() { _tex_state.clear(); }

// Get mesh for a structured grid
//
int TwoDDataRenderer::_getMeshStructured(DataMgr *dataMgr, const StructuredGrid *g, double defaultZ)
{
    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();

    auto dims = g->GetDimensions();
    VAssert(dims[2] == 1);

    _vertsWidth = dims[0];
    _vertsHeight = dims[1];
    _nindices = _vertsWidth * 2;

    // (Re)allocate space for verts
    //
    _nverts = _vertsWidth * _vertsHeight;
    _sb_verts.Alloc(_nverts * 3 * sizeof(GLfloat));
    _sb_normals.Alloc(_nverts * 3 * sizeof(GLfloat));
    _sb_indices.Alloc(2 * _vertsWidth * sizeof(GLuint));

    int rc;
    if (!rParams->GetHeightVariableName().empty()) {
        rc = _getMeshStructuredDisplaced(dataMgr, g, defaultZ);
    } else {
        rc = _getMeshStructuredPlane(dataMgr, g, defaultZ);
    }
    if (rc < 0) return (rc);

    // Compute vertex normals
    //
    GLfloat *verts = (GLfloat *)_sb_verts.GetBuf();
    GLfloat *normals = (GLfloat *)_sb_normals.GetBuf();
    ComputeNormals(verts, _vertsWidth, _vertsHeight, normals);

    // Construct indices for a triangle strip covering one row
    // of the mesh
    //
    GLuint *indices = (GLuint *)_sb_indices.GetBuf();
    for (GLuint i = 0; i < _vertsWidth; i++) indices[2 * i] = i;
    for (GLuint i = 0; i < _vertsWidth; i++) indices[2 * i + 1] = i + _vertsWidth;

    return (0);
}

// Get mesh for an unstructured grid
//
int TwoDDataRenderer::_getMeshUnStructured(DataMgr *dataMgr, const Grid *g, double defaultZ)
{
    VAssert(g->GetTopologyDim() == 2);
    auto dims = g->GetDimensions();

    // Unstructured 2d grids are stored in 1d
    //
    _vertsWidth = std::accumulate(dims.begin(), dims.end(), 1ul, std::multiplies<size_t>());
    _vertsHeight = 1;

    // Count the number of triangle vertex indices needed
    //
    size_t             maxVertexPerCell = g->GetMaxVertexPerCell();
    vector<Size_tArr3> nodes(maxVertexPerCell);
    _nindices = 0;
    Grid::ConstCellIterator citr;
    Grid::ConstCellIterator endcitr = g->ConstCellEnd();
    for (citr = g->ConstCellBegin(); citr != endcitr; ++citr) {
        const vector<size_t> &cell = *citr;
        g->GetCellNodes(Size_tArr3{cell[0], 0, 0}, nodes);

        if (nodes.size() < 3) continue;    // degenerate
        _nindices += 3 * (nodes.size() - 2);
    }

    // (Re)allocate space for verts
    //
    _nverts = _vertsWidth;
    _sb_verts.Alloc(_nverts * 3 * sizeof(GLfloat));
    _sb_normals.Alloc(_nverts * 3 * sizeof(GLfloat));
    _sb_indices.Alloc(_nindices * sizeof(GLuint));

    return (_getMeshUnStructuredHelper(dataMgr, g, defaultZ));
    return 0;
}

int TwoDDataRenderer::_getMeshUnStructuredHelper(DataMgr *dataMgr, const Grid *g, double defaultZ)
{
    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();
    // Construct the displaced (terrain following) grid using
    // a map projection, if specified.
    //
    size_t ts = rParams->GetCurrentTimestep();
    int    refLevel = rParams->GetRefinementLevel();
    int    lod = rParams->GetCompressionLevel();

    // Find box extents for ROI
    //
    vector<double> minExts, maxExts;
    g->GetUserExtents(minExts, maxExts);

    // Try to get requested refinement level or the nearest acceptable level:
    //
    string hgtvar = rParams->GetHeightVariableName();

    Grid *hgtGrid = NULL;

    if (!hgtvar.empty()) {
        int rc = DataMgrUtils::GetGrids(dataMgr, ts, hgtvar, minExts, maxExts, true, &refLevel, &lod, &hgtGrid);

        if (rc < 0) return (rc);
        VAssert(hgtGrid);
    }

    VAssert(g->GetTopologyDim() == 2);

    GLfloat *verts = (GLfloat *)_sb_verts.GetBuf();
    GLfloat *normals = (GLfloat *)_sb_normals.GetBuf();
    GLuint * indices = (GLuint *)_sb_indices.GetBuf();

    double mv = hgtGrid ? hgtGrid->GetMissingValue() : 0.0;

    // Hard-code dx and dy for gradient calculation :-(
    //
    float dx = (maxExts[0] - minExts[0]) / 1000.0;
    float dy = (maxExts[1] - minExts[1]) / 1000.0;

    //
    // Visit each node in the grid, build a list of vertices
    //
    Grid::ConstNodeIterator nitr;
    Grid::ConstNodeIterator endnitr = g->ConstNodeEnd();
    size_t                  voffset = 0;
    for (nitr = g->ConstNodeBegin(); nitr != endnitr; ++nitr) {
        vector<double> coords;

        g->GetUserCoordinates(*nitr, coords);

        // Lookup vertical coordinate displacement as a data element
        // from the
        // height variable. Note, missing values are possible if image
        // extents are out side of extents for height variable, or if
        // height variable itself contains missing values.
        //
        double deltaZ = hgtGrid ? hgtGrid->GetValue(coords) : 0.0;
        if (deltaZ == mv) deltaZ = 0.0;

        verts[voffset + 0] = coords[0];
        verts[voffset + 1] = coords[1];
        verts[voffset + 2] = deltaZ + defaultZ;

        // Compute the surface normal using central differences
        //
        computeNormal(hgtGrid, coords[0], coords[1], dx, dy, mv, normals[voffset + 0], normals[voffset + 1], normals[voffset + 2]);

        voffset += 3;
    }

    //
    // Visit each cell in the grid. For each cell triangulate it and
    // and compute an index
    // array for the triangle list
    //
    size_t                  maxVertexPerCell = g->GetMaxVertexPerCell();
    vector<Size_tArr3>      nodes(maxVertexPerCell);
    Grid::ConstCellIterator citr;
    Grid::ConstCellIterator endcitr = g->ConstCellEnd();
    size_t                  index = 0;
    for (citr = g->ConstCellBegin(); citr != endcitr; ++citr) {
        const vector<size_t> &cell = *citr;
        g->GetCellNodes(Size_tArr3{cell[0], 0, 0}, nodes);

        if (nodes.size() < 3) continue;    // degenerate

        // Compute triangle node indices, with common vertex at
        // nodes[0]
        //
        for (int i = 0; i < nodes.size() - 2; i++) {
            indices[index++] = nodes[0][0];
            indices[index++] = nodes[i + 1][0];
            indices[index++] = nodes[i + 2][0];
        }
    }

    if (hgtGrid) { delete hgtGrid; }

    return (0);
}

// Get mesh for a structured grid displaced by a height field
//
int TwoDDataRenderer::_getMeshStructuredDisplaced(DataMgr *dataMgr, const StructuredGrid *g, double defaultZ)
{
    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();
    // Construct the displaced (terrain following) grid using
    // a map projection, if specified.
    //
    size_t ts = rParams->GetCurrentTimestep();
    int    refLevel = rParams->GetRefinementLevel();
    int    lod = rParams->GetCompressionLevel();

    // Find box extents for ROI
    //
    vector<double> minExtsReq, maxExtsReq;
    rParams->GetBox()->GetExtents(minExtsReq, maxExtsReq);

    // Try to get requested refinement level or the nearest acceptable level:
    //
    string hgtvar = rParams->GetHeightVariableName();
    VAssert(!hgtvar.empty());

    Grid *hgtGrid = NULL;
    int   rc = DataMgrUtils::GetGrids(dataMgr, ts, hgtvar, minExtsReq, maxExtsReq, true, &refLevel, &lod, &hgtGrid);
    if (rc < 0) return (rc);
    VAssert(hgtGrid);

    auto dims = g->GetDimensions();
    VAssert(dims[2] == 1);

    size_t   width = dims[0];
    size_t   height = dims[1];
    GLfloat *verts = (GLfloat *)_sb_verts.GetBuf();
    double   mv = hgtGrid->GetMissingValue();
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double x, y, zdummy;
            g->GetUserCoordinates(i, j, x, y, zdummy);

            // Lookup vertical coordinate displacement as a data element from the
            // height variable. Note, missing values are possible if image
            // extents are out side of extents for height variable, or if
            // height variable itself contains missing values.
            //
            double deltaZ = hgtGrid->GetValue(x, y, 0.0);
            if (deltaZ == mv) deltaZ = 0.0;

            double z = deltaZ + defaultZ;

            //
            verts[j * width * 3 + i * 3] = x;
            verts[j * width * 3 + i * 3 + 1] = y;
            verts[j * width * 3 + i * 3 + 2] = z;
        }
    }

    delete hgtGrid;

    return (rc);
}

// Get mesh for a structured grid that is NOT displaced by a height field.
// I.e. it's planar.
//
int TwoDDataRenderer::_getMeshStructuredPlane(DataMgr *dataMgr, const StructuredGrid *g, double defaultZ)
{
    auto dims = g->GetDimensions();
    VAssert(dims[2] == 1);

    size_t   width = dims[0];
    size_t   height = dims[1];
    GLfloat *verts = (GLfloat *)_sb_verts.GetBuf();
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double x, y, zdummy;
            g->GetUserCoordinates(i, j, x, y, zdummy);

            double z = defaultZ;

            verts[j * width * 3 + i * 3] = x;
            verts[j * width * 3 + i * 3 + 1] = y;
            verts[j * width * 3 + i * 3 + 2] = z;
        }
    }

    return (0);
}

int TwoDDataRenderer::_getOrientation(DataMgr *dataMgr, string varname)
{
    vector<string> coordvars;
    bool           ok = dataMgr->GetVarCoordVars(varname, true, coordvars);
    VAssert(ok);
    VAssert(coordvars.size() == 2);

    vector<int> axes;    // order list of coordinate axes
    for (int i = 0; i < coordvars.size(); i++) {
        DC::CoordVar cvar;
        dataMgr->GetCoordVarInfo(coordvars[i], cvar);

        axes.push_back(cvar.GetAxis());
    }

    if (axes[0] == 0) {
        if (axes[1] == 1)
            return (2);    // X-Y
        else
            return (1);    // X-Z
    }

    VAssert(axes[0] == 1 && axes[2] == 2);
    return (0);    // Y-Z
}

// Sets _texWidth, _texHeight, _sb_texture
//
const GLvoid *TwoDDataRenderer::_getTexture(DataMgr *dataMgr)
{
    // See if already in cache
    //
    if (!_texStateDirty(dataMgr) && _sb_texture.GetBuf()) { return ((const GLvoid *)_sb_texture.GetBuf()); }
    _texStateClear();

    TwoDDataParams *rParams = (TwoDDataParams *)GetActiveParams();
    size_t          ts = rParams->GetCurrentTimestep();

    int refLevel = rParams->GetRefinementLevel();
    int lod = rParams->GetCompressionLevel();

    string varname = rParams->GetVariableName();
    if (varname.empty()) {
        SetErrMsg("No variable name specified");
        return (NULL);
    }

    // Find box extents for ROI
    //
    vector<double> minBoxReq, maxBoxReq;
    rParams->GetBox()->GetExtents(minBoxReq, maxBoxReq);

    Grid *g = NULL;
    int   rc = DataMgrUtils::GetGrids(dataMgr, ts, varname, minBoxReq, maxBoxReq, true, &refLevel, &lod, &g);
    if (rc < 0) return (NULL);

    if (g->GetTopologyDim() != 2) {
        SetErrMsg("Invalid variable: %s ", varname.c_str());
        return (NULL);
    }

    // For structured grid variable data are stored in a 2D array.
    // For structured grid variable data are stored in a 1D array.
    //
    auto dims = g->GetDimensions();
    if (dynamic_cast<StructuredGrid *>(g) && !ForceUnstructured) {
        _texWidth = dims[0];
        _texHeight = dims[1];
    } else {
        _texWidth = std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<size_t>());
        _texHeight = 1;
    }

    size_t         texSize = _texWidth * _texHeight;
    GLfloat *      texture = (float *)_sb_texture.Alloc(texSize * _texelSize);
    GLfloat *      texptr = texture;
    Grid::Iterator itr;
    Grid::Iterator enditr = g->end();
    //	for (itr = g->begin(minBoxReq, maxBoxReq); itr != enditr; ++itr) {
    for (itr = g->begin(); itr != enditr; ++itr) {
        float v = *itr;

        if (v == g->GetMissingValue()) {
            *texptr++ = 0.0;    // Data value
            *texptr++ = 1.0;    // Missing value flag
        } else {
            *texptr++ = v;
            *texptr++ = 0;
        }
    }

    _texStateSet(dataMgr);

    return (texture);
}
