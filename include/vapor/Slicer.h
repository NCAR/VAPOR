#ifndef SLICER_H
#define SLICER_H

#include <vapor/glutil.h>

#ifdef Darwin
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <vapor/DataMgr.h>
#include <vapor/utils.h>
#include <vapor/Renderer.h>

namespace VAPoR {

class RENDER_API Slicer {
public:
    Slicer(RenderParams* rp, DataMgr* dm);
    ~Slicer();

    RegularGrid* GetSlice( size_t sideSize );

    // The _windingOrder is the four vertices of our slice's enclosing rectangle,
    // listed in the order that they need to be listed to correctly draw the two triangles
    // that comprise our texture.
    std::vector<double> GetWindingOrder() const;

    // Returns a polygon that encloses the slice
    std::vector<glm::tvec3<double, glm::highp>> GetPolygon() const;

    // Returns a rectangle that encloses the slice
    std::vector<glm::tvec3<double, glm::highp>> GetRectangle() const;

private:
    RenderParams* _renderParams;
    DataMgr*      _dataMgr;

    // The Slicer calculates a series of vertices in 3D space that define
    // the corners of a rectangle that we sample along.  These vertices need to
    // be projected into 2D space for sampling.  This struct defines a vertex in
    // 3D space, as well as its projection in 2D space.
    struct _vertexIn2dAnd3d {
        glm::tvec3<double, glm::highp> threeD;
        glm::vec2 twoD;
    };

    void                   _updateParameters();
    int                    _get3DGrid( Grid*& grid3d );

    // _rotate(), and its utility functions
    void                   _rotate();
    glm::tvec3<double, glm::highp>                  _getOrthogonal(const glm::tvec3<double, glm::highp> u) const;
    void                       _findIntercepts(glm::tvec3<double, glm::highp> &origin, glm::tvec3<double, glm::highp> &normal, std::vector<_vertexIn2dAnd3d> &vertices, bool stretch) const;
    stack<glm::vec2>           _2DConvexHull(std::vector<_vertexIn2dAnd3d> &vertices) const;
    glm::tvec3<double, glm::highp>                  _inverseProjection(double x, double y) const;
    std::vector<glm::vec2>     _makeRectangle2D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    std::vector<glm::tvec3<double, glm::highp>>     _makePolygon3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    std::vector<glm::tvec3<double, glm::highp>>     _makeRectangle3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    std::vector<glm::tvec3<double, glm::highp>>     _makeRectangle3D2(const std::vector<_vertexIn2dAnd3d> &vertices) const;
    std::vector<glm::tvec3<double, glm::highp>>     _makeRectangle3D3(const std::vector<_vertexIn2dAnd3d> &vertices) const;

    void _generateWindingOrder();
    void      _populateData(Grid *grid) const;
    glm::tvec3<double, glm::highp> _rotateVector(glm::tvec3<double, glm::highp> vector, glm::quat rotation) const;

    glm::tvec3<double, glm::highp>              _axis1, _axis2, _normal, _origin, _rotation;
    std::vector<glm::tvec3<double, glm::highp>> _polygon3D;
    std::vector<glm::vec2> _polygon2D;
    std::vector<glm::tvec3<double, glm::highp>> _rectangle3D;
    std::vector<glm::vec2> _rectangle2D;
    std::vector<double>    _boxMin, _boxMax;
    float* _dataValues;

    size_t _textureSideSize;

    std::vector<double> _windingOrder;
    std::vector<double>  _texCoords;
};

};    // namespace VAPoR

#endif
