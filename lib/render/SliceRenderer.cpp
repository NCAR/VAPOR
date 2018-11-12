#include <sstream>
#include <string>

#include <vapor/SliceRenderer.h>
#include <vapor/SliceParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/LegacyGL.h>
#include <vapor/GLManager.h>

#define X           0
#define Y           1
#define Z           2
#define XY          0
#define XZ          1
#define YZ          2
#define NUMVERTICES 4

#define MAXTEXTURESIZE 8000

using namespace VAPoR;

static RendererRegistrar<SliceRenderer> registrar(SliceRenderer::GetClassType(), SliceParams::GetClassType());

SliceRenderer::SliceRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instanceName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, SliceParams::GetClassType(), SliceRenderer::GetClassType(), instanceName, dataMgr)
{
    _initialized = false;
    _textureWidth = 250;
    _textureHeight = 250;

    _VAO = NULL;
    _vertexVBO = NULL;
    _dataVBO = NULL;
    _EBO = NULL;

    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    assert(p);
    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);

    _colorMapSize = tf->getNumEntries();
    _colorMap = new GLfloat[_colorMapSize * 4];
    for (int i = 0; i < _colorMapSize; i++) {
        _colorMap[i * 4 + 0] = (float)i / (float)(_colorMapSize - 1);
        _colorMap[i * 4 + 1] = (float)i / (float)(_colorMapSize - 1);
        _colorMap[i * 4 + 2] = (float)i / (float)(_colorMapSize - 1);
        _colorMap[i * 4 + 3] = 1.0;
    }

    //    _dataValues = new unsigned char[_textureWidth * _textureHeight * 4];
    _dataValues = new float[_textureWidth * _textureHeight];
    _vertexPositions = new float[NUMVERTICES * 3];

    _saveCacheParams();
    _initialized = true;
}

SliceRenderer::~SliceRenderer()
{
    glDeleteTextures(1, &_textureID);
    if (_colorMap) {
        delete[] _colorMap;
        _colorMap = nullptr;
    }
    if (_dataValues) {
        delete[] _dataValues;
        _dataValues = nullptr;
    }
    if (_VAO) {
        delete _VAO;
        _VAO = nullptr;
    }
    if (_vertexVBO) {
        delete _vertexVBO;
        _vertexVBO = nullptr;
    }
    if (_dataVBO) {
        delete _dataVBO;
        _dataVBO = nullptr;
    }
    if (_EBO) {
        delete _EBO;
        _EBO = nullptr;
    }
}

/*void SliceRenderer::_initTextures() {
    glDeleteTextures(1, &_textureID);
    glGenTextures(1, &_textureID);

    glBindTexture(GL_TEXTURE_2D, _textureID);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, _textureWidth, _textureHeight,
        0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_textureData
    );
}*/

void SliceRenderer::_initTextures()
{
    glGenTextures(1, &_textureID);
    glGenTextures(1, &_colorMapTextureID);
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_vertexVBO);
    glGenBuffers(1, &_dataVBO);
    glGenBuffers(1, &_EBO);

    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, _dataVBO);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int SliceRenderer::_saveCacheParams()
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    assert(p);

    _cacheParams.varName = p->GetVariableName();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.refinementLevel = p->GetRefinementLevel();
    _cacheParams.compressionLevel = p->GetCompressionLevel();
    _cacheParams.textureSampleRate = p->GetSampleRate();
    _cacheParams.orientation = p->GetBox()->GetOrientation();

    _textureWidth = _cacheParams.textureSampleRate;
    _textureHeight = _cacheParams.textureSampleRate;
    if (_textureWidth > MAXTEXTURESIZE) _textureWidth = MAXTEXTURESIZE;
    if (_textureHeight > MAXTEXTURESIZE) _textureHeight = MAXTEXTURESIZE;

    p->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    tf->makeLut(_cacheParams.tf_lut);
    _cacheParams.tf_minMax = tf->getMinMaxMapValue();

    if (_dataValues) delete[] _dataValues;
    _dataValues = new float[_textureWidth * _textureHeight];

    int rc = _saveTextureData();
    if (rc < 0) SetErrMsg("Unable to acquire data for Slice texture");

    return rc;
}

