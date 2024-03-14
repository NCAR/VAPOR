//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   ParticleRenderer.cpp
//
//  Author: Stas Jaroszynski
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   March 2018
//
//  Description:
//          Implementation of ParticleRenderer
//

#include <sstream>
#include <string>
#include <iterator>

#include <vapor/glutil.h>    // Must be included first!!!

#include <vapor/Renderer.h>
#include <vapor/ParticleRenderer.h>
#include <vapor/ParticleParams.h>
#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/errorcodes.h>
#include <vapor/ControlExecutive.h>
#include "vapor/ShaderManager.h"
#include "vapor/debug.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "vapor/GLManager.h"
#include <vapor/LegacyGL.h>

using namespace VAPoR;

#pragma pack(push, 4)
// struct ParticleRenderer::VertexData {
//    float x, y, z;
//    float v;
//};
#pragma pack(pop)

using glm::vec3;
static inline vec3 CoordTypeToVec3(const CoordType &c) { return vec3(c[0], c[1], c[2]); }

static RendererRegistrar<ParticleRenderer> registrar(ParticleRenderer::GetClassType(), ParticleParams::GetClassType());

ParticleRenderer::ParticleRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, ParticleParams::GetClassType(), ParticleRenderer::GetClassType(), instName, dataMgr),
  _colorMapTexOffset(0)
{
    _cacheParams.ts=0;
    _cacheParams.rLevel=0;
    _cacheParams.cLevel=0;
    _cacheParams.stride=0;
    _cacheParams.radius=8.0;
    _cacheParams.directionScale=0.;
    _cacheParams.direction=false;
}

ParticleRenderer::~ParticleRenderer() {
    if (_vertexArrayId) {
        glDeleteVertexArrays(1, &_vertexArrayId);
        _vertexArrayId = 0;
    }
    if (_vertexBufferId) {
        glDeleteBuffers(1, &_vertexBufferId);
        _vertexBufferId = 0;
    }

    if (_colorMapTexId) {
        glDeleteTextures(1, &_colorMapTexId);
        _colorMapTexId = 0;
    }
}

int ParticleRenderer::_paintGL(bool)
{
#ifdef DEBUG
    auto start = chrono::steady_clock::now();
#endif

    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

    bool regenerateParticles = false;
    if (_particleCacheIsDirty()) {
        _resetParticleCache();
        regenerateParticles = true;
    }

    if (_colormapCacheIsDirty()) {
        _resetColormapCache();
        _prepareColormap();
    }

    Grid* grid = nullptr;
    std::vector<Grid*> vecGrids;
    int rc = _getGrids(grid, vecGrids);
    if (rc != 0) {
        SetErrMsg("Could not get scalar and field grids for ParticleRenderer");
        return rc;
    }

    bool renderLegacy = GetActiveParams()->GetValueLong(ParticleParams::RenderLegacyTag, false);
    if (renderLegacy) {
        _renderParticlesLegacy(grid, vecGrids);
    }
    else {
        if (regenerateParticles) {
            _generateTextureData(grid, vecGrids);
            _computeBaseRadius();
        }
        _renderParticlesHelper();
    }

    _dataMgr->UnlockGrid(grid);
    delete grid;
    for (auto g : vecGrids) {
        _dataMgr->UnlockGrid(g);
        delete g;
    }

#ifdef DEBUG
    auto end = chrono::steady_clock::now();
    cout << "Glyph time in milliseconds: "
        << chrono::duration_cast<chrono::milliseconds>(end - start).count()
        << " ms" << endl;
#endif
    return 0;
}

int ParticleRenderer::_initializeGL() { 
    glGenVertexArrays(1, &_vertexArrayId);
    glGenBuffers(1, &_vertexBufferId);

    /* Generate and configure 1D texture: _colorMapTexId */
    glGenTextures(1, &_colorMapTexId);
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_1D, 0);

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);

    assert(_VAO);
    assert(_VBO);

    return 0;
}

