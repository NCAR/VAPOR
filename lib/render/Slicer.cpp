//#include <sstream>
//#include <string>
#include <limits>
//#include <ctime>

#include <iomanip>
#include <vapor/Slicer.h>
//#include <vapor/ResourcePath.h>
#include <vapor/MyBase.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/ConvexHull.h>
#include <glm/gtc/type_ptr.hpp>
#include <vapor/GTE/MinimumAreaBox2.h>

#define X  0
#define Y  1
#define Z  2
#define XY 0
#define XZ 1
#define YZ 2

#define MAX_TEXTURE_SIZE 2000

#define DEBUG 1

using namespace VAPoR;

Slicer::Slicer( RenderParams* rp, DataMgr* dm )
{
    _renderParams = rp;
    _dataMgr      = dm;

    _windingOrder = {0.0f, 0.0f, 0.f, 
                     1.0f, 0.0f, 0.f, 
                     0.0f, 1.0f, 0.f, 
                     1.0f, 0.0f, 0.f, 
                     1.0f, 1.0f, 0.f, 
                     0.0f, 1.0f, 0.f};

    _texCoords = {0.0f, 0.0f, 1.0f, 
                  0.0f, 0.0f, 1.0f, 
                  1.0f, 0.0f, 1.0f, 
                  1.0f, 0.0f, 1.0f};

    _dataValues = nullptr;
}

Slicer::~Slicer()
{
    if (_dataValues != nullptr) {
        delete[] _dataValues;
        _dataValues = nullptr;
    }
}

RegularGrid* Slicer::GetSlice( size_t sideSize ) {
    _textureSideSize = sideSize;
    _updateParameters();
    _rotate();
    _generateWindingOrder();

    Grid* grid3d = nullptr;
    int rc = _get3DGrid( grid3d );
    if (rc < 0) return nullptr;
    
    if (_dataValues != nullptr) {
        delete[] _dataValues;
        _dataValues = nullptr;
    }
    int textureSize = _textureSideSize * _textureSideSize;
    _dataValues = new float[textureSize];  // Needs to be allocated as in gridTools.cpp
    _populateData(grid3d);

    std::vector<size_t> dims = { _textureSideSize, _textureSideSize };
    std::vector<size_t> bs   = { _textureSideSize, _textureSideSize };
    std::vector<float*> data = { _dataValues };  // Maybe use data.values() instead of dataValues?
    std::vector<double> minu = { 0,0,0 };
    std::vector<double> maxu = { 2,2,2 };
    RegularGrid* slice = new RegularGrid( dims, bs, data, _boxMin, _boxMax );

    _dataMgr->UnlockGrid(grid3d);
    delete grid3d;
    grid3d = nullptr;

    return slice;
}

std::vector<double> Slicer::GetWindingOrder() const {
    return _windingOrder;
}

std::vector<glm::tvec3<double, glm::highp>> Slicer::GetPolygon() const {
    return _polygon3D;
}

std::vector<glm::tvec3<double, glm::highp>> Slicer::GetRectangle() const {
    return _rectangle3D;
}

void Slicer::_updateParameters() {
    Box *box = _renderParams->GetBox();
    box->GetExtents(_boxMin, _boxMax);
    VAssert(_boxMin.size() == 3);
    VAssert(_boxMax.size() == 3);

    _rotation = {_renderParams->GetValueDouble(RenderParams::XSlicePlaneRotationTag, 0.), 
                 _renderParams->GetValueDouble(RenderParams::YSlicePlaneRotationTag, 0.), 
                 _renderParams->GetValueDouble(RenderParams::ZSlicePlaneRotationTag, 0.)};

    double xMid = (_boxMax[X] - _boxMin[X]) / 2. + _boxMin[X];
    double yMid = (_boxMax[Y] - _boxMin[Y]) / 2. + _boxMin[Y];
    double zMid = (_boxMax[Z] - _boxMin[Z]) / 2. + _boxMin[Z];
    _origin     = {_renderParams->GetValueDouble(RenderParams::XSlicePlaneOriginTag, xMid), 
                   _renderParams->GetValueDouble(RenderParams::YSlicePlaneOriginTag, yMid), 
                   _renderParams->GetValueDouble(RenderParams::ZSlicePlaneOriginTag, zMid)};
}

