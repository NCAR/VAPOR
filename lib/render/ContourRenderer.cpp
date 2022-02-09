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
#include <vapor/ContourParams.h>
#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>
#include <vapor/errorcodes.h>
#include <vapor/ControlExecutive.h>
#include <vapor/ShaderManager.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vapor/GLManager.h>
#include <vapor/LegacyGL.h>
#include <vapor/ArbitrarilyOrientedRegularGrid.h>

using namespace VAPoR;

#pragma pack(push, 4)
struct ContourRenderer::VertexData {
    float x, y, z;
    float v;
};
#pragma pack(pop)

static RendererRegistrar<ContourRenderer> registrar(ContourRenderer::GetClassType(), ContourParams::GetClassType());

ContourRenderer::ContourRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, ContourParams::GetClassType(), ContourRenderer::GetClassType(), instName, dataMgr), _VAO(0), _VBO(0), _nVertices(0)
{
}

ContourRenderer::~ContourRenderer()
{
    if (_VAO) glDeleteVertexArrays(1, &_VAO);
    if (_VBO) glDeleteBuffers(1, &_VBO);
    _VAO = _VBO = 0;
}

void ContourRenderer::_saveCacheParams()
{
    ContourParams *p = (ContourParams *)GetActiveParams();
    _cacheParams.varName = p->GetVariableName();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.level = p->GetRefinementLevel();
    _cacheParams.lod = p->GetCompressionLevel();
    _cacheParams.lineThickness = p->GetLineThickness();
    p->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);
    _cacheParams.contourValues = p->GetContourValues(_cacheParams.varName);
    _cacheParams.sliceRotation = p->GetSlicePlaneRotation();
    _cacheParams.sliceNormal = p->GetSlicePlaneNormal();
    _cacheParams.sliceOrigin = p->GetSlicePlaneOrigin();
    _cacheParams.sliceOffset = p->GetValueDouble(p->SliceOffsetTag, 0);
    _cacheParams.sliceResolution = p->GetValueDouble(RenderParams::SampleRateTag, 200);
    _cacheParams.sliceOrientationMode = p->GetValueLong(RenderParams::SlicePlaneOrientationModeTag, 0);
}

bool ContourRenderer::_isCacheDirty() const
{
    ContourParams *p = (ContourParams *)GetActiveParams();
    if (_cacheParams.varName != p->GetVariableName()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.level != p->GetRefinementLevel()) return true;
    if (_cacheParams.lod != p->GetCompressionLevel()) return true;
    if (_cacheParams.lineThickness != p->GetLineThickness()) return true;
    if (_cacheParams.sliceRotation != p->GetSlicePlaneRotation()) return true;
    if (_cacheParams.sliceNormal != p->GetSlicePlaneNormal()) return true;
    if (_cacheParams.sliceOrigin != p->GetSlicePlaneOrigin()) return true;
    if (_cacheParams.sliceOffset != p->GetValueDouble(p->SliceOffsetTag, 0)) return true;
    if (_cacheParams.sliceResolution != p->GetValueDouble(RenderParams::SampleRateTag, 200)) return true;
    if (_cacheParams.sliceOrientationMode != p->GetValueLong(RenderParams::SlicePlaneOrientationModeTag, 0)) return true;

    vector<double> min, max, contourValues;
    p->GetBox()->GetExtents(min, max);
    contourValues = p->GetContourValues(_cacheParams.varName);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;
    if (_cacheParams.contourValues != contourValues) return true;

    return false;
}

static CoordType ToCoordType(const vector<double> &v)
{
    CoordType c;
    for (int i = 0; i < std::min(c.size(), v.size()); i++) c[i] = v[i];
    return c;
}

static glm::vec3 ToVec3(const vector<double> &v)
{
    glm::vec3 c;
    for (int i = 0; i < std::min(c.length(), (int)v.size()); i++) c[i] = v[i];
    return c;
}

static vector<double> ToDoubleVec(const glm::vec3 &v)
{
    vector<double> c(v.length());
    for (int i = 0; i < v.length(); i++) c[i] = v[i];
    return c;
}