void SliceRenderer::_getSampleCoordinates(std::vector<double> &coords, int i, int j) const
{
    int    sampleRate = _cacheParams.textureSampleRate;
    double dx = (_cacheParams.boxMax[X] - _cacheParams.boxMin[X]) / (1 + sampleRate);
    double dy = (_cacheParams.boxMax[Y] - _cacheParams.boxMin[Y]) / (1 + sampleRate);
    double dz = (_cacheParams.boxMax[Z] - _cacheParams.boxMin[Z]) / (1 + sampleRate);

    if (_cacheParams.orientation == XY) {
        coords[X] = _cacheParams.boxMin[X] + dx * i + dx / 2.f;
        coords[Y] = _cacheParams.boxMin[Y] + dy * j + dy / 2.f;
        coords[Z] = _cacheParams.boxMin[Z];
    } else if (_cacheParams.orientation == XZ) {
        coords[X] = _cacheParams.boxMin[X] + dx * i + dx / 2.f;
        coords[Y] = _cacheParams.boxMin[Y];
        coords[Z] = _cacheParams.boxMin[Z] + dz * j + dz / 2.f;
    } else {    // Y corresponds to i, the faster axis; Z to j, the slower axis
        coords[Z] = _cacheParams.boxMin[Z] + dz * j + dz / 2.f;
        coords[Y] = _cacheParams.boxMin[Y] + dy * i + dy / 2.f;
        coords[X] = _cacheParams.boxMin[X];
    }
}

/*int SliceRenderer::_saveTextureData() {
    Grid* grid = NULL;
    int rc = DataMgrUtils::GetGrids(
        _dataMgr,
        _cacheParams.ts,
        _cacheParams.varName,
        _cacheParams.boxMin,
        _cacheParams.boxMax,
        true,
        &_cacheParams.refinementLevel,
        &_cacheParams.compressionLevel,
        &grid
    );

    grid->SetInterpolationOrder(1);

    if (rc<0) {
        SetErrMsg("Unable to acquire Grid for Slice texture");
        return(rc);
    }
    assert(grid);

    std::vector<double> textureMin, textureMax;
    _getTextureCoordinates(textureMin, textureMax);

    std::vector<double> cachedValuesForParams;

    int missing=0;
    float varValue, minValue, maxValue, missingValue;
    std::vector<double> coords(3, 0.0);
    for (int j=0; j<_textureHeight; j++) {
        for (int i=0; i<_textureWidth; i++) {
            _getSampleCoordinates(coords, i, j);

            int index = (j*_textureWidth + i)*4;

            varValue = grid->GetValue(coords);
            missingValue = grid->GetMissingValue();
            if (varValue == missingValue) {
                missing++;
                _textureData[index + 0] = 0.f;
                _textureData[index + 1] = 0.f;
                _textureData[index + 2] = 0.f;
                _textureData[index + 3] = 0.f;
                continue;
            }

            cachedValuesForParams.push_back(varValue);

            minValue = _cacheParams.tf_minMax[0];
            maxValue = _cacheParams.tf_minMax[1];
            int bin = 255 * (varValue-minValue) / (maxValue-minValue);
            bin *= 4;
            if (bin < 0)
                bin = 0;
            if (bin >= _cacheParams.tf_lut.size())
                bin = _cacheParams.tf_lut.size()-4;

            unsigned char red, green, blue, alpha;
            red   = _cacheParams.tf_lut[bin + 0]*255;
            green = _cacheParams.tf_lut[bin + 1]*255;
            blue  = _cacheParams.tf_lut[bin + 2]*255;
            alpha = _cacheParams.tf_lut[bin + 3]*255;

            _textureData[index + 0] = red;
            _textureData[index + 1] = green;
            _textureData[index + 2] = blue;
            _textureData[index + 3] = alpha;
        }
    }


    SliceParams* p = dynamic_cast<SliceParams*>(GetActiveParams());
    assert(p);
    p->SetCachedValues(cachedValuesForParams);
    return rc;
}*/

