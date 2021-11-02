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


// https://pastebin.com/fAFp6NnN
glm::vec3 SliceRenderer::_rotateVector(glm::vec3 value, glm::quat rotation) const
{
    glm::vec3 vector;
    float num12 = rotation.x + rotation.x;
    float num2 = rotation.y + rotation.y;
    float num = rotation.z + rotation.z;
    float num11 = rotation.w * num12;
    float num10 = rotation.w * num2;
    float num9 = rotation.w * num;
    float num8 = rotation.x * num12;
    float num7 = rotation.x * num2;
    float num6 = rotation.x * num;
    float num5 = rotation.y * num2;
    float num4 = rotation.y * num;
    float num3 = rotation.z * num;
    float num15 = ((value.x * ((1. - num5) - num3)) + (value.y * (num7 - num9))) + (value.z * (num6 + num10));
    float num14 = ((value.x * (num7 + num9)) + (value.y * ((1. - num8) - num3))) + (value.z * (num4 - num11));
    float num13 = ((value.x * (num6 - num10)) + (value.y * (num4 + num11))) + (value.z * ((1. - num8) - num5));
    vector.x = num15;
    vector.y = num14;
    vector.z = num13;
    return vector;
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

    _cacheParams.xRotation = p->GetValueDouble(SliceParams::XRotationTag,0);
    _cacheParams.yRotation = p->GetValueDouble(SliceParams::YRotationTag,0);
    _cacheParams.zRotation = p->GetValueDouble(SliceParams::ZRotationTag,0);

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

void SliceRenderer::_rotate()
{
    std::vector<double> boxMin = _cacheParams.boxMin;
    std::vector<double> boxMax = _cacheParams.boxMax;

    //std::vector<double> boxOrigin = {boxMax[X]-boxMin[X], boxMax[Y]-boxMin[Y], boxMax[Z]-boxMin[Z]};
    //std::vector<double> origin = {(boxMax[X]-boxMin[X])/2. + boxMin[X], 
    glm::vec3 origin = {(boxMax[X]-boxMin[X])/2. + boxMin[X], 
                        (boxMax[Y]-boxMin[Y])/2. + boxMin[Y], 
                        (boxMax[Z]-boxMin[Z])/2. + boxMin[Z]};

    glm::vec3 angles( M_PI*_cacheParams.xRotation/180., M_PI*_cacheParams.yRotation/180., M_PI*_cacheParams.zRotation/180. );
    glm::quat q = glm::quat( angles );
    glm::vec3 normal = q * glm::vec3(0,0,1);
    //glm::vec3 sliceNormal = _rotateVector(glm::vec3(0,0,1), q);

    // a(x-xo) + b(y-yo) + c(z-zo) = 0

    //normal.x*(x-origin.x) + normal.y*(y-origin.y) + normal.z*(z-origin.z) = 0
    
    //std::vector< glm::vec3 > vertices;
    std::vector< _point > vertices;
 
    auto zIntercept = [&](float x, float y) {
        if(normal.z==0) return;
        //double z = (normal.x*origin[X] + normal.y*origin[Y] + normal.z*origin[Z] - normal.x*x - normal.y*y) / normal.z;
        double z = (normal.x*origin.x + normal.y*origin.y + normal.z*origin.z - normal.x*x - normal.y*y) / normal.z;
        if (z >= boxMin[Z] && z <= boxMax[Z]) {
            //vertices.push_back( glm::vec3(x,y,z) );
            _point p = { glm::vec3(x,y,z), glm::vec2() };
            vertices.push_back(p);
            //vertices.push_back( glm::vec3(x,y,z) );
            //std::cout << "Z hit " << p.threeD.x << " " << p.threeD.y << " " << p.threeD.z << std::endl;
        }
    };
    auto yIntercept = [&](float x, float z) {
        if(normal.y==0) return;
        double y = (normal.x*origin.x + normal.y*origin.y + normal.z*origin.z - normal.x*x - normal.z*z) / normal.y;
        if (y >= boxMin[Y] && y <= boxMax[Y]) {
            //vertices.push_back( glm::vec3(x,y,z) );
            _point p = { glm::vec3(x,y,z), glm::vec2() };
            vertices.push_back(p);
            //std::cout << "Y hit " << p.threeD.x << " " << p.threeD.y << " " << p.threeD.z << std::endl;
        }
    };
    auto xIntercept = [&](float y, float z) {
        if(normal.x==0) return;
        double x = (normal.x*origin.x + normal.y*origin.y + normal.z*origin.z - normal.y*y - normal.z*z) / normal.x;
        if (x >= boxMin[X] && x <= boxMax[X]) {
            //vertices.push_back( glm::vec3(x,y,z) );
            _point p = { glm::vec3(x,y,z), glm::vec2() };
            vertices.push_back(p);
            //std::cout << "X hit " << p.threeD.x << " " << p.threeD.y << " " << p.threeD.z << std::endl;
        }
    };

    // Find any vertices that exist on the Z edges of the box
    zIntercept(boxMin[X], boxMin[Y]);
    zIntercept(boxMax[X], boxMax[Y]);
    zIntercept(boxMin[X], boxMax[Y]);
    zIntercept(boxMax[X], boxMin[Y]);
    // Find any vertices that exist on the Y edges of the box
    yIntercept(boxMin[X], boxMin[Z]);
    yIntercept(boxMax[X], boxMax[Z]);
    yIntercept(boxMin[X], boxMax[Z]);
    yIntercept(boxMax[X], boxMin[Z]);
    // Find any vertices that exist on the X edges of the box
    xIntercept(boxMin[Y], boxMin[Z]);
    xIntercept(boxMax[Y], boxMax[Z]);
    xIntercept(boxMin[Y], boxMax[Z]);
    xIntercept(boxMax[Y], boxMin[Z]);

    // Project our polygon into 2D space for convex hull algorithm to find edges
    glm::vec3 axis1 = _getOrthogonal( normal );
    glm::vec3 axis2 = glm::cross( normal, axis1 );
    
    // https://stackoverflow.com/questions/23472048/projecting-3d-points-to-2d-plane
    glm::vec2 points[vertices.size()];
    int count=0;
    for (auto& vertex : vertices) {
        double x = glm::dot(axis1, vertex.threeD-origin);
        double y = glm::dot(axis2, vertex.threeD-origin);
        vertex.twoD = {x, y};
        points[count].x = vertex.twoD.x;
        points[count].y = vertex.twoD.y;
        count++;
    }

    // Perform convex hull on our list of points that have been projected into 2D space.
    // These points define the edges of our polygon.
    stack<glm::vec2> orderedTwoDPoints = convexHull(points, sizeof(points)/sizeof(points[0]));
    std::cout << std::endl;

    stack<glm::vec2> s = orderedTwoDPoints;
    VAssert( orderedTwoDPoints.size() == vertices.size() );
  
    // Use the edge-defining list of 2D points to create an edge-defining list of 3D points.
    // Each 2D point maps to its corresponding 3D point.
    //std::vector<glm::vec3> orderedVertices;
    orderedVertices.clear();
    while(!orderedTwoDPoints.empty()) {
        glm::vec2 twoDPoint = orderedTwoDPoints.top();
        for(auto& vertex : vertices) {
            if( twoDPoint.x == vertex.twoD.x && twoDPoint.y == vertex.twoD.y ) {
                orderedVertices.push_back( glm::vec3( vertex.threeD.x, vertex.threeD.y, vertex.threeD.z ) );
                //std::cout << "2D " << glm::to_string(twoDPoint) << std::endl;
                orderedTwoDPoints.pop();
                continue;
            }
        }
    }

    // Find minimum and maximum extents of our square in 3D space
    glm::vec3 firstVertex = orderedVertices[0];

          // Point 1      2            3            4
    square={ firstVertex, firstVertex, firstVertex, firstVertex };
    for(auto& vertex : orderedVertices) {
        if(vertex.x < square[0].x) square[0].x = vertex.x;
        if(vertex.y < square[0].y) square[0].y = vertex.y;
        if(vertex.z < square[0].z) square[0].z = vertex.z;

        if(vertex.x > square[2].x) square[2].x = vertex.x;
        if(vertex.y > square[2].y) square[2].y = vertex.y;
        if(vertex.z > square[2].z) square[2].z = vertex.z;
    }
    for(auto& vertex:square) {
        //std::cout << "Square " << glm::to_string(vertex) << std::endl;
    }

    square[1].x = square[2].x;
    square[1].y = square[0].y;
    square[1].z = square[0].z;

    square[3].x = square[0].x;
    square[3].y = square[2].y;
    square[3].z = square[2].z;

    // Parallelogram axis 1
    // Find the longest edge of our polyhedron to serve as the first axis of our paralellogram
    //
    v1 = orderedVertices[orderedVertices.size()-1];
    v2 = orderedVertices[0];

    glm::vec3 longest[2] = {v1, v2};
    double length = sqrt( pow(v1.x-v2.x,2) + pow(v1.y-v2.y,2) + pow(v1.z-v2.z,2) );
    for(int i = 0; i<orderedVertices.size()-2; i++) {
        v1 = orderedVertices[i];
        v2 = orderedVertices[i+1];
        double newLength = sqrt( pow(v1.x-v2.x,2) + pow(v1.y-v2.y,2) + pow(v1.z-v2.z,2) );
        if ( newLength > length ) {
            length = newLength;
            //std::cout << "new length " << length << " " << glm::to_string(v1) << " " << glm::to_string(v2) << std::endl;
            longest[0] = v1;
            longest[1] = v2;
        }
    }

    // Parallelogram axis 2
    // Find the longest edge that's adjacent to the previously found longest edge to use
    // as the second axis of our parallelogram
    v3 = glm::vec3(std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
    v4 = glm::vec3(std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
    for(int i=0; i<orderedVertices.size(); i++) {
        glm::vec3 vertex = orderedVertices[i];

        glm::vec3 neighbor1;
        if (i==0) neighbor1 = orderedVertices[orderedVertices.size()-1];
        else neighbor1 = orderedVertices[i-1];
        double length1 = sqrt( pow(vertex.x-neighbor1.x,2) + pow(vertex.y-neighbor1.y,2) + pow(vertex.z-neighbor1.z,2) );
        
        glm::vec3 neighbor2;
        if (i==orderedVertices.size()-1) neighbor2 = orderedVertices[0];
        else neighbor2 = orderedVertices[i+1];
        double length2 = sqrt( pow(neighbor1.x-neighbor2.x,2) + pow(neighbor1.y-neighbor2.y,2) + pow(neighbor1.z-neighbor2.z,2) );

        if (length1 > length2) v3 = neighbor1;
        else v4 = neighbor2;
        //std::cout << (vertex==v1) << std::endl;
    }

    // Now solve for either v3 or v4

    std::cout << "vertices " << glm::to_string(v1) << " " << glm::to_string(v2) << " " << glm::to_string(v3) << " " << glm::to_string(v4) << std::endl;
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
    return v;
}

/*
// A utility function to find next to top in a stack
TwoDPoint SliceRenderer::_nextToTop(stack<TwoDPoint> &S) const
{
    TwoDPoint p = S.top();
    S.pop();
    TwoDPoint res = S.top();
    S.push(p);
    return res;
}

// A utility function to swap two points
void SliceRenderer::_swap(TwoDPoint &p1, TwoDPoint &p2) const
{
    TwoDPoint temp = p1;
    p1 = p2;
    p2 = temp;
}

// A utility function to return square of distance
// between p1 and p2
int SliceRenderer::_distSq(TwoDPoint p1, TwoDPoint p2) const
{
    return (p1.x - p2.x)*(p1.x - p2.x) +
          (p1.y - p2.y)*(p1.y - p2.y);
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int SliceRenderer::_orientation(TwoDPoint p, TwoDPoint q, TwoDPoint r) const
{
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;  // collinear
    return (val > 0)? 1: 2; // clock or counterclock wise
}

// A function used by library function qsort() to sort an array of
// points with respect to the first point
int SliceRenderer::_compare(const void *vp1, const void *vp2) const
{
   TwoDPoint *p1 = (TwoDPoint *)vp1;
   TwoDPoint *p2 = (TwoDPoint *)vp2;

   // Find orientation
   int o = orientation(p0, *p1, *p2);
   if (o == 0)
     return (distSq(p0, *p2) >= distSq(p0, *p1))? -1 : 1;

   return (o == 2)? -1: 1;
}

// Prints convex hull of a set of n points.
stack<TwoDPoint> convexHull(TwoDPoint points[], int n)
{
   // Find the bottommost point
   int ymin = points[0].y, min = 0;
   for (int i = 1; i < n; i++)
   {
     int y = points[i].y;

     // Pick the bottom-most or chose the left
     // most point in case of tie
     if ((y < ymin) || (ymin == y &&
         points[i].x < points[min].x))
        ymin = points[i].y, min = i;
   }

   // Place the bottom-most point at first position
   swap(points[0], points[min]);

   // Sort n-1 points with respect to the first point.
   // A point p1 comes before p2 in sorted output if p2
   // has larger polar angle (in counterclockwise
   // direction) than p1
   p0 = points[0];
   qsort(&points[1], n-1, sizeof(TwoDPoint), compare);

   // If two or more points make same angle with p0,
   // Remove all but the one that is farthest from p0
   // Remember that, in above sorting, our criteria was
   // to keep the farthest point at the end when more than
   // one points have same angle.
   int m = 1; // Initialize size of modified array
   for (int i=1; i<n; i++)
   {
       // Keep removing i while angle of i and i+1 is same
       // with respect to p0
       while (i < n-1 && orientation(p0, points[i],
                                    points[i+1]) == 0)
          i++;


       points[m] = points[i];
       m++;  // Update size of modified array
   }

   // If modified array of points has less than 3 points,
   // convex hull is not possible
   if (m < 3) return;

   // Create an empty stack and push first three points
   // to it.
   stack<TwoDPoint> S;
   S.push(points[0]);
   S.push(points[1]);
   S.push(points[2]);

   // Process remaining n-3 points
   for (int i = 3; i < m; i++)
   {
      // Keep removing top while the angle formed by
      // points next-to-top, top, and points[i] makes
      // a non-left turn
      while (S.size()>1 && orientation(nextToTop(S), S.top(), points[i]) != 2)
         S.pop();
      S.push(points[i]);
   }

    return S;

   // Now stack has the output points, print contents of stack
   while (!S.empty())
   {
       TwoDPoint p = S.top();
       cout << "(" << p.x << ", " << p.y <<")" << endl;
       S.pop();
   }
}*/

void SliceRenderer::_resetTextureCoordinates()
{
    _rotate();

    float texMinX, texMinY, texMaxX, texMaxY;

    std::vector<double> boxMin = _cacheParams.boxMin;
    std::vector<double> boxMax = _cacheParams.boxMax;
    std::vector<double> domainMin = _cacheParams.domainMin;
    std::vector<double> domainMax = _cacheParams.domainMax;
    int xAxis, yAxis;

    xAxis = X;
    yAxis = Y;
    /*if (orientation == XY) {
        xAxis = X;
        yAxis = Y;
    } else if (orientation == XZ) {
        xAxis = X;
        yAxis = Z;
    } else {    // (orientation = YZ)
        xAxis = Y;
        yAxis = Z;
    }*/

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
    std::array<double, 3> coords = {0.0, 0.0, 0.0};
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
    std::array<double, 3> coords = {0.0, 0.0, 0.0};
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
    std::array<double, 3> coords = {0.0, 0.0, 0.0};
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

    _populateDataXY(dataValues, grid);
    /*if (_cacheParams.orientation == XY)
        _populateDataXY(dataValues, grid);
    else if (_cacheParams.orientation == XZ)
        _populateDataXZ(dataValues, grid);
    else if (_cacheParams.orientation == YZ)
        _populateDataYZ(dataValues, grid);
    else
        VAssert(0);*/

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

    if (_cacheParams.xRotation != p->GetValueDouble(SliceParams::XRotationTag,0)) return true;
    if (_cacheParams.yRotation != p->GetValueDouble(SliceParams::YRotationTag,0)) return true;
    if (_cacheParams.zRotation != p->GetValueDouble(SliceParams::ZRotationTag,0)) return true;

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
    //min[Z] = sampleLocation[Z];
    //max[Z] = sampleLocation[Z];
    /*switch (box->GetOrientation()) {
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
    }*/
}

int SliceRenderer::_paintGL(bool fast)
{
    int rc = 0;

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    LegacyGL *lgl = _glManager->legacy;    

    /*lgl->Color4f(0, 1., 0, 1.);
    lgl->Begin(GL_LINES);
        if (orderedVertices.size()) {
            for (int i=0; i<orderedVertices.size()-1; i++) {
                glm::vec3 vert1 = orderedVertices[i];
                glm::vec3 vert2 = orderedVertices[i+1];
                lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
                lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
            }
            lgl->Color4f(0, 1., 0, 1.);
            glm::vec3 vert1 = orderedVertices[orderedVertices.size()-1];
            glm::vec3 vert2 = orderedVertices[0];
            lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
            lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
        }
    lgl->End();*/

    lgl = _glManager->legacy;
    lgl->Begin(GL_LINES);
        std::cout << std::endl;
        std::cout << "v1 " << glm::to_string(v1) << std::endl;
        std::cout << "v2 " << glm::to_string(v2) << std::endl;
        std::cout << "v4 " << glm::to_string(v4) << std::endl;
        lgl->Color4f(1., 1., 0, 1.);
        lgl->Vertex3f(v1.x,v1.y,v1.z);
        lgl->Vertex3f(v2.x,v2.y,v2.z);

        lgl->Color4f(.5, .5, .5, 1.);
        lgl->Vertex3f(v1.x,v1.y,v1.z);
        lgl->Vertex3f(v4.x,v4.y,v4.z);
        //lgl->Vertex3f(v3.x,v3.y,v3.z);

        //lgl->Vertex3f(v3.x,v3.y,v3.z);
        //lgl->Vertex3f(v4.x,v4.y,v4.z);

        //lgl->Vertex3f(v4.x,v4.y,v4.z);
        //lgl->Vertex3f(v1.x,v1.y,v1.z);

        /*double foo=0.;
        lgl->Color4f(1., 1., 0, 1.);
        if(square.size()) {
            for (int i=0; i<square.size()-1; i++) {
                lgl->Color4f(1., 1., foo, 1.);
                foo=foo+.25;
                glm::vec3 vert1 = square[i];
                glm::vec3 vert2 = square[i+1];
                lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
                lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
            }
            lgl->Color4f(1., 1., foo, 1.);
            glm::vec3 vert1 = square[orderedVertices.size()-1];
            glm::vec3 vert2 = square[0];
            lgl->Vertex3f(vert1.x,vert1.y,vert1.z);
            lgl->Vertex3f(vert2.x,vert2.y,vert2.z);
        
        }*/

        //lgl->Vertex3f(v1.x,v1.y,v1.z);
        //lgl->Vertex3f(v2.x,v2.y,v2.z);
        //lgl->Vertex3f(v2.x,v2.y,v2.z);
        //lgl->Vertex3f(v4.x,v4.y,v4.z);
    lgl->End();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    _initializeState();

    if (_isDataCacheDirty()) {
        rc = _resetDataCache(); if (rc < 0) {
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

    //int orientation = _cacheParams.orientation;
    _setXYVertexPositions(trimmedMin, trimmedMax);
    /*if (orientation == XY)
        _setXYVertexPositions(trimmedMin, trimmedMax);
    else if (orientation == XZ)
        _setXZVertexPositions(trimmedMin, trimmedMax);
    else if (orientation == YZ)
        _setYZVertexPositions(trimmedMin, trimmedMax);*/

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
    return Z;
    /*if (_cacheParams.orientation == XY)
        return Z;
    else if (_cacheParams.orientation == XZ)
        return Y;
    else    // (_cacheParams.orientation == XY )
        return X;*/
}