int ContourRenderer::_buildCache(bool fast)
{
    ContourParams *cParams = (ContourParams *)GetActiveParams();
    _saveCacheParams();

    vector<VertexData> vertices;

    if (cParams->GetVariableName().empty()) {
        MyBase::SetErrMsg("Missing Variable");
        return 1;
    }
    vector<double> contours = cParams->GetContourValues(_cacheParams.varName);

    CoordType boxMin = {0.0, 0.0, 0.0};
    CoordType boxMax = {0.0, 0.0, 0.0};
    Grid::CopyToArr3(_cacheParams.boxMin, boxMin);
    Grid::CopyToArr3(_cacheParams.boxMax, boxMax);

    Grid *grid = _dataMgr->GetVariable(_cacheParams.ts, _cacheParams.varName, _cacheParams.level, _cacheParams.lod, boxMin, boxMax);
    Grid *grid2 = nullptr;
    Grid *heightGrid = nullptr;
    _sliceQuad.clear();

    int dims = grid->GetGeometryDim();
    if (!_cacheParams.heightVarName.empty() && dims == 2) { heightGrid = _dataMgr->GetVariable(_cacheParams.ts, _cacheParams.heightVarName, _cacheParams.level, _cacheParams.lod, boxMin, boxMax); }
    if (grid == NULL || (heightGrid == NULL && !_cacheParams.heightVarName.empty())) {
        if (grid) delete grid;
        if (grid2) delete grid2;
        if (heightGrid) delete heightGrid;
        return -1;
    }

    if (dims == 3) {
        planeDescription pd;
        pd.boxMin = ToCoordType(_cacheParams.boxMin);
        pd.boxMax = ToCoordType(_cacheParams.boxMax);
        pd.origin = _cacheParams.sliceOrigin;
        pd.sideSize = _cacheParams.sliceResolution;
        if (_cacheParams.sliceOrientationMode == (int)RenderParams::SlicePlaneOrientationMode::Normal)
            pd.normal = _cacheParams.sliceNormal;
        else
            pd.normal = ToDoubleVec(ArbitrarilyOrientedRegularGrid::GetNormalFromRotations(_cacheParams.sliceRotation));

        auto o = ToVec3(_cacheParams.sliceOrigin);
        auto n = ToVec3(pd.normal);
        auto offsetOrigin = o + n * (float)_cacheParams.sliceOffset;
        pd.origin = ToDoubleVec(offsetOrigin);
        _finalOrigin = offsetOrigin;

        DimsType dims = {pd.sideSize, pd.sideSize, 1};

        ArbitrarilyOrientedRegularGrid *grid2d = new ArbitrarilyOrientedRegularGrid(grid, pd, dims);
        grid2 = grid;
        grid = grid2d;

        CoordType corner1, corner2, corner3, corner4;
        grid2d->GetUserCoordinates({0, 0, 0}, corner1);
        grid2d->GetUserCoordinates({0, pd.sideSize - 1, 0}, corner2);
        grid2d->GetUserCoordinates({pd.sideSize - 1, 0, 0}, corner3);
        grid2d->GetUserCoordinates({pd.sideSize - 1, pd.sideSize - 1, 0}, corner4);

        _sliceQuad = {
            glm::vec3(corner1[0], corner1[1], corner1[2]),
            glm::vec3(corner3[0], corner3[1], corner3[2]),
            glm::vec3(corner4[0], corner4[1], corner4[2]),
            glm::vec3(corner2[0], corner2[1], corner2[2]),
        };
        if (fast) {
            _cacheParams.varName = "";
            if (grid) delete grid;
            if (grid2) delete grid2;
            if (heightGrid) delete heightGrid;
            return 0;
        }
    }

    double mv = grid->GetMissingValue();
    float  Z0 = GetDefaultZ(_dataMgr, _cacheParams.ts);

    Grid::ConstCellIterator it = grid->ConstCellBegin(boxMin, boxMax);

    size_t           maxNodes = grid->GetMaxVertexPerCell();
    vector<DimsType> nodes(maxNodes);

    vector<float>     values(maxNodes);
    vector<CoordType> coords(maxNodes);

    Grid::ConstCellIterator end = grid->ConstCellEnd();
    for (; it != end; ++it) {
        const DimsType &cell = *it;
        grid->GetCellNodes(cell, nodes);

        bool hasMissing = false;
        for (int i = 0; i < nodes.size(); i++) {
            grid->GetUserCoordinates(nodes[i], coords[i]);
            values[i] = grid->GetValueAtIndex(nodes[i]);
            if (values[i] == mv) { hasMissing = true; }
        }
        if (hasMissing) continue;

        for (int ci = 0; ci != contours.size(); ci++) {
            for (int a = nodes.size() - 1, b = 0; b < nodes.size(); a++, b++) {
                if (a == nodes.size()) a = 0;
                float contour = contours[ci];

                if ((values[a] <= contour && values[b] <= contour) || (values[a] > contour && values[b] > contour)) continue;

                float t = (contour - values[a]) / (values[b] - values[a]);
                float v[3];
                v[0] = coords[a][0] + t * (coords[b][0] - coords[a][0]);
                v[1] = coords[a][1] + t * (coords[b][1] - coords[a][1]);
                v[2] = coords[a][2] + t * (coords[b][2] - coords[a][2]);

                if (dims == 2) v[2] = Z0;

                if (heightGrid) {
                    float aHeight = heightGrid->GetValueAtIndex(nodes[a]);
                    float bHeight = heightGrid->GetValueAtIndex(nodes[b]);
                    v[2] = aHeight + t * (bHeight - aHeight);
                }

                vertices.push_back({v[0], v[1], v[2], contour});
            }
        }
    }

    _nVertices = vertices.size();
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (grid) delete grid;
    if (grid2) delete grid2;
    if (heightGrid) delete heightGrid;

    return 0;
}