int Slicer::_get3DGrid( Grid*& grid3d ) {
    int rLevel = _renderParams->GetRefinementLevel();
    int cLevel = _renderParams->GetCompressionLevel();
    int   rc = DataMgrUtils::GetGrids(_dataMgr, 
                                      _renderParams->GetCurrentTimestep(), 
                                      _renderParams->GetVariableName(), 
                                      _boxMin, _boxMax, true,
                                      &rLevel, &cLevel, &grid3d);
    if (rc < 0) {
        Wasp::MyBase::SetErrMsg("Unable to acquire Grid for Slice texture");
        return rc;
    }
    VAssert(grid3d);
    grid3d->SetInterpolationOrder(1);

    return 0;
}

void Slicer::_rotate()
{
    // which we will use to project our polygon into 2D space.  First rotate XY plane with quaternion.
    glm::tvec3<double, glm::highp> angles(M_PI * _rotation.x / 180., M_PI * _rotation.y / 180., M_PI * _rotation.z / 180.);
    glm::tquat<double> q = glm::quat(angles);

    // We will sample the slice in 2D coordinates.
    // So we first define a basis function of three orthogonal vectors (_normal, _axis1, _axis2).
    _normal = q * glm::tvec3<double, glm::highp>(0, 0, 1);
    _axis1 = _getOrthogonal(_normal);
    _axis2 = glm::cross(_normal, _axis1);

    // Next we define a polygon that defines our slice.  To do this we
    // find where our plane intercepts the X, Y, and Z edges of our Box extents.
    //
    // Each _vertexIn2dAnd3d in 'vertices' holds a glm::tvec3<double, glm::highp> representing a point 3D space,
    // and a glm::vec2 storing its location in 2D according to our basis function.
    std::vector<_vertexIn2dAnd3d> vertices;
    _findIntercepts(_origin, _normal, vertices, false);

    std::vector<gte::Vector2<double>> mVertices;    
    //mVertices.resize(vertices.size());
    //for (auto& v: vertices) {
    for (int i=0; i<vertices.size(); i++) {
        glm::tvec3<double, glm::highp> v3d = vertices[i].threeD;
        double x = glm::dot(_axis1, v3d-_origin);
        double y = glm::dot(_axis2, v3d-_origin);
        gte::Vector2<double> v{x,y};
        mVertices.push_back(v);
    }

    typedef gte::BSRational<gte::UIntegerAP32> MABRational;
    gte::MinimumAreaBox2<double, MABRational> mab2;
    gte::OrientedBox2<double> minimalBox = mab2(vertices.size(), &mVertices[0]);
    std::array<gte::Vector2<double>, 4> vertex;
    minimalBox.GetVertices(vertex);



    // Use Convex Hull to get an ordered list of vertices
    stack<glm::vec2> polygonStack2D = _2DConvexHull(vertices);

    // _rectangle2D
    _rectangle2D.clear();
    _rectangle2D.resize(4);
    _rectangle2D[0] = glm::vec2(vertex[0][0], vertex[0][1]);
    _rectangle2D[1] = glm::vec2(vertex[1][0], vertex[1][1]);
    _rectangle2D[2] = glm::vec2(vertex[3][0], vertex[3][1]);
    _rectangle2D[3] = glm::vec2(vertex[2][0], vertex[2][1]);
    /*_rectangle2D.clear();
    stack<glm::vec2> s = polygonStack2D;
    while (!s.empty()) {
        glm::vec2 point = s.top();
        _rectangle2D.push_back(point);
        s.pop();
    }*/

    // _polygon3D
    stack<glm::vec2> s = polygonStack2D;
    //s = polygonStack2D;
    _polygon3D = _makePolygon3D(vertices, s);

    // _rectangle3D
    s = polygonStack2D;
    _rectangle3D.clear();
    int const lookup[4][2] = { { 0, 1 }, { 1, 3 }, { 3, 2 }, { 2, 0 } };
    _rectangle3D.push_back( _inverseProjection( vertex[0][0], vertex[0][1] ));
    _rectangle3D.push_back( _inverseProjection( vertex[1][0], vertex[1][1] ));
    _rectangle3D.push_back( _inverseProjection( vertex[3][0], vertex[3][1] ));
    _rectangle3D.push_back( _inverseProjection( vertex[2][0], vertex[2][1] ));
    /*for (int i=0; i<4; i++) {
        //float x = minimalBox[i][0];
        //float y = minimalBox[i][1];
        float x = vertex[i][0];
        float y = vertex[i][1];
        glm::tvec3<double, glm::highp> point = _inverseProjection( x, y );
        _rectangle3D.push_back(point);
    }*/
    /*while (!s.empty()) {
        float x = s.top().x;
        float y = s.top().y;
        glm::tvec3<double, glm::highp> point = _inverseProjection( x, y );
        _rectangle3D.push_back(point);
        s.pop();
    }
    std::cout << _rectangle3D.size() << std::endl;*/

    //_rectangle3D = _makeRectangle3D2(vertices);



    /*double x0 = glm::dot(_axis1, _rectangle3D[0]-_origin);    // Find 3D point's projected X coordinate
    double y0 = glm::dot(_axis2, _rectangle3D[0]-_origin);    // Find 3D point's projected Y coordinate
    double x1 = glm::dot(_axis1, _rectangle3D[1]-_origin);    // Find 3D point's projected X coordinate
    double y1 = glm::dot(_axis2, _rectangle3D[1]-_origin);    // Find 3D point's projected Y coordinate
    _rectangle2D = {glm::vec2(x0,y0), 
                    glm::vec2(x1,y0),
                    glm::vec2(x1,y1),
                    glm::vec2(x0,y1)};*/
    


    // At this point, 'vertices' has a vector<glm::tvec3<double, glm::highp>> that describe the 3D points of our polygon,
    // and a vector<glm::vec2> that describe the 2D points of our polygon.
    // The 3D and 2D values map to eachother.  IE - 3D element 0 corresponds the the coordinates of 2D element 0.

    // Find a rectangle that encompasses our 3D polygon by finding the min/max bounds along our 2D points.
    // We will sample along the X/Y axes of this rectangle.
    //stack<glm::vec2> s = polygonStack2D;
//    s = polygonStack2D;
//    _rectangle2D = _makeRectangle2D(vertices, s);

    // Map our rectangle's 2D edges back into 3D space, to get an
    // ordered list of vertices for our data-enclosing rectangle.
//    s = polygonStack2D;
//    _polygon3D = _makePolygon3D(vertices, s);

    // Define a rectangle that encloses our polygon in 3D space.  We will sample along this
    // rectangle to generate our 2D texture.
//    s = polygonStack2D;
//    _rectangle3D = _makeRectangle3D(vertices, s);
}

