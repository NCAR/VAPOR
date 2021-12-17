#pragma once

#include <vapor/glutil.h>
#ifdef Darwin
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#include <glm/glm.hpp>

class RegularGrid;

struct planeDescription {
    std::vector<double> origin;
    std::vector<double> rotation;
    std::vector<double> boxMin;
    std::vector<double> boxMax;
};

glm::tvec3<double, glm::highp> getOrthogonal(
    const glm::tvec3<double, glm::highp>& u
);

void rotate(
    glm::tvec3<double, glm::highp>& rotation,
    glm::tvec3<double, glm::highp>& normal,
    glm::tvec3<double, glm::highp>& axis1,
    glm::tvec3<double, glm::highp>& axis2
);

void findIntercepts(
    const glm::tvec3<double, glm::highp>& origin, 
    const std::vector<double>& boxMin,
    const std::vector<double>& boxMax,
    const glm::tvec3<double, glm::highp>& normal, 
    std::vector<glm::tvec3<double, glm::highp>> &vertices
);

void getMinimumAreaRectangle(
    const std::vector<glm::tvec3<double, glm::highp>> &vertices,
    const glm::tvec3<double, glm::highp>& origin,
    const glm::tvec3<double, glm::highp>& axis1,
    const glm::tvec3<double, glm::highp>& axis2,
    std::vector<glm::tvec2<double, glm::highp>>& rectangle2D,
    std::vector<glm::tvec3<double, glm::highp>>& rectangle3D
);

void populateData(
    VAPoR::Grid *grid,
    size_t sideSize,
    const glm::tvec3<double, glm::highp>& origin,
    const glm::tvec3<double, glm::highp>& axis1,
    const glm::tvec3<double, glm::highp>& axis2,
    const std::vector<glm::tvec2<double, glm::highp>>& rectangle2D,
    float* dataValues
); 

void getWindingOrder(
    std::vector<double>& windingOrder,
    const std::vector<glm::tvec3<double, glm::highp>>& rectangle3D
);

VAPoR::RegularGrid* SliceGridAlongPlane(
    VAPoR::Grid *grid3d, 
    planeDescription description, 
    size_t sideSize, 
    float *data, 
    std::vector<double>& windingOrder, 
    std::vector<double>& rectangle3D
);

