#include <sstream>
#include <string>

#include <ctime>

#include <vapor/SliceRenderer.h>
#include <vapor/SliceParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/LegacyGL.h>
#include <vapor/GLManager.h>
#include <vapor/ResourcePath.h>
#include <vapor/DataMgrUtils.h>

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

#define MAX_TEXTURE_SIZE 2000

using namespace VAPoR;

static RendererRegistrar<SliceRenderer> registrar(SliceRenderer::GetClassType(), SliceParams::GetClassType());

SliceRenderer::SliceRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instanceName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, SliceParams::GetClassType(), SliceRenderer::GetClassType(), instanceName, dataMgr)
{
    _initialized = false;
    _textureSideSize = 200;

    _vertexCoords = {0.0f, 0.0f, 0.f, 1.0f, 0.0f, 0.f, 0.0f, 1.0f, 0.f, 1.0f, 0.0f, 0.f, 1.0f, 1.0f, 0.f, 0.0f, 1.0f, 0.f};

    _texCoords = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    _VAO = 0;
    _vertexVBO = 0;
    _texCoordVBO = 0;
    _colorMapTextureID = 0;
    _dataValueTextureID = 0;

    _cacheParams.domainMin.resize(3, 0.f);
    _cacheParams.domainMax.resize(3, 1.f);

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
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(double), _vertexCoords.data(), GL_STATIC_DRAW);
}

int SliceRenderer::_resetDataCache()
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);

    _cacheParams.varName = p->GetVariableName();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.refinementLevel = p->GetRefinementLevel();
    _cacheParams.compressionLevel = p->GetCompressionLevel();
    _cacheParams.textureSampleRate = p->GetSampleRate();
    _cacheParams.orientation = p->GetBox()->GetOrientation();

    _textureSideSize = _cacheParams.textureSampleRate;
    if (_textureSideSize > MAX_TEXTURE_SIZE) _textureSideSize = MAX_TEXTURE_SIZE;

    _resetBoxCache();
    _resetColormapCache();

    int rc;
    rc = _saveTextureData();
    if (rc < 0) {
        SetErrMsg("Unable to acquire data for Slice texture");
        return rc;
    }

    return rc;
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

int SliceRenderer::_resetBoxCache()
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    VAssert(p);
    _getModifiedExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    int rc = _dataMgr->GetVariableExtents(_cacheParams.ts, _cacheParams.varName, _cacheParams.refinementLevel, _cacheParams.compressionLevel, _cacheParams.domainMin, _cacheParams.domainMax);
    if (rc < 0) {
        SetErrMsg("Unable to determine domain extents for %s", _cacheParams.varName.c_str());
        return rc;
    }

    _setVertexPositions();
    _resetTextureCoordinates();
    return rc;
}

void SliceRenderer::_resetTextureCoordinates()
{
    float texMinX, texMinY, texMaxX, texMaxY;

    std::vector<double> boxMin = _cacheParams.boxMin;
    std::vector<double> boxMax = _cacheParams.boxMax;
    std::vector<double> domainMin = _cacheParams.domainMin;
    std::vector<double> domainMax = _cacheParams.domainMax;

    int xAxis, yAxis;
    int orientation = _cacheParams.orientation;
    if (orientation == XY) {
        xAxis = X;
        yAxis = Y;
    } else if (orientation == XZ) {
        xAxis = X;
        yAxis = Z;
    } else {    // (orientation = YZ)
        xAxis = Y;
        yAxis = Z;
    }

    texMinX = (boxMin[xAxis] - domainMin[xAxis]) / (domainMax[xAxis] - domainMin[xAxis]);
    texMaxX = (boxMax[xAxis] - domainMin[xAxis]) / (domainMax[xAxis] - domainMin[xAxis]);
    texMinY = (boxMin[yAxis] - domainMin[yAxis]) / (domainMax[yAxis] - domainMin[yAxis]);
    texMaxY = (boxMax[yAxis] - domainMin[yAxis]) / (domainMax[yAxis] - domainMin[yAxis]);

    _texCoords.clear();
    _texCoords = {texMinX, texMinY, texMaxX, texMinY, texMinX, texMaxY, texMaxX, texMinY, texMaxX, texMaxY, texMinX, texMaxY};

    glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * _texCoords.size(),
                    _texCoords.data()    //&_texCoords[0]
    );
}

