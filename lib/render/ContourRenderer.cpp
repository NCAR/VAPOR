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
//          Implementation of ContourRenderer
//

#include <sstream>
#include <string>
#include <iterator>

#include <vapor/glutil.h>    // Must be included first!!!

#include <vapor/Renderer.h>
#include <vapor/ContourRenderer.h>
#include <vapor/Visualizer.h>
//#include <vapor/params.h>
#include <vapor/ContourParams.h>
//#include <vapor/AnimationParams.h>
#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>
#include <vapor/errorcodes.h>
#include <vapor/GetAppPath.h>
#include <vapor/ControlExecutive.h>
#include "vapor/debug.h"

using namespace VAPoR;

static RendererRegistrar<ContourRenderer> registrar(ContourRenderer::GetClassType(), ContourParams::GetClassType());

ContourRenderer::ContourRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, ContourParams::GetClassType(), ContourRenderer::GetClassType(), instName, dataMgr), _drawList(0)
{
}

ContourRenderer::~ContourRenderer()
{
    if (_drawList) glDeleteLists(_drawList, 1);
}

void ContourRenderer::_saveCacheParams()
{
    ContourParams *p = (ContourParams *)GetActiveParams();
    _cacheParams.varName = p->GetVariableName();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.level = p->GetRefinementLevel();
    _cacheParams.lod = p->GetCompressionLevel();
    _cacheParams.useSingleColor = p->UseSingleColor();
    _cacheParams.lineThickness = p->GetLineThickness();
    p->GetConstantColor(_cacheParams.constantColor);
    p->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);
    _cacheParams.contourValues = p->GetContourValues(_cacheParams.varName);

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    _cacheParams.opacity = tf->getOpacityScale();
    _cacheParams.contourColors.resize(_cacheParams.contourValues.size() * 3);
    for (int i = 0; i < _cacheParams.contourValues.size(); i++) tf->rgbValue(_cacheParams.contourValues[i], &_cacheParams.contourColors[i * 3]);
}

bool ContourRenderer::_isCacheDirty() const
{
    ContourParams *p = (ContourParams *)GetActiveParams();
    if (_cacheParams.varName != p->GetVariableName()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.level != p->GetRefinementLevel()) return true;
    if (_cacheParams.lod != p->GetCompressionLevel()) return true;
    if (_cacheParams.useSingleColor != p->UseSingleColor()) return true;
    if (_cacheParams.lineThickness != p->GetLineThickness()) return true;

    vector<double> min, max, contourValues;
    p->GetBox()->GetExtents(min, max);
    contourValues = p->GetContourValues(_cacheParams.varName);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;
    if (_cacheParams.contourValues != contourValues) return true;

    float constantColor[3];
    p->GetConstantColor(constantColor);
    if (memcmp(_cacheParams.constantColor, constantColor, sizeof(constantColor))) return true;

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    vector<float>   contourColors(_cacheParams.contourValues.size() * 3);
    for (int i = 0; i < _cacheParams.contourValues.size(); i++) tf->rgbValue(_cacheParams.contourValues[i], &contourColors[i * 3]);
    if (_cacheParams.contourColors != contourColors) return true;
    if (_cacheParams.opacity != tf->getOpacityScale()) return true;

    return false;
}

int ContourRenderer::_buildCache()
{
    ContourParams *cParams = (ContourParams *)GetActiveParams();
    _saveCacheParams();

    glNewList(_drawList, GL_COMPILE);
    glLineWidth(_cacheParams.lineThickness);
    if (cParams->GetVariableName().empty()) {
        glEndList();
        return 0;
    }
    MapperFunction *tf = cParams->GetMapperFunc(_cacheParams.varName);
    vector<double>  contours = cParams->GetContourValues(_cacheParams.varName);
    float(*contourColors)[4] = new float[contours.size()][4];
    if (!_cacheParams.useSingleColor)
        for (int i = 0; i < contours.size(); i++) tf->rgbValue(contours[i], contourColors[i]);
    else
        for (int i = 0; i < contours.size(); i++) memcpy(contourColors[i], _cacheParams.constantColor, sizeof(_cacheParams.constantColor));
    for (int i = 0; i < contours.size(); i++) contourColors[i][3] = _cacheParams.opacity;

    Grid *grid = _dataMgr->GetVariable(_cacheParams.ts, _cacheParams.varName, _cacheParams.level, _cacheParams.lod, _cacheParams.boxMin, _cacheParams.boxMax);
    Grid *heightGrid = NULL;
    if (!_cacheParams.heightVarName.empty()) {
        heightGrid = _dataMgr->GetVariable(_cacheParams.ts, _cacheParams.heightVarName, _cacheParams.level, _cacheParams.lod, _cacheParams.boxMin, _cacheParams.boxMax);
    }
    // StructuredGrid *sGrid = dynamic_cast<StructuredGrid *>(grid);

    if (grid == NULL || (heightGrid == NULL && !_cacheParams.heightVarName.empty())) {
        glEndList();
        return -1;
    }

    double mv = grid->GetMissingValue();

    Grid::ConstCellIterator it = grid->ConstCellBegin(_cacheParams.boxMin, _cacheParams.boxMax);

    Grid::ConstCellIterator end = grid->ConstCellEnd();
    for (; it != end; ++it) {
        vector<size_t>         cell = *it;
        vector<vector<size_t>> nodes;
        grid->GetCellNodes(cell, nodes);

        vector<double> *coords = new vector<double>[nodes.size()];
        float *         values = new float[nodes.size()];
        bool            hasMissing = false;
        for (int i = 0; i < nodes.size(); i++) {
            grid->GetUserCoordinates(nodes[i], coords[i]);
            values[i] = grid->GetValue(coords[i]);
            if (values[i] == mv) { hasMissing = true; }
        }
        if (hasMissing) continue;

        glBegin(GL_LINES);

        for (int ci = 0; ci != contours.size(); ci++) {
            glColor4fv(contourColors[ci]);
            for (int a = nodes.size() - 1, b = 0; b < nodes.size(); a++, b++) {
                if (a == nodes.size()) a = 0;
                double contour = contours[ci];

                if ((values[a] <= contour && values[b] <= contour) || (values[a] > contour && values[b] > contour)) continue;

                float t = (contour - values[a]) / (values[b] - values[a]);
                float v[3];
                v[0] = coords[a][0] + t * (coords[b][0] - coords[a][0]);
                v[1] = coords[a][1] + t * (coords[b][1] - coords[a][1]);
                v[2] = 0;

                if (heightGrid) {
                    float aHeight = heightGrid->GetValue(coords[a]);
                    float bHeight = heightGrid->GetValue(coords[b]);
                    v[2] = aHeight + t * (bHeight - aHeight);
                }

                glVertex3fv(v);
            }
        }
        glEnd();
        delete[] coords;
        delete[] values;
    }

    glEndList();
    delete[] contourColors;
    return 0;
}

int ContourRenderer::_paintGL()
{
    int rc = 0;
    if (_isCacheDirty()) rc = _buildCache();

    glCallList(_drawList);

    return rc;
}

int ContourRenderer::_initializeGL()
{
    _drawList = glGenLists(1);
    return 0;
}