static void SetupParticlePointGL(const int VAO, const int VBO, const bool dynamicSize) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    int elSize = sizeof(glm::vec4);
    if (dynamicSize) elSize += sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, elSize, NULL);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, elSize, (void *)sizeof(glm::vec3));
    if (dynamicSize)
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, elSize, (void *)sizeof(glm::vec4));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    if (dynamicSize)
        glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void SetupParticleDirectionGL(const int VAO, const int VBO, const bool dynamicSize) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    int elSize = sizeof(float)*7;
    if (dynamicSize) elSize += sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, elSize, NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, elSize, (void *)sizeof(glm::vec3));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, elSize, (void *)(2*sizeof(glm::vec3)));
    if (dynamicSize)
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, elSize, (void *)(sizeof(float)*7));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    if (dynamicSize)
        glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool ParticleRenderer::_particleCacheIsDirty() const {
    auto p = GetActiveParams();

    if (_cacheParams.ts        != p->GetCurrentTimestep()    ) return true;
    if (_cacheParams.rLevel    != p->GetRefinementLevel()    ) return true;
    if (_cacheParams.cLevel    != p->GetCompressionLevel()   ) return true;

    VAPoR::CoordType min, max;
    p->GetBox()->GetExtents(min, max);
    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

    if (_cacheParams.stride    != p->GetValueLong( ParticleParams::StrideTag, 1)) return true;
    if (_cacheParams.varName   != p->GetVariableName()       ) return true;
    if (_cacheParams.fieldVars != p->GetFieldVariableNames() ) return true;
    if (_cacheParams.radiusVarName != p->GetValueString( ParticleParams::RenderRadiusVariableTag, "")) return true;
    if (_cacheParams.direction != (bool)p->GetValueLong( ParticleParams::ShowDirectionTag, false)) return true;
    if (_cacheParams.directionScale != p->GetValueDouble( ParticleParams::DirectionScaleTag, false)) return true;

    return false;
}

void ParticleRenderer::_resetParticleCache() {
    auto p = GetActiveParams();
    _cacheParams.ts      = p->GetCurrentTimestep();
    _cacheParams.rLevel  = p->GetRefinementLevel();
    _cacheParams.cLevel  = p->GetCompressionLevel();

    VAPoR::CoordType min, max;
    p->GetBox()->GetExtents(min, max);
    _cacheParams.boxMin = min;
    _cacheParams.boxMax = max;
   
    _cacheParams.direction = (bool)p->GetValueLong(ParticleParams::ShowDirectionTag, false); 
    _cacheParams.directionScale = p->GetValueDouble(ParticleParams::DirectionScaleTag, 1.);
    _cacheParams.stride = p->GetValueLong(ParticleParams::StrideTag, 1);
    _cacheParams.varName = p->GetVariableName();
    _cacheParams.fieldVars = p->GetFieldVariableNames();
    _cacheParams.radiusVarName = p->GetValueString( ParticleParams::RenderRadiusVariableTag, "");
}

bool ParticleRenderer::_colormapCacheIsDirty() const {
    auto p = GetActiveParams();
    std::vector<float> tf_lut;
    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    tf->makeLut(tf_lut);
    if (_cacheParams.tf_lut    != tf_lut)                  return true;
    if (_cacheParams.tf_minMax != tf->getMinMaxMapValue()) return true;
    return false;
}

void ParticleRenderer::_resetColormapCache() {
    auto p = GetActiveParams();
    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    std::vector<float> tf_lut;
    tf->makeLut(tf_lut);
    std::vector<double> minMax = tf->getMinMaxMapValue();
    _cacheParams.tf_lut    = tf_lut;
    _cacheParams.tf_minMax = minMax;
}

