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

//
// Generate wireframe line drawing list of line segments for a single
// cell. Make use of drawList to avoid drawing line segments shared
// by multiple cells
//
void WireFrameRenderer::_drawCell(const GLuint *cellNodeIndices, int n, bool layered, const vector<GLuint> &nodeMap, GLuint invalidIndex, vector<unsigned int> &indices, DrawList &drawList) const
{
    int count = layered ? n / 2 : n;
    for (int i = 0; i < count; i++) {
        GLuint idx0 = nodeMap[cellNodeIndices[i]];
        VAssert(idx0 != invalidIndex);

        GLuint idx1 = nodeMap[cellNodeIndices[(i + 1) % count]];
        VAssert(idx1 != invalidIndex);

        // Don't draw line segment if it's already been drawn
        //
        if (drawList.InList(idx0, idx1)) continue;

        indices.push_back(idx0);
        indices.push_back(idx1);
    }

    if (!layered) return;

    // if layered the coordinates are ordered bottom face first, then top face
    //
    for (int i = 0; i < count; i++) {
        GLuint idx0 = nodeMap[cellNodeIndices[i + count]];
        VAssert(idx0 != invalidIndex);

        GLuint idx1 = nodeMap[cellNodeIndices[((i + 1) % count) + count]];
        VAssert(idx1 != invalidIndex);

        if (drawList.InList(idx0, idx1)) continue;

        indices.push_back(idx0);
        indices.push_back(idx1);
    }

    // Now draw edges between top and bottom face
    //
    for (int i = 0; i < count; i++) {
        GLuint idx0 = nodeMap[cellNodeIndices[i]];
        VAssert(idx0 != invalidIndex);

        GLuint idx1 = nodeMap[cellNodeIndices[i + count]];
        VAssert(idx1 != invalidIndex);

        if (drawList.InList(idx0, idx1)) continue;

        indices.push_back(idx0);
        indices.push_back(idx1);
    }
}

// Generate list of vertices shared by all line segments, and populate
// 'nodeMap': a map from a node's Grid index to its offset in the
// list of vertices.
//
void WireFrameRenderer::_buildCacheVertices(const Grid *grid, const Grid *heightGrid, vector<GLuint> &nodeMap) const
{
    double mv = grid->GetMissingValue();
    float  defaultZ = _getDefaultZ(_dataMgr, _cacheParams.ts);
    size_t numNodes = Wasp::VProduct(grid->GetDimensions());

    // Pre-allocate vertices vector upfront for better performance
    //
    vector<VertexData> vertices;
    vertices.reserve(numNodes);

    // Visit each grid node. For each node store node's coordinates and
    // assigned color in 'vertices'. Create 'nodeMap': mapping from a
    // grid node's index to its offset in 'vertices'
    //
    Grid::ConstNodeIterator nodeItr = grid->ConstNodeBegin();
    Grid::ConstNodeIterator nodeEnd = grid->ConstNodeEnd();
    Grid::ConstCoordItr     coordItr = grid->ConstCoordBegin();
    Grid::ConstCoordItr     coordEnd = grid->ConstCoordEnd();
    for (; nodeItr != nodeEnd; ++nodeItr, ++coordItr) {
        // Get current node's coordinates
        //
        double coord[3];
        coord[0] = (*coordItr)[0];
        coord[1] = (*coordItr)[1];

        if (grid->GetGeometryDim() > 2) {
            coord[2] = (*coordItr)[2];
        } else {
            if (heightGrid) {
                coord[2] = heightGrid->GetValueAtIndex(*nodeItr);
            } else {
                coord[2] = defaultZ;
            }
        }

        // Get current node's color
        //
        float dataValue = grid->GetValueAtIndex((*nodeItr).data());
        float color[4];

        if (dataValue == mv) {
            color[0] = 0.0;
            color[1] = 0.0;
            color[2] = 0.0;
            color[3] = 0.0;
        } else if (_cacheParams.useSingleColor) {
            color[0] = _cacheParams.constantColor[0];
            color[1] = _cacheParams.constantColor[1];
            color[2] = _cacheParams.constantColor[2];
            color[3] = _cacheParams.constantOpacity;
        } else {
            size_t n = _cacheParams.tf_lut.size() >> 2;
            int    index = (dataValue - _cacheParams.tf_minmax[0]) / (_cacheParams.tf_minmax[1] - _cacheParams.tf_minmax[0]) * (n - 1);
            if (index < 0) { index = 0; }
            if (index >= n) { index = n - 1; }
            color[0] = _cacheParams.tf_lut[4 * index + 0];
            color[1] = _cacheParams.tf_lut[4 * index + 1];
            color[2] = _cacheParams.tf_lut[4 * index + 2];
            color[3] = _cacheParams.tf_lut[4 * index + 3];
        }

        // Create an entry in nodeMap
        //
        size_t index = Wasp::LinearizeCoords(*nodeItr, grid->GetDimensions());

        if (vertices.size() > std::numeric_limits<GLuint>::max()) {
#ifndef NDEBUG
            MyBase::SetDiagMsg("WireFrameRenderer() : exceeded numeric limits");
#endif
            continue;
        }
        nodeMap[index] = vertices.size();

        vertices.push_back({(float)coord[0], (float)coord[1], (float)coord[2], color[0], color[1], color[2], color[3]});
    }

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_DYNAMIC_DRAW);
}

