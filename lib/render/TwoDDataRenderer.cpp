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

#include <vapor/Proj4API.h>
#include <vapor/CFuncs.h>
#include <vapor/AnimationParams.h>
#include <vapor/ShaderMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/TwoDDataRenderer.h>
#include <vapor/TwoDDataParams.h>

using namespace VAPoR;

namespace {

const string EffectBaseName = "2DData";
const string EffectName = "2DData";

const int dataTexUnit = 0;        // GL_TEXTURE0
const int colormapTexUnit = 1;    // GL_TEXTURE1

}    // namespace

//
// Register class with object factory!!!
//
static RendererRegistrar<TwoDDataRenderer> registrar(TwoDDataRenderer::GetClassType(), TwoDDataParams::GetClassType());

TwoDDataRenderer::TwoDDataRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: TwoDRenderer(pm, winName, dataSetName, TwoDDataParams::GetClassType(), TwoDDataRenderer::GetClassType(), instName, dataMgr)
{
    _texWidth = 0;
    _texHeight = 0;
    _texelSize = 8;
    _currentTimestep = 0;
    _currentRefLevel = -1;
    _currentLod = -1;
    _currentVarname.clear();
    _currentBoxMinExts.clear();
    _currentBoxMaxExts.clear();
    _currentTimestepTex = 0;
    _currentHgtVar.clear();
    _currentBoxMinExtsTex.clear();
    _currentBoxMaxExtsTex.clear();
    _vertsWidth = 0;
    _vertsHeight = 0;
    _colormap = NULL;
    _colormapsize = 0;

    _cMapTexID = 0;

    TwoDDataParams *  rp = (TwoDDataParams *)GetActiveParams();
    TransferFunction *tf = rp->MakeTransferFunc(rp->GetVariableName());

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
//#define	NOSHADER
#ifndef NOSHADER
    if (!_shaderMgr) {
        SetErrMsg("Programmable shading not available");
        return (-1);
    }

    if (!_shaderMgr->EffectExists(EffectName)) {
        int rc = _shaderMgr->DefineEffect(EffectBaseName, "", EffectName);
        if (rc < 0) return (-1);
    }
#endif

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

int TwoDDataRenderer::_paintGL()
{
    if (printOpenGLError() != 0) return (-1);

    TwoDDataParams *rp = (TwoDDataParams *)GetActiveParams();

    TransferFunction *tf = rp->MakeTransferFunc(rp->GetVariableName());
    tf->makeLut(_colormap);
    vector<double> crange = tf->getMinMaxMapValue();

    int rc;
#ifndef NOSHADER

    rc = _shaderMgr->EnableEffect(EffectName);
    if (rc < 0) return (-1);

    // 2D Data LIGHT parameters hard coded
    //
    _shaderMgr->UploadEffectData(EffectName, "lightingEnabled", (int)false);
    _shaderMgr->UploadEffectData(EffectName, "kd", (float)0.6);
    _shaderMgr->UploadEffectData(EffectName, "ka", (float)0.3);
    _shaderMgr->UploadEffectData(EffectName, "ks", (float)0.1);
    _shaderMgr->UploadEffectData(EffectName, "expS", (float)16.0);
    _shaderMgr->UploadEffectData(EffectName, "lightDirection", (float)0.0, (float)0.0, (float)1.0);
    _shaderMgr->UploadEffectData(EffectName, "minLUTValue", (float)crange[0]);
    _shaderMgr->UploadEffectData(EffectName, "maxLUTValue", (float)crange[1]);

    _shaderMgr->UploadEffectData(EffectName, "colormap", colormapTexUnit);
    _shaderMgr->UploadEffectData(EffectName, "dataTexture", dataTexUnit);

#endif

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _cMapTexID);
    glEnable(GL_TEXTURE_1D);

    // Really only need to reload colormap texture if it changes
    //
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, _colormapsize, GL_RGBA, GL_FLOAT, _colormap);

    glActiveTexture(GL_TEXTURE0);
    rc = TwoDRenderer::_paintGL();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);
    glDisable(GL_TEXTURE_1D);

#ifndef NOSHADER
    _shaderMgr->DisableEffect();
#endif

    return (rc);
}

