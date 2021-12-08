#ifndef SLICERENDERER_H
#define SLICERENDERER_H

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

class RENDER_API SliceRenderer : public Renderer {
public:
    SliceRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    static string GetClassType() { return ("Slice"); }

    virtual ~SliceRenderer();

protected:
    virtual int _initializeGL();
    virtual int _paintGL(bool fast);

private:
    struct {
        string              varName;
        string              heightVarName;
        size_t              ts;
        int                 refinementLevel;
        int                 compressionLevel;
        int                 textureSampleRate;
        int                 orientation;
        double              xRotation;
        double              yRotation;
        double              zRotation;
        double              xOrigin;
        double              yOrigin;
        double              zOrigin;
        std::vector<float>  tf_lut;
        std::vector<double> tf_minMax;
        std::vector<double> boxMin, boxMax;
        std::vector<double> domainMin, domainMax;
        std::vector<double> sampleLocation;
    } _cacheParams;

    // The SliceRenderer calculates a series of vertices in 3D space that define
    // the corners of a rectangle that we sample along.  These vertices need to
    // be projected into 2D space for sampling.  This struct defines a vertex in
    // 3D space, as well as its projection in 2D space.
    struct _vertexIn2dAnd3d {
        glm::vec3 threeD;
        glm::vec2 twoD;
    };

    void                   _rotate();
    void                   _findIntercepts(glm::vec3 &origin, glm::vec3 &normal, std::vector<_vertexIn2dAnd3d> &vertices, bool stretch) const;
    stack<glm::vec2>       _2DConvexHull(std::vector<_vertexIn2dAnd3d> &vertices) const;
    glm::vec3              _inverseProjection(float x, float y) const;
    std::vector<glm::vec2> _makeRectangle2D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    std::vector<glm::vec3> _makeRectangle3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    std::vector<glm::vec3> _makePolygon3D(const std::vector<_vertexIn2dAnd3d> &vertices, stack<glm::vec2> &polygon2D) const;
    void                   _drawDebugPolygons() const;

    void _initVAO();
    void _initTexCoordVBO();
    void _initVertexVBO();

    bool      _isColormapCacheDirty() const;
    bool      _isDataCacheDirty() const;
    bool      _isBoxCacheDirty() const;
    void      _getModifiedExtents(vector<double> &min, vector<double> &max) const;
    int       _saveCacheParams();
    void      _resetColormapCache();
    int       _resetBoxCache();
    int       _resetDataCache();
    void      _initTextures();
    void      _createDataTexture(float *dataValues);
    int       _saveTextureData();
    int       _saveTextureData2();
    void      _populateData(float *dataValues, Grid *grid) const;
    glm::vec3 _getOrthogonal(const glm::vec3 u) const;
    glm::vec3 _rotateVector(glm::vec3 vector, glm::quat rotation) const;

    double _newWaySeconds;
    double _newWayInlineSeconds;
    double _oldWaySeconds;

    int _getConstantAxis() const;

    void _configureShader();
    void _resetState();
    void _initializeState();

    void _setVertexPositions();

    glm::vec3              _axis1, _axis2, _normal, _origin;
    std::vector<glm::vec3> _polygon3D;
    std::vector<glm::vec3> _rectangle3D;
    std::vector<glm::vec2> _rectangle2D;

    bool _initialized;
    int  _textureSideSize;
    int  _xSamples;
    int  _ySamples;

    GLuint _colorMapTextureID;
    GLuint _dataValueTextureID;

    std::vector<double> _vertexCoords;
    std::vector<float>  _texCoords;

    GLuint _VAO;
    GLuint _vertexVBO;
    GLuint _texCoordVBO;

    int _colorMapSize;

    void _clearCache() { _cacheParams.varName.clear(); }
};

};    // namespace VAPoR

#endif
