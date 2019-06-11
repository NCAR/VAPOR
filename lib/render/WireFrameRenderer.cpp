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
#include <vapor/ControlExecutive.h>
#include "vapor/GLManager.h"
#include "vapor/debug.h"

using namespace VAPoR;

#pragma pack(push, 4)
struct WireFrameRenderer::VertexData {
    float x, y, z;
    float r, g, b, a;
};
#pragma pack(pop)

static RendererRegistrar<WireFrameRenderer> registrar(WireFrameRenderer::GetClassType(), WireFrameParams::GetClassType());

WireFrameRenderer::WireFrameRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, WireFrameParams::GetClassType(), WireFrameRenderer::GetClassType(), instName, dataMgr), _VAO(0), _VBO(0), _EBO(0)
{
}

WireFrameRenderer::~WireFrameRenderer()
{
    if (_VAO) glDeleteVertexArrays(1, &_VAO);
    if (_VBO) glDeleteBuffers(1, &_VBO);
    if (_EBO) glDeleteBuffers(1, &_EBO);
    _VAO = _VBO = _EBO = 0;
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

void WireFrameRenderer::_drawCell(vector<VertexData> &vertices, vector<unsigned int> &indices, const float *verts, const float *colors, int n, bool layered)
{
    int baseIndex = vertices.size();
    for (int i = 0; i < n; i++) { vertices.push_back({verts[3 * i], verts[3 * i + 1], verts[3 * i + 2], colors[4 * i], colors[4 * i + 1], colors[4 * i + 2], colors[4 * i + 3]}); }

    int count = layered ? n / 2 : n;
    for (int i = 0; i < count; i++) {
        // indices[i] = i;
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + ((i + 1) % count));
    }

    if (!layered) return;

    // if layered the coordinates are ordered bottom face first, then top face
    //
    for (int i = 0; i < count; i++) {
        // indices[i] = i + count;
        indices.push_back(baseIndex + i + count);
        indices.push_back(baseIndex + ((i + 1) % count) + count);
    }

    // Now draw edges between top and bottom face
    //
    for (int i = 0; i < count; i++) {
        // indices[2*i+0] = i;
        // indices[2*i+1] = i + count;
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + i + count);
    }
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

    vector<VertexData>   vertices;
    vector<unsigned int> indices;

    double mv = grid->GetMissingValue();

#if 0    // VAPOR_3_1_0
    
    // Performance of the bounding box version of the Cell iterator
    // is too slow. So we render the entire grid and use clipping planes
    // to restrict the displayed region
    //
    Grid::ConstCellIterator it = grid->ConstCellBegin(
                                                      _cacheParams.boxMin, _cacheParams.boxMax
                                                      );
#else
    Grid::ConstCellIterator it = grid->ConstCellBegin();
#endif

    size_t nverts = grid->GetMaxVertexPerCell();
    bool   layered = grid->GetTopologyDim() == 3;

    float *                 coordsArray = new float[nverts * 3];
    float *                 colorsArray = new float[nverts * 4];
    Grid::ConstCellIterator end = grid->ConstCellEnd();

    float defaultZ = _getDefaultZ(_dataMgr, _cacheParams.ts);

    size_t  maxNodes = grid->GetMaxVertexPerCell();
    size_t  nodeDim = grid->GetNodeDimensions().size();
    size_t *nodes = (size_t *)alloca(sizeof(size_t) * maxNodes * nodeDim);

    size_t  coordDim = grid->GetGeometryDim();
    double *coord = (double *)alloca(sizeof(double) * coordDim);

    for (; it != end; ++it) {
        int numNodes;
        grid->GetCellNodes((*it).data(), nodes, numNodes);

        for (int i = 0; i < numNodes; i++) {
            grid->GetUserCoordinates(&nodes[i * nodeDim], coord);

            coordsArray[3 * i + 0] = coord[0];
            coordsArray[3 * i + 1] = coord[1];

            if (coordDim == 3) {
                coordsArray[3 * i + 2] = coord[2];
            } else if (heightGrid) {
                coordsArray[3 * i + 2] = heightGrid->GetValueAtIndex(&nodes[i * nodeDim]);
            } else {
                coordsArray[3 * i + 2] = defaultZ;
            }

            float dataValue = grid->GetValueAtIndex(&nodes[i * nodeDim]);
            if (dataValue == mv) {
                colorsArray[4 * i + 0] = 0.0;
                colorsArray[4 * i + 1] = 0.0;
                colorsArray[4 * i + 2] = 0.0;
                colorsArray[4 * i + 3] = 0.0;
            } else if (_cacheParams.useSingleColor) {
                colorsArray[4 * i + 0] = _cacheParams.constantColor[0];
                colorsArray[4 * i + 1] = _cacheParams.constantColor[1];
                colorsArray[4 * i + 2] = _cacheParams.constantColor[2];
                colorsArray[4 * i + 3] = _cacheParams.constantOpacity;
            } else {
                size_t n = _cacheParams.tf_lut.size() >> 2;
                int    index = (dataValue - _cacheParams.tf_minmax[0]) / (_cacheParams.tf_minmax[1] - _cacheParams.tf_minmax[0]) * (n - 1);
                if (index < 0) { index = 0; }
                if (index >= n) { index = n - 1; }
                colorsArray[4 * i + 0] = _cacheParams.tf_lut[4 * index + 0];
                colorsArray[4 * i + 1] = _cacheParams.tf_lut[4 * index + 1];
                colorsArray[4 * i + 2] = _cacheParams.tf_lut[4 * index + 2];
                colorsArray[4 * i + 3] = _cacheParams.tf_lut[4 * index + 3];
            }
        }

        _drawCell(vertices, indices, coordsArray, colorsArray, numNodes, layered);
    }

    delete[] coordsArray;
    delete[] colorsArray;

    if (grid) delete grid;
    if (heightGrid) delete heightGrid;

    _nIndices = indices.size();
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return 0;
}

int WireFrameRenderer::_paintGL(bool fast)
{
    int rc = 0;
    if (_isCacheDirty()) rc = _buildCache();

    SmartShaderProgram shader = _glManager->shaderManager->GetSmartShader("Wireframe");
    if (!shader.IsValid()) return -1;

    EnableClipToBox(_glManager->shaderManager->GetShader("Wireframe"));
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    glBindVertexArray(_VAO);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glDrawElements(GL_LINES, _nIndices, GL_UNSIGNED_INT, 0);

    DisableClippingPlanes();
    glBindVertexArray(0);

    return rc;
}

int WireFrameRenderer::_initializeGL()
{
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_EBO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(struct VertexData, r));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return 0;
}