int SliceRenderer::_saveTextureData()
{
    Grid *grid = NULL;
    int   rc =
        DataMgrUtils::GetGrids(_dataMgr, _cacheParams.ts, _cacheParams.varName, _cacheParams.boxMin, _cacheParams.boxMax, true, &_cacheParams.refinementLevel, &_cacheParams.compressionLevel, &grid);

    grid->SetInterpolationOrder(1);

    if (rc < 0) {
        SetErrMsg("Unable to acquire Grid for Slice texture");
        return (rc);
    }
    assert(grid);

    _setVertexPositions();

    float               varValue, minValue, maxValue, missingValue;
    std::vector<double> coords(3, 0.0);
    for (int j = 0; j < _textureHeight; j++) {
        for (int i = 0; i < _textureWidth; i++) {
            _getSampleCoordinates(coords, i, j);

            int index = (j * _textureWidth + i) * 2;

            varValue = grid->GetValue(coords);
            missingValue = grid->GetMissingValue();
            if (varValue == missingValue) {
                // The second element of our shader indicates a missing value
                // if it's not equal to 0, so set it to 1.f here
                _dataValues[index] = 1.f;
                _dataValues[index + 1] = 1.f;
                continue;
            }

            // The second element of our shader indicates a missing value
            // if it's not equal to 0, so set it to 0 here
            _dataValues[index] = varValue;
            _dataValues[index + 1] = 0.f;
        }
    }

    size_t dataSize = _textureWidth * _textureHeight * 2 * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, _dataVBO);
    glBufferData(GL_ARRAY_BUFFER, dataSize, &_dataValues, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, NUMVERTICES * 3 * sizeof(float), &_vertexPositions, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(EL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned int), &_indexValues, GL_STATIC_DRAW);

    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    assert(p);
    p->SetCachedValues(cachedValuesForParams);
    return rc;
}

void SliceRenderer::_getTextureCoordinates(std::vector<double> &textureMin, std::vector<double> &textureMax)
{
    textureMin.clear();
    textureMax.clear();

    std::vector<double> boxMin = _cacheParams.boxMin;
    std::vector<double> boxMax = _cacheParams.boxMax;

    int orientation = _cacheParams.orientation;
    if (orientation == XY) {
        textureMin.push_back(boxMin[X]);
        textureMin.push_back(boxMin[Y]);
        textureMax.push_back(boxMax[X]);
        textureMax.push_back(boxMax[Y]);
    }
    if (orientation == XZ) {
        textureMin.push_back(boxMin[X]);
        textureMin.push_back(boxMin[Z]);
        textureMax.push_back(boxMax[X]);
        textureMax.push_back(boxMax[Z]);
    }
    if (orientation == YZ) {
        textureMin.push_back(boxMin[Y]);
        textureMin.push_back(boxMin[Z]);
        textureMax.push_back(boxMax[Y]);
        textureMax.push_back(boxMax[Z]);
    }
}

bool SliceRenderer::_isCacheDirty() const
{
    SliceParams *p = dynamic_cast<SliceParams *>(GetActiveParams());
    assert(p);

    if (_cacheParams.varName != p->GetVariableName()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.refinementLevel != p->GetRefinementLevel()) return true;
    if (_cacheParams.compressionLevel != p->GetCompressionLevel()) return true;

    if (_cacheParams.textureSampleRate != p->GetSampleRate()) return true;

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    vector<float>   tf_lut;
    tf->makeLut(tf_lut);
    if (_cacheParams.tf_lut != tf_lut) return true;
    if (_cacheParams.tf_minMax != tf->getMinMaxMapValue()) return true;

    vector<double> min, max;
    Box *          box = p->GetBox();
    box->GetExtents(min, max);
    int orientation = box->GetOrientation();

    if (_cacheParams.orientation != orientation) return true;
    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

    return false;
}

int SliceRenderer::_initializeGL() { return 0; }