std::vector<double> SliceRenderer::_calculateDeltas() const
{
    int    sampleRate = _textureSideSize;
    double dx = (_cacheParams.domainMax[X] - _cacheParams.domainMin[X]) / (1 + sampleRate);
    double dy = (_cacheParams.domainMax[Y] - _cacheParams.domainMin[Y]) / (1 + sampleRate);
    double dz = (_cacheParams.domainMax[Z] - _cacheParams.domainMin[Z]) / (1 + sampleRate);

    std::vector<double> deltas = {dx, dy, dz};
    return deltas;
}

void SliceRenderer::_populateDataXY(float *dataValues, Grid *grid) const
{
    std::vector<double> deltas = _calculateDeltas();
    float               varValue, missingValue;
    std::vector<double> coords(3, 0.0);
    coords[X] = _cacheParams.domainMin[X] + deltas[X] / 2.f;
    coords[Y] = _cacheParams.domainMin[Y] + deltas[Y] / 2.f;
    coords[Z] = _cacheParams.boxMin[Z];

    int index = 0;
    for (int j = 0; j < _textureSideSize; j++) {
        coords[X] = _cacheParams.domainMin[X];

        for (int i = 0; i < _textureSideSize; i++) {
            varValue = grid->GetValue(coords);
            missingValue = grid->GetMissingValue();
            if (varValue == missingValue)
                dataValues[index + 1] = 1.f;
            else
                dataValues[index + 1] = 0.f;

            dataValues[index] = varValue;

            index += 2;
            coords[X] += deltas[X];
        }
        coords[Y] += deltas[Y];
    }
}

void SliceRenderer::_populateDataXZ(float *dataValues, Grid *grid) const
{
    std::vector<double> deltas = _calculateDeltas();
    float               varValue, missingValue;
    std::vector<double> coords(3, 0.0);
    coords[X] = _cacheParams.domainMin[X];
    coords[Y] = _cacheParams.boxMin[Y];
    coords[Z] = _cacheParams.domainMin[Z];

    int index = 0;
    for (int j = 0; j < _textureSideSize; j++) {
        coords[X] = _cacheParams.domainMin[X];

        for (int i = 0; i < _textureSideSize; i++) {
            varValue = grid->GetValue(coords);
            missingValue = grid->GetMissingValue();
            if (varValue == missingValue)
                dataValues[index + 1] = 1.f;
            else
                dataValues[index + 1] = 0.f;

            dataValues[index] = varValue;

            index += 2;
            coords[X] += deltas[X];
        }
        coords[Z] += deltas[Z];
    }
}

void SliceRenderer::_populateDataYZ(float *dataValues, Grid *grid) const
{
    std::vector<double> deltas = _calculateDeltas();
    float               varValue, missingValue;
    std::vector<double> coords(3, 0.0);
    coords[X] = _cacheParams.boxMin[X];
    coords[Y] = _cacheParams.domainMin[Y];
    coords[Z] = _cacheParams.domainMin[Z];

    int index = 0;
    for (int j = 0; j < _textureSideSize; j++) {
        coords[Y] = _cacheParams.domainMin[Y];

        for (int i = 0; i < _textureSideSize; i++) {
            varValue = grid->GetValue(coords);
            missingValue = grid->GetMissingValue();
            if (varValue == missingValue)
                dataValues[index + 1] = 1.f;
            else
                dataValues[index + 1] = 0.f;

            dataValues[index] = varValue;

            index += 2;
            coords[Y] += deltas[Y];
        }
        coords[Z] += deltas[Z];
    }
}

int SliceRenderer::_saveTextureData()
{
    Grid *grid = nullptr;
    int   rc =
        DataMgrUtils::GetGrids(_dataMgr, _cacheParams.ts, _cacheParams.varName, _cacheParams.boxMin, _cacheParams.boxMax, true, &_cacheParams.refinementLevel, &_cacheParams.compressionLevel, &grid);

    if (rc < 0) {
        SetErrMsg("Unable to acquire Grid for Slice texture");
        return (rc);
    }
    VAssert(grid);

    grid->SetInterpolationOrder(1);

    _setVertexPositions();

    int    textureSize = 2 * _textureSideSize * _textureSideSize;
    float *dataValues = new float[textureSize];

    if (_cacheParams.orientation == XY)
        _populateDataXY(dataValues, grid);
    else if (_cacheParams.orientation == XZ)
        _populateDataXZ(dataValues, grid);
    else if (_cacheParams.orientation == YZ)
        _populateDataYZ(dataValues, grid);
    else
        VAssert(0);

    _createDataTexture(dataValues);

    delete[] dataValues;
    _dataMgr->UnlockGrid(grid);
    delete grid;
    grid = nullptr;

    return rc;
}