//
// Generate connectivity list for line segments joining cell nodes.
//
size_t WireFrameRenderer::_buildCacheConnectivity(const Grid *grid, const vector<GLuint> &nodeMap) const
{
    size_t         invalidIndex = std::numeric_limits<size_t>::max();
    size_t         numNodes = Wasp::VProduct(grid->GetDimensions());
    bool           layered = grid->GetTopologyDim() == 3;
    size_t         maxVertsPerCell = grid->GetMaxVertexPerCell();
    vector<size_t> cellNodeIndices(maxVertsPerCell * grid->GetDimensions().size());
    vector<GLuint> cellNodeIndicesLinear(maxVertsPerCell);

    size_t numCells = Wasp::VProduct(grid->GetCellDimensions());
    size_t maxLineIndices = numCells * (layered ? maxVertsPerCell / 2 * 3 : maxVertsPerCell * 2);

    // Pre-allocate memory for (much) better performance.
    //
    vector<unsigned int> indices;
    indices.reserve(maxLineIndices);

    {
        // drawList keeps track of line segments that have already been
        // drawn so that we can avoid duplicates. Avoiding duplicates
        // reduces the memory requirements substantially
        //
        DrawList drawList(numNodes, 10);

        // Loop over cells, drawing edges of each cell
        //
        Grid::ConstCellIterator cellItr = grid->ConstCellBegin();
        Grid::ConstCellIterator cellEnd = grid->ConstCellEnd();
        for (; cellItr != cellEnd; ++cellItr) {
            int numNodes;
            grid->GetCellNodes((*cellItr).data(), cellNodeIndices.data(), numNodes);

            for (int i = 0; i < numNodes; i++) {
                int    ndim = grid->GetDimensions().size();
                size_t idx = Wasp::LinearizeCoords(cellNodeIndices.data() + (i * ndim), grid->GetDimensions().data(), ndim);

                if (idx > std::numeric_limits<GLuint>::max()) {
#ifndef NDEBUG
                    MyBase::SetDiagMsg("WireFrameRenderer() : exceeded numeric limits");
#endif
                    continue;
                }
                cellNodeIndicesLinear[i] = idx;
            }

            _drawCell(cellNodeIndicesLinear.data(), numNodes, layered, nodeMap, invalidIndex, indices, drawList);
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return (indices.size());
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

    size_t         numNodes = Wasp::VProduct(grid->GetDimensions());
    GLuint         invalidIndex = std::numeric_limits<GLuint>::max();
    vector<GLuint> nodeMap(numNodes, invalidIndex);

    _buildCacheVertices(grid, heightGrid, nodeMap);

    _nIndices = _buildCacheConnectivity(grid, nodeMap);

    if (grid) delete grid;
    if (heightGrid) delete heightGrid;
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