/*int SliceRenderer::_paintGL(bool fast) {
    int rc = 0;

    if (_isCacheDirty()) {
        rc = _saveCacheParams();
    }

    _initTextures();

    std::vector<double> min = _cacheParams.boxMin;
    std::vector<double> max = _cacheParams.boxMax;

    glEnable(GL_DEPTH_TEST);

    LegacyGL *lgl = _glManager->legacy;
    lgl->EnableTexture();

    int orientation = _cacheParams.orientation;
    if (orientation == XY)
        _renderXY(min, max);
    else if (orientation == XZ)
        _renderXZ(min, max);
    else if (orientation == YZ)
        _renderYZ(min, max);
    //_render(orientation, min, max);

    lgl->DisableTexture();

    return rc;
}*/

int SliceRenderer::_paintGL(bool fast)
{
    if (printOpenGLError() != 0) return -1;

    ShaderProgram *s = _glManager->shaderManager->GetShader("2DData");
    s->Bind();
    s->SetUniform("minLUTValue", (float)_cacheParams.tf_minMax[0]);
    s->SetUniform("maxLUTValue", (float)_cacheParams.tf_minMax[1]);

    glActiveTexture(GL_TEXTURE0);
}

void SliceRenderer::_render(int orientation, std::vector<double> min, std::vector<double> max) const
{
    int plane;
    if (orientation == Z)
        plane = XY;
    else if (orientation == Y)
        plane = XZ;
    else
        plane = YZ;

    cout << "orientation/plane " << orientation << " " << plane << endl;
    min[plane] = max[plane];

    LegacyGL *lgl = _glManager->legacy;

    lgl->Begin(GL_TRIANGLES);
    lgl->TexCoord2f(0.f, 0.f);
    lgl->Vertex3f(min[X], min[Y], min[Z]);
    lgl->TexCoord2f(1.f, 0.f);
    lgl->Vertex3f(max[X], min[Y], min[Z]);
    lgl->TexCoord2f(1.f, 1.f);
    lgl->Vertex3f(max[X], max[Y], max[Z]);

    lgl->TexCoord2f(0.f, 0.f);
    lgl->Vertex3f(min[X], min[Y], min[Z]);
    lgl->TexCoord2f(1.f, 1.f);
    lgl->Vertex3f(max[X], max[Y], max[Z]);
    lgl->TexCoord2f(0.f, 1.f);
    lgl->Vertex3f(min[X], max[Y], max[Z]);
    lgl->End();
}

void SliceRenderer::_setVertexPositions()
{
    std::vector<double> min = _cacheParams.boxMin;
    std::vector<double> max = _cacheParams.boxMax;
    int                 orientation = _cacheParams.orientation;

    if (orientation == XY)
        _setXYVertexPositions(min, max);
    else if (orientation == XZ)
        _setXZVertexPositions(min, max);
    else if (orientation == YZ)
        _setXZVertexPositions(min, max);
}

void SliceRenderer::_setXYVertexPositions(std::vector<double> min, std::vector<double> max)
{
    double zCoord = min[Z];

    flaot tempArray[] = {
        min[X], min[Y], zCoord, max[X], min[Y], zCoord, max[X], max[Y], zCoord, min[X], max[Y], zCoord,
    };

    /*float tempArray[] = {
        min[X], min[Y], zCoord,     // triangle 1
        max[X], min[Y], zCoord,
        max[X], max[Y], zCoord,

        min[X], min[Y], zCoord,     // triangle 2
        max[X], max[Y], zCoord,
        min[X], max[Y], zCoord
    };*/

    std::copy(tempArray, temparray + NUMVERTICES * 3, _vertexPositions);
}

void SliceRenderer::_setXZVertexPositions(std::vector<double> min, std::vector<double> max)
{
    double yCoord = min[Y];

    flaot tempArray[] = {min[X], yCoord, min[Z], max[X], yCoord, min[Z], max[X], yCoord, max[Z], min[X], yCoord, max[Z]};

    /*float tempArray[] = {
        min[X], yCoord, min[Z],     // triangle 1
        max[X], yCoord, min[Z],
        max[X], yCoord, max[Z],

        min[X], yCoord, min[Z],     // triangle 2
        max[X], yCoord, max[Z],
        min[X], yCoord, max[Z]
    };*/

    std::copy(tempArray, temparray + NUMVERTICES * 3, _vertexPositions);
}

