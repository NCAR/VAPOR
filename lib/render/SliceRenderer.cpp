#include <sstream>
#include <string>
#include <limits>
#include <ctime>
#include <iomanip>

#include <vapor/SliceRenderer.h>
#include <vapor/SliceParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/LegacyGL.h>
#include <vapor/GLManager.h>
#include <vapor/ResourcePath.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/ArbitrarilyOrientedRegularGrid.h>

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

using namespace VAPoR;

static vector<double> ToDoubleVec(const glm::vec3 &v)
{
    vector<double> c(v.length());
    for (int i = 0; i < v.length(); i++) c[i] = v[i];
    return c;
}

static RendererRegistrar<SliceRenderer> registrar(SliceRenderer::GetClassType(), SliceParams::GetClassType());

SliceRenderer::SliceRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instanceName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, SliceParams::GetClassType(), SliceRenderer::GetClassType(), instanceName, dataMgr)
{
    _initialized = false;

    _windingOrder = {0.0f, 0.0f, 0.f, 1.0f, 0.0f, 0.f, 0.0f, 1.0f, 0.f, 1.0f, 0.0f, 0.f, 1.0f, 1.0f, 0.f, 0.0f, 1.0f, 0.f};
    _rectangle3D = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    _VAO = 0;
    _vertexVBO = 0;
    _texCoordVBO = 0;
    _colorMapTextureID = 0;
    _dataValueTextureID = 0;

    _cacheParams.textureSampleRate = 200;

    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);
    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    _colorMapSize = tf->getNumEntries();
}

SliceRenderer::~SliceRenderer()
{
    if (_VAO != 0) {
        glDeleteVertexArrays(1, &_VAO);
        _VAO = 0;
    }

    if (_vertexVBO != 0) {
        glDeleteBuffers(1, &_vertexVBO);
        _vertexVBO = 0;
    }

    if (_texCoordVBO != 0) {
        glDeleteBuffers(1, &_texCoordVBO);
        _texCoordVBO = 0;
    }

    if (_colorMapTextureID != 0) {
        glDeleteTextures(1, &_colorMapTextureID);
        _colorMapTextureID = 0;
    }

    if (_dataValueTextureID != 0) {
        glDeleteTextures(1, &_dataValueTextureID);
        _dataValueTextureID = 0;
    }
}

int SliceRenderer::_initializeGL()
{
    _initVAO();
    _initTexCoordVBO();
    _initVertexVBO();

    _initialized = true;

    return 0;
}

void SliceRenderer::_initVAO()
{
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);
}

void SliceRenderer::_initTexCoordVBO()
{
    if (_texCoordVBO != 0) glDeleteBuffers(1, &_texCoordVBO);

    glGenBuffers(1, &_texCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
    glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(double) * _rectangle3D.size(), _rectangle3D.data(), GL_DYNAMIC_DRAW);
}

void SliceRenderer::_initVertexVBO()
{
    if (_vertexVBO != 0) glDeleteBuffers(1, &_vertexVBO);

    glGenBuffers(1, &_vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(double), _windingOrder.data(), GL_STATIC_DRAW);
}

void SliceRenderer::_resetCache()
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);

    _cacheParams.varName = p->GetVariableName();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.refinementLevel = p->GetRefinementLevel();
    _cacheParams.compressionLevel = p->GetCompressionLevel();
    _cacheParams.orientation = p->GetBox()->GetOrientation();

    _cacheParams.xRotation = p->GetValueDouble(RenderParams::XSlicePlaneRotationTag, 0);
    _cacheParams.yRotation = p->GetValueDouble(RenderParams::YSlicePlaneRotationTag, 0);
    _cacheParams.zRotation = p->GetValueDouble(RenderParams::ZSlicePlaneRotationTag, 0);

    _cacheParams.xOrigin = p->GetXSlicePlaneOrigin();
    _cacheParams.yOrigin = p->GetYSlicePlaneOrigin();
    _cacheParams.zOrigin = p->GetZSlicePlaneOrigin();


    _cacheParams.textureSampleRate = p->GetValueDouble(RenderParams::SampleRateTag, 200);
    _cacheParams.sliceOrientationMode = p->GetValueLong(RenderParams::SlicePlaneOrientationModeTag, 0);

    _cacheParams.sliceRotation = p->GetSlicePlaneRotation();
    _cacheParams.sliceNormal = p->GetSlicePlaneNormal();
    _cacheParams.sliceOffset = p->GetValueDouble(p->SliceOffsetTag, 0);

    _getExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    // clang-format off
    _dataMgr->GetVariableExtents(_cacheParams.ts, 
                                 _cacheParams.varName, 
                                 _cacheParams.refinementLevel, 
                                 _cacheParams.compressionLevel, 
                                 _cacheParams.domainMin, 
                                 _cacheParams.domainMax
    );
    // clang-format on
}

