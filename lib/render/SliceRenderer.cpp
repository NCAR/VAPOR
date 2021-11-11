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
#include <glm/gtx/string_cast.hpp>
#include <vapor/ConvexHull.h>

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

#define MAX_TEXTURE_SIZE 2000

//#define DEBUG 1

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

int SliceRenderer::_resetDataCache(bool fast)
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

    _cacheParams.xRotation = p->GetValueDouble(SliceParams::XRotationTag,0);
    _cacheParams.yRotation = p->GetValueDouble(SliceParams::YRotationTag,0);
    _cacheParams.zRotation = p->GetValueDouble(SliceParams::ZRotationTag,0);

    /*if (fast) {
        _textureSideSize = 50;
        std::cout << "_resetDataCache fast " << _textureSideSize << std::endl;
    }
    else {
        _textureSideSize = _cacheParams.textureSampleRate;
        std::cout << "_resetDataCache slow " << _textureSideSize << std::endl;
    }*/
    //_textureSideSize = _cacheParams.textureSampleRate;
    //if (_textureSideSize > MAX_TEXTURE_SIZE) _textureSideSize = MAX_TEXTURE_SIZE;
     
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
    return rc;
}

void SliceRenderer::_rotate()
{
    std::vector<double> boxMin = _cacheParams.boxMin;
    std::vector<double> boxMax = _cacheParams.boxMax;

    _origin = {(boxMax[X]-boxMin[X])/2. + boxMin[X], 
                        (boxMax[Y]-boxMin[Y])/2. + boxMin[Y], 
                        (boxMax[Z]-boxMin[Z])/2. + boxMin[Z]};

    // which we will use to project our polygon into 2D space.  First rotate XY plane with quaternion.
    glm::vec3 angles( M_PI*_cacheParams.xRotation/180., M_PI*_cacheParams.yRotation/180., M_PI*_cacheParams.zRotation/180. );
    glm::quat q = glm::quat( angles );

    // We will sample the slice in 2D coordinates.  
    // So we first define a basis function of three orthogonal vectors (_normal, _axis1, _axis2).
    _normal = q * glm::vec3(0,0,1);
    _axis1 = _getOrthogonal( _normal );
    _axis2 = glm::cross( _normal, _axis1 );

    // Next we define a polygon that defines our slice.  To do this we
    // find where our plane intercepts the X, Y, and Z edges of our Box extents.
    // 
    // Each _vertex in 'vertices' holds a glm::vec3 representing a point 3D space, 
    // and a glm::vec2 storing its location in 2D according to our basis function.
    std::vector< _vertex > vertices;
    _findIntercepts(_origin, _normal, vertices, false);

    // Use Convex Hull to get an ordered list of vertices
    stack<glm::vec2> orderedTwoDPoints = _2DConvexHull( vertices );

    // At this point, 'vertices' has a vector<glm::vec3> that describe the 3D points of our polygon,
    // and a vector<glm::vec2> that describe the 2D points of our polygon.
    // The 3D and 2D values map to eachother.  IE - 3D element 0 corresponds the the coordinates of 2D element 0.

    // Find a rectangle that encompasses our 3D polygon by finding the min/max bounds along our 2D points.
    // We will sample along the X/Y axes of this rectangle.
    stack<glm::vec2> s = orderedTwoDPoints;
    _rectangle2D = _makeRectangle2D( vertices, s );
 
    // Map our rectangle's 2D edges back into 3D space, to get an 
    // ordered list of vertices for our data-enclosing rectangle. 
    s = orderedTwoDPoints;
    _polygon3D = _makePolygon3D( vertices, s);

    // Define a rectangle that encloses our polygon in 3D space.  We will sample along this
    // rectangle to generate our 2D texture.
    _rectangle3D = _makeRectangle3D( vertices, orderedTwoDPoints );
}

