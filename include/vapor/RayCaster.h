#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <vapor/glutil.h>
#ifndef WIN32
#include <sys/time.h>
#endif

#include <vapor/Renderer.h>
#include "vapor/RayCasterParams.h"
#include "vapor/GLManager.h"

#include <glm/glm.hpp>

namespace VAPoR 
{

class RENDER_API RayCaster : public Renderer
{
public:

    RayCaster(  const ParamsMgr*  pm, 
                std::string&      winName, 
                std::string&      dataSetName,
                std::string       paramsType,
                std::string       classType,
                std::string&      instName, 
                DataMgr*          dataMgr);

    virtual ~RayCaster();

protected:
    // C++ stuff
    // pure virtual functions from Renderer
    int _initializeGL();
    int _paintGL( bool fast );
    void _clearCache() {};

    // Makes RayCaster an abstract class that cannot be instantiated,
    //   and it's up to the subclasses to decide which shader to load.
    //   It returns 0 upon success, and non-zero upon errors.
    virtual int _load3rdPassShaders() = 0;

    enum CastingMode
    {
        FixedStep = 1, CellTraversal = 2
    };

    class UserCoordinates
    {
        // Note: class UserCoordinates lives completely inside of class RayCaster, 
        //   and is solely used by class RayCaster. Thus for simplicity, it has all
        //   of its member variables and methods public. 
        public:
        //              Y
        //              |   Z (coming out the screen)
        //              |  /
        //              | /
        //              |/
        //            0 --------X
        float *frontFace,       *backFace;           // user coordinates, size == bx * by * 3
        float *rightFace,       *leftFace;           // user coordinates, size == by * bz * 3
        float *topFace,         *bottomFace;         // user coordinates, size == bx * bz * 3   
        float *dataField;                // data field of this volume
        unsigned char* missingValueMask; // 0 == is missing value; non-zero == not missing value
        float* vertCoords;               // vertex coordinates in user coordinates
        float* secondVarData;            // values of a second variable
        unsigned char* secondVarMask;    // 0 == is missing value; non-zero == not missing value

        size_t  dims[3];                 // num. of samples along each axis. 

        /* Also keep the current meta data */ 
        size_t      myCurrentTimeStep;
        std::string myVariableName;
        std::string my2ndVarName;
        int         myRefinementLevel, myCompressionLevel;
        float       myGridMin[3], myGridMax[3];   // Keeps the min and max of the current grid.
                                                  // !!NOT!! the value retrieved from params.

        // A few flags to indicate if certain data is out of date
        bool        dataFieldUpToDate;
        bool        vertCoordsUpToDate;
        bool        secondVarUpToDate;

        /* Member functions */
        UserCoordinates();    
       ~UserCoordinates();  

        //
        // It returns 0 upon success, and non-zero upon errors.
        //
        int GetCurrentGrid( const RayCasterParams* params,
                                  DataMgr*         dataMgr,
                                  StructuredGrid** gridpp ) const;

        void CheckUpToDateStatus( const RayCasterParams* params, 
                                  const StructuredGrid*  grid, 
                                        DataMgr*         dataMgr,
                                        bool             use2ndVar );
        //
        // Update meta data, as well as pointers: 6 faces + dataField + missingValueMask
        //   It returns 0 upon success, and non-zero upon errors:
        // 
        int UpdateFaceAndData(    const RayCasterParams* params,
                                  const StructuredGrid*  grid, 
                                        DataMgr*         dataMgr );
        //
        // Update pointers: xyCoords and zCoords
        // |-- Note: meta data is updated in UpdateFaceAndData(), but *NOT* here, so
        // |         UpdateFaceAndData() needs to be called priori to UpdateVertCoords().
        // |-- It returns 0 upon success, and non-zero upon errors:
        //
        int UpdateVertCoords(   const   RayCasterParams* params,
                                const   StructuredGrid*  grid, 
                                        DataMgr*         dataMgr );

        int Update2ndVariable(  const   RayCasterParams* params,
                                        DataMgr*         dataMgr );

