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

    struct UserCoordinates {
        float *frontFace, *backFace;    // user coordinates, size == bx * by * 3
        float *rightFace, *leftFace;    // user coordinates, size == by * bz * 3
        float *topFace, *bottomFace;    // user coordinates, size == bx * bz * 3
        size_t dims[3];                 // num. of samples along each axis

        UserCoordinates();     // constructor
        ~UserCoordinates();    // destructor
        void Fill(const VAPoR::StructuredGrid *grid);
    };

    struct CacheParams {
        std::string          varName;
        size_t               ts;
        int                  level;
        int                  lod;
        std::vector<double>  boxMin, boxMax;
        std::vector<GLfloat> colormap;
        UserCoordinates      userCoords;
    };

    CacheParams _cacheParams;
    void        _saveCacheParams(bool considerUserCoord);    // True: consider user coordinates too
                                                             // False: not consider user coordinates
    bool _isCacheDirty() const;

    // OpenGL stuff
    GLuint _volumeTextureUnit;      // GL_TEXTURE0
    GLuint _colormapTextureUnit;    // GL_TEXTURE1
    // GLuint              _coordmapTextureUnit;         // GL_TEXTURE2 ??
    GLuint _volumeCoordinateTextureUnit;    // GL_TEXTURE3

    //
    // Draw faces using triangle strips
    // Used in the 1st pass volume rendering
    //
    virtual void _drawVolumeFaces(const float *frontFace, const float *backFace, const float *rightFace, const float *leftFace, const float *topFace, const float *bottomFace,
                                  const double *volumeMin,    // array of 3 values
                                  const double *volumeMax,
                                  const size_t *dims,    // num. of grid points
                                  bool          frontFacing);

};    // End of class DirectVolumeRenderer

};    // End of namespace VAPoR

#endif