void SliceRenderer::_findIntercepts(glm::vec3& _origin, glm::vec3& _normal, std::vector<_vertex>& vertices, bool stretch) const {
    // Lambdas for finding intercepts on the XYZ edges of the Box 
    // Plane equation: 
    //     _normal.x*(x-_origin.x) + _normal.y*(y-_origin.y) + _normal.z*(z-_origin.z) = 0
    auto zIntercept = [&](float x, float y) {
        if(_normal.z==0) return;
        double z = (_normal.x*_origin.x + _normal.y*_origin.y + _normal.z*_origin.z - _normal.x*x - _normal.y*y) / _normal.z;
        if (z >= _cacheParams.boxMin[Z] && z <= _cacheParams.boxMax[Z]) {
            _vertex p = { glm::vec3(x,y,z), glm::vec2() };
            vertices.push_back(p);
        }
    };
    auto yIntercept = [&](float x, float z) {
        if(_normal.y==0) return;
        double y = (_normal.x*_origin.x + _normal.y*_origin.y + _normal.z*_origin.z - _normal.x*x - _normal.z*z) / _normal.y;
        if (y >= _cacheParams.boxMin[Y] && y <= _cacheParams.boxMax[Y]) {
            _vertex p = { glm::vec3(x,y,z), glm::vec2() };
            vertices.push_back(p);
        }
    };
    auto xIntercept = [&](float y, float z) {
        if(_normal.x==0) return;
        double x = (_normal.x*_origin.x + _normal.y*_origin.y + _normal.z*_origin.z - _normal.y*y - _normal.z*z) / _normal.x;
        if (x >= _cacheParams.boxMin[X] && x <= _cacheParams.boxMax[X]) {
            _vertex p = { glm::vec3(x,y,z), glm::vec2() };
            vertices.push_back(p);
        }
    };

    // Find vertices that exist on the Z edges of the Box
    zIntercept(_cacheParams.boxMin[X], _cacheParams.boxMin[Y]);
    zIntercept(_cacheParams.boxMax[X], _cacheParams.boxMax[Y]);
    zIntercept(_cacheParams.boxMin[X], _cacheParams.boxMax[Y]);
    zIntercept(_cacheParams.boxMax[X], _cacheParams.boxMin[Y]);
    // Find any vertices that exist on the Y edges of the Box
    yIntercept(_cacheParams.boxMin[X], _cacheParams.boxMin[Z]);
    yIntercept(_cacheParams.boxMax[X], _cacheParams.boxMax[Z]);
    yIntercept(_cacheParams.boxMin[X], _cacheParams.boxMax[Z]);
    yIntercept(_cacheParams.boxMax[X], _cacheParams.boxMin[Z]);
    // Find any vertices that exist on the X edges of the Box
    xIntercept(_cacheParams.boxMin[Y], _cacheParams.boxMin[Z]);
    xIntercept(_cacheParams.boxMax[Y], _cacheParams.boxMax[Z]);
    xIntercept(_cacheParams.boxMin[Y], _cacheParams.boxMax[Z]);
    xIntercept(_cacheParams.boxMax[Y], _cacheParams.boxMin[Z]);
}

stack<glm::vec2> SliceRenderer::_2DConvexHull( std::vector<_vertex>& vertices ) const {
    // We now have a set of vertices along the Box's XYZ intercepts.  The edges of these vertices define where 
    // the user should see data.  To find the connectivity/edges of these vertices, we project them into a 2D
    // coordinate system using our basis function and performing Convex Hull.

    // Project our 3D points onto the 2D plane using our basis function (_normal, _axis1, and _axis2)
    glm::vec2 unorderedTwoDPoints[vertices.size()];
    int count=0;
    for (auto& vertex : vertices) {
        double x = glm::dot(_axis1, vertex.threeD-_origin);  // Find 3D point's projected X coordinate
        double y = glm::dot(_axis2, vertex.threeD-_origin);  // Find 3D point's projected Y coordinate
        vertex.twoD = {x, y};
        unorderedTwoDPoints[count].x = vertex.twoD.x;
        unorderedTwoDPoints[count].y = vertex.twoD.y;
        count++;
    }

    // Perform convex hull on our list of 2D points,
    // which defines the outer edges of our polygon
    stack<glm::vec2> orderedTwoDPoints = convexHull( unorderedTwoDPoints, sizeof(unorderedTwoDPoints)/sizeof(unorderedTwoDPoints[0]) );
    return orderedTwoDPoints;
}