void SliceRenderer::_resetColormapCache()
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    tf->makeLut(_cacheParams.tf_lut);
    _cacheParams.tf_minMax = tf->getMinMaxMapValue();

    if (_colorMapTextureID != 0) glDeleteTextures(1, &_colorMapTextureID);

    glGenTextures(1, &_colorMapTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureID);
#ifdef DEBUG
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#else
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, _colorMapSize, 0, GL_RGBA, GL_FLOAT, &_cacheParams.tf_lut[0]);
}

// clang-format off

int SliceRenderer::_regenerateSlice()
{
    Grid *grid3d = nullptr;
    int   rc = _getGrid3D(grid3d);
    if (rc < 0) return -1;

    // Get data values from a slice
    std::shared_ptr<float> dataValues(new float[_textureSideSize * _textureSideSize]);
    planeDescription       pd;
    pd.sideSize = _textureSideSize;

    if (_cacheParams.sliceOrientationMode == (int)RenderParams::SlicePlaneOrientationMode::Normal)
        pd.normal = _cacheParams.sliceNormal;
    else
        pd.normal = ToDoubleVec(ArbitrarilyOrientedRegularGrid::GetNormalFromRotations(_cacheParams.sliceRotation));

    glm::vec3 o = {_cacheParams.xOrigin, _cacheParams.yOrigin, _cacheParams.zOrigin};
    glm::vec3 n = {pd.normal[0], pd.normal[1], pd.normal[2]};
    glm::vec3 offsetOrigin = o + n * (float)_cacheParams.sliceOffset;
    pd.origin = ToDoubleVec(offsetOrigin);

    auto normal = ArbitrarilyOrientedRegularGrid::GetNormalFromRotations({_cacheParams.xRotation, _cacheParams.yRotation, _cacheParams.zRotation});
    pd.normal = {normal[0], normal[1], normal[2]};
    pd.boxMin = _cacheParams.boxMin;
    pd.boxMax = _cacheParams.boxMax;

    VAPoR::DimsType dims = { (size_t)_textureSideSize, (size_t)_textureSideSize, 1 };

    ArbitrarilyOrientedRegularGrid* slice = new ArbitrarilyOrientedRegularGrid(
        grid3d,
        pd,
        dims
    );

    CoordType corner1, corner2, corner3, corner4;
    slice->GetUserCoordinates( {0,                  0,                  0}, corner1);
    slice->GetUserCoordinates( {0,                  _textureSideSize-1, 0}, corner2);
    slice->GetUserCoordinates( {_textureSideSize-1, 0,                  0}, corner3);
    slice->GetUserCoordinates( {_textureSideSize-1, _textureSideSize-1, 0}, corner4);

    _windingOrder = {corner1[0], corner1[1], corner1[2],
                     corner3[0], corner3[1], corner3[2],
                     corner2[0], corner2[1], corner2[2],
                     corner3[0], corner3[1], corner3[2],
                     corner4[0], corner4[1], corner4[2],
                     corner2[0], corner2[1], corner2[2]};

    _rectangle3D  = {corner1[0], corner1[1], corner1[2],
                     corner3[0], corner3[1], corner3[2],
                     corner4[0], corner4[1], corner4[2],
                     corner2[0], corner2[1], corner2[2]};

    delete grid3d;
    if (slice == nullptr) {
        Wasp::MyBase::SetErrMsg("Unable to perform SliceGridAlongPlane() with current Grid");
        return -1;
    }

    // Apply opacity to missing values
    float missingValue = slice->GetMissingValue();
    int                    textureSize = 2 * _textureSideSize * _textureSideSize;
    std::unique_ptr<float> textureValues(new float[textureSize]);
    for (size_t i = 0; i < textureSize / 2; i++) {
        DimsType dim = {i%_textureSideSize, i/_textureSideSize, 0};
        float dataValue = slice->GetValueAtIndex( dim );
        if (dataValue == missingValue)
            textureValues.get()[i * 2 + 1] = 1.f;
        else
            textureValues.get()[i * 2 + 1] = 0.f;

        textureValues.get()[i * 2] = dataValue;
    }

    _createDataTexture(textureValues);

    delete slice;

    return 0;
}