void Slicer::_findIntercepts(glm::tvec3<double, glm::highp> &_origin, glm::tvec3<double, glm::highp> &_normal, std::vector<_vertexIn2dAnd3d> &vertices, bool stretch) const
{
    // Lambdas for finding intercepts on the XYZ edges of the Box
    // Plane equation:
    //     _normal.x*(x-_origin.x) + _normal.y*(y-_origin.y) + _normal.z*(z-_origin.z) = 0

    auto zIntercept = [&](double x, double y) {
        if (_normal.z == 0) return;
        double z = (_normal.x * _origin.x + _normal.y * _origin.y + _normal.z * _origin.z - _normal.x * x - _normal.y * y) / _normal.z;
        if (z >= _boxMin[Z] && z <= _boxMax[Z]) {
            _vertexIn2dAnd3d p = {glm::tvec3<double, glm::highp>(x, y, z), glm::vec2()};
            vertices.push_back(p);
        }
    };
    auto yIntercept = [&](double x, double z) {
        if (_normal.y == 0) return;
        double y = (_normal.x * _origin.x + _normal.y * _origin.y + _normal.z * _origin.z - _normal.x * x - _normal.z * z) / _normal.y;
        if (y >= _boxMin[Y] && y <= _boxMax[Y]) {
            _vertexIn2dAnd3d p = {glm::tvec3<double, glm::highp>(x, y, z), glm::vec2()};
            vertices.push_back(p);
        }
    };
    auto xIntercept = [&](double y, double z) {
        if (_normal.x == 0) return;
        double x = (_normal.x * _origin.x + _normal.y * _origin.y + _normal.z * _origin.z - _normal.y * y - _normal.z * z) / _normal.x;
        if (x >= _boxMin[X] && x <= _boxMax[X]) {
            _vertexIn2dAnd3d p = {glm::tvec3<double, glm::highp>(x, y, z), glm::vec2()};
            vertices.push_back(p);
        }
    };

    // Find vertices that exist on the Z edges of the Box
    zIntercept(_boxMin[X], _boxMin[Y]);
    zIntercept(_boxMax[X], _boxMax[Y]);
    zIntercept(_boxMin[X], _boxMax[Y]);
    zIntercept(_boxMax[X], _boxMin[Y]);
    // Find any vertices that exist on the Y edges of the Box
    yIntercept(_boxMin[X], _boxMin[Z]);
    yIntercept(_boxMax[X], _boxMax[Z]);
    yIntercept(_boxMin[X], _boxMax[Z]);
    yIntercept(_boxMax[X], _boxMin[Z]);
    // Find any vertices that exist on the X edges of the Box
    xIntercept(_boxMin[Y], _boxMin[Z]);
    xIntercept(_boxMax[Y], _boxMax[Z]);
    xIntercept(_boxMin[Y], _boxMax[Z]);
    xIntercept(_boxMax[Y], _boxMin[Z]);
}