void SliceRenderer::_createDataTexture(float *dataValues)
{
    if (_dataValueTextureID != 0) glDeleteTextures(1, &_dataValueTextureID);

    glGenTextures(1, &_dataValueTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _dataValueTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _textureSideSize, _textureSideSize, 0, GL_RG, GL_FLOAT, dataValues);
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

    if (_cacheParams.textureSampleRate != p->GetSampleRate()) return true;

    vector<double> min, max;
    Box *          box = p->GetBox();
    int            orientation = box->GetOrientation();
    if (_cacheParams.orientation != orientation) return true;
    // Special case: if our plane shifts its position along its orthognal axis,
    // then we will need to return true and resample our data.  If its extents
    // change along its perimeter, then we will just reconfigure the texture
    // coordinates via _resetBoxCache in our _paintGL routine.
    _getModifiedExtents(min, max);

    if (min != _cacheParams.boxMin) return true;
    if (max != _cacheParams.boxMax) return true;

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

    vector<double> sampleLocation = p->GetValueDoubleVec(p->SampleLocationTag);
    VAssert(sampleLocation.size() == 3);
    switch (box->GetOrientation()) {
    case XY:
        min[Z] = sampleLocation[Z];
        max[Z] = sampleLocation[Z];
        break;
    case XZ:
        min[Y] = sampleLocation[Y];
        max[Y] = sampleLocation[Y];
        break;
    case YZ:
        min[X] = sampleLocation[X];
        max[X] = sampleLocation[X];
        break;
    default: VAssert(0);
    }
}

int SliceRenderer::_paintGL(bool fast)
{
    int rc = 0;

    _initializeState();

    if (_isDataCacheDirty()) {
        rc = _resetDataCache();
        if (rc < 0) {
            _resetState();
            return rc;    // error message already set by _resetDataCache()
        }
    } else {
        if (_isColormapCacheDirty()) _resetColormapCache();

        if (_isBoxCacheDirty()) {
            rc = _resetBoxCache();
            if (rc < 0) {
                _resetState();
                return rc;    // error message already set by _resetBoxCache()
            }
        }
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

void SliceRenderer::_setVertexPositions()
{
    std::vector<double> trimmedMin(3, 0.f);
    std::vector<double> trimmedMax(3, 1.f);

    // If the box is outside of our domain, trim the vertex coordinates to be within
    // the domain.
    for (int i = 0; i < _cacheParams.boxMax.size(); i++) {
        trimmedMin[i] = max(_cacheParams.boxMin[i], _cacheParams.domainMin[i]);
        trimmedMax[i] = min(_cacheParams.boxMax[i], _cacheParams.domainMax[i]);
    }

    int orientation = _cacheParams.orientation;
    if (orientation == XY)
        _setXYVertexPositions(trimmedMin, trimmedMax);
    else if (orientation == XZ)
        _setXZVertexPositions(trimmedMin, trimmedMax);
    else if (orientation == YZ)
        _setYZVertexPositions(trimmedMin, trimmedMax);

    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * sizeof(double), _vertexCoords.data());
}

void SliceRenderer::_setXYVertexPositions(std::vector<double> min, std::vector<double> max)
{
    double zCoord = min[Z];

    std::vector<double> temp = {min[X], min[Y], zCoord, max[X], min[Y], zCoord, min[X], max[Y], zCoord, max[X], min[Y], zCoord, max[X], max[Y], zCoord, min[X], max[Y], zCoord};

    _vertexCoords = temp;
}

void SliceRenderer::_setXZVertexPositions(std::vector<double> min, std::vector<double> max)
{
    double              yCoord = min[Y];
    std::vector<double> temp = {min[X], yCoord, min[Z], max[X], yCoord, min[Z], min[X], yCoord, max[Z], max[X], yCoord, min[Z], max[X], yCoord, max[Z], min[X], yCoord, max[Z]};

    _vertexCoords = temp;
}

void SliceRenderer::_setYZVertexPositions(std::vector<double> min, std::vector<double> max)
{
    double              xCoord = min[X];
    std::vector<double> temp = {xCoord, min[Y], min[Z], xCoord, max[Y], min[Z], xCoord, min[Y], max[Z], xCoord, max[Y], min[Z], xCoord, max[Y], max[Z], xCoord, min[Y], max[Z]};

    _vertexCoords = temp;
}

int SliceRenderer::_getConstantAxis() const
{
    if (_cacheParams.orientation == XY)
        return Z;
    else if (_cacheParams.orientation == XZ)
        return Y;
    else    // (_cacheParams.orientation == XY )
        return X;
}
