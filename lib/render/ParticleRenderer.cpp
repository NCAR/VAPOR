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
#include <vapor/Particle.h>
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

static RendererRegistrar<ParticleRenderer> registrar(ParticleRenderer::GetClassType(), ParticleParams::GetClassType());

ParticleRenderer::ParticleRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, ParticleParams::GetClassType(), ParticleRenderer::GetClassType(), instName, dataMgr),
  _colorMapTexOffset(0)
{
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
    int rc = 0;
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

    auto p = GetActiveParams();

    MapperFunction *mf = p->GetMapperFunc(p->GetVariableName());
    float           LUT[256 * 4];
    mf->makeLut(LUT);

    CoordType minExt = {0.0, 0.0, 0.0};
    CoordType maxExt = {0.0, 0.0, 0.0};
    p->GetBox()->GetExtents(minExt, maxExt);

#define PD3(v) printf("%s = %f, %f, %f\n", #v, v[0], v[1], v[2])
    string varName = p->GetVariableName();
    Grid * grid = _dataMgr->GetVariable(p->GetCurrentTimestep(), varName, p->GetRefinementLevel(), p->GetCompressionLevel(), minExt, maxExt, true);
    if (!grid) return -1;

    vector<long> values;

    size_t         stride = max(1L, p->GetValueLong(ParticleParams::StrideTag, 1));
    bool           showDir = p->GetValueLong(ParticleParams::ShowDirectionTag, 0);
    float          dirScale = p->GetValueDouble(ParticleParams::DirectionScaleTag, 1);
    vector<Grid *> vecGrids;
    if (showDir) {
        vector<string> vecNames = p->GetFieldVariableNames();
        vector<string> mainVarCoords;
        _dataMgr->GetVarCoordVars(varName, true, mainVarCoords);

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

            Grid *ng = _dataMgr->GetVariable(p->GetCurrentTimestep(), var, p->GetRefinementLevel(), p->GetCompressionLevel(), minExt, maxExt, true);
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

    auto   dims = grid->GetDimensions();
    size_t nCoords = 1;
    for (const auto d : dims) nCoords *= d;

    long                        renderedParticles = 0;
    auto                        node = grid->ConstNodeBegin(minExt, maxExt);
    auto                        nodeEnd = grid->ConstNodeEnd();
    CoordType                   coordsBuf;
    vector<Grid::ConstIterator> dirs;
    std::vector<glm::vec4>      particles;
    for (auto g : vecGrids) dirs.push_back(g->cbegin(minExt, maxExt));
    for (size_t i = 0; node != nodeEnd; ++node, ++i) {
        if (i % stride) {
            if (showDir)
                for (auto &it : dirs) ++it;
            continue;
        }

        const float value = grid->GetValueAtIndex(*node);
        grid->GetUserCoordinates(*node, coordsBuf);
        const glm::vec3 p(coordsBuf[0], coordsBuf[1], coordsBuf[2]);
        particles.push_back(glm::vec4(p[0], p[1], p[2], value));

        renderedParticles++;

        if (showDir) {
            const glm::vec3 n(*(dirs[0]), *(dirs[1]), *(dirs[2]));
            const glm::vec3 p2 = p + n * dirScale;
            particles.push_back(glm::vec4(p2[0], p2[1], p2[2], value));
            for (auto &it : dirs) ++it;
        }
        else 
            particles.push_back(glm::vec4(p[0], p[1], p[2], value));
    }

    _prepareColormap();
    _renderParticles(particles);

    //    printf("Rendered %li particles\n", renderedParticles);

    _dataMgr->UnlockGrid(grid);
    delete grid;
    for (auto g : vecGrids) {
        _dataMgr->UnlockGrid(g);
        delete g;
    }
    return rc;
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

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), NULL);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)sizeof(glm::vec3));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return 0; 
}

