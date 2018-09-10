#include <sstream>
#include <string>

#include <vapor/SliceRenderer.h>
#include <vapor/SliceParams.h>
#include <vapor/ControlExecutive.h>

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

using namespace VAPoR;

static RendererRegistrar<SliceRenderer> registrar(SliceRenderer::GetClassType(), SliceParams::GetClassType());

SliceRenderer::SliceRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instanceName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, SliceParams::GetClassType(), SliceRenderer::GetClassType(), instanceName, dataMgr)
{
    _textureWidth = 250;
    _textureHeight = 250;

    SliceParams *   p = (SliceParams *)GetActiveParams();
    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);

    _colorMapSize = tf->getNumEntries();
    _colorMap = new GLfloat[_colorMapSize * 4];
    for (int i = 0; i < _colorMapSize; i++) {
        _colorMap[i * 4 + 0] = (float)i / (float)(_colorMapSize - 1);
        _colorMap[i * 4 + 1] = (float)i / (float)(_colorMapSize - 1);
        _colorMap[i * 4 + 2] = (float)i / (float)(_colorMapSize - 1);
        _colorMap[i * 4 + 3] = 1.0;
    }

    _textureData = new unsigned char[_textureWidth * _textureHeight * 4];
}

SliceRenderer::~SliceRenderer()
{
    glDeleteTextures(1, &_texture);
    if (_colorMap) delete[] _colorMap;
    if (_textureData) delete[] _textureData;
}

void SliceRenderer::_initTexture()
{
    glDeleteTextures(1, &_texture);
    glGenTextures(1, &_texture);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureWidth, _textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)_textureData);

    // Do write to the z buffer
    glDepthMask(GL_TRUE);
}

void SliceRenderer::_saveCacheParams()
{
    SliceParams *p = (SliceParams *)GetActiveParams();

    _cacheParams.varName = p->GetVariableName();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.refinementLevel = p->GetRefinementLevel();
    _cacheParams.compressionLevel = p->GetCompressionLevel();
    _cacheParams.textureSampleRates = p->GetSampleRates();

    _textureWidth = _cacheParams.textureSampleRates[X];
    _textureHeight = _cacheParams.textureSampleRates[Y];

    p->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    tf->makeLut(_cacheParams.tf_lut);
    _cacheParams.tf_minMax = tf->getMinMaxMapValue();

    if (_textureData) delete[] _textureData;
    _textureData = new unsigned char[_textureWidth * _textureHeight * 4];

    int rc = _saveTextureData();
    if (rc < 0) SetErrMsg("Unable to acquire data for Slice texture");
}

int SliceRenderer::_saveTextureData()
{
    Grid *grid = NULL;
    int   rc =
        DataMgrUtils::GetGrids(_dataMgr, _cacheParams.ts, _cacheParams.varName, _cacheParams.boxMin, _cacheParams.boxMax, true, &_cacheParams.refinementLevel, &_cacheParams.compressionLevel, &grid);

    if (rc < 0) return (-1);
    assert(grid);

    int orientation = _getOrientation();

    double dx = (_cacheParams.boxMax[X] - _cacheParams.boxMin[X]) / _textureWidth;
    double dy = (_cacheParams.boxMax[Y] - _cacheParams.boxMin[Y]) / _textureHeight;
    // double dz = (_cacheParams.boxMax[Z]-_cacheParams.boxMin[Z])/zFrequency;

    float               varValue, minValue, maxValue, missingValue;
    std::vector<double> coords(3, 0.0);
    for (int j = 0; j < _textureHeight; j++) {
        for (int i = 0; i < _textureWidth; i++) {
            coords[X] = _cacheParams.boxMin[X] + dx * i;
            coords[Y] = _cacheParams.boxMin[Y] + dy * j;
            coords[Z] = _cacheParams.boxMin[Z];    // + dx*i;

            int index = (j * _textureWidth + i) * 4;

            varValue = grid->GetValue(coords);
            missingValue = grid->GetMissingValue();
            if (varValue == missingValue) {
                _textureData[index + 0] = 0.f;
                _textureData[index + 1] = 0.f;
                _textureData[index + 2] = 0.f;
                _textureData[index + 3] = 0.f;
                continue;
            }

            minValue = _cacheParams.tf_minMax[0];
            maxValue = _cacheParams.tf_minMax[1];
            int bin = 255 * (varValue - minValue) / (maxValue - minValue);
            bin *= 4;

            unsigned char red, green, blue, alpha;
            red = _cacheParams.tf_lut[bin + 0] * 255;
            green = _cacheParams.tf_lut[bin + 1] * 255;
            blue = _cacheParams.tf_lut[bin + 2] * 255;
            alpha = _cacheParams.tf_lut[bin + 3] * 255;
            _textureData[index + 0] = red;
            _textureData[index + 1] = green;
            _textureData[index + 2] = blue;
            _textureData[index + 3] = alpha;
        }
    }

    return 0;
}