using glm::vec3;

void ContourRenderer::_outlineSliceQuad() const
{
    LegacyGL *lgl = _glManager->legacy;
    lgl->Color3f(0, 1, 0);
    lgl->Begin(GL_LINE_STRIP);
    for (auto v : _sliceQuad) lgl->Vertex(v);
    if (_sliceQuad.size()) lgl->Vertex(_sliceQuad[0]);
    lgl->End();
}

int ContourRenderer::_paintGL(bool fast)
{
    int rc = 0;
    if (_isCacheDirty()) {
        rc = _buildCache(fast);
        if (fast) {
            _outlineSliceQuad();
            return 0;
        }
    }
    if (rc != 0) return rc;

    RenderParams *  rp = GetActiveParams();
    MapperFunction *tf = rp->GetMapperFunc(rp->GetVariableName());
    float           lut[4 * 256];
    tf->makeLut(lut);
    if (rp->UseSingleColor()) {
        float c[3];
        rp->GetConstantColor(c);
        for (int i = 0; i < 256; i++) {
            lut[i * 4 + 0] = c[0];
            lut[i * 4 + 1] = c[1];
            lut[i * 4 + 2] = c[2];
        }
    }
    _lutTexture.TexImage(GL_RGBA8, 256, 0, 0, GL_RGBA, GL_FLOAT, lut);

    ShaderProgram *shader = _glManager->shaderManager->GetShader("Contour");
    if (shader == nullptr) return -1;
    shader->Bind();
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("minLUTValue", tf->getMinMapValue());
    shader->SetUniform("maxLUTValue", tf->getMaxMapValue());
    shader->SetSampler("colormap", _lutTexture);

    // glLineWidth(_cacheParams.lineThickness);
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(_VAO);
    glDrawArrays(GL_LINES, 0, _nVertices);

    glBindVertexArray(0);
    shader->UnBind();

    return rc;
}

int ContourRenderer::_initializeGL()
{
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);
    glGenBuffers(1, &_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(struct VertexData, v));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    _lutTexture.Generate();

    return 0;
}
