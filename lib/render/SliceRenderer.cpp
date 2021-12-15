#include <sstream>
#include <string>
#include <limits>
#include <ctime>

#include <vapor/SliceRenderer.h>
#include <vapor/SliceParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/LegacyGL.h>
#include <vapor/GLManager.h>
#include <vapor/ResourcePath.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/ConvexHull.h>
#include <vapor/Slicer.h>

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

#define DEBUG 1

using namespace VAPoR;

static RendererRegistrar<SliceRenderer> registrar(SliceRenderer::GetClassType(), SliceParams::GetClassType());

SliceRenderer::SliceRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instanceName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, SliceParams::GetClassType(), SliceRenderer::GetClassType(), instanceName, dataMgr)
{
    _initialized = false;

    _vertexCoords = {0.0f, 0.0f, 0.f, 1.0f, 0.0f, 0.f, 0.0f, 1.0f, 0.f, 1.0f, 0.0f, 0.f, 1.0f, 1.0f, 0.f, 0.0f, 1.0f, 0.f};

    _texCoords = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    _VAO = 0;
    _vertexVBO = 0;
    _texCoordVBO = 0;
    _colorMapTextureID = 0;
    _dataValueTextureID = 0;

    _cacheParams.domainMin.resize(3, 0.f);
    _cacheParams.domainMax.resize(3, 1.f);
    _cacheParams.textureSampleRate = 200;

    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);
    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    _colorMapSize = tf->getNumEntries();
    
    _slicer = new Slicer( p, _dataMgr );
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

    if (_slicer != nullptr) {
        delete _slicer;
        _slicer = nullptr;
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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _texCoords.size(), _texCoords.data(), GL_DYNAMIC_DRAW);
}

void SliceRenderer::_initVertexVBO()
{
    if (_vertexVBO != 0) glDeleteBuffers(1, &_vertexVBO);

    glGenBuffers(1, &_vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(double), _slicer->GetWindingOrder().data(), GL_STATIC_DRAW);
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

    _cacheParams.xOrigin = p->GetValueDouble(RenderParams::XSlicePlaneOriginTag, 0);
    _cacheParams.yOrigin = p->GetValueDouble(RenderParams::YSlicePlaneOriginTag, 0);
    _cacheParams.zOrigin = p->GetValueDouble(RenderParams::ZSlicePlaneOriginTag, 0);

    _cacheParams.textureSampleRate = p->GetValueDouble(RenderParams::SampleRateTag, 200);

    _getModifiedExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    _resetColormapCache();
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
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, _colorMapSize, 0, GL_RGBA, GL_FLOAT, &_cacheParams.tf_lut[0]);
}

void SliceRenderer::_regenerateSlice()
{
    RegularGrid* slice = _slicer->GetSlice( _textureSideSize );
    float* dataValues = slice->GetBlks()[0];
    float missingValue = slice->GetMissingValue();

    int    textureSize = 2 * _textureSideSize * _textureSideSize;
    float *textureValues = new float[textureSize];
    for (size_t i=0; i<textureSize/2; i++) {
        float dataValue = dataValues[i];
        if (dataValue == missingValue)
            textureValues[i*2+1] = 1.f;
        else
            textureValues[i*2+1] = 0.f;
        
        textureValues[i*2] = dataValues[i];

        //std::cout << "tex " << dataValue << " " << textureValues[i*2+1] << std::endl;
    }

    _createDataTexture(textureValues);

    delete slice;
    delete[] textureValues;
}

void SliceRenderer::_createDataTexture(float *dataValues)
{
    if (_dataValueTextureID != 0) glDeleteTextures(1, &_dataValueTextureID);

    glGenTextures(1, &_dataValueTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _dataValueTextureID);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _textureSideSize, _textureSideSize, 0, GL_RG, GL_FLOAT, dataValues);

    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * sizeof(double), _slicer->GetWindingOrder().data());
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

    if (_cacheParams.xOrigin != p->GetValueDouble(RenderParams::XSlicePlaneOriginTag, 0)) return true;
    if (_cacheParams.yOrigin != p->GetValueDouble(RenderParams::YSlicePlaneOriginTag, 0)) return true;
    if (_cacheParams.zOrigin != p->GetValueDouble(RenderParams::ZSlicePlaneOriginTag, 0)) return true;

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
    vector<double> min, max;
    _getModifiedExtents(min, max);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;
    return false;
}

