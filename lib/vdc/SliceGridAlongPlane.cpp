#include <limits>
#include <iomanip>

#include <glm/gtc/type_ptr.hpp>
#include <GTE/MinimumAreaBox2.h>
#include <vapor/RegularGrid.h>
#include <vapor/SliceGridAlongPlane.h>

// clang-format off

// Huges-Moller algorithm to get an orthogonal vector
// https://blog.selfshadow.com/2011/10/17/perp-vectors/
glm::tvec3<double, glm::highp> getOrthogonal(const glm::tvec3<double, glm::highp>& u) 
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

void rotate(
    glm::tvec3<double, glm::highp>& rotation, 
    glm::tvec3<double, glm::highp>& normal, 
    glm::tvec3<double, glm::highp>& axis1, 
    glm::tvec3<double, glm::highp>& axis2 
) {
    // which we will use to project our polygon into 2D space.  First rotate XY plane with quaternion.
    glm::tvec3<double, glm::highp> angles(
        M_PI * rotation.x / 180.,
        M_PI * rotation.y / 180., 
        M_PI * rotation.z / 180.
    );
    glm::tquat<double> q = glm::quat(angles);

    // We will sample the slice in 2D coordinates.
    // So we first define a basis function of three orthogonal vectors (normal, _axis1, _axis2).
    normal = q * glm::tvec3<double, glm::highp>(0, 0, 1);
    axis1 = getOrthogonal(normal);
    axis2 = glm::cross(normal, axis1);
}