std::vector<glm::vec2> SliceRenderer::_makeRectangle2D( const std::vector<_vertex>& vertices, stack<glm::vec2>& orderedTwoDPoints ) const {
    std::vector<glm::vec2> rectangle2D={ glm::vec2(), glm::vec2() };
    while(!orderedTwoDPoints.empty()) {
        glm::vec2 vertex = orderedTwoDPoints.top();
        if(vertex.x < rectangle2D[0].x) rectangle2D[0].x = vertex.x;
        if(vertex.y < rectangle2D[0].y) rectangle2D[0].y = vertex.y;
        if(vertex.x > rectangle2D[1].x) rectangle2D[1].x = vertex.x;
        if(vertex.y > rectangle2D[1].y) rectangle2D[1].y = vertex.y;
        orderedTwoDPoints.pop(); 
    }
    return rectangle2D;
}

// Map our rectangle's 2D edges back into 3D space, to get an 
// ordered list of vertices for our data-enclosing polygon. 
std::vector<glm::vec3> SliceRenderer::_makePolygon3D( const std::vector<_vertex>& vertices, stack<glm::vec2>& polygon2D ) const {
    //_polygon3D.clear();
    std::vector<glm::vec3> polygon3D;
    while(!polygon2D.empty()) {
        glm::vec2 twoDPoint = polygon2D.top();
        for(auto& vertex : vertices) {
            if( twoDPoint.x == vertex.twoD.x && twoDPoint.y == vertex.twoD.y ) {
                polygon3D.push_back( glm::vec3( vertex.threeD.x, vertex.threeD.y, vertex.threeD.z ) );
                polygon2D.pop();
                break;
            }
        }
    }
    return polygon3D;
}

std::vector<glm::vec3> SliceRenderer::_makeRectangle3D( const std::vector<_vertex>& vertices, stack<glm::vec2>& polygon2D ) const {
    // Define a rectangle that encloses our polygon in 3D space.  We will sample along this
    // rectangle to generate our 2D texture.
    std::vector<glm::vec3> rectangle3D = { glm::vec3(), glm::vec3(), glm::vec3(), glm::vec3() };
    auto inverseProjection = [&](float x, float y) {
        glm::vec3 point;
        point = _origin + x*_axis1 + y*_axis2;
        return point;
    };
    rectangle3D = { glm::vec3(), glm::vec3(), glm::vec3(), glm::vec3() };
    rectangle3D[3] = inverseProjection( _rectangle2D[0].x, _rectangle2D[0].y );
    rectangle3D[0] = inverseProjection( _rectangle2D[1].x, _rectangle2D[0].y );
    rectangle3D[1] = inverseProjection( _rectangle2D[1].x, _rectangle2D[1].y );
    rectangle3D[2] = inverseProjection( _rectangle2D[0].x, _rectangle2D[1].y );

    return rectangle3D;
}

// Huges-Moller algorithm
// https://blog.selfshadow.com/2011/10/17/perp-vectors/
glm::vec3 SliceRenderer::_getOrthogonal(const glm::vec3 u) const {
    glm::vec3 a = abs(u);
    glm::vec3 v;
    if (a.x <= a.y && a.x <= a.z)
        v = glm::vec3(0, -u.z, u.y);
    else if (a.y <= a.x && a.y <= a.z)
        v = glm::vec3(-u.z, 0, u.x);
    else
        v = glm::vec3(-u.y, u.x, 0);
    v = glm::normalize(v);
    return v;
}