void SliceRenderer::_getModifiedExtents(vector<double> &min, vector<double> &max) const
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);
    Box *box = p->GetBox();

    min.resize(3);
    max.resize(3);
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

    if (_isDataCacheDirty() || _isBoxCacheDirty() || _isColormapCacheDirty() ) {
        _resetCache();

        // If we're in fast mode, degrade the quality of the slice for better interactivity
        if (fast) {
            _textureSideSize = 50;
        } else {
            _textureSideSize = _cacheParams.textureSampleRate;
        }

        _regenerateSlice();
    }

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

void SliceRenderer::_drawDebugPolygons() const
{
    // 3D green polygon that shows where we should see data
    std::vector<glm::vec3> polygon = _slicer->GetPolygon();
    LegacyGL *lgl = _glManager->legacy;
    /*lgl->Color4f(0, 1., 0, 1.);
    lgl->Begin(GL_LINES);
    if (polygon.size()) {
        for (int i = 0; i < polygon.size() - 1; i++) {
            glm::vec3 vert1 = polygon[i];
            glm::vec3 vert2 = polygon[i + 1];
            lgl->Vertex3f(vert1.x, vert1.y, vert1.z);
            lgl->Vertex3f(vert2.x, vert2.y, vert2.z);
        }
        lgl->Color4f(0, 1., 0, 1.);
        glm::vec3 vert1 = polygon[polygon.size() - 1];
        glm::vec3 vert2 = polygon[0];
        lgl->Vertex3f(vert1.x, vert1.y, vert1.z);
        lgl->Vertex3f(vert2.x, vert2.y, vert2.z);
    }
    lgl->End();

    // 3D yellow enclosing rectangle that defines the perimeter of our texture
    // This can and often will extend beyond the Box
    std::vector<glm::vec3> rectangle = _slicer->GetRectangle();
    lgl = _glManager->legacy;
    lgl->Begin(GL_LINES);
    lgl->Color4f(1., 1., 0., 1.);
    if (rectangle.size()) {
        for (int i = 0; i < rectangle.size() - 1; i++) {
            glm::vec3 vert1 = rectangle[i];
            glm::vec3 vert2 = rectangle[i + 1];
            lgl->Vertex3f(vert1.x, vert1.y, vert1.z);
            lgl->Vertex3f(vert2.x, vert2.y, vert2.z);
        }
        glm::vec3 vert1 = rectangle[3];
        glm::vec3 vert2 = rectangle[0];
        lgl->Vertex3f(vert1.x, vert1.y, vert1.z);
        lgl->Vertex3f(vert2.x, vert2.y, vert2.z);
    }
    lgl->End();*/


    double* wo = _slicer->GetWindingOrder().data();
    lgl = _glManager->legacy;
    lgl->Begin(GL_LINES);
    lgl->Color4f(0., 1., 0., 1.);        // green
    lgl->Vertex3f(wo[0], wo[1], wo[2]);
    lgl->Vertex3f(wo[3], wo[4], wo[5]);
    lgl->Color4f(0., 1., 1., 1.);        // teal
    lgl->Vertex3f(wo[3], wo[4], wo[5]);
    lgl->Vertex3f(wo[6], wo[7], wo[8]);
    lgl->Color4f(1., 1., 1., 1.);        // white
    lgl->Vertex3f(wo[6], wo[7], wo[8]);
    lgl->Vertex3f(wo[0], wo[1], wo[2]);

    lgl->Color4f(.9, .9, 1., 1.);
    lgl->Vertex3f(wo[9], wo[10], wo[11]);
    lgl->Vertex3f(wo[12], wo[13], wo[14]);
    lgl->Color4f(1., 0., 0., 1.);
    lgl->Vertex3f(wo[12], wo[13], wo[14]);
    lgl->Vertex3f(wo[15], wo[16], wo[17]);
    lgl->Color4f(0., 1., 1., 1.);
    lgl->Vertex3f(wo[15], wo[16], wo[17]);
    lgl->Vertex3f(wo[9], wo[10], wo[11]);
    lgl->End();
}

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