void findIntercepts(
    const glm::tvec3<double, glm::highp>& origin, 
    const VAPoR::CoordType& boxMin,
    const VAPoR::CoordType& boxMax,
    const glm::tvec3<double, glm::highp>& normal, 
    std::vector<glm::tvec3<double, glm::highp>> &vertices
)  {
    // Lambdas for finding intercepts on the XYZ edges of the Box
    // Plane equation:
    //     normal.x*(x-origin.x) + normal.y*(y-origin.y) + normal.z*(z-origin.z) = 0

    auto zIntercept = [&](double x, double y) {
        if (normal.z == 0) return;
        double z = (normal.x * origin.x + normal.y * origin.y + normal.z * origin.z - normal.x * x - normal.y * y) / normal.z;
        if (z >= boxMin[2] && z <= boxMax[2]) {
            glm::tvec3<double, glm::highp> p(x, y, z);
            vertices.push_back(p);
        }
    };
    auto yIntercept = [&](double x, double z) {
        if (normal.y == 0) return;
        double y = (normal.x * origin.x + normal.y * origin.y + normal.z * origin.z - normal.x * x - normal.z * z) / normal.y;
        if (y >= boxMin[1] && y <= boxMax[1]) {
            glm::tvec3<double, glm::highp> p(x, y, z);
            vertices.push_back(p);
        }
    };
    auto xIntercept = [&](double y, double z) {
        if (normal.x == 0) return;
        double x = (normal.x * origin.x + normal.y * origin.y + normal.z * origin.z - normal.y * y - normal.z * z) / normal.x;
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

void getMinimumAreaRectangle(
    const std::vector<glm::tvec3<double, glm::highp>>& vertices,
    const glm::tvec3<double, glm::highp>& origin,
    const glm::tvec3<double, glm::highp>& axis1,
    const glm::tvec3<double, glm::highp>& axis2,
    std::vector<glm::tvec2<double, glm::highp>>& rectangle2D,
    std::vector<glm::tvec3<double, glm::highp>>& rectangle3D
) {

    std::vector<gte::Vector2<double>> gteVertices;
    for (int i=0; i<vertices.size(); i++) {
        glm::tvec3<double, glm::highp> v3d = vertices[i];
        double x = glm::dot(axis1, v3d-origin);
        double y = glm::dot(axis2, v3d-origin);
        gte::Vector2<double> v{x,y};
        gteVertices.push_back(v);
    }

    typedef gte::BSRational<gte::UIntegerAP32> MABRational;
    gte::MinimumAreaBox2<double, MABRational> mab2;
    gte::OrientedBox2<double> minimalBox = mab2(vertices.size(), &gteVertices[0]);
    std::array<gte::Vector2<double>, 4> rectangle;
    minimalBox.GetVertices(rectangle);

    // rectangle2D
    rectangle2D.clear();
    rectangle2D.resize(4);
    rectangle2D[0] = glm::vec2(rectangle[0][0], rectangle[0][1]);
    rectangle2D[1] = glm::vec2(rectangle[1][0], rectangle[1][1]);
    rectangle2D[2] = glm::vec2(rectangle[3][0], rectangle[3][1]);
    rectangle2D[3] = glm::vec2(rectangle[2][0], rectangle[2][1]);

    // rectangle3D
    rectangle3D.clear();
    rectangle3D.resize(4);
    rectangle3D[0] = origin + rectangle[0][0]*axis1 + rectangle[0][1]*axis2;
    rectangle3D[1] = origin + rectangle[1][0]*axis1 + rectangle[1][1]*axis2;
    rectangle3D[2] = origin + rectangle[3][0]*axis1 + rectangle[3][1]*axis2;
    rectangle3D[3] = origin + rectangle[2][0]*axis1 + rectangle[2][1]*axis2;
}

void populateData(
    const VAPoR::Grid *grid, 
    size_t sideSize,
    const glm::tvec3<double, glm::highp>& origin,
    const glm::tvec3<double, glm::highp>& axis1,
    const glm::tvec3<double, glm::highp>& axis2,
    const std::vector<glm::tvec2<double, glm::highp>>& rectangle2D,
    std::unique_ptr<float>& dataValues
) {
    glm::tvec2<double, glm::highp> delta( (rectangle2D[1].x-rectangle2D[0].x)/sideSize, (rectangle2D[1].y-rectangle2D[0].y)/sideSize );
    glm::tvec2<double, glm::highp> offset = {delta.x / 2., delta.y / 2.};

    double xScanlineIncrement = (rectangle2D[3].x-rectangle2D[0].x)/sideSize;
    double yScanlineIncrement = (rectangle2D[3].y-rectangle2D[0].y)/sideSize;

    size_t index = 0;
    for (size_t j = 0; j < sideSize; j++) {
        double xStart = rectangle2D[0].x + offset.x + j*xScanlineIncrement;
        double yStart = rectangle2D[0].y + offset.y + j*yScanlineIncrement;
        for (size_t i = 0; i < sideSize; i++) {
            double x = xStart + i*delta.x;
            double y = yStart + i*delta.y;
            glm::tvec3<double, glm::highp> samplePoint = origin + x*axis1 + y*axis2;
            VAPoR::CoordType p = {samplePoint.x, samplePoint.y, samplePoint.z};
            dataValues.get()[index] = grid->GetValue(p);
            index++;
        }
    }
}

void getWindingOrder(
    std::vector<double>& windingOrder, 
    const std::vector<glm::tvec3<double, glm::highp>>& rectangle3D
) {
    if (rectangle3D.empty()) return;

    std::vector<double> temp = {
        rectangle3D[0].x, rectangle3D[0].y, rectangle3D[0].z, // green/teal
        rectangle3D[1].x, rectangle3D[1].y, rectangle3D[1].z, // prpl/red 
        rectangle3D[3].x, rectangle3D[3].y, rectangle3D[3].z, // white/green
        rectangle3D[1].x, rectangle3D[1].y, rectangle3D[1].z, // prpl/red 
        rectangle3D[2].x, rectangle3D[2].y, rectangle3D[2].z, // red/teal
        rectangle3D[3].x, rectangle3D[3].y, rectangle3D[3].z  // white/green
    };
    
    windingOrder = temp;
}

VAPoR::RegularGrid* SliceGridAlongPlane(
    const VAPoR::Grid *grid3d,
    planeDescription description,
    size_t sideSize,
    std::unique_ptr<float>& dataValues,
    std::vector<double>& windingOrder,
    std::vector<double>& rectangle3D
) {

    glm::tvec3<double, glm::highp> normal, axis1, axis2;
    glm::tvec3<double, glm::highp> rotation(
        description.rotation[0],
        description.rotation[1],
        description.rotation[2]
    );
    rotate( rotation, normal, axis1, axis2 );
    
    // Find the vertices where our plane intercepts the edges of the Box
    std::vector<glm::tvec3<double, glm::highp>> vertices;
    glm::tvec3<double, glm::highp> origin(
        description.origin[0],
        description.origin[1],
        description.origin[2]
    );
    findIntercepts(origin, description.boxMin, description.boxMax, normal, vertices);

    // Find the minimum-area-rectangle that encloses the vertices that are along the edges
    // of the Box.  Define this rectangle in 3D space, and in a newly projecteed 2D space.
    std::vector<glm::tvec2<double, glm::highp>> rectangle2D;
    std::vector<glm::tvec3<double, glm::highp>> tmpRectangle3D;
    getMinimumAreaRectangle( vertices, origin, axis1, axis2, rectangle2D, tmpRectangle3D );
    rectangle3D.clear();
    rectangle3D = { tmpRectangle3D[0].x, tmpRectangle3D[0].y, tmpRectangle3D[0].z,
                    tmpRectangle3D[1].x, tmpRectangle3D[1].y, tmpRectangle3D[1].z,
                    tmpRectangle3D[2].x, tmpRectangle3D[2].y, tmpRectangle3D[2].z,
                    tmpRectangle3D[3].x, tmpRectangle3D[3].y, tmpRectangle3D[3].z};

    // Pick sample points along our 2D rectangle, and project those points back into 3D space
    // to query our 3D grid for data values.
    populateData( grid3d, sideSize, origin, axis1, axis2, rectangle2D, dataValues );

    // Define the winding order for the two triangles that comprise the texture
    // for our data.
    getWindingOrder( windingOrder, tmpRectangle3D );

    // Finally generate the grid
    VAPoR::DimsType dims = { sideSize, sideSize, 1 };
    VAPoR::DimsType bs   = { sideSize, sideSize, 1 };
    std::vector<float*> data = { dataValues.get() };
    VAPoR::RegularGrid* slice = new VAPoR::RegularGrid( dims, 
                                                        bs, 
                                                        data, 
                                                        description.boxMin, 
                                                        description.boxMax 
                                                    );

    return slice;
}

// clang-format on
