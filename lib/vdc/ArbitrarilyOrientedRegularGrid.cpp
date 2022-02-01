#include <limits>
#include <iomanip>

#include <vapor/utils.h>

#include <glm/gtc/type_ptr.hpp>
#include <GTE/MinimumAreaBox2.h>
#include <vapor/RegularGrid.h>
#include <vapor/ArbitrarilyOrientedRegularGrid.h>

using namespace std;
using namespace VAPoR;

// clang-format off

ArbitrarilyOrientedRegularGrid::ArbitrarilyOrientedRegularGrid(
    const VAPoR::Grid *grid3d,
    planeDescription& pd,
    const DimsType& dims
) : RegularGrid(
        dims, 
        {{pd.sideSize, pd.sideSize, 1}}, 
        {(_myBlks = new float[dims[0]*dims[1]*dims[2]])}, 
        {{0.,0.,0.}}, 
        {{1.,1.,1.}}
) {
    SetMissingValue( grid3d->GetMissingValue() );

    _sideSize = pd.sideSize;
    _rotation = {pd.rotation[0], pd.rotation[1], pd.rotation[2]};

    CoordType gridMin, gridMax;
    grid3d->GetUserExtents(gridMin, gridMax);

    // .vs3 files before Vapor3.6 will try to initialize Slices with origin values set to 0, which
    // can sometimes lie outside of the domain.  If this happens, we need to configure a new origin
    // within the 3D grid's bounds.
    for (int i=0; i<3; i++) {
        _origin[i] = (pd.origin[i] <= gridMin[i] && pd.origin[i] >= gridMax[i]) ? pd.origin[i] : (gridMax[i]-gridMin[i])/2 + gridMin[i];
        if (pd.origin[i] <= gridMax[i] && pd.origin[i] >= gridMin[i]) 
            _origin[i] = pd.origin[i];
        else 
            _origin[i] = (gridMax[i]-gridMin[i])/2 + gridMin[i];
    }

    // Rotate the plane via quaternion method
    _rotate();

    // Find the vertices where our plane intercepts the edges of the Box
    std::vector<glm::tvec3<double, glm::highp>> vertices;
    _findIntercepts(gridMin, gridMax, vertices);

    // Find the minimum-area-rectangle that encloses the vertices that are along the edges
    // of the Box.  Define this rectangle in 3D space, and in a newly projecteed 2D space.
    std::vector<glm::tvec3<double, glm::highp>> tmpRectangle3D;
    _getMinimumAreaRectangle( vertices );

    // Pick sample points along our 2D rectangle, and project those points back into 3D space
    // to query our 3D grid for data values.
    _populateData( grid3d, pd );
}

ArbitrarilyOrientedRegularGrid::~ArbitrarilyOrientedRegularGrid() {
    if (_myBlks != nullptr)
        delete [] _myBlks;
}

// Huges-Moller algorithm to get an orthogonal vector
// https://blog.selfshadow.com/2011/10/17/perp-vectors
glm::tvec3<double, glm::highp> ArbitrarilyOrientedRegularGrid::_getOrthogonal(const glm::tvec3<double, glm::highp>& u) 
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

void ArbitrarilyOrientedRegularGrid::_rotate() {
    // which we will use to project our polygon into 2D space.  First rotate XY plane with quaternion.
    glm::tvec3<double, glm::highp> angles(
        M_PI * _rotation.x / 180.,
        M_PI * _rotation.y / 180., 
        M_PI * _rotation.z / 180.
    );
    glm::tquat<double> q = glm::quat(angles);

    // We will sample the slice in 2D coordinates.
    // So we first define a basis function of three orthogonal vectors (normal, _axis1, _axis2).
    _normal = q * glm::tvec3<double, glm::highp>(0, 0, 1);
    _axis1 = _getOrthogonal(_normal);
    _axis2 = glm::cross(_normal, _axis1);
}