stack<glm::vec2> Slicer::_2DConvexHull(std::vector<_vertexIn2dAnd3d> &vertices) const
{
    VAssert(vertices.size() > 0);

    // We now have a set of vertices along the Box's XYZ intercepts.  The edges of these vertices define where
    // the user should see data.  To find the connectivity/edges of these vertices, we will first project them into a 2D
    // coordinate system using our basis function, and then perform Convex Hull on those 2D coordinates.

    // Project our 3D points onto the 2D plane using our basis function (_normal, _axis1, and _axis2)
    glm::vec2 *unorderedTwoDPoints = new glm::vec2[vertices.size()];
    int        count = 0;
    for (auto &vertex : vertices) {
        double x = glm::dot(_axis1, vertex.threeD - _origin);    // Find 3D point's projected X coordinate
        double y = glm::dot(_axis2, vertex.threeD - _origin);    // Find 3D point's projected Y coordinate
        vertex.twoD = {x, y};
        unorderedTwoDPoints[count].x = vertex.twoD.x;
        unorderedTwoDPoints[count].y = vertex.twoD.y;
        count++;
    }

    // Perform convex hull on our list of 2D points,
    // which defines the outer edges of our polygon
    stack<glm::vec2> orderedTwoDPoints = convexHull(unorderedTwoDPoints, count);

    if (unorderedTwoDPoints != nullptr) {
        delete[] unorderedTwoDPoints;
        unorderedTwoDPoints = nullptr;
    }

    return orderedTwoDPoints;
}

std::vector<glm::vec2> Slicer::_makeRectangle2D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &orderedTwoDPoints) const
{
    std::vector<glm::vec2> rectangle2D = {glm::vec2(), glm::vec2()};
    while (!orderedTwoDPoints.empty()) {
        glm::vec2 vertex = orderedTwoDPoints.top();
        if (vertex.x < rectangle2D[0].x) rectangle2D[0].x = vertex.x;
        if (vertex.y < rectangle2D[0].y) rectangle2D[0].y = vertex.y;
        if (vertex.x > rectangle2D[1].x) rectangle2D[1].x = vertex.x;
        if (vertex.y > rectangle2D[1].y) rectangle2D[1].y = vertex.y;
        orderedTwoDPoints.pop();
    }
    return rectangle2D;
}

// Map our rectangle's 2D edges back into 3D space, to get an
// ordered list of vertices for our data-enclosing polygon.
std::vector<glm::tvec3<double, glm::highp>> Slicer::_makePolygon3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const
{
    std::vector<glm::tvec3<double, glm::highp>> polygon3D;
    while (!polygon2D.empty()) {
        glm::vec2 twoDPoint = polygon2D.top();
        for (auto &vertex : vertices) {
            if (twoDPoint.x == vertex.twoD.x && twoDPoint.y == vertex.twoD.y) {
                polygon3D.push_back(glm::tvec3<double, glm::highp>(vertex.threeD.x, vertex.threeD.y, vertex.threeD.z));
                polygon2D.pop();
                break;
            }
        }
    }
    return polygon3D;
}