// clang-format on

int SliceRenderer::_getGrid3D(Grid *&grid3d) const
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    int          rLevel = p->GetRefinementLevel();
    int          cLevel = p->GetCompressionLevel();
    int          rc = DataMgrUtils::GetGrids(_dataMgr, p->GetCurrentTimestep(), p->GetVariableName(), _cacheParams.boxMin, _cacheParams.boxMax, true, &rLevel, &cLevel, &grid3d);
    if (rc < 0) {
        Wasp::MyBase::SetErrMsg("Unable to acquire Grid for Slice texture");
        return rc;
    }
    VAssert(grid3d);
    grid3d->SetInterpolationOrder(1);

    return 0;
}

void SliceRenderer::_createDataTexture(std::unique_ptr<float> &dataValues)
{
    if (_dataValueTextureID != 0) glDeleteTextures(1, &_dataValueTextureID);

    glGenTextures(1, &_dataValueTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _dataValueTextureID);
#ifdef DEBUG
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _textureSideSize, _textureSideSize, 0, GL_RG, GL_FLOAT, dataValues.get());

    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * sizeof(double), _windingOrder.data());
}

bool SliceRenderer::_isDataCacheDirty() const
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);

    if (_cacheParams.varName != p->GetVariableName()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.refinementLevel != p->GetRefinementLevel()) return true;
    if (_cacheParams.compressionLevel != p->GetCompressionLevel()) return true;

    if (_cacheParams.xRotation != p->GetValueDouble(RenderParams::XSlicePlaneRotationTag, 0)) return true;
    if (_cacheParams.yRotation != p->GetValueDouble(RenderParams::YSlicePlaneRotationTag, 0)) return true;
    if (_cacheParams.zRotation != p->GetValueDouble(RenderParams::ZSlicePlaneRotationTag, 0)) return true;

    if (_cacheParams.xOrigin != p->GetXSlicePlaneOrigin()) return true;
    if (_cacheParams.yOrigin != p->GetYSlicePlaneOrigin()) return true;
    if (_cacheParams.zOrigin != p->GetZSlicePlaneOrigin()) return true;

    if (_cacheParams.sliceOffset != p->GetValueDouble(p->SliceOffsetTag, 0)) return true;
    if (_cacheParams.sliceNormal != p->GetSlicePlaneNormal()) return true;

    if (_cacheParams.textureSampleRate != p->GetValueDouble(RenderParams::SampleRateTag, 200)) return true;

    return false;
}

bool SliceRenderer::_isColormapCacheDirty() const
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    vector<float>   tf_lut;
    tf->makeLut(tf_lut);
    if (_cacheParams.tf_lut != tf_lut) return true;
    if (_cacheParams.tf_minMax != tf->getMinMaxMapValue()) return true;
    return false;
}

bool SliceRenderer::_isBoxCacheDirty() const
{
    VAPoR::CoordType min, max;
    _getExtents(min, max);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;
    return false;
}

void SliceRenderer::_getExtents(VAPoR::CoordType &min, VAPoR::CoordType &max) const
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);
    Box *box = p->GetBox();

    box->GetExtents(min, max);
    VAssert(min.size() == 3);
    VAssert(max.size() == 3);
}

int SliceRenderer::_paintGL(bool fast)
{
    int rc = 0;

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);


#ifdef DEBUG
    _drawDebugPolygons();
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    _initializeState();

    if (_isDataCacheDirty() || _isBoxCacheDirty()) {
        _resetCache();

        // If we're in fast mode, degrade the quality of the slice for better interactivity
        if (fast) {
            _textureSideSize = 50;
        } else {
            _textureSideSize = _cacheParams.textureSampleRate;
        }

        int rc = _regenerateSlice();
        if (rc < 0) return -1;
    }

    if (_isColormapCacheDirty()) _resetColormapCache();

    _configureShader();
    if (CheckGLError() != 0) {
        _resetState();
        return -1;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _dataValueTextureID);

    glBindVertexArray(_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    _resetState();

    if (CheckGLError() != 0) { return -1; }

    return rc;
}