int ParticleRenderer::_renderParticlesHelper()
{
    auto rp = GetActiveParams();
    float radiusBase = rp->GetValueDouble(ParticleParams::RenderRadiusBaseTag, -1);
    float radius = radiusBase * rp->GetValueDouble(ParticleParams::RenderRadiusScalarTag, 1.);

    string shaderKey;
    if (_cacheParams.direction)
        shaderKey = "ParticleDirection";
    else
        shaderKey = "ParticlePoint";

    if (!_cacheParams.radiusVarName.empty())
        shaderKey += ":DYNAMIC_RADIUS";

    ShaderProgram *shader = _glManager->shaderManager->GetShader(shaderKey);
    if (!shader) return -1;

    double m[16];
    double cameraPosD[3], cameraUpD[3], cameraDirD[3];
    _paramsMgr->GetViewpointParams(_winName)->GetModelViewMatrix(m);
    _paramsMgr->GetViewpointParams(_winName)->ReconstructCamera(m, cameraPosD, cameraUpD, cameraDirD);
    glm::vec3 cameraDir = glm::vec3(cameraDirD[0], cameraDirD[1], cameraDirD[2]);
    glm::vec3 cameraPos = glm::vec3(cameraPosD[0], cameraPosD[1], cameraPosD[2]);

    shader->Bind();
    shader->SetUniform("P", _glManager->matrixManager->GetProjectionMatrix());
    shader->SetUniform("MV", _glManager->matrixManager->GetModelViewMatrix());
    shader->SetUniform("aspect", _glManager->matrixManager->GetProjectionAspectRatio());
    shader->SetUniform("radius", radius);
    shader->SetUniform("dirScale", (float)rp->GetValueDouble(ParticleParams::DirectionScaleTag, 1.));
    shader->SetUniform("lightingEnabled", (bool)rp->GetValueLong(ParticleParams::LightingEnabledTag, true));
    shader->SetUniform("scales", _getScales());
    shader->SetUniform("cameraPos", cameraPos);
    shader->SetUniform("lightDir", cameraDir);
    shader->SetUniform("phongAmbient", (float)rp->GetValueDouble(ParticleParams::PhongAmbientTag, 0.4));
    shader->SetUniform("phongDiffuse", (float)rp->GetValueDouble(ParticleParams::PhongDiffuseTag, 0.8));
    shader->SetUniform("phongSpecular", (float)rp->GetValueDouble(ParticleParams::PhongSpecularTag, 0.0));
    shader->SetUniform("phongShininess", (float)(100 * powf(rp->GetValueDouble(ParticleParams::PhongShininessTag, 0.01), 2)));

    shader->SetUniform("mapRange", glm::make_vec2(_colorMapRange));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    shader->SetUniform("LUT", 0);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBindVertexArray(_VAO);

    glDrawArrays(GL_POINTS, 0, _particlesCount);
    GL_ERR_BREAK();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
    shader->UnBind();
    DisableClippingPlanes();

    return 0;
}

int ParticleRenderer::_getGrids(Grid*& grid, std::vector<Grid*>& vecGrids) const {
#define PD3(v) printf("%s = %f, %f, %f\n", #v, v[0], v[1], v[2])
    string varName = _cacheParams.varName;
    grid = _dataMgr->GetVariable(_cacheParams.ts, _cacheParams.varName, _cacheParams.rLevel, _cacheParams.cLevel, _cacheParams.boxMin, _cacheParams.boxMax, true);
    if (!grid) {
        SetErrMsg("Cannot acquire grid for variable \"%s\"", _cacheParams.varName.c_str());
        return -1;
    }

    vector<string> vecNames = _cacheParams.fieldVars;
    vector<string> mainVarCoords;
    _dataMgr->GetVarCoordVars(_cacheParams.varName, true, mainVarCoords);

    if (_cacheParams.direction) {
        for (auto var : vecNames) {
            vector<string> varCoords;
            _dataMgr->GetVarCoordVars(var, true, varCoords);

            if (mainVarCoords != varCoords) {
                if (grid) {
                    _dataMgr->UnlockGrid(grid);
                    delete grid;
                }
                for (auto g : vecGrids) {
                    _dataMgr->UnlockGrid(g);
                    delete g;
                }
                SetErrMsg("Variable \"%s\" on different grid from main variable", var.c_str());
                return -1;
            }

            Grid *ng = _dataMgr->GetVariable(_cacheParams.ts, var, _cacheParams.rLevel, _cacheParams.cLevel, _cacheParams.boxMin, _cacheParams.boxMax, true);
            if (!ng) {
                if (grid) {
                    _dataMgr->UnlockGrid(grid);
                    delete grid;
                }
                for (auto g : vecGrids) {
                    _dataMgr->UnlockGrid(g);
                    delete g;
                }
                SetErrMsg("Cannot read var \"%s\"", var.c_str());
                return -1;
            }
            vecGrids.push_back(ng);
        }
    }

    if (!_cacheParams.radiusVarName.empty()) {
        vecGrids.push_back(_dataMgr->GetVariable(_cacheParams.ts, _cacheParams.radiusVarName, _cacheParams.rLevel, _cacheParams.cLevel, _cacheParams.boxMin, _cacheParams.boxMax, true));
    }

    return 0;
}

