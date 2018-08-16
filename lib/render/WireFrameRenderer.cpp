//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   WireFrameRenderer.cpp
//
//  Author: John Clyne
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   June 2018
//
//  Description:
//          Implementation of WireFrameRenderer
//

#include <sstream>
#include <string>
#include <iterator>

#include <vapor/glutil.h>    // Must be included first!!!

#include <vapor/Renderer.h>
#include <vapor/WireFrameParams.h>
#include <vapor/WireFrameRenderer.h>
#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>
#include <vapor/errorcodes.h>
#include <vapor/GetAppPath.h>
#include <vapor/ControlExecutive.h>
#include "vapor/debug.h"

using namespace VAPoR;

static RendererRegistrar<WireFrameRenderer> registrar(WireFrameRenderer::GetClassType(), WireFrameParams::GetClassType());

namespace {

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

};    // namespace

WireFrameRenderer::WireFrameRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, WireFrameParams::GetClassType(), WireFrameRenderer::GetClassType(), instName, dataMgr), _drawList(0)
{
}

WireFrameRenderer::~WireFrameRenderer()
{
    if (_drawList) glDeleteLists(_drawList, 3);
}

void WireFrameRenderer::_saveCacheParams()
{
    WireFrameParams *p = (WireFrameParams *)GetActiveParams();
    _cacheParams.varName = p->GetVariableName();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.level = p->GetRefinementLevel();
    _cacheParams.lod = p->GetCompressionLevel();
    _cacheParams.useSingleColor = p->UseSingleColor();
    p->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    tf->makeLut(_cacheParams.tf_lut);

    _cacheParams.tf_minmax = tf->getMinMaxMapValue();

    _cacheParams.constantColor = p->GetConstantColor();
    _cacheParams.constantOpacity = p->GetConstantOpacity();
}

bool WireFrameRenderer::_isCacheDirty() const
{
    WireFrameParams *p = (WireFrameParams *)GetActiveParams();
    if (_cacheParams.varName != p->GetVariableName()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.level != p->GetRefinementLevel()) return true;
    if (_cacheParams.lod != p->GetCompressionLevel()) return true;
    if (_cacheParams.useSingleColor != p->UseSingleColor()) return true;
    if (_cacheParams.constantColor != p->GetConstantColor()) return true;
    if (_cacheParams.constantOpacity != p->GetConstantOpacity()) return true;

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    vector<float>   tf_lut;
    tf->makeLut(tf_lut);
    if (_cacheParams.tf_lut != tf_lut) return (true);
    if (_cacheParams.tf_minmax != tf->getMinMaxMapValue()) return (true);

    vector<double> min, max;
    p->GetBox()->GetExtents(min, max);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

    return false;
}

void WireFrameRenderer::_drawCell(const vector<vector<size_t>> &nodes, const vector<size_t> &dims, bool layered) const
{
    GLuint *indices1 = new GLuint[nodes.size()];
    for (int i = 0; i < nodes.size(); i++) { indices1[i] = Wasp::LinearizeCoords(nodes[i], dims); }

    int count = layered ? nodes.size() / 2 : nodes.size();
    glDrawElements(GL_LINE_LOOP, count, GL_UNSIGNED_INT, indices1);

    if (!layered) {
        delete[] indices1;
        return;
    }

    // if layered the coordinates are ordered bottom face first, then top face
    //

    glDrawElements(GL_LINE_LOOP, count, GL_UNSIGNED_INT, indices1 + count);

    // Now draw edges between top and bottom face
    //
    GLuint *indices2 = new GLuint[nodes.size()];
    for (int i = 0; i < count; i++) {
        indices2[2 * i + 0] = indices1[i];
        indices2[2 * i + 1] = indices1[i + count];
    }
    glDrawElements(GL_LINES, 2 * count, GL_UNSIGNED_INT, indices2);

    delete[] indices1;
    delete[] indices2;
}