#ifdef DEBUG
void SliceRenderer::_drawDebugPolygons()
{
    // 3D yellow enclosing rectangle that defines the perimeter of our texture
    // This can and often will extend beyond the Box
    // std::vector<glm::vec3> rectangle = _slicer->GetRectangle();
    LegacyGL *lgl = _glManager->legacy;
    lgl->Begin(GL_LINES);
    lgl->Color4f(1., 1., 0., 1.);
    lgl->Vertex3f(_rectangle3D[0], _rectangle3D[1], _rectangle3D[2]);
    lgl->Vertex3f(_rectangle3D[3], _rectangle3D[4], _rectangle3D[5]);

    lgl->Vertex3f(_rectangle3D[3], _rectangle3D[4], _rectangle3D[5]);
    lgl->Vertex3f(_rectangle3D[6], _rectangle3D[7], _rectangle3D[8]);

    lgl->Vertex3f(_rectangle3D[6], _rectangle3D[7], _rectangle3D[8]);
    lgl->Vertex3f(_rectangle3D[9], _rectangle3D[10], _rectangle3D[11]);

    lgl->Vertex3f(_rectangle3D[9], _rectangle3D[10], _rectangle3D[11]);
    lgl->Vertex3f(_rectangle3D[0], _rectangle3D[1], _rectangle3D[2]);
    lgl->End();


    // Winding order
    double *wo = _windingOrder.data();
    lgl = _glManager->legacy;
    lgl->Begin(GL_LINES);
    lgl->Color4f(0., 1., 0., 1.);    // green
    lgl->Vertex3f(wo[0], wo[1], wo[2]);
    lgl->Vertex3f(wo[3], wo[4], wo[5]);
    lgl->Color4f(0., 1., 1., 1.);    // teal
    lgl->Vertex3f(wo[3], wo[4], wo[5]);
    lgl->Vertex3f(wo[6], wo[7], wo[8]);
    lgl->Color4f(1., 1., 1., 1.);    // white
    lgl->Vertex3f(wo[6], wo[7], wo[8]);
    lgl->Vertex3f(wo[0], wo[1], wo[2]);

    lgl->Color4f(.9, .9, 1., 1.);    // purple
    lgl->Vertex3f(wo[9], wo[10], wo[11]);
    lgl->Vertex3f(wo[12], wo[13], wo[14]);
    lgl->Color4f(1., 0., 0., 1.);    // red
    lgl->Vertex3f(wo[12], wo[13], wo[14]);
    lgl->Vertex3f(wo[15], wo[16], wo[17]);
    lgl->Color4f(0., 1., 1., 1.);    // yellow
    lgl->Vertex3f(wo[15], wo[16], wo[17]);
    lgl->Vertex3f(wo[9], wo[10], wo[11]);
    lgl->End();
}
#endif

void SliceRenderer::_configureShader()
{
    ShaderProgram *s = _glManager->shaderManager->GetShader("Slice");
    s->Bind();

    // One vertex shader uniform vec4
    s->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());

    // Remaining fragment shader uniform floats
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);
    float opacity = p->GetConstantOpacity();
    s->SetUniform("constantOpacity", opacity);
    s->SetUniform("minLUTValue", (float)_cacheParams.tf_minMax[0]);
    s->SetUniform("maxLUTValue", (float)_cacheParams.tf_minMax[1]);

    // And finally our uniform samplers
    GLint colormapLocation;
    colormapLocation = s->GetUniformLocation("colormap");
    glUniform1i(colormapLocation, 0);

    GLint dataValuesLocation;
    dataValuesLocation = s->GetUniformLocation("dataValues");
    glUniform1i(dataValuesLocation, 1);
}

void SliceRenderer::_initializeState()
{
    _glManager->matrixManager->MatrixModeModelView();
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
}

void SliceRenderer::_resetState()
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ShaderProgram::UnBind();

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}
