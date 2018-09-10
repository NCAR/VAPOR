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
        std::vector<float>  tf_lut;
        std::vector<double> tf_minMax;
        std::vector<double> boxMin, boxMax;
    } _cacheParams;

    int  _buildCache();
    bool _isCacheDirty() const;
    void _saveCacheParams();
    void _initTexture();
    int  _saveTextureData();

    GLuint         _texture;
    int            _textureWidth;
    int            _textureHeight;
    unsigned char *_textureData;

    int      _colorMapSize;
    GLfloat *_colorMap;
};

};    // namespace VAPoR

#endif