std::vector<glm::tvec3<double, glm::highp>> Slicer::_makeRectangle3D3(const std::vector<_vertexIn2dAnd3d> &vertices) const
{
    /*for (auto vertex : _polygon3D) {
        if (vertex.x
        _rectangle3D.push_back(vertex);
    }*/
}

//std::vector<glm::tvec3<double, glm::highp>> Slicer::_makeRectangle3D2(std::vector<glm::tvec3<double, glm::highp>> &polygon3D) const
std::vector<glm::tvec3<double, glm::highp>> Slicer::_makeRectangle3D2(const std::vector<_vertexIn2dAnd3d> &vertices) const
{
    glm::tvec3<double, glm::highp> min = _polygon3D[0];
    glm::tvec3<double, glm::highp> max = _polygon3D[0];

    for (auto vertex : _polygon3D) {
        if (vertex.x < min.x) min.x = vertex.x;
        if (vertex.y < min.y) min.y = vertex.y;
        if (vertex.z < min.z) min.z = vertex.z;
        if (vertex.x > max.x) max.x = vertex.x;
        if (vertex.y > max.y) max.y = vertex.y;
        if (vertex.z > max.z) max.z = vertex.z;
    }

    std::vector<glm::tvec3<double, glm::highp>> rectangle3D = {glm::tvec3<double, glm::highp>(min.x, min.y, min.z),
                                          glm::tvec3<double, glm::highp>(max.x, max.y, min.z),
                                          glm::tvec3<double, glm::highp>(max.x, max.y, max.z),
                                          glm::tvec3<double, glm::highp>(min.x, min.y, max.z)};

    // We know the plane exists at (min min min) and (max max max)
    // Where are the other two corners of the plane?

    return rectangle3D;        
}

std::vector<glm::tvec3<double, glm::highp>> Slicer::_makeRectangle3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const
{
    // Define a rectangle that encloses our polygon in 3D space.  We will sample along this
    // rectangle to generate our 2D texture.
    std::vector<glm::tvec3<double, glm::highp>> rectangle3D = {glm::tvec3<double, glm::highp>(), glm::tvec3<double, glm::highp>(), glm::tvec3<double, glm::highp>(), glm::tvec3<double, glm::highp>()};
    rectangle3D = {glm::tvec3<double, glm::highp>(), glm::tvec3<double, glm::highp>(), glm::tvec3<double, glm::highp>(), glm::tvec3<double, glm::highp>()};
    rectangle3D[3] = _inverseProjection(_rectangle2D[0].x, _rectangle2D[0].y);
    rectangle3D[0] = _inverseProjection(_rectangle2D[1].x, _rectangle2D[0].y);
    rectangle3D[1] = _inverseProjection(_rectangle2D[1].x, _rectangle2D[1].y);
    rectangle3D[2] = _inverseProjection(_rectangle2D[0].x, _rectangle2D[1].y);

    return rectangle3D;
}

// Huges-Moller algorithm to get an orthogonal vector
// https://blog.selfshadow.com/2011/10/17/perp-vectors/
glm::tvec3<double, glm::highp> Slicer::_getOrthogonal(const glm::tvec3<double, glm::highp> u) const
{
    glm::tvec3<double, glm::highp> a = abs(u);
    glm::tvec3<double, glm::highp> v;
    if (a.x <= a.y && a.x <= a.z)
        v = glm::tvec3<double, glm::highp>(0, -u.z, u.y);
    else if (a.y <= a.x && a.y <= a.z)
        v = glm::tvec3<double, glm::highp>(-u.z, 0, u.x);
    else
        v = glm::tvec3<double, glm::highp>(-u.y, u.x, 0);
    v = glm::normalize(v);
    return v;
}

glm::tvec3<double, glm::highp> Slicer::_inverseProjection(double x, double y) const
{
    glm::tvec3<double, glm::highp> point;
    point = _origin + (double)x * _axis1 + (double)y * _axis2;
    return point;
}

