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

#define DEBUG 1

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
        VAPoR::CoordType    boxMin, boxMax;
        VAPoR::CoordType    domainMin, domainMax;
        std::vector<double> sampleLocation;
    } _cacheParams;

    void _initVAO();
    void _initTexCoordVBO();
    void _initVertexVBO();

    bool _isColormapCacheDirty() const;
    bool _isDataCacheDirty() const;
    bool _isBoxCacheDirty() const;
    void _getExtents(VAPoR::CoordType &min, VAPoR::CoordType &max) const;
    void _resetColormapCache();
    void _resetCache();
    void _createDataTexture(std::unique_ptr<float> &dataValues);
    int  _regenerateSlice();
    int  _getGrid3D(Grid *&grid) const;
#ifdef DEBUG
    void _drawDebugPolygons();
#endif

    void _configureShader();
    void _resetState();
    void _initializeState();

    bool _initialized;
    size_t  _textureSideSize;

    GLuint _colorMapTextureID;
    GLuint _dataValueTextureID;

    std::vector<double> _windingOrder;
    std::vector<double> _rectangle3D;

    GLuint _VAO;
    GLuint _vertexVBO;
    GLuint _texCoordVBO;

    int _colorMapSize;

    void _clearCache() { _cacheParams.varName.clear(); }
};

};    // namespace VAPoR

#endif