int ParticleRenderer::_renderParticles(const std::vector<glm::vec4>& particles)
{
    typedef struct {
        glm::vec3  p;
        float v;
    } Vertex;
    vector<Vertex> vertices;
    vector<int>    sizes;
    vector<Vertex> sv;

    int nStreams = particles.size();
    for (int s = 0; s < nStreams; s+=2) {
        glm::vec4 p = particles[s];
        glm::vec4 p2 = particles[s+1];
        const vector<flow::Particle> stream = {flow::Particle(p[0],p[1],p[2],0.,p[3]), flow::Particle(p2[0],p2[1],p2[2],0.,p2[3])};
        sv.clear();
        int sn = stream.size();

        for (int i = 0; i < sn + 1; i++) {
            if (i == sn) {
                int svn = sv.size();

                if (svn < 2) {
                    sv.clear();
                    continue;
                }

                glm::vec3 prep(-normalize(sv[1].p - sv[0].p) + sv[0].p);
                glm::vec3 post(normalize(sv[svn - 1].p - sv[svn - 2].p) + sv[svn - 1].p);

                size_t vn = vertices.size();
                vertices.resize(vn + svn + 2);
                vertices[vn] = {prep, sv[0].v};
                vertices[vertices.size() - 1] = {post, sv[svn - 1].v};

                memcpy(vertices.data() + vn + 1, sv.data(), sizeof(Vertex) * svn);

                sizes.push_back(svn + 2);
                sv.clear();
            } else {
                const flow::Particle &p = stream[i];
                sv.push_back({p.location, p.value});
            }
        }
    }

    assert(glIsVertexArray(_VAO) == GL_TRUE);
    assert(glIsBuffer(_VBO) == GL_TRUE);

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    _streamSizes = sizes;

    _renderParticlesHelper();

    return 0;
}

int ParticleRenderer::_renderParticlesHelper(bool renderDirection)
{
    auto rp = GetActiveParams();

    float radiusBase = rp->GetValueDouble(ParticleParams::RenderRadiusBaseTag, -1);
    if (radiusBase == -1) {
        CoordType mind, maxd;

        // Need to find a non-empty variable from color mapping or velocity variables.
        std::string nonEmptyVarName = rp->GetColorMapVariableName();
        assert(!nonEmptyVarName.empty());

        _dataMgr->GetVariableExtents(rp->GetCurrentTimestep(), nonEmptyVarName, rp->GetRefinementLevel(), rp->GetCompressionLevel(), mind, maxd);
        glm::vec3  min(mind[0], mind[1], mind[2]);
        glm::vec3  max(maxd[0], maxd[1], maxd[2]);
        glm::vec3  lens = max - min;
        float largestDim = glm::max(lens.x, glm::max(lens.y, lens.z));
        radiusBase = largestDim / 560.f;
        rp->SetValueDouble(ParticleParams::RenderRadiusBaseTag, "", radiusBase);
    }
    float radiusScalar = rp->GetValueDouble(ParticleParams::RadiusTag, 1.);
    float radius = radiusBase * radiusScalar;

    ShaderProgram *shader = nullptr;
    if (rp->GetValueLong(ParticleParams::ShowDirectionTag, 0))
        shader = _glManager->shaderManager->GetShader("FlowTubes"); 
    else
        shader = _glManager->shaderManager->GetShader("FlowGlyphsSphereSplat");
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
    shader->SetUniform("lightingEnabled", true);
    shader->SetUniform("scales", _getScales());
    shader->SetUniform("cameraPos", cameraPos);
    shader->SetUniform("lightDir", cameraDir);
    shader->SetUniform("phongAmbient", (float)rp->GetValueDouble(ParticleParams::PhongAmbientTag, 0.4));
    shader->SetUniform("phongDiffuse", (float)rp->GetValueDouble(ParticleParams::PhongDiffuseTag, 0.8));
    shader->SetUniform("phongSpecular", (float)rp->GetValueDouble(ParticleParams::PhongSpecularTag, 0.0));
    shader->SetUniform("phongShininess", (float)rp->GetValueDouble(ParticleParams::PhongShininessTag, 2.));


    VAPoR::MapperFunction *mapperFunc = rp->GetMapperFunc(rp->GetColorMapVariableName());
    mapperFunc->makeLut(_colorMap);
    assert(_colorMap.size() % 4 == 0);
    std::vector<double> range = mapperFunc->getMinMaxMapValue();
    _colorMapRange[0] = float(range[0]);
    _colorMapRange[1] = float(range[1]);
    _colorMapRange[2] = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ? (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f;
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());
    shader->SetUniform("mapRange", glm::make_vec2(_colorMapRange));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    shader->SetUniform("LUT", 0);

    EnableClipToBox(shader, 0.01);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBindVertexArray(_VAO);

    size_t offset = 0;
    for (int n : _streamSizes) {
        shader->SetUniform("nVertices", n);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, offset, n);
        offset += n;
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);
    shader->UnBind();
    DisableClippingPlanes();

    return 0;
}

void ParticleRenderer::_prepareColormap() {
    VAPoR::MapperFunction *mapperFunc = GetActiveParams()->GetMapperFunc(GetActiveParams()->GetColorMapVariableName()); // This is the line that's not const
    mapperFunc->makeLut(_colorMap);
    assert(_colorMap.size() % 4 == 0);
    std::vector<double> range = mapperFunc->getMinMaxMapValue();
    _colorMapRange[0] = float(range[0]);
    _colorMapRange[1] = float(range[1]);
    _colorMapRange[2] = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ? (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f;
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());
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