bool SliceRenderer::_isCacheDirty() const
{
    SliceParams *p = (SliceParams *)GetActiveParams();

    if (_cacheParams.varName != p->GetVariableName()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.refinementLevel != p->GetRefinementLevel()) return true;
    if (_cacheParams.compressionLevel != p->GetCompressionLevel()) return true;

    if (_cacheParams.textureSampleRates != p->GetSampleRates()) return true;

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.varName);
    vector<float>   tf_lut;
    tf->makeLut(tf_lut);
    if (_cacheParams.tf_lut != tf_lut) return (true);
    if (_cacheParams.tf_minMax != tf->getMinMaxMapValue()) return (true);

    vector<double> min, max;
    p->GetBox()->GetExtents(min, max);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

    return false;
}

int SliceRenderer::_initializeGL() { return 0; }

int SliceRenderer::_paintGL(bool fast)
{
    if (_isCacheDirty()) { _saveCacheParams(); }

    _initTexture();

    std::vector<double> min = _cacheParams.boxMin;
    std::vector<double> max = _cacheParams.boxMax;

    cout << "XY " << (min[Z] == max[Z]) << endl;
    cout << "XZ " << (min[Y] == max[Y]) << endl;
    cout << "YZ " << (min[X] == max[X]) << endl << endl;

    int orientation = _getOrientation();
    if (orientation == XY)
        _renderXY(min, max);
    else if (orientation == XZ)
        _renderXZ(min, max);
    else if (orientation == YZ)
        _renderYZ(min, max);

    return 0;
}

int SliceRenderer::_getOrientation() const
{
    std::vector<double> min = _cacheParams.boxMin;
    std::vector<double> max = _cacheParams.boxMax;

    if (min[Z] == max[Z])    // XY Plane
        return XY;
    else if (min[Y] == max[Y])    // XZ Plane
        return XZ;
    else if (min[X] == max[X])    // YZ Plane
        return YZ;
}

void SliceRenderer::_renderXY(std::vector<double> min, std::vector<double> max) const
{
    double zCoord = min[Z];

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(min[X], min[Y], zCoord);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(max[X], min[Y], zCoord);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(max[X], max[Y], zCoord);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(min[X], min[Y], zCoord);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(max[X], max[Y], zCoord);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(min[X], max[Y], zCoord);

    glEnd();
}

void SliceRenderer::_renderXZ(std::vector<double> min, std::vector<double> max) const
{
    double yCoord = min[Y];

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(min[X], yCoord, min[Z]);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(max[X], yCoord, min[Z]);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(max[X], yCoord, max[Z]);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(min[X], yCoord, min[Z]);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(max[X], yCoord, max[Z]);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(min[X], yCoord, max[Z]);

    /*glTexCoord2f(0.0f, 0.0f); glVertex3f(min[X], yCoord, min[Z]);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(max[X], yCoord, min[Z]);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(max[X], yCoord, max[Z]);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(min[X], yCoord, min[Z]);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(max[X], yCoord, max[Z]);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(min[X], yCoord, max[Z]);*/

    glEnd();
}

void SliceRenderer::_renderYZ(std::vector<double> min, std::vector<double> max) const
{
    double xCoord = min[X];

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(xCoord, min[Y], min[Z]);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(xCoord, max[Y], min[Z]);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(xCoord, max[Y], max[Z]);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(xCoord, min[Y], min[Z]);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(xCoord, max[Y], max[Z]);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(xCoord, min[Y], max[Z]);

    glEnd();
}
