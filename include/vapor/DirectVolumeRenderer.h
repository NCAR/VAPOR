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

    static std::string GetClassType() { return ("DirectVolumeRenderer"); }

protected:
    // pure virtual functions that are required to implement
    int _initializeGL();
    int _paintGL();

private:
    // C++ stuff
    const std::string _effectNameStr = "DVR";

    void _printGLInfo() const;

    struct UserCoordinates {
        //              Y
        //              |   Z (coming out the screen)
        //              |  /
        //              | /
        //              |/
        //            0 --------X
        float *frontFace, *backFace;    // user coordinates, size == bx * by * 3
        float *rightFace, *leftFace;    // user coordinates, size == by * bz * 3
        float *topFace, *bottomFace;    // user coordinates, size == bx * bz * 3
        size_t dims[3];                 // num. of samples along each axis

        UserCoordinates();     // constructor
        ~UserCoordinates();    // destructor
        void Fill(const VAPoR::StructuredGrid *grid);
    };

    struct Cache {
        std::string          varName;
        size_t               ts;
        int                  level;
        int                  lod;
        float                boxMin[3], boxMax[3];
        std::vector<GLfloat> colormap;
        UserCoordinates      userCoords;
    };

    Cache _cache;
    void  _saveCacheParams(bool considerUserCoord);    // True: consider user coordinates too
    bool  _isCacheDirty() const;

    // OpenGL stuff
    GLuint _volumeTextureUnit;      // GL_TEXTURE0
    GLuint _colormapTextureUnit;    // GL_TEXTURE1
    // GLuint              _coordmapTextureUnit;         // GL_TEXTURE2 ??
    GLuint _volumeCoordinateTextureUnit;    // GL_TEXTURE3
    GLuint _frameBufferId;
    GLuint _backFaceTextureId;
    GLuint _frontFaceTextureId;
    GLuint _depthBufferId;

    GLuint _vertexArrayId;
    GLuint _1stPassShaderId;
    GLuint _2ndPassShaderId;
    GLuint _3rdPassShaderId;
    GLuint _quadShaderId;

    //
    // Draw faces using triangle strips
    // Used in the 1st pass volume rendering
    //
    void _drawVolumeFaces(const float *frontFace, const float *backFace, const float *rightFace, const float *leftFace, const float *topFace, const float *bottomFace,
                          const float * boxmin,    // Three elements: minimal X, Y, Z of this volume.
                          const float * boxmax,    // Three elements: maximal X, Y, Z of this volume.
                          const size_t *dims,      // num. of grid points in X, Y, Z for 6 faces.
                          int           whichPass);          // 1st, 2nd, or 3rd pass?

    //
    // Draw a quad that helps examine the texture.
    //
    void _drawQuad();

    //
    // Texture initializations
    //
    void _initializeTextures();

    //
    // Simple shader compilation
    //
    GLuint _loadShaders(const char *vertex_file_path, const char *fragment_file_path);

    //
    // Get current Model View Projection matrix that can be passed to shaders
    //   Note: MVP should be a memory space of 16 GLfloats that is already allocated.
    //         The MVP matrix is stored in a colume-major fashion.
    void _getMVPMatrix(GLfloat *MVP) const;

};    // End of class DirectVolumeRenderer

};    // End of namespace VAPoR

#endif
