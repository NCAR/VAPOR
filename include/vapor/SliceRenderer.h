#ifndef SLICERENDERER_H
#define SLICERENDERER_H

#include <vapor/glutil.h>

#ifdef Darwin
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

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
        std::vector<float>  tf_lut;
        std::vector<double> tf_minMax;
        std::vector<double> boxMin, boxMax;
        std::vector<double> domainMin, domainMax;
        std::vector<double> sampleLocation;
    } _cacheParams;

    void _initVAO();
    void _initTexCoordVBO();
    void _initVertexVBO();

    bool _isColormapCacheDirty() const;
    bool _isDataCacheDirty() const;
    bool _isBoxCacheDirty() const;
    void _getModifiedExtents(vector<double> &min, vector<double> &max) const;
    int  _saveCacheParams();
    void _resetColormapCache();
    int  _resetBoxCache();
    int  _resetDataCache();
    void _initTextures();
    void _createDataTexture(float *dataValues);
    int  _saveTextureData();
    void _populateDataXY(float *dataValues, Grid *grid) const;
    void _populateDataXZ(float *dataValues, Grid *grid) const;
    void _populateDataYZ(float *dataValues, Grid *grid) const;

    double _newWaySeconds;
    double _newWayInlineSeconds;
    double _oldWaySeconds;

    std::vector<double> _calculateDeltas() const;

    int _getConstantAxis() const;

    void _configureShader();
    void _resetState();
    void _initializeState();
    void _resetTextureCoordinates();

    void _setVertexPositions();
    void _setXYVertexPositions(std::vector<double> min, std::vector<double> max);
    void _setXZVertexPositions(std::vector<double> min, std::vector<double> max);
    void _setYZVertexPositions(std::vector<double> min, std::vector<double> max);

    bool _initialized;
    int  _textureSideSize;

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