void SliceRenderer::_populateData( float* dataValues, Grid* grid ) const {
    float               varValue, missingValue;

    auto inverseProjection = [&](float x, float y) {
        glm::vec3 point;
        point = _origin + x*_axis1 + y*_axis2;
        return point;
    };

    glm::vec2 delta = (_rectangle2D[1]-_rectangle2D[0]);
    delta.x = delta.x/_xSamples;
    delta.y = delta.y/_ySamples;
    glm::vec2 offset = {delta.x/2., delta.y/2.};

    int index = 0;
    for (int j = 0; j < _ySamples; j++) {
        for (int i = 0; i < _xSamples; i++) {
            glm::vec3 samplePoint = inverseProjection( offset.x+_rectangle2D[0].x+i*delta.x, _rectangle2D[0].y+offset.y+j*delta.y );
            std::vector<double> p = {samplePoint.x, samplePoint.y, samplePoint.z};
            varValue = grid->GetValue( p );
            missingValue = grid->GetMissingValue();
            if (varValue == missingValue ||
                samplePoint.x > _cacheParams.boxMax[X] ||
                samplePoint.y > _cacheParams.boxMax[Y] ||
                samplePoint.z > _cacheParams.boxMax[Z] ||
                samplePoint.x < _cacheParams.boxMin[X] ||
                samplePoint.y < _cacheParams.boxMin[Y] ||
                samplePoint.z < _cacheParams.boxMin[Z]
                )
                dataValues[index + 1] = 1.f;
            else
                dataValues[index + 1] = 0.f;

            dataValues[index] = varValue;

            index += 2;
        }
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

    _rotate();
    _setVertexPositions();

    std::vector<double> stretch = GetViewpointParams()->GetStretchFactors();
    float dx = (_rectangle2D[1].x-_rectangle2D[0].x);
    float dy = (_rectangle2D[1].y-_rectangle2D[0].y);
    float xyRatio = dx/dy;
    if(xyRatio >= 1) {
        _xSamples = _textureSideSize;
        _ySamples = _textureSideSize/xyRatio;
    }
    else {
        _xSamples = _textureSideSize*xyRatio;
        std::cout << "xSamples " << _xSamples << " " << stretch[2] << std::endl;
        _ySamples = _textureSideSize;
    }
  
    std::cout << "xsamples ysamples " << _xSamples << " " << _ySamples <<std::endl;
 
    int    textureSize = 2 * _xSamples * _ySamples;
    float *dataValues = new float[textureSize];

    _populateData(dataValues, grid);

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
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _xSamples, _ySamples, 0, GL_RG, GL_FLOAT, dataValues);
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

    if (_cacheParams.xRotation != p->GetValueDouble(SliceParams::XRotationTag,0)) return true;
    if (_cacheParams.yRotation != p->GetValueDouble(SliceParams::YRotationTag,0)) return true;
    if (_cacheParams.zRotation != p->GetValueDouble(SliceParams::ZRotationTag,0)) return true;

    if (_cacheParams.textureSampleRate != p->GetSampleRate()) return true;

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
}

int SliceRenderer::_paintGL(bool fast)
{
    int rc = 0;

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

   
    //if (fast)
        

    if (fast) {
        // 3D green polygon that shows where we should see data
        LegacyGL *lgl = _glManager->legacy;    
        lgl->Color4f(0, 1., 0, 1.);
        lgl->Begin(GL_LINES);
            if (_polygon3D.size()) {
                for (int i=0; i<_polygon3D.size()-1; i++) {
                    glm::vec3 vert1 = _polygon3D[i];
                    glm::vec3 vert2 = _polygon3D[i+1];
                    lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
                    lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
                }
                lgl->Color4f(0, 1., 0, 1.);
                glm::vec3 vert1 = _polygon3D[_polygon3D.size()-1];
                glm::vec3 vert2 = _polygon3D[0];
                lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
                lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
            }
        lgl->End();
    }

#ifdef DEBUG 
    // 3D yellow enclosing rectangle that defines the perimeter of our texture
    // This can and often will extend beyond the Box
    lgl = _glManager->legacy;
    lgl->Begin(GL_LINES);
        double foo=0.;
        if(_rectangle3D.size()) {
            for (int i=0; i<_rectangle3D.size()-1; i++) {
                lgl->Color4f(1., 1., foo, 1.);
                glm::vec3 vert1 = _rectangle3D[i];
                glm::vec3 vert2 = _rectangle3D[i+1];
                lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
                lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
                foo=foo+.33;
            }
            lgl->Color4f(1., 1., foo, 1.);
            glm::vec3 vert1 = _rectangle3D[3];
            glm::vec3 vert2 = _rectangle3D[0];
            lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
            lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
        
        }
    lgl->End();
#endif 
      
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    _initializeState();

    // If we're in fast mode, degrade the quality of the slice for better interactivity
    if (fast) {
        _textureSideSize = 50;
    }
    else {
        _textureSideSize = _cacheParams.textureSampleRate;
    }
    if (_textureSideSize > MAX_TEXTURE_SIZE) _textureSideSize = MAX_TEXTURE_SIZE;

    if (_isDataCacheDirty() || !fast) {
        rc = _resetDataCache(fast); 
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
    if (_rectangle3D.empty() ) return;
    std::vector<double> temp = 
                    {_rectangle3D[3].x, _rectangle3D[3].y, _rectangle3D[3].z,
                     _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z,
                     _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z,
                     _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z,
                     _rectangle3D[1].x, _rectangle3D[1].y, _rectangle3D[1].z,
                     _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z};
    
    _vertexCoords = temp;

    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 3 * sizeof(double), _vertexCoords.data());
}