void SliceRenderer::_setYZVertexPositions(std::vector<double> min, std::vector<double> max)
{
    double xCoord = min[X];

    flaot tempArray[] = {xCoord, min[Y], min[Z], xCoord, max[Y], min[Z], xCoord, max[Y], max[Z], xCoord, min[Y], max[Z]};
    /*float tempArray [] = {
        xCoord, min[Y], min[Z],     // triangle 1
        xCoord, max[Y], min[Z],
        xCoord, max[Y], max[Z],

        xCoord, min[Y], min[Z],     // triangle 2
        xCoord, max[Y], max[Z],
        xCoord, min[Y], max[Z]
    };*/

    std::copy(tempArray, temparray + NUMVERTICES * 3, _vertexPositions);
}

/*void SliceRenderer::_renderXY(
    std::vector<double> min,
    std::vector<double> max
) const {
    double zCoord = min[Z];

    LegacyGL *lgl = _glManager->legacy;

    lgl->Begin(GL_TRIANGLES);
    lgl->TexCoord2f(0.f, 0.f); lgl->Vertex3f(min[X], min[Y], zCoord);
    lgl->TexCoord2f(1.f, 0.f); lgl->Vertex3f(max[X], min[Y], zCoord);
    lgl->TexCoord2f(1.f, 1.f); lgl->Vertex3f(max[X], max[Y], zCoord);

    lgl->TexCoord2f(0.f, 0.f); lgl->Vertex3f(min[X], min[Y], zCoord);
    lgl->TexCoord2f(1.f, 1.f); lgl->Vertex3f(max[X], max[Y], zCoord);
    lgl->TexCoord2f(0.f, 1.f); lgl->Vertex3f(min[X], max[Y], zCoord);
    lgl->End();
}

void SliceRenderer::_renderXZ(
    std::vector<double> min,
    std::vector<double> max
) const {
    double yCoord = min[Y];

    LegacyGL *lgl = _glManager->legacy;

    lgl->Begin(GL_TRIANGLES);
    lgl->TexCoord2f(0.0f, 0.0f); lgl->Vertex3f(min[X], yCoord, min[Z]);
    lgl->TexCoord2f(1.0f, 0.0f); lgl->Vertex3f(max[X], yCoord, min[Z]);
    lgl->TexCoord2f(1.0f, 1.0f); lgl->Vertex3f(max[X], yCoord, max[Z]);

    lgl->TexCoord2f(0.0f, 0.0f); lgl->Vertex3f(min[X], yCoord, min[Z]);
    lgl->TexCoord2f(1.0f, 1.0f); lgl->Vertex3f(max[X], yCoord, max[Z]);
    lgl->TexCoord2f(0.0f, 1.0f); lgl->Vertex3f(min[X], yCoord, max[Z]);
    lgl->End();
}

void SliceRenderer::_renderYZ(
    std::vector<double> min,
    std::vector<double> max
) const {
    double xCoord = min[X];

    LegacyGL *lgl = _glManager->legacy;

    lgl->Begin(GL_TRIANGLES);
    lgl->TexCoord2f(0.0f, 0.0f); lgl->Vertex3f(xCoord, min[Y], min[Z]);
    lgl->TexCoord2f(1.0f, 0.0f); lgl->Vertex3f(xCoord, max[Y], min[Z]);
    lgl->TexCoord2f(1.0f, 1.0f); lgl->Vertex3f(xCoord, max[Y], max[Z]);

    lgl->TexCoord2f(0.0f, 0.0f); lgl->Vertex3f(xCoord, min[Y], min[Z]);
    lgl->TexCoord2f(1.0f, 1.0f); lgl->Vertex3f(xCoord, max[Y], max[Z]);
    lgl->TexCoord2f(0.0f, 1.0f); lgl->Vertex3f(xCoord, min[Y], max[Z]);
    lgl->End();
}*/