const GLvoid *TwoDDataRenderer::_GetTexture(DataMgr *dataMgr, GLsizei &width, GLsizei &height, GLint &internalFormat, GLenum &format, GLenum &type, size_t &texelSize)
{
    internalFormat = GL_RG32F;
    format = GL_RG;
    type = GL_FLOAT;
    texelSize = _texelSize;

    TwoDDataParams *myParams = (TwoDDataParams *)GetActiveParams();

    GLvoid *texture = (GLvoid *)_getTexture(dataMgr);
    if (!texture) return (NULL);

    width = _texWidth;
    height = _texHeight;
    return (texture);
}

int TwoDDataRenderer::_GetMesh(DataMgr *dataMgr, GLfloat **verts, GLfloat **normals, GLsizei &width, GLsizei &height)
{
    width = 0;
    height = 0;

    // See if already in cache
    //
    if (!_gridStateDirty() && _sb_verts.GetBuf()) {
        width = _vertsWidth;
        height = _vertsHeight;
        *verts = (GLfloat *)_sb_verts.GetBuf();
        *normals = (GLfloat *)_sb_normals.GetBuf();
        return (0);
    }
    _gridStateClear();

    AnimationParams *myAnimationParams = GetAnimationParams();
    TwoDDataParams * myParams = (TwoDDataParams *)GetActiveParams();
    int              refLevel = myParams->GetRefinementLevel();
    int              lod = myParams->GetCompressionLevel();

    // Find box extents for ROI
    //
    vector<double> minBoxReq, maxBoxReq;
    size_t         ts = myAnimationParams->GetCurrentTimestep();
    myParams->GetBox()->GetExtents(minBoxReq, maxBoxReq);

    // Get scene scaling factors
    //
    vector<double> stretchFac = myParams->GetStretchFactors();
    assert(stretchFac.size() == 3);

    string varname = myParams->GetVariableName();
    int    orientation = _getOrientation(dataMgr, varname);
    if (orientation != 2) {
        SetErrMsg("Only XY plane orientations currently supported");
        return (-1);
    }

    StructuredGrid *sg = NULL;
    int             rc = DataMgrUtils::GetGrids(dataMgr, ts, varname, minBoxReq, maxBoxReq, true, &refLevel, &lod, &sg);
    if (rc < 0) return (-1);

    assert(sg);

    vector<size_t> dims;
    sg->GetDimensions(dims);
    assert(dims.size() == 2);

    _vertsWidth = dims[0];
    _vertsHeight = dims[1];

    // (Re)allocate space for verts
    //
    size_t   vertsSize = _vertsWidth * _vertsHeight * 3;
    GLfloat *dummy = (float *)_sb_verts.Alloc(vertsSize * sizeof(*dummy));
    dummy = (float *)_sb_normals.Alloc(vertsSize * sizeof(*dummy));

    rc = 0;
    double defaultZ = minBoxReq[2];
    if (!myParams->GetHeightVariableName().empty()) {
        rc = _getMeshDisplaced(dataMgr, sg, stretchFac, defaultZ);
    } else {
        rc = _getMeshPlane(dataMgr, sg, stretchFac, defaultZ);
    }
    dataMgr->UnlockGrid(sg);
    delete sg;

    if (rc < 0) return (-1);

    _gridStateSet();

    // Compute vertex normals
    //
    *verts = (GLfloat *)_sb_verts.GetBuf();
    *normals = (GLfloat *)_sb_normals.GetBuf();
    _ComputeNormals(*verts, _vertsWidth, _vertsHeight, *normals);

    width = _vertsWidth;
    height = _vertsHeight;

    return (0);
}

bool TwoDDataRenderer::_gridStateDirty() const
{
    TwoDDataParams * myParams = (TwoDDataParams *)GetActiveParams();
    AnimationParams *myAnimationParams = GetAnimationParams();

    int            refLevel = myParams->GetRefinementLevel();
    int            lod = myParams->GetCompressionLevel();
    string         hgtVar = myParams->GetHeightVariableName();
    int            ts = myAnimationParams->GetCurrentTimestep();
    vector<double> boxMinExts, boxMaxExts;
    myParams->GetBox()->GetExtents(boxMinExts, boxMaxExts);

    return (refLevel != _currentRefLevel || lod != _currentLod || hgtVar != _currentHgtVar || ts != _currentTimestep || boxMinExts != _currentBoxMinExts || boxMaxExts != _currentBoxMaxExts);
}