        void FillCoordsXYPlane( const   StructuredGrid*  grid,  // Input 
                                size_t  planeIdx,               // Input: which plane to retrieve
                                float*  coords );               // Output buffer allocated by caller
        void FillCoordsYZPlane( const   StructuredGrid*  grid,  // Input 
                                size_t  planeIdx,               // Input
                                float*  coords );               // Output 
        void FillCoordsXZPlane( const   StructuredGrid*  grid,  // Input 
                                size_t  planeIdx,               // Input
                                float*  coords );               // Output 
        void IterateAGrid(      const   StructuredGrid*  grid,
                                size_t                   numOfVert, // Length of buffers.
                                float*                   dataBuf,   // Need to be already allocated.
                                unsigned char*           maskBuf ); // Need to be already allocated.
                                
    };  // end of class UserCoordinates 

    UserCoordinates     _userCoordinates;
    std::vector<float>  _colorMap;
    float               _colorMapRange[3];   // min, max, and diff values.
    glm::mat4           _currentMV;          // model view matrix in use
    GLint               _currentViewport[4]; // current viewport in use
    bool                _isIntel;

    // OpenGL stuff
    // textures
    GLuint              _backFaceTextureId;
    GLuint              _frontFaceTextureId;
    GLuint              _volumeTextureId;
    GLuint              _missingValueTextureId;
    GLuint              _colorMapTextureId;
    GLuint              _vertCoordsTextureId;
    GLuint              _depthTextureId;
    GLuint              _2ndVarDataTexId;
    GLuint              _2ndVarMaskTexId;
    const  GLint        _backFaceTexOffset;
    const  GLint        _frontFaceTexOffset;
    const  GLint        _volumeTexOffset;
    const  GLint        _colorMapTexOffset;
    const  GLint        _missingValueTexOffset;
    const  GLint        _vertCoordsTexOffset;
    const  GLint        _depthTexOffset;
    const  GLint        _2ndVarDataTexOffset;
    const  GLint        _2ndVarMaskTexOffset;

    // buffers and vertex arrays
    GLuint              _frameBufferId;
    GLuint              _vertexArrayId;
    GLuint              _vertexBufferId;    // Keeps user coordinates of 6 faces.
    GLuint              _indexBufferId;     // Auxiliary indices for efficiently drawing triangle strips.
    GLuint              _vertexAttribId;    // Attribute of vertices: (i, j k) logical indices.

    // shaders
    ShaderProgram*      _1stPassShader;
    ShaderProgram*      _2ndPassShader;
    ShaderProgram*      _3rdPassShader;
    ShaderProgram*      _3rdPassMode1Shader;
    ShaderProgram*      _3rdPassMode2Shader;

    //
    // Render the volume surface using triangle strips
    //   This is a subroutine used by _drawVolumeFaces().
    //
    void _renderTriangleStrips(int whichPass, int castingMode ) const;

    void _drawVolumeFaces(     int                whichPass, 
                               int                whichCastingMode,
                               const std::vector<size_t>&   cameraCellIdx,
                               const glm::mat4&   inversedMV  = glm::mat4(0.0f),
                               bool               fast        = false         ) const;

    void _load3rdPassUniforms( int                castingMode,
                               bool               fast,
                               bool               insideVolume ) const;

    virtual void _3rdPassSpecialHandling( bool fast, int castingMode ) const;
    virtual void _colormapSpecialHandling( ); // Cannot be const due to other subroutines.
    virtual bool _use2ndVariable( const RayCasterParams* params ) const;
    virtual void _update2ndVarTextures( );

    // 
    // Initialization for 1) framebuffers and 2) textures 
    //   It returns 0 upon success, and non-zero upon errors.
    //
    int  _initializeFramebufferTextures();

    int  _selectDefaultCastingMethod() const;

    void _updateViewportWhenNecessary( const GLint* viewport );
    void _updateColormap( RayCasterParams* params );
    void _updateDataTextures();
    int  _updateVertCoordsTexture( const glm::mat4& MV );
    void _configure3DTextureNearestInterpolation() const;
    void _configure3DTextureLinearInterpolation() const;
    void _configure2DTextureLinearInterpolation() const;

    double _getElapsedSeconds( const struct timeval* begin, const struct timeval* end ) const;

};  // End of class RayCaster

};  // End of namespace VAPoR

#endif 
