#ifndef RAYCASTER_H
#define RAYCASTER_H

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
#include <vapor/RayCasterParams.h>
#include <vapor/ShaderMgr.h>
#include <vapor/DataMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/Grid.h>
#include <vapor/utils.h>
#include <vapor/GetAppPath.h>

namespace VAPoR {

class RENDER_API RayCaster : public Renderer {
public:
    RayCaster(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string paramsType, std::string classType, std::string &instName, DataMgr *dataMgr);

    virtual ~RayCaster();

protected:
    // C++ stuff
    // pure virtual functions from Renderer
    int _initializeGL();
    int _paintGL(bool fast);

    // Makes RayCaster an abstract class that cannot be instantiated,
    //   and it's up to the subclasses to decide which shader to load.
    virtual void _loadShaders() = 0;

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

        int *  frontFaceAttrib, *backFaceAttrib;    // Logical indices, size == bx * by * 3
        int *  rightFaceAttrib, *leftFaceAttrib;    // Logical indices, size == by * bz * 3
        int *  topFaceAttrib, *bottomFaceAttrib;    // Logical indices, size == bx * bz * 3
        float *xyCoords;                            // X-Y coordinate values
        float *zCoords;                             // Z coordinate values

        float  valueRange[2];           // min and max values of the volume
        size_t dims[3];                 // num. of samples along each axis
        float  boxMin[3], boxMax[3];    // bounding box of the current volume
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
        StructuredGrid *GetCurrentGrid(const RayCasterParams *params, DataMgr *dataMgr) const;
        bool            IsMetadataUpToDate(const RayCasterParams *params, DataMgr *dataMgr) const;
        //
        // Update meta data, as well as pointers:
        //   6 faces + dataField + missingValueMask
        //
        bool UpdateFaceAndData(const RayCasterParams *params, DataMgr *dataMgr);
        //
        // Update pointers: xyCoords and zCoords
        //   Note: meta data is updated in UpdateFaceAndData(), but *NOT* here, so
        //         UpdateFaceAndData() needs to be called priori to UpdateCurviCoords().
        //
        bool UpdateCurviCoords(const RayCasterParams *params, DataMgr *dataMgr);
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
    GLuint _xyCoordsTextureId;        // GL_TEXTURE5
    GLuint _zCoordsTextureId;         // GL_TEXTURE6

    // buffers
    GLuint _frameBufferId;
    GLuint _depthBufferId;
    GLuint _xyCoordsBufferId;
    GLuint _zCoordsBufferId;
    GLenum _drawBuffers[2];    // Draw buffers for the 1st and 2nd pass

    // vertex arrays
    GLuint _vertexArrayId;
    GLuint _vertexBufferId;    // Keeps user coordinates of 6 faces.
    GLuint _indexBufferId;     // Auxiliary indices for efficiently drawing triangle strips.
    GLuint _vertexAttribId;    // Attribute of vertices: (i, j k) logical indices.

    // shaders
    GLuint _1stPassShaderId;
    GLuint _2ndPassShaderId;
    GLuint _3rdPassShaderId;
    GLuint _3rdPassMode1ShaderId;
    GLuint _3rdPassMode2ShaderId;

    // current viewport in use
    GLint _currentViewport[4];

    //
    // Render the volume surface using triangle strips
    //   This is a subroutine used by _drawVolumeFaces().
    //
    void _renderTriangleStrips(int whichPass, long castingMode) const;

    void _drawVolumeFaces(int whichPass, long whichCastingMode, bool insideACell = false, const GLfloat *ModelView = nullptr, const GLfloat *InversedMV = nullptr, bool fast = false);

    void _load3rdPassUniforms(long castingMode, const GLfloat *MVP, const GLfloat *ModelView, const GLfloat *InversedMV, bool fast) const;

    virtual void _3rdPassSpecialHandling(bool fast, long castingMode);

    //
    // Initialization for 1) framebuffers and 2) textures
    //
    void _initializeFramebufferTextures();

    //
    // Simple shader compilation
    //
    GLuint _compileShaders(const char *vertex_file_path, const char *fragment_file_path);

    //
    // Get current Model View Projection matrix that can be passed to shaders
    //   Note: MVP should be a memory space of 16 GLfloats that is already allocated.
    //         The MVP matrix is stored in a colume-major fashion.
    void _getMVPMatrix(GLfloat *MVP) const;

    double _getElapsedSeconds(const struct timeval *begin, const struct timeval *end) const;

    //
    // Helper matrix manipulation functions
    //
    bool _mesa_invert_matrix_general(GLfloat out[16], const GLfloat in[16]);
    void _mesa_transposef(GLfloat to[16], const GLfloat from[16]);
    void _printMatrix(const GLfloat m[16]);

    //
    // Multiply a 4-value vector by a 4x4 matrix.
    //
    void _matMultiVec(const GLfloat *matrix, const GLfloat *vecIn, GLfloat *vecOut) const;

};    // End of class RayCaster

};    // End of namespace VAPoR

#endif