void TwoDDataRenderer::_gridStateClear()
{
    _currentRefLevel = 0;
    _currentLod = 0;
    _currentHgtVar.clear();
    _currentTimestep = -1;
    _currentBoxMinExts.clear();
    _currentBoxMaxExts.clear();
}

void TwoDDataRenderer::_gridStateSet()
{
    TwoDDataParams * myParams = (TwoDDataParams *)GetActiveParams();
    AnimationParams *myAnimationParams = GetAnimationParams();
    _currentRefLevel = myParams->GetRefinementLevel();
    _currentLod = myParams->GetCompressionLevel();
    _currentHgtVar = myParams->GetHeightVariableName();
    _currentTimestep = myAnimationParams->GetCurrentTimestep();
    myParams->GetBox()->GetExtents(_currentBoxMinExts, _currentBoxMaxExts);
}

bool TwoDDataRenderer::_texStateDirty(DataMgr *dataMgr) const
{
    TwoDDataParams * myParams = (TwoDDataParams *)GetActiveParams();
    AnimationParams *myAnimationParams = GetAnimationParams();

    int            ts = myAnimationParams->GetCurrentTimestep();
    vector<double> boxMinExts, boxMaxExts;
    myParams->GetBox()->GetExtents(boxMinExts, boxMaxExts);
    string varname = myParams->GetVariableName();

    return (_currentTimestepTex != ts || _currentBoxMinExtsTex != boxMinExts || _currentBoxMaxExtsTex != boxMaxExts || _currentVarname != varname);
}

void TwoDDataRenderer::_texStateSet(DataMgr *dataMgr)
{
    TwoDDataParams * myParams = (TwoDDataParams *)GetActiveParams();
    AnimationParams *myAnimationParams = GetAnimationParams();
    string           varname = myParams->GetVariableName();

    _currentTimestepTex = myAnimationParams->GetCurrentTimestep();
    myParams->GetBox()->GetExtents(_currentBoxMinExtsTex, _currentBoxMaxExtsTex);
    _currentVarname = varname;
}

void TwoDDataRenderer::_texStateClear()
{
    _currentTimestepTex = -1;
    _currentBoxMinExtsTex.clear();
    _currentBoxMaxExtsTex.clear();
    _currentVarname.clear();
}

int TwoDDataRenderer::_getMeshDisplaced(DataMgr *dataMgr, StructuredGrid *sg, const vector<double> &scaleFac, double defaultZ)
{
    // Construct the displaced (terrain following) grid using
    // a map projection, if specified.
    //
    AnimationParams *myAnimationParams = GetAnimationParams();
    size_t           ts = myAnimationParams->GetCurrentTimestep();

    TwoDDataParams *myParams = (TwoDDataParams *)GetActiveParams();
    int             refLevel = myParams->GetRefinementLevel();
    int             lod = myParams->GetCompressionLevel();

    // Get user extents of sg. Use these to get the height variable
    //
    vector<double> minExtsReq, maxExtsReq;
    sg->GetUserExtents(minExtsReq, maxExtsReq);

    // Try to get requested refinement level or the nearest acceptable level:
    //
    string hgtvar = myParams->GetHeightVariableName();
    assert(!hgtvar.empty());

    StructuredGrid *hgtGrid = NULL;
    int             rc = DataMgrUtils::GetGrids(dataMgr, ts, hgtvar, minExtsReq, maxExtsReq, true, &refLevel, &lod, &hgtGrid);
    if (rc < 0) return (rc);
    assert(hgtGrid);

    vector<size_t> dims;
    sg->GetDimensions(dims);
    assert(dims.size() == 2);

    size_t   width = dims[0];
    size_t   height = dims[1];
    GLfloat *verts = (GLfloat *)_sb_verts.GetBuf();
    double   mv = hgtGrid->GetMissingValue();
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double x, y, z;
            sg->GetUserCoordinates(i, j, 0, &x, &y, &z);

            // Lookup vertical coordinate displacement as a data element from the
            // height variable. Note, missing values are possible if image
            // extents are out side of extents for height variable, or if
            // height variable itself contains missing values.
            //
            double deltaZ = hgtGrid->GetValue(x, y, 0.0);
            if (deltaZ == mv) deltaZ = 0.0;

            z = deltaZ + defaultZ;

            // Finally apply stretch factors
            //
            verts[j * width * 3 + i * 3] = x * scaleFac[0];
            verts[j * width * 3 + i * 3 + 1] = y * scaleFac[1];
            verts[j * width * 3 + i * 3 + 2] = z * scaleFac[2];
        }
    }

    dataMgr->UnlockGrid(hgtGrid);
    delete hgtGrid;

    return (rc);
}

