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

    RegularGrid* GetSlice();

    virtual ~Slicer();

private:
    RenderParams* _renderParams;
    DataMgr*      _dataMgr;

    // The Slicer calculates a series of vertices in 3D space that define
    // the corners of a rectangle that we sample along.  These vertices need to
    // be projected into 2D space for sampling.  This struct defines a vertex in
    // 3D space, as well as its projection in 2D space.
    struct _vertexIn2dAnd3d {
        glm::vec3 threeD;
        glm::vec2 twoD;
    };

    void                   _updateParameters();

    int                    _get3DGrid( Grid*& grid3d );

    // _rotate() function, and its utility functions
    void                   _rotate();
    glm::vec3                  _getOrthogonal(const glm::vec3 u) const;
    void                       _findIntercepts(glm::vec3 &origin, glm::vec3 &normal, std::vector<_vertexIn2dAnd3d> &vertices, bool stretch) const;
    stack<glm::vec2>           _2DConvexHull(std::vector<_vertexIn2dAnd3d> &vertices) const;
    glm::vec3                  _inverseProjection(float x, float y) const;
    std::vector<glm::vec2>     _makeRectangle2D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    std::vector<glm::vec3>     _makePolygon3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    std::vector<glm::vec3>     _makeRectangle3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;

    void                   _generateWindingOrder();

    void      _populateData(float *dataValues, Grid *grid) const;
    glm::vec3 _rotateVector(glm::vec3 vector, glm::quat rotation) const;

    glm::vec3              _axis1, _axis2, _normal, _origin, _rotation;
    std::vector<glm::vec3> _polygon3D;
    std::vector<glm::vec3> _rectangle3D;
    std::vector<glm::vec2> _rectangle2D;
    std::vector<double>    _boxMin, _boxMax;

    size_t _textureSideSize;

    std::vector<double> _windingOrder;
    std::vector<float>  _texCoords;
};

};    // namespace VAPoR

#endif