int WireFrameRenderer::_buildCache()
{
    WireFrameParams *rParams = (WireFrameParams *)GetActiveParams();
    _saveCacheParams();

    if (rParams->GetVariableName().empty()) { return 0; }

    Grid *grid = _dataMgr->GetVariable(_cacheParams.ts, _cacheParams.varName, _cacheParams.level, _cacheParams.lod, _cacheParams.boxMin, _cacheParams.boxMax);
    if (!grid) return (-1);

    Grid *heightGrid = NULL;
    if (!_cacheParams.heightVarName.empty()) {
        heightGrid = _dataMgr->GetVariable(_cacheParams.ts, _cacheParams.heightVarName, _cacheParams.level, _cacheParams.lod, _cacheParams.boxMin, _cacheParams.boxMax);
        if (!heightGrid) {
            delete grid;
            return (-1);
        }
    }

    glNewList(_drawList, GL_COMPILE);

    // Set up coordinate and color buffers
    //
    double mv = grid->GetMissingValue();

    size_t nVertsTotal = vproduct(grid->GetDimensions());

    float *coordsArray = new float[nVertsTotal * 3];
    float *colorsArray = new float[nVertsTotal * 4];

    Grid::ConstIterator itr = grid->cbegin();
    Grid::ConstIterator end_itr = grid->cend();

    Grid::ConstCoordItr coord_itr = grid->ConstCoordBegin();
    Grid::ConstCoordItr end_coord_itr = grid->ConstCoordEnd();

    float  defaultZ = _getDefaultZ(_dataMgr, _cacheParams.ts);
    size_t index = 0;
    for (; itr != end_itr && coord_itr != end_coord_itr; ++itr, ++coord_itr, ++index) {
        const vector<double> &coord = *coord_itr;

        coordsArray[3 * index + 0] = coord[0];
        coordsArray[3 * index + 1] = coord[1];

        if (coord.size() == 3) {
            coordsArray[3 * index + 2] = coord[2];
        } else if (heightGrid) {
            coordsArray[3 * index + 2] = heightGrid->GetValue(*coord_itr);
        } else {
            coordsArray[3 * index + 2] = defaultZ;
        }

        float dataValue = *itr;
        if (dataValue == mv) {
            colorsArray[4 * index + 0] = 0.0;
            colorsArray[4 * index + 1] = 0.0;
            colorsArray[4 * index + 2] = 0.0;
            colorsArray[4 * index + 3] = 0.0;
        } else if (_cacheParams.useSingleColor) {
            colorsArray[4 * index + 0] = _cacheParams.constantColor[0];
            colorsArray[4 * index + 1] = _cacheParams.constantColor[1];
            colorsArray[4 * index + 2] = _cacheParams.constantColor[2];
            colorsArray[4 * index + 3] = _cacheParams.constantOpacity;
        } else {
            size_t n = _cacheParams.tf_lut.size() >> 2;
            int    lut = (dataValue - _cacheParams.tf_minmax[0]) / (_cacheParams.tf_minmax[1] - _cacheParams.tf_minmax[0]) * (n - 1);
            if (lut < 0) { lut = 0; }
            if (lut >= n) { lut = n - 1; }
            colorsArray[4 * index + 0] = _cacheParams.tf_lut[4 * lut + 0];
            colorsArray[4 * index + 1] = _cacheParams.tf_lut[4 * lut + 1];
            colorsArray[4 * index + 2] = _cacheParams.tf_lut[4 * lut + 2];
            colorsArray[4 * index + 3] = _cacheParams.tf_lut[4 * lut + 3];
        }
    }

    if (grid->GetTopologyDim() == 3) {
        EnableClipToBox();
    } else {
        EnableClipToBox2DXY();
    }

    glVertexPointer(3, GL_FLOAT, 0, coordsArray);
    glColorPointer(4, GL_FLOAT, 0, colorsArray);

    glEndList();

    // Now draw the individual cells. Note: each shared cell edge
    // gets drawn twice. Oops
    //
    bool layered = grid->GetTopologyDim() == 3;

    // Create slow and fast draw lists with connectivity information
    //

    // Slow drawing display list
    //
    glNewList(_drawList + 1, GL_COMPILE);

    Grid::ConstCellIterator cell_itr = grid->ConstCellBegin();
    Grid::ConstCellIterator end_cell_itr = grid->ConstCellEnd();

    for (; cell_itr != end_cell_itr; ++cell_itr) {
        vector<vector<size_t>> nodes;
        grid->GetCellNodes(*cell_itr, nodes);

        _drawCell(nodes, grid->GetDimensions(), layered);
    }
    DisableClippingPlanes();

    glEndList();

    // Fast drawing display list
    //
    glNewList(_drawList + 2, GL_COMPILE);

    cell_itr = grid->ConstCellBegin();
    end_cell_itr = grid->ConstCellEnd();

    size_t count = 0;
    for (; cell_itr != end_cell_itr; ++cell_itr) {
        vector<vector<size_t>> nodes;
        grid->GetCellNodes(*cell_itr, nodes);

        if (count % 99 == 0) { _drawCell(nodes, grid->GetDimensions(), layered); }
        count++;
    }
    DisableClippingPlanes();

    glEndList();

    delete[] coordsArray;
    delete[] colorsArray;

    if (grid) delete grid;
    if (heightGrid) delete heightGrid;

    return 0;
}

int WireFrameRenderer::_paintGL(bool fast)
{
    int rc = 0;
    if (_isCacheDirty()) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        rc = _buildCache();

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }

    glCallList(_drawList);

    if (!fast) {
        glCallList(_drawList + 1);
    } else {
        // Fast draw display list
        //
        glCallList(_drawList + 2);
    }

    return rc;
}

int WireFrameRenderer::_initializeGL()
{
    // Three drawing list: one for vertex coordinates, one for vertex
    // connectivity during "slow" drawing, and one for vertex connectivity
    // during fast drawing
    //
    _drawList = glGenLists(3);
    assert(_drawList != 0);

    return 0;
}
