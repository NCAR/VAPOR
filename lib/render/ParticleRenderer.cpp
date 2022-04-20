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
#include <vapor/FlowRenderer.h>
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

ParticleRenderer::~ParticleRenderer() {}

int ParticleRenderer::_paintGL(bool)
{
    //FlowRenderer fr(_paramsMgr,_winName,_dataSetName,_instName,_dataMgr);
    //fr._print();

    //flow::Advection* adv;
    //fr._renderAdvection(adv);

    int rc = 0;
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

    auto p = GetActiveParams();

    MapperFunction *mf = p->GetMapperFunc(p->GetVariableName());
    float           LUT[256 * 4];
    mf->makeLut(LUT);
    float mapMin = mf->getMinMapValue();
    float mapMax = mf->getMaxMapValue();
    float mapDif = mapMax - mapMin;

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

    auto *lgl = _glManager->legacy;
    lgl->Color3f(1, 1, 1);
    lgl->Begin(showDir ? GL_LINES : GL_POINTS);

    long                        renderedParticles = 0;
    auto                        node = grid->ConstNodeBegin(minExt, maxExt);
    auto                        nodeEnd = grid->ConstNodeEnd();
    CoordType                   coordsBuf;
    vector<Grid::ConstIterator> dirs;
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

        _renderAdvection(p);

        lgl->Color4fv(&LUT[4 * glm::clamp((int)(255 * (value - mapMin) / mapDif), 0, 255)]);
        lgl->Vertex3fv((float *)&p);
        renderedParticles++;

        if (showDir) {
            const glm::vec3 n(*(dirs[0]), *(dirs[1]), *(dirs[2]));
            const glm::vec3 p2 = p + n * dirScale;
            lgl->Vertex3fv((float *)&p2);

            for (auto &it : dirs) ++it;
        }
    }
    lgl->End();

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