void Slicer::_populateData(Grid *grid) const
{
    //glm::vec2 delta = (_rectangle2D[1] - _rectangle2D[0]);
    glm::vec2 delta = {_rectangle2D[1].x-_rectangle2D[0].x, _rectangle2D[1].y-_rectangle2D[0].y};
    delta.x = delta.x / _textureSideSize;
    delta.y = delta.y / _textureSideSize;
    glm::vec2 offset = {delta.x / 2., delta.y / 2.};

    double xScanlineIncrement = (_rectangle2D[3].x-_rectangle2D[0].x)/_textureSideSize;
    double yScanlineIncrement = (_rectangle2D[3].y-_rectangle2D[0].y)/_textureSideSize;

    int index = 0;
    for (int j = 0; j < _textureSideSize; j++) {
        double xStart = _rectangle2D[0].x + offset.x + j*xScanlineIncrement;
        double yStart = _rectangle2D[0].y + offset.y + j*yScanlineIncrement;
        for (int i = 0; i < _textureSideSize; i++) {
            double x = xStart + i*delta.x;
            double y = yStart + i*delta.y;
            glm::tvec3<double, glm::highp> samplePoint = _inverseProjection(x,y);
            //glm::tvec3<double, glm::highp> samplePoint = _inverseProjection(offset.x + _rectangle2D[0].x + i * delta.x, _rectangle2D[0].y + offset.y + j * delta.y);
            CoordType p = {samplePoint.x, samplePoint.y, samplePoint.z};
            _dataValues[index] = grid->GetValue(p);
            //std::cout << std::setprecision(6) << "p/v " << grid->GetValue(p) << " " << samplePoint.x << " " << samplePoint.y << " " << samplePoint.z << " " << x << " " << y << std::endl;
            //std::cout << std::setprecision(6) << "p/v " << grid->GetValue(p) << " " << x << " " << y << std::endl;
            index++;
        }
    }
}

void Slicer::_generateWindingOrder()
{
    if (_rectangle3D.empty()) return;
    /*std::vector<double> temp = {_rectangle3D[3].x, _rectangle3D[3].y, _rectangle3D[3].z, 
                                _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z,
                                _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z, 
                                _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z,
                                _rectangle3D[1].x, _rectangle3D[1].y, _rectangle3D[1].z, 
                                _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z};*/


                                                                                         // yellow = teal
    /*std::vector<double> temp = {_rectangle3D[3].x, _rectangle3D[3].y, _rectangle3D[3].z, // 1 white/green -> 2 green/teal       XX
                                _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z, // 2 green/teal  -> 5 prpl/red
                                _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z, // 3 teal/white  -> 1 white/green
                                _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z, // 4 teal/prpl   -> 5 prpl/red
                                _rectangle3D[1].x, _rectangle3D[1].y, _rectangle3D[1].z, // 5 prpl/red    -> 6 red/teal         XX
                                _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z};// 6 red/teal    -> 1 white/green*/
   
    // Nope 
    /*std::vector<double> temp = {
                                 1 _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z, // green/teal
                                 2 _rectangle3D[1].x, _rectangle3D[1].y, _rectangle3D[1].z, // prpl/red 
                                 3 _rectangle3D[3].x, _rectangle3D[3].y, _rectangle3D[3].z, // white/green
                                 4 _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z, // red/teal
                                 5 _rectangle3D[3].x, _rectangle3D[3].y, _rectangle3D[3].z, // white/green
                                 6 _rectangle3D[1].x, _rectangle3D[1].y, _rectangle3D[1].z, // prpl/red */
    std::vector<double> temp = {
                                 _rectangle3D[0].x, _rectangle3D[0].y, _rectangle3D[0].z, // green/teal
                                 _rectangle3D[1].x, _rectangle3D[1].y, _rectangle3D[1].z, // prpl/red 
                                 _rectangle3D[3].x, _rectangle3D[3].y, _rectangle3D[3].z, // white/green
                                 _rectangle3D[1].x, _rectangle3D[1].y, _rectangle3D[1].z, // prpl/red 
                                 _rectangle3D[2].x, _rectangle3D[2].y, _rectangle3D[2].z, // red/teal
                                 _rectangle3D[3].x, _rectangle3D[3].y, _rectangle3D[3].z}; // white/green



    _windingOrder = temp;
}