void ArbitrarilyOrientedRegularGrid::_findIntercepts(
    const VAPoR::CoordType& boxMin,
    const VAPoR::CoordType& boxMax,
    std::vector<glm::tvec3<double, glm::highp>> &vertices
)  {
    // Lambdas for finding intercepts on the XYZ edges of the Box
    // Plane equation:
    //     normal.x*(x-origin.x) + normal.y*(y-origin.y) + normal.z*(z-origin.z) = 0

    auto zIntercept = [&](double x, double y) {
        if (_normal.z == 0) return;
        double z = (_normal.x * _origin.x + _normal.y * _origin.y + _normal.z * _origin.z - _normal.x * x - _normal.y * y) / _normal.z;
        if (z >= boxMin[2] && z <= boxMax[2]) {
            glm::tvec3<double, glm::highp> p(x, y, z);
            vertices.push_back(p);
        }
    };
    auto yIntercept = [&](double x, double z) {
        if (_normal.y == 0) return;
        double y = (_normal.x * _origin.x + _normal.y * _origin.y + _normal.z * _origin.z - _normal.x * x - _normal.z * z) / _normal.y;
        if (y >= boxMin[1] && y <= boxMax[1]) {
            glm::tvec3<double, glm::highp> p(x, y, z);
            vertices.push_back(p);
        }
    };
    auto xIntercept = [&](double y, double z) {
        if (_normal.x == 0) return;
        double x = (_normal.x * _origin.x + _normal.y * _origin.y + _normal.z * _origin.z - _normal.y * y - _normal.z * z) / _normal.x;
        if (x >= boxMin[0] && x <= boxMax[0]) {
            glm::tvec3<double, glm::highp> p(x, y, z);
            vertices.push_back(p);
        }
    };

    // Find vertices that exist on the Z edges of the Box
    zIntercept(boxMin[0], boxMin[1]);
    zIntercept(boxMax[0], boxMax[1]);
    zIntercept(boxMin[0], boxMax[1]);
    zIntercept(boxMax[0], boxMin[1]);
    // Find any vertices that exist on the Y edges of the Box
    yIntercept(boxMin[0], boxMin[2]);
    yIntercept(boxMax[0], boxMax[2]);
    yIntercept(boxMin[0], boxMax[2]);
    yIntercept(boxMax[0], boxMin[2]);
    // Find any vertices that exist on the X edges of the Box
    xIntercept(boxMin[1], boxMin[2]);
    xIntercept(boxMax[1], boxMax[2]);
    xIntercept(boxMin[1], boxMax[2]);
    xIntercept(boxMax[1], boxMin[2]);
}

void ArbitrarilyOrientedRegularGrid::_getMinimumAreaRectangle(
    const std::vector<glm::tvec3<double, glm::highp>>& vertices
) {

    std::vector<gte::Vector2<double>> gteVertices;
    for (int i=0; i<vertices.size(); i++) {
        glm::tvec3<double, glm::highp> v3d = vertices[i];
        double x = glm::dot(_axis1, v3d-_origin);
        double y = glm::dot(_axis2, v3d-_origin);
        gte::Vector2<double> v{x,y};
        gteVertices.push_back(v);
    }

    typedef gte::BSRational<gte::UIntegerAP32> MABRational;
    gte::MinimumAreaBox2<double, MABRational> mab2;
    gte::OrientedBox2<double> minimalBox = mab2(vertices.size(), &gteVertices[0]);
    std::array<gte::Vector2<double>, 4> rectangle;
    minimalBox.GetVertices(rectangle);

    // _rectangle2D
    _rectangle2D.clear();
    _rectangle2D.resize(4);
    _rectangle2D[0] = glm::vec2(rectangle[0][0], rectangle[0][1]);
    _rectangle2D[1] = glm::vec2(rectangle[1][0], rectangle[1][1]);
    _rectangle2D[2] = glm::vec2(rectangle[3][0], rectangle[3][1]);
    _rectangle2D[3] = glm::vec2(rectangle[2][0], rectangle[2][1]);
}

void ArbitrarilyOrientedRegularGrid::GetUserCoordinates(const DimsType &indices, CoordType &coords) const
{
    // For now, we ignore rotated grids that have greater than 3 dimensions
    // Therefore we only index on i an j, but not k
    //
    size_t i = indices[0];
    size_t j = indices[1];

    glm::tvec2<double, glm::highp> delta( (_rectangle2D[1].x-_rectangle2D[0].x)/_sideSize, (_rectangle2D[1].y-_rectangle2D[0].y)/_sideSize );

    double xScanlineIncrement = (_rectangle2D[3].x-_rectangle2D[0].x)/_sideSize;
    double yScanlineIncrement = (_rectangle2D[3].y-_rectangle2D[0].y)/_sideSize;

    double x = _rectangle2D[0].x + j*xScanlineIncrement + i*delta.x;
    double y = _rectangle2D[0].y + j*yScanlineIncrement + i*delta.y;

    glm::tvec3<double, glm::highp> samplePoint = _origin + x*_axis1 + y*_axis2;
    coords = {samplePoint.x, samplePoint.y, samplePoint.z};
}

void ArbitrarilyOrientedRegularGrid::_populateData(
    const VAPoR::Grid *grid, 
    const planeDescription& description
) {
    VAPoR::CoordType min = description.boxMin;
    VAPoR::CoordType max = description.boxMax;
    float missingValue = grid->GetMissingValue();
    size_t index = 0;

    for (size_t j = 0; j < _sideSize; j++) {
        for (size_t i = 0; i < _sideSize; i++) {
            VAPoR::CoordType p;
            GetUserCoordinates({i,j,1}, p);

            if ( p[0] < min[0] || p[0] > max[0] ||
                 p[1] < min[1] || p[1] > max[1] ||
                 p[2] < min[2] || p[2] > max[2] ) {
                _myBlks[index] = missingValue;
            }
            else {
                _myBlks[index] = grid->GetValue(p);
            }
            index++;
        }
    }
}

// clang-format on