int TwoDDataRenderer::_getMeshPlane(DataMgr *dataMgr, StructuredGrid *sg, const vector<double> &scaleFac, double defaultZ)
{
    vector<size_t> dims;
    sg->GetDimensions(dims);
    assert(dims.size() == 2);

    size_t   width = dims[0];
    size_t   height = dims[1];
    GLfloat *verts = (GLfloat *)_sb_verts.GetBuf();
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            double x, y, z;
            sg->GetUserCoordinates(i, j, 0, &x, &y, &z);

            z = defaultZ;

            // Finally apply stretch factors
            //
            verts[j * width * 3 + i * 3] = x * scaleFac[0];
            verts[j * width * 3 + i * 3 + 1] = y * scaleFac[1];
            verts[j * width * 3 + i * 3 + 2] = z * scaleFac[2];
        }
    }

    return (0);
}

int TwoDDataRenderer::_getOrientation(DataMgr *dataMgr, string varname)
{
    vector<string> coordvars;
    bool           ok = dataMgr->GetVarCoordVars(varname, true, coordvars);
    assert(ok);
    assert(coordvars.size() == 2);

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

    assert(axes[0] == 1 && axes[2] == 2);
    return (0);    // Y-Z
}

// Sets _texWidth, _texHeight, _sb_texture
//
const GLvoid *TwoDDataRenderer::_getTexture(DataMgr *dataMgr)
{
    // See if already in cache
    //
    if (!_texStateDirty(dataMgr) && _sb_texture.GetBuf()) {
        cout << "_getTexture already cached" << endl;
        return ((const GLvoid *)_sb_texture.GetBuf());
    }
    _texStateClear();

    AnimationParams *myAnimationParams = GetAnimationParams();
    size_t           ts = myAnimationParams->GetCurrentTimestep();

    TwoDDataParams *myParams = (TwoDDataParams *)GetActiveParams();
    int             refLevel = myParams->GetRefinementLevel();
    int             lod = myParams->GetCompressionLevel();

    string varname = myParams->GetVariableName();
    if (varname.empty()) {
        SetErrMsg("No variable name specified");
        return (NULL);
    }

    size_t ndim;
    dataMgr->GetNumDimensions(varname, ndim);
    if (ndim != 2) {
        SetErrMsg("Invalid variable: %s ", varname.c_str());
        return (NULL);
    }

    // Find box extents for ROI
    //
    vector<double> minBoxReq, maxBoxReq;
    myParams->GetBox()->GetExtents(minBoxReq, maxBoxReq);

    StructuredGrid *sg = NULL;
    int             rc = DataMgrUtils::GetGrids(dataMgr, ts, varname, minBoxReq, maxBoxReq, true, &refLevel, &lod, &sg);

    if (rc < 0) return (NULL);

    vector<size_t> dims;
    sg->GetDimensions(dims);
    assert(dims.size() == 2);

    _texWidth = dims[0];
    _texHeight = dims[1];

    size_t   texSize = _texWidth * _texHeight;
    GLfloat *texture = (float *)_sb_texture.Alloc(texSize * _texelSize);
    GLfloat *texptr = texture;

    StructuredGrid::Iterator itr;
    for (itr = sg->begin(); itr != sg->end(); ++itr) {
        float v = *itr;

        if (v == sg->GetMissingValue()) {
            *texptr++ = 0.0;    // Data value
            *texptr++ = 1.0;    // Missing value flag
        } else {
            *texptr++ = v;
            *texptr++ = 0;
        }
    }

    _texStateSet(dataMgr);

    // Unlock the StructuredGrid
    //
    dataMgr->UnlockGrid(sg);

    return (texture);
}