template<typename T> static void UploadDataBuffer(T buffer) {
    glBufferData(GL_ARRAY_BUFFER, sizeof(T) * buffer.size(), buffer.data(), GL_STATIC_DRAW);
}

void ParticleRenderer::_generateTextureData(const Grid* grid, const std::vector<Grid*>& vecGrids) {
    bool      showDir = _cacheParams.direction;
    bool      dynamicSize = !_cacheParams.radiusVarName.empty();
    size_t    stride = max(1L, (long)_cacheParams.stride);
    auto      node = grid->ConstNodeBegin(_cacheParams.boxMin, _cacheParams.boxMax);
    auto      nodeEnd = grid->ConstNodeEnd();
    CoordType coordsBuf;
    vector<Grid::ConstIterator> dirs;

    if (showDir) for (int i = 0; i < 3; i++) dirs.push_back(vecGrids[i]->cbegin(_cacheParams.boxMin, _cacheParams.boxMax));
    if (dynamicSize) dirs.push_back(vecGrids[vecGrids.size()-1]);

    struct PointDataT {vec3 pos; float val;}; vector<PointDataT> pointData;
    struct DirectionDataT {vec3 pos; vec3 norm; float val;}; vector<DirectionDataT> directionData;
    struct PointDynSizeDataT {vec3 pos; float val; float radius;}; vector<PointDynSizeDataT> pointDynSizeData;
    struct DirectionDynSizeDataT {vec3 pos; vec3 norm; float val; float radius;}; vector<DirectionDynSizeDataT> directionDynSizeData;

    _particlesCount = grid->GetCoordDimensions(1)[0] / stride;

    if (showDir) {
        SetupParticleDirectionGL(_VAO, _VBO, dynamicSize);
        directionData.reserve(_particlesCount);
    } else {
        SetupParticlePointGL(_VAO, _VBO, dynamicSize);
        pointData.reserve(_particlesCount);
    }
    assert(glIsVertexArray(_VAO) == GL_TRUE);
    assert(glIsBuffer(_VBO) == GL_TRUE);

    for (size_t i = 0; node != nodeEnd;) {
        if (i % stride) goto step;
        {
            const float value = grid->GetValueAtIndex(*node);
            grid->GetUserCoordinates(*node, coordsBuf);
            const vec3 p = CoordTypeToVec3(coordsBuf);

            if (showDir){
                const glm::vec3 norm(*(dirs[0]), *(dirs[1]), *(dirs[2]));
                if (dynamicSize)
                    directionDynSizeData.push_back({p, norm, value, *(dirs[dirs.size()-1])});
                else
                    directionData.push_back({p, norm, value});
            } else {
                if (dynamicSize)
                    pointDynSizeData.push_back({p, value, *(dirs[dirs.size()-1])});
                else
                    pointData.push_back({p, value});
            }
        }
        step:
        ++node, ++i;
        for (auto &it : dirs) ++it;
    }

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    if (dynamicSize) {
        if (showDir) UploadDataBuffer(directionDynSizeData);
        else UploadDataBuffer(pointDynSizeData);
    } else {
        if (showDir) UploadDataBuffer(directionData);
        else UploadDataBuffer(pointData);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleRenderer::_computeBaseRadius() {
    auto rp = GetActiveParams();
    CoordType mind, maxd;

    // Need to find a non-empty variable from color mapping or velocity variables.
    std::string nonEmptyVarName = rp->GetColorMapVariableName();
    assert(!nonEmptyVarName.empty());

    _dataMgr->GetVariableExtents(_cacheParams.ts, nonEmptyVarName, _cacheParams.rLevel, _cacheParams.cLevel, mind, maxd);
    glm::vec3  min(mind[0], mind[1], mind[2]);
    glm::vec3  max(maxd[0], maxd[1], maxd[2]);
    glm::vec3  lens = max - min;
    float largestDim = glm::max(lens.x, glm::max(lens.y, lens.z));
    float radiusBase = largestDim / 560.f;

    if (!_cacheParams.radiusVarName.empty()) {
        vector<double> dataRange;
        _dataMgr->GetDataRange(_cacheParams.ts, _cacheParams.radiusVarName, _cacheParams.rLevel, _cacheParams.cLevel, _cacheParams.boxMin, _cacheParams.boxMax, dataRange);
        radiusBase /= (dataRange[0]+dataRange[1])/2;
    }

    rp->SetValueDouble(ParticleParams::RenderRadiusBaseTag, "", radiusBase);
}

void ParticleRenderer::_renderParticlesLegacy(const Grid* grid, const std::vector<Grid*>& vecGrids) const {
    auto p = GetActiveParams();
    MapperFunction* mf = p->GetMapperFunc(p->GetVariableName());
    float LUT[256*4];
    mf->makeLut(LUT);

    float mapMin = mf->getMinMapValue();
    float mapMax = mf->getMaxMapValue();
    float mapDif = mapMax - mapMin;
    bool  showDir = _cacheParams.direction;

    LegacyGL* lgl = _glManager->legacy;
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    lgl->Color3f(1, 1, 1);
    lgl->Begin(showDir ? GL_LINES : GL_POINTS);

    auto      node = grid->ConstNodeBegin(_cacheParams.boxMin, _cacheParams.boxMax);
    auto      nodeEnd = grid->ConstNodeEnd();
    CoordType coordsBuf;
    vector<Grid::ConstIterator> dirs;
    for (auto g : vecGrids) dirs.push_back(g->cbegin(_cacheParams.boxMin, _cacheParams.boxMax));

    size_t stride = max(1L, (long)_cacheParams.stride);
    float  dirScale = _cacheParams.directionScale;

    for (size_t i = 0; node != nodeEnd; ++node, ++i) {
        if (i % stride) {
            if (showDir)
                for (auto &it : dirs) ++it;
            continue;
        }

        const float value = grid->GetValueAtIndex(*node);
        grid->GetUserCoordinates(*node, coordsBuf);
        const glm::vec3 p(coordsBuf[0], coordsBuf[1], coordsBuf[2]);

        lgl->Color4fv(&LUT[4 * glm::clamp((int)(255 * (value - mapMin) / mapDif), 0, 255)]);
        lgl->Vertex3fv((float *)&p);

        if (showDir) {
            const glm::vec3 n(*(dirs[0]), *(dirs[1]), *(dirs[2]));
            const glm::vec3 p2 = p + n * dirScale;
            lgl->Vertex3fv((float *)&p2);

            for (auto &it : dirs) ++it;
        }
    }
    lgl->End();
}

void ParticleRenderer::_prepareColormap() {
    assert(_cacheParams.tf_lut.size() % 4 == 0);
    _colorMapRange[0] = _cacheParams.tf_minMax[0];
    _colorMapRange[1] = _cacheParams.tf_minMax[1];
    _colorMapRange[2] = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ? (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f;
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _cacheParams.tf_lut.size() / 4, 0, GL_RGBA, GL_FLOAT, _cacheParams.tf_lut.data());
}

glm::vec3 ParticleRenderer::_getScales() {
    string                  myVisName = GetVisualizer();  // This is the line that's not const
    VAPoR::ViewpointParams *vpp = _paramsMgr->GetViewpointParams(myVisName);
    string                  datasetName = GetMyDatasetName();
    Transform *             tDataset = vpp->GetTransform(datasetName);
    Transform *             tRenderer = GetActiveParams()->GetTransform();

    vector<double> scales = tDataset->GetScales();
    vector<double> rendererScales = tRenderer->GetScales();

    scales[0] *= rendererScales[0];
    scales[1] *= rendererScales[1];
    scales[2] *= rendererScales[2];

    return glm::vec3(scales[0], scales[1], scales[2]);
}
