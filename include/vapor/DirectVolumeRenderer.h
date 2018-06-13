#ifndef DIRECTVOLUMERENDERER_H
#define DIRECTVOLUMERENDERER_H

#include <GL/glew.h>

#ifdef Darwin
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <vapor/Visualizer.h>
#include <vapor/DVRParams.h>
#include <vapor/ShaderMgr.h>
#include <vapor/DataMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/Grid.h>
#include <vapor/utils.h>

namespace VAPoR {

class RENDER_API DirectVolumeRenderer : public Renderer {
public:
    DirectVolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr);

    virtual ~DirectVolumeRenderer();

    static std::string GetClassType() { return ("Direct_Volume_Renderer"); }

protected:
    // pure virtual functions that are required to implement
    int _initializeGL();
    int _paintGL();

private:
    // C++ stuff
    struct {
        std::string          varName;
        size_t               ts;
        int                  level;
        int                  lod;
        std::vector<double>  boxMin, boxMax;
        std::vector<GLfloat> colormap;
    } _cacheParams;
    void _saveCacheParams();
    bool _isCacheDirty() const;

    // OpenGL stuff
    const std::string _effectNameStr = "DVR";

    GLuint _volumeTextureUnit;      // GL_TEXTURE0
    GLuint _colormapTextureUnit;    // GL_TEXTURE1
    // GLuint              _coordmapTextureUnit;         // GL_TEXTURE2 ??
    GLuint _volumeCoordinateTextureUnit;    // GL_TEXTURE3

};    // End of class DirectVolumeRenderer

};    // End of namespace VAPoR

#endif
