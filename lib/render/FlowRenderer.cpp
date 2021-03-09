#include "vapor/glutil.h"
#include "vapor/FlowRenderer.h"
#include "vapor/Particle.h"
#include "vapor/AdvectionIO.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cctype>    // std::isspace
#include <random>
#include <algorithm>
#include <vapor/Progress.h>

#define GL_ERROR -20

using namespace VAPoR;
using glm::vec3;
using glm::vec4;

static RendererRegistrar<FlowRenderer> registrar(FlowRenderer::GetClassType(), FlowParams::GetClassType());

// Constructor
FlowRenderer::FlowRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, FlowParams::GetClassType(), FlowRenderer::GetClassType(), instName, dataMgr), _velocityField(9),    // big enough to hold velocities for 3 time steps.
  _colorField(3),                                                                                                                        // big enough to hold scalars for 3 time steps.
  _colorMapTexOffset(0)
{
}

// Destructor
FlowRenderer::~FlowRenderer()
{
    // Delete vertex arrays
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

std::string FlowRenderer::_getColorbarVariableName() const { return GetActiveParams()->GetColorMapVariableName(); }

int FlowRenderer::_initializeGL()
{
    // First prepare the VelocityField
    _velocityField.AssignDataManager(_dataMgr);
    _colorField.AssignDataManager(_dataMgr);
    _timestamps = _dataMgr->GetTimeCoordinates();

    // Followed by real OpenGL initializations
    ShaderProgram *shader = nullptr;
    if ((shader = _glManager->shaderManager->GetShader("FlowLine")))
        _shader = shader;
    else
        return GL_ERROR;

    /* Create Vertex Array Object (VAO) */
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

int FlowRenderer::_outputFlowLines()
{
    auto *params = dynamic_cast<FlowParams *>(GetActiveParams());
    assert(params != nullptr);

    // Retrieve additional variables that users require to sample
    const auto addiVars = params->GetFlowOutputMoreVariables();

    // Identify variables that an advection already has, but the user doesn't
    // include in addiVars. Remove them.
    //
    // start by listing all variables as potentially to be removed
    auto removeVars = _advection.GetPropertyVarNames();
    auto containV = [&addiVars](const std::string &v) { return std::find(addiVars.cbegin(), addiVars.cend(), v) != addiVars.cend(); };
    // remove those confirmed by the user from the "to remove" list
    removeVars.erase(std::remove_if(removeVars.begin(), removeVars.end(), containV), removeVars.end());
    for (const auto &rmV : removeVars) {
        _advection.RemoveParticleProperty(rmV);

        if (_2ndAdvection) _2ndAdvection->RemoveParticleProperty(rmV);
    }

    for (const auto &v : addiVars) {
        // Create a VaporField with this variable
        flow::VaporField varField(2);
        varField.AssignDataManager(_dataMgr);
        varField.UpdateParams(params);
        varField.ScalarName = v;

        // Sample values along the pathlines.
        // Note that the advection class will do nothing if this variable already exists.
        // Also note that the advection class will do a simple copy if this variable is
        //   the same as particle values.
        _advection.CalculateParticleProperties(&varField);
        if (_2ndAdvection) _2ndAdvection->CalculateParticleProperties(&varField);
    }

    // In case of steady flow, output the number of particles that
    // equals to the advection steps.
    // In the case of unsteady flow, output particles that are up to
    // the advection timestamp.
    int rv;
    if (params->GetIsSteady()) {
        rv = flow::OutputFlowlinesNumSteps(&_advection, params->GetFlowlineOutputFilename().c_str(), params->GetSteadyNumOfSteps(), _dataMgr->GetMapProjection(), false);
    } else {
        rv = flow::OutputFlowlinesMaxTime(&_advection, params->GetFlowlineOutputFilename().c_str(), _timestamps.at(params->GetCurrentTimestep()), _dataMgr->GetMapProjection(), false);
    }
    if (rv != 0) {
        MyBase::SetErrMsg("Output flow lines wrong!");
        return rv;
    }

    if (_2ndAdvection) {    // bi-directional advection
        if (params->GetIsSteady()) {
            rv = flow::OutputFlowlinesNumSteps(_2ndAdvection.get(), params->GetFlowlineOutputFilename().c_str(), params->GetSteadyNumOfSteps(), _dataMgr->GetMapProjection(), true);
        } else {
            rv = flow::OutputFlowlinesMaxTime(_2ndAdvection.get(), params->GetFlowlineOutputFilename().c_str(), _timestamps.at(params->GetCurrentTimestep()), _dataMgr->GetMapProjection(), true);
        }
        if (rv != 0) {
            MyBase::SetErrMsg("Output flow lines wrong!");
            return rv;
        }
    }

    return 0;
}

int FlowRenderer::_paintGL(bool fast)
{
    FlowParams *params = dynamic_cast<FlowParams *>(GetActiveParams());
    int         rv;    // return value

    _velocityField.DefaultZ = Renderer::GetDefaultZ(_dataMgr, params->GetCurrentTimestep());

    if (params->GetNeedFlowlineOutput()) {
        rv = _outputFlowLines();
        params->SetNeedFlowlineOutput(false);
        if (rv != 0) return rv;
    }

    if (_updateFlowCacheAndStates(params) != 0) {
        MyBase::SetErrMsg("Parameters not ready!");
        return flow::PARAMS_ERROR;
    }

    if (_velocityStatus != FlowStatus::UPTODATE || _colorStatus != FlowStatus::UPTODATE) _renderStatus = FlowStatus::SIMPLE_OUTOFDATE;

    _velocityField.UpdateParamAndVarNames(params);
    _colorField.UpdateParamAndVarNames(params);

    // In case there's 0 variable selected, meaning that more than 2 of the velocity
    // variable names are empty strings, then the paint routine aborts.
    if (_velocityField.GetNumOfEmptyVelocityNames() > 2) {
        MyBase::SetErrMsg("Please provide at least 1 field variables for advection!");
        return flow::PARAMS_ERROR;
    }

    if (_velocityStatus == FlowStatus::SIMPLE_OUTOFDATE) {
        // First step is to re-calculate deltaT
        rv = _velocityField.CalcDeltaTFromCurrentTimeStep(_cache_deltaT);
        if (rv == flow::FIELD_ALL_ZERO) {
            MyBase::SetErrMsg("The velocity field seems to contain only invalid values!");
            return flow::PARAMS_ERROR;
        } else if (rv != 0) {
            MyBase::SetErrMsg("Update deltaT failed!");
            return rv;
        }

        // Obtain seeds for Flow Renderer.
        std::vector<flow::Particle> seeds;
        if (_cache_seedGenMode == FlowSeedMode::UNIFORM)
            rv = _genSeedsRakeUniform(seeds);
        else if (_cache_seedGenMode == FlowSeedMode::RANDOM)
            rv = _genSeedsRakeRandom(seeds);
        else if (_cache_seedGenMode == FlowSeedMode::RANDOM_BIAS)
            rv = _genSeedsRakeRandomBiased(seeds);
        else if (_cache_seedGenMode == FlowSeedMode::LIST)
            rv = _genSeedsFromList(seeds);

        if (rv != 0) {
            MyBase::SetErrMsg("Generating seeds failed!");
            return flow::NO_SEED_PARTICLE_YET;
        }

        // Note on UseSeedParticles(): this is the only function that resets
        //   all the streams inside of an Advection class.
        //   It should immediately be followed by a function to set its periodicity
        _advection.UseSeedParticles(seeds);
        rv = _updateAdvectionPeriodicity(&_advection);
        if (rv != 0) {
            MyBase::SetErrMsg("Update Advection Periodicity failed!");
            return flow::GRID_ERROR;
        }
        if (_2ndAdvection)    // bi-directional advection
        {
            _2ndAdvection->UseSeedParticles(seeds);
            rv = _updateAdvectionPeriodicity(_2ndAdvection.get());
            if (rv != 0) {
                MyBase::SetErrMsg("Update Advection Periodicity failed!");
                return flow::GRID_ERROR;
            }
        }

        _advectionComplete = false;
        _velocityStatus = FlowStatus::UPTODATE;
    } else if (_velocityStatus == FlowStatus::TIME_STEP_OOD) {
        _advectionComplete = false;
        _velocityStatus = FlowStatus::UPTODATE;
    }

    if (!params->UseSingleColor()) {
        if (_colorStatus == FlowStatus::SIMPLE_OUTOFDATE) {
            _advection.ResetParticleValues();
            _coloringComplete = false;
            _colorStatus = FlowStatus::UPTODATE;
            if (_2ndAdvection)    // bi-directional advection
                _2ndAdvection->ResetParticleValues();
        } else if (_colorStatus == FlowStatus::TIME_STEP_OOD) {
            _coloringComplete = false;
            _colorStatus = FlowStatus::UPTODATE;
        }
    }

    if (!_advectionComplete) {
        auto deltaT = _cache_deltaT;
        rv = flow::ADVECT_HAPPENED;

        // Advection scheme 1: advect a maximum number of steps.
        // This scheme is used for steady flow
        if (params->GetIsSteady()) {
            // If the advection is single-directional
            if (params->GetFlowDirection() == 1)    // backward integration
                deltaT *= -1.0;
            long numOfSteps = params->GetSteadyNumOfSteps();

            Progress::StartIndefinite("Performing flowline calculations");
            Progress::Update(0);
            _advection.AdvectSteps(&_velocityField, deltaT, numOfSteps);

            // If the advection is bi-directional
            if (_2ndAdvection) {
                assert(deltaT > 0.0);
                auto deltaT2 = deltaT * -1.0;

                _2ndAdvection->AdvectSteps(&_velocityField, deltaT2, numOfSteps);
            }
            Progress::Finish();
        }

        // Advection scheme 2: advect to a certain timestamp.
        // This scheme is used for unsteady flow
        else {
            for (int i = 1; i <= _cache_currentTS; i++) { rv = _advection.AdvectTillTime(&_velocityField, _timestamps.at(i - 1), deltaT, _timestamps.at(i)); }
        }

        _advectionComplete = true;
    }

    if (!_coloringComplete) {
        rv = _advection.CalculateParticleValues(&_colorField, true);
        if (_2ndAdvection)    // bi-directional advection
            rv = _2ndAdvection->CalculateParticleValues(&_colorField, true);
        _coloringComplete = true;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    _prepareColormap(params);

    rv = 0;

    if (params->GetValueLong("old_render", 0)) {
        _renderFromAnAdvectionLegacy(&_advection, params, fast);
        /* If the advection is bi-directional */
        if (_2ndAdvection) _renderFromAnAdvectionLegacy(_2ndAdvection.get(), params, fast);
    } else {
        // Workaround for how bi-directional was implemented.
        // The rendering caches the flow data on the GPU however it
        // only caches one advection at a time. Since when using bidirectional
        // flow it results in two separate advections, we need to reset the cache
        // before each half is drawn.
        if (_2ndAdvection) _renderStatus = FlowStatus::SIMPLE_OUTOFDATE;

        rv |= _renderAdvection(&_advection);
        /* If the advection is bi-directional */
        if (_2ndAdvection) {
            _renderStatus = FlowStatus::SIMPLE_OUTOFDATE;
            rv |= _renderAdvection(_2ndAdvection.get());
        }
    }

    _restoreGLState();

    return rv;
}

int FlowRenderer::_renderAdvection(const flow::Advection *adv)
{
    FlowParams *rp = dynamic_cast<FlowParams *>(GetActiveParams());

    if (_renderStatus != FlowStatus::UPTODATE) {
        int nStreams = adv->GetNumberOfStreams();

        typedef struct {
            vec3  p;
            float v;
        } Vertex;
        vector<Vertex> vertices;
        vector<int>    sizes;
        vector<Vertex> sv;

        // If streams are larger than this then need to skip remaining
        size_t maxSamples = rp->GetSteadyNumOfSteps() + 1;

        // First calculate the starting time stamp. Copied from legacy.
        double startingTime = _timestamps[0];
        if (!_cache_isSteady) {
            startingTime = _timestamps[0];
            // note that _cache_currentTS is cast to a signed integer.
            if (int(_cache_currentTS) - _cache_pastNumOfTimeSteps > 0) startingTime = _timestamps[_cache_currentTS - _cache_pastNumOfTimeSteps];
        }

        for (int s = 0; s < nStreams; s++) {
            const vector<flow::Particle> &stream = adv->GetStreamAt(s);
            sv.clear();
            int sn = stream.size();
            if (_cache_isSteady) sn = std::min(sn, (int)maxSamples);

            for (int i = 0; i < sn + 1; i++) {
                // "IsSpecial" means don't render this sample.
                if (i == sn || stream[i].IsSpecial()) {
                    int svn = sv.size();

                    if (svn < 2) {
                        sv.clear();
                        continue;
                    }

                    vec3 prep(-normalize(sv[1].p - sv[0].p) + sv[0].p);
                    vec3 post(normalize(sv[svn - 1].p - sv[svn - 2].p) + sv[svn - 1].p);

                    size_t vn = vertices.size();
                    vertices.resize(vn + svn + 2);
                    vertices[vn] = {prep, sv[0].v};
                    vertices[vertices.size() - 1] = {post, sv[svn - 1].v};

                    memcpy(vertices.data() + vn + 1, sv.data(), sizeof(Vertex) * svn);

                    sizes.push_back(svn + 2);
                    sv.clear();
                } else {
                    const flow::Particle &p = stream[i];

                    if (_cache_isSteady) {
                        sv.push_back({p.location, p.value});
                    } else {
                        if (p.time > _timestamps.at(_cache_currentTS)) continue;
                        if (p.time >= startingTime) sv.push_back({p.location, p.value});
                    }
                }
            }

            _renderStatus = FlowStatus::UPTODATE;
        }

        assert(glIsVertexArray(_VAO) == GL_TRUE);
        assert(glIsBuffer(_VBO) == GL_TRUE);

        glBindVertexArray(_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        _streamSizes = sizes;
    }

    bool show_dir = rp->GetValueLong(FlowParams::RenderShowStreamDirTag, false);

    _renderAdvectionHelper(show_dir);
    if (show_dir) _renderAdvectionHelper(false);

    return 0;
}

int FlowRenderer::_renderAdvectionHelper(bool renderDirection)
{
    auto rp = GetActiveParams();

    FlowParams::RenderType renderType = (FlowParams::RenderType)rp->GetValueLong(FlowParams::RenderTypeTag, FlowParams::RenderTypeStream);
    FlowParams::GlpyhType  glyphType = (FlowParams::GlpyhType)rp->GetValueLong(FlowParams::RenderGlyphTypeTag, FlowParams::GlpyhTypeSphere);

    bool  geom3d = rp->GetValueLong(FlowParams::RenderGeom3DTag, false);
    float radiusBase = rp->GetValueDouble(FlowParams::RenderRadiusBaseTag, -1);
    if (radiusBase == -1) {
        vector<double> mind, maxd;

        // Need to find a non-empty variable from color mapping or velocity variables.
        std::string nonEmptyVarName = rp->GetColorMapVariableName();
        if (nonEmptyVarName.empty()) {
            for (auto it = _velocityField.VelocityNames.cbegin(); it != _velocityField.VelocityNames.cend(); ++it) {
                if (!it->empty()) {
                    nonEmptyVarName = *it;
                    break;
                }
            }
        }
        assert(!nonEmptyVarName.empty());

        _dataMgr->GetVariableExtents(rp->GetCurrentTimestep(), nonEmptyVarName, rp->GetRefinementLevel(), rp->GetCompressionLevel(), mind, maxd);
        vec3  min(mind[0], mind[1], mind[2]);
        vec3  max(maxd[0], maxd[1], maxd[2]);
        vec3  lens = max - min;
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
    vec3 cameraDir = vec3(cameraDirD[0], cameraDirD[1], cameraDirD[2]);
    vec3 cameraPos = vec3(cameraPosD[0], cameraPosD[1], cameraPosD[2]);

    shader->Bind();
    shader->SetUniform("P", _glManager->matrixManager->GetProjectionMatrix());
    shader->SetUniform("MV", _glManager->matrixManager->GetModelViewMatrix());
    shader->SetUniform("aspect", _glManager->matrixManager->GetProjectionAspectRatio());
    shader->SetUniform("radius", radius);
    shader->SetUniform("lightingEnabled", true);
    shader->SetUniform("glyphStride", glyphStride);
    shader->SetUniform("showOnlyLeadingSample", (bool)rp->GetValueLong(FlowParams::RenderGlyphOnlyLeadingTag, false));
    shader->SetUniform("scales", _getScales());
    shader->SetUniform("cameraPos", cameraPos);
    if (rp->GetValueLong(FlowParams::RenderLightAtCameraTag, true))
        shader->SetUniform("lightDir", cameraDir);
    else
        shader->SetUniform("lightDir", vec3(0, 0, -1));
    shader->SetUniform("phongAmbient", (float)rp->GetValueDouble(FlowParams::PhongAmbientTag, 0));
    shader->SetUniform("phongDiffuse", (float)rp->GetValueDouble(FlowParams::PhongDiffuseTag, 0));
    shader->SetUniform("phongSpecular", (float)rp->GetValueDouble(FlowParams::PhongSpecularTag, 0));
    shader->SetUniform("phongShininess", (float)rp->GetValueDouble(FlowParams::PhongShininessTag, 0));

    shader->SetUniform("mapRange", glm::make_vec2(_colorMapRange));

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

glm::vec3 FlowRenderer::_getScales()
{
    string                  myVisName = GetVisualizer();
    VAPoR::ViewpointParams *vpp = _paramsMgr->GetViewpointParams(myVisName);
    string                  datasetName = GetMyDatasetName();
    Transform *             tDataset = vpp->GetTransform(datasetName);
    Transform *             tRenderer = GetActiveParams()->GetTransform();

    vector<double> scales = tDataset->GetScales();
    vector<double> rendererScales = tRenderer->GetScales();

    scales[0] *= rendererScales[0];
    scales[1] *= rendererScales[1];
    scales[2] *= rendererScales[2];

    return vec3(scales[0], scales[1], scales[2]);
}

int FlowRenderer::_renderFromAnAdvectionLegacy(const flow::Advection *adv, FlowParams *params, bool fast)
{
    size_t numOfStreams = adv->GetNumberOfStreams();
    auto   numOfPart = params->GetSteadyNumOfSteps() + 1;
    bool   singleColor = params->UseSingleColor();

    if (_cache_isSteady) {
        std::vector<float> vec;
        for (size_t s = 0; s < numOfStreams; s++) {
            const auto &stream = adv->GetStreamAt(s);
            for (size_t i = 0; i < stream.size() && i < numOfPart; i++) {
                const auto &p = stream[i];
                _particleHelper1(vec, p, singleColor);
            }    // Finish processing a stream
            if (!vec.empty()) {
                _drawALineStrip(vec.data(), vec.size() / 4, singleColor);
                vec.clear();
            }
        }     // Finish processing all streams
    } else    // Unsteady flow (only occurs with forward direction)
    {
        // First calculate the starting time stamp
        double startingTime = _timestamps[0];
        if (int(_cache_currentTS) - _cache_pastNumOfTimeSteps > 0) startingTime = _timestamps[_cache_currentTS - _cache_pastNumOfTimeSteps];

        std::vector<float> vec;
        for (size_t s = 0; s < numOfStreams; s++) {
            const auto &stream = adv->GetStreamAt(s);
            for (const auto &p : stream) {
                if (p.IsSpecial())    // If p is a separator, directly send it to the helper function
                {
                    _particleHelper1(vec, p, singleColor);
                } else    // Otherwise, examine its timestamp to decide how to handle
                {         // Finish this stream once we go beyond the current TS
                    if (p.time > _timestamps.at(_cache_currentTS)) break;

                    // Only start this stream if the current time stamp passes startingTime
                    if (p.time >= startingTime) _particleHelper1(vec, p, singleColor);
                }
            }    // Finish processing a stream

            if (!vec.empty()) {
                _drawALineStrip(vec.data(), vec.size() / 4, singleColor);
                vec.clear();
            }
        }    // Finish processing all streams
    }

    return 0;
}

void FlowRenderer::_particleHelper1(std::vector<float> &vec, const flow::Particle &p, bool singleColor) const
{
    if (!p.IsSpecial())    // p isn't a separator
    {
        vec.push_back(p.location.x);
        vec.push_back(p.location.y);
        vec.push_back(p.location.z);
        vec.push_back(p.value);
    } else if (vec.size() > 0)    // p is a separator and vec is non-empty
    {
        _drawALineStrip(vec.data(), vec.size() / 4, singleColor);
        vec.clear();
    }
}

int FlowRenderer::_drawALineStrip(const float *buf, size_t numOfParts, bool singleColor) const
{
    // Make some OpenGL function calls
    glm::mat4 modelview = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();
    _shader->Bind();
    _shader->SetUniform("MV", modelview);
    _shader->SetUniform("Projection", projection);
    _shader->SetUniform("colorMapRange", glm::make_vec3(_colorMapRange));
    _shader->SetUniform("singleColor", int(singleColor));
    Renderer::EnableClipToBox(_shader, 0.01);

    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    _shader->SetUniform("colorMapTexture", _colorMapTexOffset);

    glBindVertexArray(_vertexArrayId);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * numOfParts, buf, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glDrawArrays(GL_LINE_STRIP, 0, numOfParts);

    // Some OpenGL cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    glBindVertexArray(0);

    return 0;
}

int FlowRenderer::_updateFlowCacheAndStates(const FlowParams *params)
{
    /*
     * This function servers two purposes:
     * 1) update the cached parameters, and 2) determine if the advection is out of date.
     *
     * Strategy:
     * First, compare parameters that if changed, they would put both steady and unsteady
     * streams out of date.
     * Second, branch into steady and unsteady cases, and deal with them separately.
     */

    // Check seed generation mode
    if (_cache_seedGenMode != static_cast<FlowSeedMode>(params->GetSeedGenMode())) {
        _cache_seedGenMode = static_cast<FlowSeedMode>(params->GetSeedGenMode());
        _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Check seed input filename
    if (_cache_seedInputFilename != params->GetSeedInputFilename()) {
        _cache_seedInputFilename = params->GetSeedInputFilename();
        // we only update status if the current seed generation mode IS seed list.
        if (_cache_seedGenMode == FlowSeedMode::LIST) {
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }

    // Check velocity variable names
    // If names not the same, entire stream is out of date
    // Note: variable names are kept in VaporFields.
    // Note: RenderParams always returns arrays of size 3 here.
    std::vector<std::string> varnames = params->GetFieldVariableNames();
    if ((varnames.at(0) != _velocityField.VelocityNames[0]) || (varnames.at(1) != _velocityField.VelocityNames[1]) || (varnames.at(2) != _velocityField.VelocityNames[2])) {
        _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        // When new velocity variables are selected, new particles will be generated,
        // so we should declare color status out of date too.
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Check color mapping variable names
    std::string colorVarName = params->GetColorMapVariableName();
    if (colorVarName != _colorField.ScalarName) { _colorStatus = FlowStatus::SIMPLE_OUTOFDATE; }

    // Check compression parameters
    // If these parameters not the same, entire stream is out of date
    if (_cache_refinementLevel != params->GetRefinementLevel()) {
        _cache_refinementLevel = params->GetRefinementLevel();
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
    }
    if (_cache_compressionLevel != params->GetCompressionLevel()) {
        _cache_compressionLevel = params->GetCompressionLevel();
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Check velocity multiplier
    // If the multiplier is changed, then the entire stream is out of date
    const float mult = params->GetVelocityMultiplier();
    if (_cache_velocityMltp != mult) {
        _cache_velocityMltp = mult;
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Check periodicity
    // If periodicity changes along any dimension, then the entire stream is out of date
    // Note: FlowParams return a vector of size either 2 or 3.
    bool       diff = false;
    const auto peri = params->GetPeriodic();
    if (peri.size() != _cache_periodic.size())
        diff = true;
    else {
        for (int i = 0; i < peri.size(); i++)
            if (peri[i] != _cache_periodic[i]) {
                diff = true;
                break;
            }
    }
    if (diff) {
        _cache_periodic = peri;
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Check the rake defined by 6 or 4 extents.
    // We update these parameters anyway, and decide if the advection is out of date in rake mode.
    diff = false;
    const auto rake = params->GetRake();
    if (rake.size() != _cache_rake.size())
        diff = true;
    else {
        for (int i = 0; i < rake.size(); i++)
            if (rake[i] != _cache_rake[i]) {
                diff = true;
                break;
            }
    }
    if (diff) {
        _cache_rake = rake;
        // Mark out-of-date if we're currently using any mode that involves a rake
        if (_cache_seedGenMode != FlowSeedMode::LIST) {
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }

    // Check the gridded number of seeds in the rake
    diff = false;
    const auto gridNOS = params->GetGridNumOfSeeds();
    if (gridNOS.size() != _cache_gridNumOfSeeds.size())
        diff = true;
    else {
        for (int i = 0; i < gridNOS.size(); i++)
            if (gridNOS[i] != _cache_gridNumOfSeeds[i]) {
                diff = true;
                break;
            }
    }
    if (diff) {
        _cache_gridNumOfSeeds = gridNOS;
        if (_cache_seedGenMode == FlowSeedMode::UNIFORM) {
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }

    // Check the random num of seeds
    const auto randNOS = params->GetRandomNumOfSeeds();
    if (randNOS != _cache_randNumOfSeeds) {
        _cache_randNumOfSeeds = randNOS;
        if (_cache_seedGenMode == FlowSeedMode::RANDOM || _cache_seedGenMode == FlowSeedMode::RANDOM_BIAS) {
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }

    // Check the bias variable and bias strength
    const auto rakeBiasVariable = params->GetRakeBiasVariable();
    const auto rakeBiasStrength = params->GetRakeBiasStrength() * 100;
    if (_cache_rakeBiasStrength != rakeBiasStrength || _cache_rakeBiasVariable.compare(rakeBiasVariable) != 0) {
        _cache_rakeBiasVariable = rakeBiasVariable;
        _cache_rakeBiasStrength = rakeBiasStrength;

        if (_cache_seedGenMode == FlowSeedMode::RANDOM_BIAS) {
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }

    //
    // Now we branch into steady and unsteady cases, and treat them separately
    //
    if (params->GetIsSteady()) {
        if (_cache_isSteady)    // steady state isn't changed
        {
            if (params->GetSteadyNumOfSteps() > _cache_steadyNumOfSteps) {
                if (_colorStatus == FlowStatus::UPTODATE) _colorStatus = FlowStatus::TIME_STEP_OOD;
                if (_velocityStatus == FlowStatus::UPTODATE) _velocityStatus = FlowStatus::TIME_STEP_OOD;
            }
            if (params->GetSteadyNumOfSteps() != _cache_steadyNumOfSteps) _renderStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();

            if (_cache_currentTS != params->GetCurrentTimestep()) {
                _cache_currentTS = params->GetCurrentTimestep();
                _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
                _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
            }
        } else    // switched from unsteady to steady. Everything is out of date in this case.
        {
            _cache_isSteady = true;
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();
            _cache_currentTS = params->GetCurrentTimestep();
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }

        if (_cache_flowDir != static_cast<FlowDir>(params->GetFlowDirection())) {
            _cache_flowDir = static_cast<FlowDir>(params->GetFlowDirection());
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
            if (_cache_flowDir == FlowDir::BI_DIR)
                _2ndAdvection.reset(new flow::Advection());
            else
                _2ndAdvection.reset(nullptr);
        }
    } else    // in case of unsteady flow
    {
        if (!_cache_isSteady)    // unsteady state isn't changed
        {
            // First consider if the advection needs to be updated.
            if (_cache_currentTS < params->GetCurrentTimestep()) {
                if (_colorStatus == FlowStatus::UPTODATE) { _colorStatus = FlowStatus::TIME_STEP_OOD; }
                if (_velocityStatus == FlowStatus::UPTODATE) { _velocityStatus = FlowStatus::TIME_STEP_OOD; }
            }

            // Second consider if the rendering needs to be updated.
            if (_cache_currentTS != params->GetCurrentTimestep() || _cache_pastNumOfTimeSteps != params->GetPastNumOfTimeSteps()) { _renderStatus = FlowStatus::SIMPLE_OUTOFDATE; }
            _cache_currentTS = params->GetCurrentTimestep();
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();
            _cache_pastNumOfTimeSteps = params->GetPastNumOfTimeSteps();
        } else    // switched from steady to unsteady
        {
            _cache_isSteady = false;
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();
            _cache_currentTS = params->GetCurrentTimestep();
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }

        if (_cache_seedInjInterval != params->GetSeedInjInterval()) {
            _cache_seedInjInterval = params->GetSeedInjInterval();
            _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }

    return 0;
}

void FlowRenderer::_dupSeedsNewTime(std::vector<flow::Particle> &seeds, size_t firstN, double newTime) const
{
    VAssert(firstN <= seeds.size());
    for (size_t i = 0; i < firstN; i++) seeds.emplace_back(seeds[i].location, newTime);
}

int FlowRenderer::_genSeedsRakeUniform(std::vector<flow::Particle> &seeds) const
{
    FlowParams *params = dynamic_cast<FlowParams *>(GetActiveParams());
    VAssert(params);

    // sanity check: rake extents and uniform seed numbers match dims
    size_t dim = _cache_gridNumOfSeeds.size();
    VAssert(dim == 2 || dim == 3);
    VAssert(_cache_rake.size() == dim * 2);

    // Populate the list of seeds
    float start[3], step[3];
    for (size_t i = 0; i < dim; i++) {
        step[i] = (_cache_rake[i * 2 + 1] - _cache_rake[i * 2]) / float(_cache_gridNumOfSeeds[i]);
        start[i] = _cache_rake[i * 2];
    }
    if (dim == 2) {
        start[2] = Renderer::GetDefaultZ(_dataMgr, params->GetCurrentTimestep());
        step[2] = 0.0f;
    }
    size_t seedsZ = 1;
    if (dim == 3) { seedsZ = _cache_gridNumOfSeeds[2]; }

    auto      timeVal = _timestamps.at(0);    // Default time value
    glm::vec3 loc;
    seeds.clear();
    seeds.reserve(seedsZ * _cache_gridNumOfSeeds[1] * _cache_gridNumOfSeeds[0]);
    for (long k = 0; k < seedsZ; k++) {
        for (long j = 0; j < _cache_gridNumOfSeeds[1]; j++) {
            for (long i = 0; i < _cache_gridNumOfSeeds[0]; i++) {
                loc.x = start[0] + (float(i) + 0.5f) * step[0];
                loc.y = start[1] + (float(j) + 0.5f) * step[1];
                loc.z = start[2] + (float(k) + 0.5f) * step[2];
                seeds.emplace_back(loc, timeVal);
            }
        }
    }

    // If in unsteady case and there are multiple seed injections,
    //   we insert more seeds.
    if (!_cache_isSteady && _cache_seedInjInterval > 0) {
        size_t firstN = seeds.size();
        // Check every time step available, see if we need to inject seeds at that time step
        for (size_t ts = 1; ts < _timestamps.size(); ts++)
            if (ts % _cache_seedInjInterval == 0) { _dupSeedsNewTime(seeds, firstN, _timestamps[ts]); }
    }

    return 0;
}

int FlowRenderer::_genSeedsFromList(std::vector<flow::Particle> &seeds) const
{
    FlowParams *params = dynamic_cast<FlowParams *>(GetActiveParams());
    VAssert(params);

    // Read seed locations (X, Y, Z) from a file.
    std::vector<flow::Particle> read_from_disk = flow::InputSeedsCSV(params->GetSeedInputFilename());
    if (read_from_disk.empty()) return flow::NO_SEED_PARTICLE_YET;

    // Set seed time to be the time stamp at step 0
    double timeVal = _timestamps.at(0);
    for (auto &seed : read_from_disk) seed.time = timeVal;

    seeds = std::move(read_from_disk);

    // If in unsteady case and there are multiple seed injections, we insert more seeds.
    if (!_cache_isSteady && _cache_seedInjInterval > 0) {
        size_t firstN = seeds.size();
        // Check every time step available, see if we need to inject seeds at that time step
        for (size_t ts = 1; ts < _timestamps.size(); ts++) {
            if (ts % _cache_seedInjInterval == 0) { _dupSeedsNewTime(seeds, firstN, _timestamps[ts]); }
        }
    }

    return 0;
}

int FlowRenderer::_genSeedsRakeRandom(std::vector<flow::Particle> &seeds) const
{
    FlowParams *params = dynamic_cast<FlowParams *>(GetActiveParams());

    VAssert(_cache_rake.size() == 6 || _cache_rake.size() == 4);
    int dim = _cache_rake.size() / 2;
    for (int i = 0; i < dim; i++) VAssert(_cache_rake[i * 2 + 1] >= _cache_rake[i * 2]);

    /* Create uniform distributions along 2 or 3 dimensions */
    /* Use a fixed value for the generator seed.          */
    unsigned int                          randSeed = 32;
    std::mt19937                          gen(randSeed);    // Standard mersenne_twister_engine
    std::uniform_real_distribution<float> distX(_cache_rake[0], _cache_rake[1]);
    std::uniform_real_distribution<float> distY(_cache_rake[2], _cache_rake[3]);

    auto timeVal = _timestamps.at(0);
    seeds.resize(_cache_randNumOfSeeds);
    if (dim == 3) {
        std::uniform_real_distribution<float> distZ(_cache_rake[4], _cache_rake[5]);
        for (long i = 0; i < _cache_randNumOfSeeds; i++) {
            seeds[i].location.x = distX(gen);
            seeds[i].location.y = distY(gen);
            seeds[i].location.z = distZ(gen);
            seeds[i].time = timeVal;
        }
    } else {
        const float dfz = Renderer::GetDefaultZ(_dataMgr, params->GetCurrentTimestep());
        for (long i = 0; i < _cache_randNumOfSeeds; i++) {
            seeds[i].location.x = distX(gen);
            seeds[i].location.y = distY(gen);
            seeds[i].location.z = dfz;
            seeds[i].time = timeVal;
        }
    }

    // If in unsteady case and there are multiple seed injections, we insert more seeds.
    if (!_cache_isSteady && _cache_seedInjInterval > 0) {
        size_t firstN = seeds.size();
        // Check every time step available, see if we need to inject seeds at that time step
        for (size_t ts = 1; ts < _timestamps.size(); ts++) {
            if (ts % _cache_seedInjInterval == 0) { _dupSeedsNewTime(seeds, firstN, _timestamps[ts]); }
        }
    }

    return 0;
}

int FlowRenderer::_genSeedsRakeRandomBiased(std::vector<flow::Particle> &seeds) const
{
    FlowParams *params = dynamic_cast<FlowParams *>(GetActiveParams());

    VAssert(_cache_rake.size() == 6 || _cache_rake.size() == 4);
    int dim = _cache_rake.size() / 2;
    for (int i = 0; i < dim; i++) VAssert(_cache_rake[i * 2 + 1] >= _cache_rake[i * 2]);
    std::vector<double> rakeExtMin(dim, 0);
    std::vector<double> rakeExtMax(dim, 0);
    for (int i = 0; i < dim; i++) {
        rakeExtMin[i] = _cache_rake[i * 2];
        rakeExtMax[i] = _cache_rake[i * 2 + 1];
    }

    const auto numOfSeedsNeeded = _cache_randNumOfSeeds;

    /* request a grid representing the rake area */
    Grid *grid = _dataMgr->GetVariable(params->GetCurrentTimestep(), _cache_rakeBiasVariable, params->GetRefinementLevel(), params->GetCompressionLevel(), rakeExtMin, rakeExtMax);
    if (grid == nullptr) {
        MyBase::SetErrMsg("Not able to get a grid!");
        return flow::GRID_ERROR;
    }

    /*
     * The bias strategy is:
     * We generate more random seeds than needed, and then sort them.
     * The first batch of these seeds are used as the final seeds.
     */

    /* Create three uniform distributions in 3 dimensions */
    unsigned int                          procID = 32;
    std::mt19937                          gen(procID);    // Standard mersenne_twister engine
    std::uniform_real_distribution<float> distX(_cache_rake[0], _cache_rake[1]);
    std::uniform_real_distribution<float> distY(_cache_rake[2], _cache_rake[3]);

    // Now we generate many seeds.
    // We test missing values in case 1) the bias variable does have missing values, and 2)
    // the rake extents are outside of the bias variable.
    // Thus, we only keep random seeds that are falling on non-missing-value locations.
    glm::vec3           loc;
    std::vector<double> locD(3);
    auto                timeVal = _timestamps.at(0);
    // This is the total number of seeds to generate, based on the bias strength.
    auto numOfSeedsToGen = numOfSeedsNeeded * (std::abs(_cache_rakeBiasStrength) + 1);
    long numOfTrials = 0;
    seeds.clear();
    seeds.reserve(numOfSeedsToGen);    // For performance reasons
    // Note: in the case that too many random seeds fall on missing values,
    // we set a limit of 10 times numOfSeedsToGen.
    long  numOfTrialLimit = 10 * numOfSeedsToGen;
    float val, mv = grid->GetMissingValue();
    if (dim == 3) {
        std::uniform_real_distribution<float> distZ(_cache_rake[4], _cache_rake[5]);
        while (numOfTrials < numOfTrialLimit && seeds.size() < numOfSeedsToGen) {
            loc.x = distX(gen);
            locD[0] = loc.x;
            loc.y = distY(gen);
            locD[1] = loc.y;
            loc.z = distZ(gen);
            locD[2] = loc.z;
            val = grid->GetValue(locD);
            if (val != mv) seeds.emplace_back(loc, timeVal, val);
            numOfTrials++;
        }
    } else    // dim == 2
    {
        const auto dfz = Renderer::GetDefaultZ(_dataMgr, params->GetCurrentTimestep());
        loc.z = dfz;
        locD[2] = dfz;
        while (numOfTrials < numOfTrialLimit && seeds.size() < numOfSeedsToGen) {
            loc.x = distX(gen);
            locD[0] = loc.x;
            loc.y = distY(gen);
            locD[1] = loc.y;
            val = grid->GetValue(locD);
            if (val != mv) seeds.emplace_back(loc, timeVal, val);
            numOfTrials++;
        }
    }

    delete grid;    // Delete the temporary grid

    // If we reach numOfTrialLimit without collecting enough seeds, bail.
    if (numOfTrials == numOfTrialLimit && seeds.size() < numOfSeedsNeeded) {
        seeds.clear();
        return flow::GRID_ERROR;
    }

    // How we sort all seeds based on their values
    auto ascLambda = [](const flow::Particle &p1, const flow::Particle &p2) { return p1.value < p2.value; };
    auto desLambda = [](const flow::Particle &p1, const flow::Particle &p2) { return p2.value < p1.value; };
    if (_cache_rakeBiasStrength < 0) {
        std::nth_element(seeds.begin(), seeds.begin() + numOfSeedsNeeded, seeds.end(), ascLambda);
    } else {
        std::nth_element(seeds.begin(), seeds.begin() + numOfSeedsNeeded, seeds.end(), desLambda);
    }

    seeds.resize(numOfSeedsNeeded);    // We only take first chunck of seeds that we need
    seeds.shrink_to_fit();             // Free up some memory
    for (auto &e : seeds) {            // reset the value field of each particle
        e.value = 0.0;
    }

    // If in unsteady case and there are multiple seed injections, we insert more seeds.
    if (!_cache_isSteady && _cache_seedInjInterval > 0) {
        size_t firstN = seeds.size();
        // Check every time step available, see if we need to inject seeds at that time step
        for (size_t ts = 1; ts < _timestamps.size(); ts++) {
            if (ts % _cache_seedInjInterval == 0) { _dupSeedsNewTime(seeds, firstN, _timestamps[ts]); }
        }
    }

    return 0;
}

void FlowRenderer::_prepareColormap(FlowParams *params)
{
    if (params->UseSingleColor()) {
        float singleColor[4];
        params->GetConstantColor(singleColor);
        singleColor[3] = 1.0f;    // 1.0 in alpha channel
        _colorMap.resize(8);      // _colorMap will have 2 RGBA values
        for (int i = 0; i < 8; i++) _colorMap[i] = singleColor[i % 4];
        _colorMapRange[0] = 0.0f;     // min value of the color map
        _colorMapRange[1] = 0.0f;     // max value of the color map
        _colorMapRange[2] = 1e-5f;    // diff of color map. Has to be non-zero though.
    } else {
        // This is the line that's not const
        VAPoR::MapperFunction *mapperFunc = params->GetMapperFunc(params->GetColorMapVariableName());
        mapperFunc->makeLut(_colorMap);
        assert(_colorMap.size() % 4 == 0);
        std::vector<double> range = mapperFunc->getMinMaxMapValue();
        _colorMapRange[0] = float(range[0]);
        _colorMapRange[1] = float(range[1]);
        _colorMapRange[2] = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ? (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f;
    }
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTexId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());
}

void FlowRenderer::_restoreGLState() const
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);
}

int FlowRenderer::_updateAdvectionPeriodicity(flow::Advection *advc)
{
    glm::vec3 minxyz, maxxyz;
    int       rv = _velocityField.GetVelocityIntersection(_cache_currentTS, minxyz, maxxyz);
    if (rv != 0) return rv;

    if (_cache_periodic[0])
        advc->SetXPeriodicity(true, minxyz.x, maxxyz.x);
    else
        advc->SetXPeriodicity(false, 0.0f, 1.0f);

    if (_cache_periodic[1])
        advc->SetYPeriodicity(true, minxyz.y, maxxyz.y);
    else
        advc->SetYPeriodicity(false, 0.0f, 1.0f);

    if (_cache_periodic.size() == 2)
        advc->SetZPeriodicity(false, 0.0f, 1.0f);
    else {
        if (_cache_periodic[2])
            advc->SetZPeriodicity(true, minxyz.z, maxxyz.z);
        else
            advc->SetZPeriodicity(false, 0.0f, 1.0f);
    }

    return 0;
}

void FlowRenderer::_printFlowStatus(const std::string &prefix, FlowStatus stat) const
{
    std::cout << prefix << " :  ";
    if (stat == FlowStatus::SIMPLE_OUTOFDATE)
        std::cout << "simple out-of-date";
    else if (stat == FlowStatus::TIME_STEP_OOD)
        std::cout << "time step out-of-date";
    else if (stat == FlowStatus::UPTODATE)
        std::cout << "up to date";
    std::cout << std::endl;
}

#ifndef WIN32
double FlowRenderer::_getElapsedSeconds(const struct timeval *begin, const struct timeval *end) const { return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec) / 1000000.0); }
#endif