//int ParticleRenderer::_renderAdvection(const flow::Advection *adv)
int ParticleRenderer::_renderAdvection(const glm::vec3& p)
{
    //std::cout << "_renderAdvection() " << p[0] << " " << p[1] << " " << p[2] << std::endl;
    //FlowParams *rp = dynamic_cast<FlowParams *>(GetActiveParams());

    //if (_renderStatus != FlowStatus::UPTODATE) {
        //int nStreams = adv->GetNumberOfStreams();
        int nStreams = 1;

        typedef struct {
            glm::vec3  p;
            float v;
        } Vertex;
        vector<Vertex> vertices;
        vector<int>    sizes;
        vector<Vertex> sv;

        // If streams are larger than this then need to skip remaining
        //size_t maxSamples = rp->GetSteadyNumOfSteps() + 1;
        size_t maxSamples = 1;

        // First calculate the starting time stamp. Copied from legacy.
        //double startingTime = _timestamps[0];
        double startingTime = 0;
        /*if (!_cache_isSteady) {
            startingTime = _timestamps[0];
            // note that _cache_currentTS is cast to a signed integer.
            if (int(_cache_currentTS) - _cache_pastNumOfTimeSteps > 0) startingTime = _timestamps[_cache_currentTS - _cache_pastNumOfTimeSteps];
        }*/

        for (int s = 0; s < nStreams; s++) {
            std::cout << "Foo " << nStreams << std::endl;
            //const vector<flow::Particle> &stream = adv->GetStreamAt(s);
            const vector<flow::Particle> stream = {flow::Particle(p[0],p[1],p[2],0.,29.), flow::Particle(p[0],p[1],p[2]+50,0.,29.)};
            sv.clear();
            int sn = stream.size();
            //if (_cache_isSteady) sn = std::min(sn, (int)maxSamples);

            for (int i = 0; i < sn + 1; i++) {
                std::cout << "Bar " << i << " " << sn+1 << std::endl;
                // "IsSpecial" means don't render this sample.
                if (i == sn || stream[i].IsSpecial()) {
                    std::cout << "Baz" << std::endl;
                    int svn = sv.size();

                    if (svn < 2) {
                        sv.clear();
                        std::cout << "done" << std::endl;
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
                    std::cout << "Boo" << std::endl;
                    const flow::Particle &p = stream[i];

                    //if (_cache_isSteady) {
                        sv.push_back({p.location, p.value});
                    /*} else {
                        if (p.time > _timestamps.at(_cache_currentTS)) continue;
                        if (p.time >= startingTime) sv.push_back({p.location, p.value});
                    }*/
                }
            }

            //_renderStatus = FlowStatus::UPTODATE;
        }

        assert(glIsVertexArray(_VAO) == GL_TRUE);
        assert(glIsBuffer(_VBO) == GL_TRUE);

        glBindVertexArray(_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        _streamSizes = sizes;
        std::cout << "_streamSizes " << _streamSizes.size() << std::endl;
    //}

    //bool show_dir = rp->GetValueLong(FlowParams::RenderShowStreamDirTag, false);

    //_renderAdvectionHelper(show_dir);
    _renderAdvectionHelper();
    //if (show_dir) _renderAdvectionHelper(false);

    return 0;
}

int ParticleRenderer::_renderAdvectionHelper(bool renderDirection)
{
    auto rp = GetActiveParams();

    FlowParams::RenderType renderType = (FlowParams::RenderType)rp->GetValueLong(FlowParams::RenderTypeTag, FlowParams::RenderTypeStream);
    FlowParams::GlpyhType  glyphType = (FlowParams::GlpyhType)rp->GetValueLong(FlowParams::RenderGlyphTypeTag, FlowParams::GlpyhTypeSphere);

    bool  geom3d = rp->GetValueLong(FlowParams::RenderGeom3DTag, false);
    float radiusBase = rp->GetValueDouble(FlowParams::RenderRadiusBaseTag, -1);
    if (radiusBase == -1) {
        CoordType mind, maxd;

        // Need to find a non-empty variable from color mapping or velocity variables.
        std::string nonEmptyVarName = rp->GetColorMapVariableName();
        /*if (nonEmptyVarName.empty()) {
            for (auto it = _velocityField.VelocityNames.cbegin(); it != _velocityField.VelocityNames.cend(); ++it) {
                if (!it->empty()) {
                    nonEmptyVarName = *it;
                    break;
                }
            }
        }*/
        assert(!nonEmptyVarName.empty());

        _dataMgr->GetVariableExtents(rp->GetCurrentTimestep(), nonEmptyVarName, rp->GetRefinementLevel(), rp->GetCompressionLevel(), mind, maxd);
        glm::vec3  min(mind[0], mind[1], mind[2]);
        glm::vec3  max(maxd[0], maxd[1], maxd[2]);
        glm::vec3  lens = max - min;
        float largestDim = glm::max(lens.x, glm::max(lens.y, lens.z));
        radiusBase = largestDim / 560.f;
        rp->SetValueDouble(FlowParams::RenderRadiusBaseTag, "", radiusBase);
    }
    float radiusScalar = rp->GetValueDouble(FlowParams::RenderRadiusScalarTag, 1);
    float radius = radiusBase * radiusScalar;
    int   glyphStride = rp->GetValueLong(FlowParams::RenderGlyphStrideTag, 5);

    ShaderProgram *shader = nullptr;

    if (renderType == FlowParams::RenderTypeStream) {
        if (geom3d)
            if (renderDirection)
                shader = _glManager->shaderManager->GetShader("FlowGlyphsTubeDirArrow");
            else
                shader = _glManager->shaderManager->GetShader("FlowTubes");
        else if (renderDirection)
            shader = _glManager->shaderManager->GetShader("FlowGlyphsLineDirArrow2D");
        else
            shader = _glManager->shaderManager->GetShader("FlowLines");
    } else if (renderType == FlowParams::RenderTypeSamples) {
        if (glyphType == FlowParams::GlpyhTypeSphere)
            if (geom3d)
                shader = _glManager->shaderManager->GetShader("FlowGlyphsSphereSplat");
            else
                shader = _glManager->shaderManager->GetShader("FlowGlyphsSphere2D");
        else if (geom3d)
            shader = _glManager->shaderManager->GetShader("FlowGlyphsArrow");
        else
            shader = _glManager->shaderManager->GetShader("FlowGlyphsArrow2D");
    } else {
        shader = _glManager->shaderManager->GetShader("FlowLines");
    }

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
    shader->SetUniform("glyphStride", glyphStride);
    shader->SetUniform("showOnlyLeadingSample", (bool)rp->GetValueLong(FlowParams::RenderGlyphOnlyLeadingTag, false));
    //shader->SetUniform("scales", _getScales());
    shader->SetUniform("scales", glm::vec3(1000.,1000.,1000.));
    shader->SetUniform("cameraPos", cameraPos);
    if (rp->GetValueLong(FlowParams::RenderLightAtCameraTag, true))
        shader->SetUniform("lightDir", cameraDir);
    else
        shader->SetUniform("lightDir", glm::vec3(0, 0, -1));
    shader->SetUniform("phongAmbient", (float)rp->GetValueDouble(FlowParams::PhongAmbientTag, 0));
    shader->SetUniform("phongDiffuse", (float)rp->GetValueDouble(FlowParams::PhongDiffuseTag, 0));
    shader->SetUniform("phongSpecular", (float)rp->GetValueDouble(FlowParams::PhongSpecularTag, 0));
    shader->SetUniform("phongShininess", (float)rp->GetValueDouble(FlowParams::PhongShininessTag, 0));

    //shader->SetUniform("mapRange", glm::make_vec2(_colorMapRange));
    float foo[] = {0.f,30.f,1e-5f};
    shader->SetUniform("mapRange", glm::make_vec2(foo));

    shader->SetUniform("fade_tails", (bool)rp->GetValueLong(FlowParams::RenderFadeTailTag, 0));
    shader->SetUniform("fade_start", (int)rp->GetValueLong(FlowParams::RenderFadeTailStartTag, 10));
    shader->SetUniform("fade_length", (int)rp->GetValueLong(FlowParams::RenderFadeTailLengthTag, 10));
    shader->SetUniform("fade_stop", (int)rp->GetValueLong(FlowParams::RenderFadeTailStopTag, 0));

    shader->SetUniform("density", renderType == FlowParams::RenderTypeDensity);
    shader->SetUniform("falloff", (float)rp->GetValueDouble(FlowParams::RenderDensityFalloffTag, 1));
    shader->SetUniform("tone", (float)rp->GetValueDouble(FlowParams::RenderDensityToneMappingTag, 1));

    //    Features supported by shaders but not implemented in GUI/not finished
    //
    //    shader->SetUniform("constantColorEnabled", false);
    //    shader->SetUniform("Color", vec3(1.0f));
    //    shader->SetUniform("antiAlias", (bool)rp->GetValueLong("anti_alias", 0));
    //    auto bcd = rp->GetValueDoubleVec("border_color");
    //    if (bcd.size())
    //        shader->SetUniform("borderColor", vec3((float)bcd[0], bcd[1], bcd[2]));
    //    shader->SetUniform("border", (float)rp->GetValueDouble("border", 0));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    shader->SetUniform("LUT", 0);

    EnableClipToBox(shader, 0.01);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBindVertexArray(_VAO);

    if (renderType == FlowParams::RenderTypeDensity) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        if (rp->GetValueLong("invert", false))
            glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        else
            glBlendEquation(GL_FUNC_ADD);
        glDepthMask(GL_FALSE);
    }

    size_t offset = 0;
    for (int n : _streamSizes) {
        std::cout << "n " << n << std::endl;
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
