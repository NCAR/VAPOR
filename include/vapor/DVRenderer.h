#ifndef DVRENDERER_H
#define DVRENDERER_H

#include <GL/glew.h>
#include <sys/time.h>

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

class RENDER_API DVRenderer : public Renderer {
public:
    DVRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr);

    virtual ~DVRenderer();

    static std::string GetClassType() { return ("Volume"); }

protected:
    // pure virtual functions that are required to implement
    int _initializeGL();
    int _paintGL(bool fast);

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
        float *        frontFace, *backFace;    // user coordinates, size == bx * by * 3
        float *        rightFace, *leftFace;    // user coordinates, size == by * bz * 3
        float *        topFace, *bottomFace;    // user coordinates, size == bx * bz * 3
        float *        dataField;               // data field of this volume
        unsigned char *missingValueMask;        // 0 == is missing value; non-zero == not missing value
        float          valueRange[2];           // min and max values of the volume
        size_t         dims[3];                 // num. of samples along each axis
        float          boxMin[3], boxMax[3];    // bounding box of the current volume
                                                // !! NOTE boxMin and boxMax most likely differ from extents from  params !!

        //             0---------2
        //              |       |
        //              |       |
        //              |       |
        //             1|_______|3
        // Also keep the coordinates of 4 vertices of the the near clipping plane.
        float nearCoords[12];

        /* Also keep the current meta data */
        size_t      myCurrentTimeStep;
        std::string myVariableName;
        int         myRefinementLevel, myCompressionLevel;

        /* Member functions */
        UserCoordinates();
        ~UserCoordinates();
        StructuredGrid *GetCurrentGrid(const DVRParams *params, DataMgr *dataMgr) const;
        bool            IsUpToDate(const DVRParams *params, DataMgr *dataMgr) const;
        bool            UpdateCoordinates(const DVRParams *params, DataMgr *dataMgr);
    };    // end of struct UserCoordinates

    UserCoordinates    _userCoordinates;
    std::vector<float> _colorMap;
    float              _colorMapRange[2];

    // OpenGL stuff
    // textures
    GLuint _backFaceTextureId;        // GL_TEXTURE0
    GLuint _frontFaceTextureId;       // GL_TEXTURE1
    GLuint _volumeTextureId;          // GL_TEXTURE2
    GLuint _missingValueTextureId;    // GL_TEXTURE3
    GLuint _colorMapTextureId;        // GL_TEXTURE4

    // buffers
    GLuint _frameBufferId;
    GLuint _depthBufferId;
    GLenum _drawBuffers[2];    // Draw buffers for the 1st and 2nd pass

    // vertex arrays
    GLuint _vertexArrayId;
    GLuint _vertexBufferId;
    GLuint _indexBufferId;

    // shaders
    GLuint _1stPassShaderId;
    GLuint _2ndPassShaderId;
    GLuint _3rdPassShaderId;

    //
    // Draw faces using triangle strips
    //
    void _drawVolumeFaces(int whichPass, bool insideACell = false, const GLfloat *ModelView = nullptr, const GLfloat *InversedMV = nullptr);

    //
    // Initialization for 1) framebuffers and 2) textures
    //
    void _initializeFramebufferTextures();

    //
    // Simple shader compilation
    //
    GLuint _loadShaders(const char *vertex_file_path, const char *fragment_file_path);

    //
    // Get current Model View Projection matrix that can be passed to shaders
    //   Note: MVP should be a memory space of 16 GLfloats that is already allocated.
    //         The MVP matrix is stored in a colume-major fashion.
    void _getMVPMatrix(GLfloat *MVP) const;

    //
    // Multiply a 4-value vector by a 4x4 matrix.
    //
    void _matMultiVec(const GLfloat *matrix, const GLfloat *vecIn, GLfloat *vecOut) const;

    double _getElapsedSeconds(const struct timeval *begin, const struct timeval *end) const;

    //
    // Helper matrix manipulation functions
    //
    bool _mesa_invert_matrix_general(GLfloat out[16], const GLfloat in[16]);
    void _mesa_transposef(GLfloat to[16], const GLfloat from[16]);
    void _printMatrix(const GLfloat m[16]);

};    // End of class DVRenderer

};    // End of namespace VAPoR

#endif
