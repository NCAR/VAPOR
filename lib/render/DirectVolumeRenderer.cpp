#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/DirectVolumeRenderer.h>
#include <iostream>
#include <sstream>

//
// OpenGL debug output
// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDebugMessageInsert.xhtml
//
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<DirectVolumeRenderer> registrar(DirectVolumeRenderer::GetClassType(), DVRParams::GetClassType());

// Constructor
DirectVolumeRenderer::DirectVolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, DVRParams::GetClassType(), DirectVolumeRenderer::GetClassType(), instName, dataMgr)
{
    _backFaceTextureId = 0;
    _frontFaceTextureId = 0;
    _volumeTextureId = 0;
    _missingValueTextureId = 0;
    _colorMapTextureId = 0;
    _frameBufferId = 0;
    _depthBufferId = 0;

    _vertexArrayId = 0;
    _1stPassShaderId = 0;
    _2ndPassShaderId = 0;
    _3rdPassShaderId = 0;

    _drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    _drawBuffers[1] = GL_COLOR_ATTACHMENT1;
}

// Destructor
DirectVolumeRenderer::~DirectVolumeRenderer()
{
    // delete textures
    if (_backFaceTextureId) {
        glDeleteTextures(1, &_backFaceTextureId);
        _backFaceTextureId = 0;
    }
    if (_frontFaceTextureId) {
        glDeleteTextures(1, &_frontFaceTextureId);
        _frontFaceTextureId = 0;
    }
    if (_volumeTextureId) {
        glDeleteTextures(1, &_volumeTextureId);
        _volumeTextureId = 0;
    }
    if (_missingValueTextureId) {
        glDeleteTextures(1, &_missingValueTextureId);
        _missingValueTextureId = 0;
    }
    if (_colorMapTextureId) {
        glDeleteTextures(1, &_colorMapTextureId);
        _colorMapTextureId = 0;
    }

    // delete buffers
    if (_frameBufferId) {
        glDeleteBuffers(1, &_frameBufferId);
        _frameBufferId = 0;
    }
    if (_depthBufferId) {
        glDeleteBuffers(1, &_depthBufferId);
        _depthBufferId = 0;
    }

    // delete vertex arrays
    if (_vertexArrayId) {
        glDeleteVertexArrays(1, &_vertexArrayId);
        _vertexArrayId = 0;
    }

    // delete shader programs
    if (_1stPassShaderId) {
        glDeleteProgram(_1stPassShaderId);
        _1stPassShaderId = 0;
    }
    if (_2ndPassShaderId) {
        glDeleteProgram(_2ndPassShaderId);
        _2ndPassShaderId = 0;
    }
    if (_3rdPassShaderId) {
        glDeleteProgram(_3rdPassShaderId);
        _3rdPassShaderId = 0;
    }
}

// Constructor
DirectVolumeRenderer::UserCoordinates::UserCoordinates()
{
    frontFace = nullptr;
    backFace = nullptr;
    rightFace = nullptr;
    leftFace = nullptr;
    topFace = nullptr;
    bottomFace = nullptr;
    dataField = nullptr;
    missingValueMask = nullptr;
    for (int i = 0; i < 3; i++) {
        dims[i] = 0;
        boxMin[i] = 0;
        boxMax[i] = 0;
    }

    myCurrentTimeStep = 0;
    myVariableName = "";
    myRefinementLevel = -1;
    myCompressionLevel = -1;
}

// Destructor
DirectVolumeRenderer::UserCoordinates::~UserCoordinates()
{
    if (frontFace) {
        delete[] frontFace;
        frontFace = nullptr;
    }
    if (backFace) {
        delete[] backFace;
        backFace = nullptr;
    }
    if (rightFace) {
        delete[] rightFace;
        rightFace = nullptr;
    }
    if (leftFace) {
        delete[] leftFace;
        leftFace = nullptr;
    }
    if (topFace) {
        delete[] topFace;
        topFace = nullptr;
    }
    if (bottomFace) {
        delete[] bottomFace;
        bottomFace = nullptr;
    }
    if (dataField) {
        delete[] dataField;
        dataField = nullptr;
    }
    if (missingValueMask) {
        delete[] missingValueMask;
        missingValueMask = nullptr;
    }
}

bool DirectVolumeRenderer::UserCoordinates::isUpToDate(const DVRParams *params)
{
    if (myCurrentTimeStep != params->GetCurrentTimestep() || myVariableName != params->GetVariableName() || myRefinementLevel != params->GetRefinementLevel()
        || myCompressionLevel != params->GetCompressionLevel()) {
        return false;
    } else
        return true;
}

bool DirectVolumeRenderer::UserCoordinates::updateCoordinates(const DVRParams *params, DataMgr *dataMgr)
{
    myCurrentTimeStep = params->GetCurrentTimestep();
    myVariableName = params->GetVariableName();
    myRefinementLevel = params->GetRefinementLevel();
    myCompressionLevel = params->GetCompressionLevel();

    std::vector<double> extMin, extMax;
    params->GetBox()->GetExtents(extMin, extMax);
    StructuredGrid *grid = dynamic_cast<StructuredGrid *>(dataMgr->GetVariable(myCurrentTimeStep, myVariableName, myRefinementLevel, myCompressionLevel, extMin, extMax));
    if (grid == nullptr) {
        std::cerr << "UserCoordinates::updateCoordinates() isn't on a StructuredGrid" << std::endl;
        return false;
    } else {
        /* update member variables */
        grid->GetUserExtents(extMin, extMax);
        for (int i = 0; i < 3; i++) {
            boxMin[i] = (float)extMin[i];
            boxMax[i] = (float)extMax[i];
        }
        std::vector<size_t> gridDims = grid->GetDimensions();
        dims[0] = gridDims[0];
        dims[1] = gridDims[1];
        dims[2] = gridDims[2];
        grid->GetRange(valueRange);

        double buf[3];

        // Save front face user coordinates ( z == dims[2] - 1 )
        if (frontFace) delete[] frontFace;
        frontFace = new float[dims[0] * dims[1] * 3];
        size_t idx = 0;
        for (size_t y = 0; y < dims[1]; y++)
            for (size_t x = 0; x < dims[0]; x++) {
                grid->GetUserCoordinates(x, y, dims[2] - 1, buf[0], buf[1], buf[2]);
                frontFace[idx++] = (float)buf[0];
                frontFace[idx++] = (float)buf[1];
                frontFace[idx++] = (float)buf[2];
            }

        // Save back face user coordinates ( z == 0 )
        if (backFace) delete[] backFace;
        backFace = new float[dims[0] * dims[1] * 3];
        idx = 0;
        for (size_t y = 0; y < dims[1]; y++)
            for (size_t x = 0; x < dims[0]; x++) {
                grid->GetUserCoordinates(x, y, 0, buf[0], buf[1], buf[2]);
                backFace[idx++] = (float)buf[0];
                backFace[idx++] = (float)buf[1];
                backFace[idx++] = (float)buf[2];
            }

        // Save right face user coordinates ( x == dims[0] - 1 )
        if (rightFace) delete[] rightFace;
        rightFace = new float[dims[1] * dims[2] * 3];
        idx = 0;
        for (size_t z = 0; z < dims[2]; z++)
            for (size_t y = 0; y < dims[1]; y++) {
                grid->GetUserCoordinates(dims[0] - 1, y, z, buf[0], buf[1], buf[2]);
                rightFace[idx++] = (float)buf[0];
                rightFace[idx++] = (float)buf[1];
                rightFace[idx++] = (float)buf[2];
            }

        // Save left face user coordinates ( x == 0 )
        if (leftFace) delete[] leftFace;
        leftFace = new float[dims[1] * dims[2] * 3];
        idx = 0;
        for (size_t z = 0; z < dims[2]; z++)
            for (size_t y = 0; y < dims[1]; y++) {
                grid->GetUserCoordinates(0, y, z, buf[0], buf[1], buf[2]);
                leftFace[idx++] = (float)buf[0];
                leftFace[idx++] = (float)buf[1];
                leftFace[idx++] = (float)buf[2];
            }

        // Save top face user coordinates ( y == dims[1] - 1 )
        if (topFace) delete[] topFace;
        topFace = new float[dims[0] * dims[2] * 3];
        idx = 0;
        for (size_t z = 0; z < dims[2]; z++)
            for (size_t x = 0; x < dims[0]; x++) {
                grid->GetUserCoordinates(x, dims[1] - 1, z, buf[0], buf[1], buf[2]);
                topFace[idx++] = (float)buf[0];
                topFace[idx++] = (float)buf[1];
                topFace[idx++] = (float)buf[2];
            }

        // Save bottom face user coordinates ( y == 0 )
        if (bottomFace) delete[] bottomFace;
        bottomFace = new float[dims[0] * dims[2] * 3];
        idx = 0;
        for (size_t z = 0; z < dims[2]; z++)
            for (size_t x = 0; x < dims[0]; x++) {
                grid->GetUserCoordinates(x, 0, z, buf[0], buf[1], buf[2]);
                bottomFace[idx++] = (float)buf[0];
                bottomFace[idx++] = (float)buf[1];
                bottomFace[idx++] = (float)buf[2];
            }

        // Save the data field values and missing values
        if (dataField) delete[] dataField;
        size_t numOfVertices = dims[0] * dims[1] * dims[2];
        dataField = new float[numOfVertices];
        if (!dataField) return false;

        if (missingValueMask) delete[] missingValueMask;
        missingValueMask = new unsigned char[numOfVertices];

        StructuredGrid::ConstIterator valItr = grid->cbegin();    // Iterator for data field values
        float                         valueRange1o = 1.0f / (valueRange[1] - valueRange[0]);

        if (grid->HasMissingData()) {
            float missingValue = grid->GetMissingValue();
            float dataValue;
            for (size_t i = 0; i < numOfVertices; i++) {
                dataValue = float(*valItr);
                if (dataValue == missingValue) {
                    dataField[i] = 0.0f;
                    missingValueMask[i] = 255;
                } else {
                    dataField[i] = (dataValue - valueRange[0]) * valueRange1o;
                    missingValueMask[i] = 0;
                }
                ++valItr;
            }
        } else    // No missing value!
        {
            for (size_t i = 0; i < numOfVertices; i++) {
                dataField[i] = (float(*valItr) - valueRange[0]) * valueRange1o;
                ++valItr;
            }
            std::memset(missingValueMask, 0, numOfVertices);
        }
    }

    delete grid;
    return true;
}

int DirectVolumeRenderer::_initializeGL()
{
    // Enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    const char vgl1[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVR1stPass.vgl";
    const char fgl1[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVR1stPass.fgl";
    _1stPassShaderId = _loadShaders(vgl1, fgl1);

    const char vgl2[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVR2ndPass.vgl";
    const char fgl2[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVR2ndPass.fgl";
    _2ndPassShaderId = _loadShaders(vgl2, fgl2);

    const char vgl3[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVR3rdPass.vgl";
    const char fgl3[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVR3rdPass.fgl";
    _3rdPassShaderId = _loadShaders(vgl3, fgl3);

    // Create Vertex Array Object (VAO)
    glGenVertexArrays(1, &_vertexArrayId);
    glBindVertexArray(_vertexArrayId);

    _printGLInfo();

    _initializeFramebufferTextures();

    return 0;
}

int DirectVolumeRenderer::_paintGL()
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    DVRParams *params = dynamic_cast<DVRParams *>(GetActiveParams());

    /* Gather user coordinates */
    if (!_userCoordinates.isUpToDate(params)) {
        _userCoordinates.updateCoordinates(params, _dataMgr);

        /* Also attach the new data to 3D textures: _volumeTextureId, _missingValueTextureId */
        glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, _userCoordinates.dims[0], _userCoordinates.dims[1], _userCoordinates.dims[2], 0, GL_RED, GL_FLOAT, _userCoordinates.dataField);
        glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, _userCoordinates.dims[0], _userCoordinates.dims[1], _userCoordinates.dims[2], 0, GL_RED, GL_UNSIGNED_BYTE, _userCoordinates.missingValueMask);
        glBindTexture(GL_TEXTURE_3D, 0);
    }

    /* Gather the color map */
    params->GetMapperFunc()->makeLut(_colorMap);
    _colorMapRange[0] = params->GetMapperFunc()->getMinMapValue();
    _colorMapRange[1] = params->GetMapperFunc()->getMaxMapValue();
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());
    glBindTexture(GL_TEXTURE_1D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
    glViewport(0, 0, viewport[2], viewport[3]);
    _drawVolumeFaces(1);    // 1st pass, render back facing polygons to texture0 of the framebuffer
    _drawVolumeFaces(2);    // 2nd pass, render front facing polygons to texture1 of the framebuffer

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewport[2], viewport[3]);

    _drawVolumeFaces(3);    // 3rd pass, perform ray casting

    return 0;
}

void DirectVolumeRenderer::_initializeFramebufferTextures()
{
    /* Create an Frame Buffer Object for the back side of the volume. */
    glGenFramebuffers(1, &_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);

    /* Get viewport dimensions */
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    /* Generate back-facing texture */
    glGenTextures(1, &_backFaceTextureId);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport[2], viewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);

    /* Configure the back-facing texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Generate front-facing texture */
    glGenTextures(1, &_frontFaceTextureId);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport[2], viewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);

    /* Configure the front-faceing texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Depth buffer */
    glGenRenderbuffers(1, &_depthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewport[2], viewport[3]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBufferId);

    /* Set "_backFaceTextureId" as colour attachement #0, and "_frontFaceTextureId" as attachement #1 */
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _backFaceTextureId, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _frontFaceTextureId, 0);
    glDrawBuffers(2, _drawBuffers);

    /* Check if framebuffer is complete */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "_openGLInitialization(): Framebuffer failed!!" << std::endl; }

    /* Bind the default frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* Generate and configure 3D texture: _volumeTextureId */
    glGenTextures(1, &_volumeTextureId);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);

    /* Configure _volumeTextureId */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate and configure 3D texture: _missingValueTextureId */
    glGenTextures(1, &_missingValueTextureId);
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);

    /* Configure _missingValueTextureId */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate and configure 1D texture: _colorMapTextureId */
    glGenTextures(1, &_colorMapTextureId);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);

    /* Configure _colorMapTextureId */
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    /* Bind the default textures */
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void DirectVolumeRenderer::_printGLInfo() const
{
    std::cout << "    **** System Info ****" << std::endl;
    std::cout << "    OpenGL version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL   version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "    Vendor         : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "    Renderer       : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "    **** System Info ****" << std::endl;
}

void DirectVolumeRenderer::_drawVolumeFaces(int whichPass)
{
    assert(whichPass == 1 || whichPass == 2 || whichPass == 3);

    const float *frontFace = _userCoordinates.frontFace;
    const float *backFace = _userCoordinates.backFace;
    const float *rightFace = _userCoordinates.rightFace;
    const float *leftFace = _userCoordinates.leftFace;
    const float *topFace = _userCoordinates.topFace;
    const float *bottomFace = _userCoordinates.bottomFace;

    size_t bx = _userCoordinates.dims[0];
    size_t by = _userCoordinates.dims[1];
    size_t bz = _userCoordinates.dims[2];

    const float *ptr = nullptr;
    size_t       idx;
    size_t       numOfVertices;
    GLuint       uniformLocation;

    GLfloat MVP[16];
    _getMVPMatrix(MVP);

    /* Set up shader uniforms, OpenGL states, etc. */
    if (whichPass == 1)    // render back-facing polygons
    {
        glUseProgram(_1stPassShaderId);
        uniformLocation = glGetUniformLocation(_1stPassShaderId, "MVP");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, MVP);
        uniformLocation = glGetUniformLocation(_1stPassShaderId, "boxMin");
        glUniform3fv(uniformLocation, 1, _userCoordinates.boxMin);
        uniformLocation = glGetUniformLocation(_1stPassShaderId, "boxMax");
        glUniform3fv(uniformLocation, 1, _userCoordinates.boxMax);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(0.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_GEQUAL);
        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 0, black);    // clear GL_COLOR_ATTACHMENT0
    } else if (whichPass == 2)                  // render front-facing polygons
    {
        glUseProgram(_2ndPassShaderId);
        uniformLocation = glGetUniformLocation(_2ndPassShaderId, "MVP");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, MVP);
        uniformLocation = glGetUniformLocation(_2ndPassShaderId, "boxMin");
        glUniform3fv(uniformLocation, 1, _userCoordinates.boxMin);
        uniformLocation = glGetUniformLocation(_2ndPassShaderId, "boxMax");
        glUniform3fv(uniformLocation, 1, _userCoordinates.boxMax);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL);
        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 1, black);    // clear GL_COLOR_ATTACHMENT1
    } else                                      // the ray-casting pass
    {
        // Pass in uniforms
        glUseProgram(_3rdPassShaderId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "MVP");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, MVP);

        GLfloat ModelView[16], InversedMV[16], TransposedInverseMV[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "ModelView");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, ModelView);

        bool success = _mesa_invert_matrix_general(InversedMV, ModelView);
        assert(success);
        _mesa_transposef(TransposedInverseMV, InversedMV);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "transposedInverseMV");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, TransposedInverseMV);

        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "valueRange");
        glUniform2fv(uniformLocation, 1, _userCoordinates.valueRange);

        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "colorMapRange");
        glUniform2fv(uniformLocation, 1, _colorMapRange);

        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "boxMin");
        glUniform3fv(uniformLocation, 1, _userCoordinates.boxMin);

        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "boxMax");
        glUniform3fv(uniformLocation, 1, _userCoordinates.boxMax);

        float volumeDimensions[3] = {(float)_userCoordinates.dims[0], (float)_userCoordinates.dims[1], (float)_userCoordinates.dims[2]};
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "volumeDimensions");
        glUniform3fv(uniformLocation, 1, volumeDimensions);

        float maxVolumeDim = volumeDimensions[0] > volumeDimensions[1] ? volumeDimensions[0] : volumeDimensions[1];
        maxVolumeDim = maxVolumeDim > volumeDimensions[2] ? maxVolumeDim : volumeDimensions[2];
        float stepSize1D = 0.5f / maxVolumeDim;    // approximately 2 samples per cell
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "stepSize1D");
        glUniform1f(uniformLocation, stepSize1D);

        // Pass in textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "backFaceTexture");
        glUniform1i(uniformLocation, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "frontFaceTexture");
        glUniform1i(uniformLocation, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "volumeTexture");
        glUniform1i(uniformLocation, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "missingValueMaskTexture");
        glUniform1i(uniformLocation, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "colorMapTexture");
        glUniform1i(uniformLocation, 4);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }

    glEnableVertexAttribArray(0);
    GLuint vertexBufferId = 0;
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

    // Render front face:
    numOfVertices = bx * 2;
    GLfloat *vertexPositionBuffer = new GLfloat[numOfVertices * 3];
    for (int y = 0; y < by - 1; y++)    // strip by strip
    {
        idx = 0;
        for (int x = 0; x < bx; x++) {
            ptr = frontFace + ((y + 1) * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
            ptr = frontFace + (y * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
        }
        glBufferData(GL_ARRAY_BUFFER, numOfVertices * 3 * 4, vertexPositionBuffer, GL_STREAM_DRAW);
        glVertexAttribPointer(0,    // need to match attribute 0 in the shader
                              3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numOfVertices);
    }

    // Render back face:
    for (int y = 0; y < by - 1; y++) {
        idx = 0;
        for (int x = 0; x < bx; x++) {
            ptr = backFace + (y * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
            ptr = backFace + ((y + 1) * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
        }
        glBufferData(GL_ARRAY_BUFFER, numOfVertices * 3 * 4, vertexPositionBuffer, GL_STREAM_DRAW);
        glVertexAttribPointer(0,    // need to match attribute 0 in the shader
                              3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numOfVertices);
    }

    // Render top face:
    for (int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (int x = 0; x < bx; x++) {
            ptr = topFace + (z * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
            ptr = topFace + ((z + 1) * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
        }
        glBufferData(GL_ARRAY_BUFFER, numOfVertices * 3 * 4, vertexPositionBuffer, GL_STREAM_DRAW);
        glVertexAttribPointer(0,    // need to match attribute 0 in the shader
                              3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numOfVertices);
    }

    // Render bottom face:
    for (int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (int x = 0; x < bx; x++) {
            ptr = bottomFace + ((z + 1) * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
            ptr = bottomFace + (z * bx + x) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
        }
        glBufferData(GL_ARRAY_BUFFER, numOfVertices * 3 * 4, vertexPositionBuffer, GL_STREAM_DRAW);
        glVertexAttribPointer(0,    // need to match attribute 0 in the shader
                              3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numOfVertices);
    }
    delete[] vertexPositionBuffer;

    // Render right face:
    numOfVertices = by * 2;
    vertexPositionBuffer = new GLfloat[numOfVertices * 3];
    for (int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (int y = 0; y < by; y++) {
            ptr = rightFace + ((z + 1) * by + y) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
            ptr = rightFace + (z * by + y) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
        }
        glBufferData(GL_ARRAY_BUFFER, numOfVertices * 3 * 4, vertexPositionBuffer, GL_STREAM_DRAW);
        glVertexAttribPointer(0,    // need to match attribute 0 in the shader
                              3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numOfVertices);
    }

    // Render left face:
    for (int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (int y = 0; y < by; y++) {
            ptr = leftFace + (z * by + y) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
            ptr = leftFace + ((z + 1) * by + y) * 3;
            std::memcpy(vertexPositionBuffer + idx, ptr, 12);
            idx += 3;
        }
        glBufferData(GL_ARRAY_BUFFER, numOfVertices * 3 * 4, vertexPositionBuffer, GL_STREAM_DRAW);
        glVertexAttribPointer(0,    // need to match attribute 0 in the shader
                              3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numOfVertices);
    }
    delete[] vertexPositionBuffer;

    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &vertexBufferId);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glDepthMask(GL_TRUE);

    glUseProgram(0);
}

GLuint DirectVolumeRenderer::_loadShaders(const char *vertex_file_path, const char *fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string   VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    } else {
        printf("Impossible to open %s. Are you in the right directory ?\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string   FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int   InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void DirectVolumeRenderer::_getMVPMatrix(GLfloat *MVP) const
{
    GLfloat ModelView[16];
    GLfloat Projection[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);      // This is from OpenGL 2...
    glGetFloatv(GL_PROJECTION_MATRIX, Projection);    // This is from OpenGL 2...

    // MVP = Projection * ModelView
    GLfloat tmp;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            tmp = 0.0f;
            for (int idx = 0; idx < 4; idx++) tmp += ModelView[i * 4 + idx] * Projection[idx * 4 + j];
            // Because all matrices are colume-major, this is the correct order.
            MVP[i * 4 + j] = tmp;
        }
}

//===================================================================
// The following invert and transpose functions are from mesa 17.3.9:
// https://mesa.freedesktop.org/archive/mesa-17.3.9.tar.xz
//===================================================================
/**
 * Compute inverse of 4x4 transformation matrix.
 *
 * \param in pointer to a memory space which represents a 4x4 colume-major matrix.
 * \param out pointer to a memory space which will hold the inverse matrix
 *
 * \return true for success, false for failure (\p singular matrix).
 *
 * \author
 * Code contributed by Jacques Leroy jle@star.be
 *
 * Calculates the inverse matrix by performing the gaussian matrix reduction
 * with partial pivoting followed by back/substitution with the loops manually
 * unrolled.
 */
bool DirectVolumeRenderer::_mesa_invert_matrix_general(GLfloat out[16], const GLfloat in[16])
{
/**
 * References an element of 4x4 matrix.
 * Calculate the linear storage index of the element and references it.
 */
#define MAT(m, r, c) (m)[(c)*4 + (r)]
/**
 * Swaps the values of two floating point variables.
 */
#define SWAP_ROWS(a, b)    \
    {                      \
        GLfloat *_tmp = a; \
        (a) = (b);         \
        (b) = _tmp;        \
    }

    const GLfloat *m = in;
    GLfloat        wtmp[4][8];
    GLfloat        m0, m1, m2, m3, s;
    GLfloat *      r0, *r1, *r2, *r3;

    r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

    r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1), r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3), r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

    r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1), r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3), r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

    r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1), r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3), r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

    r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1), r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3), r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

    /* choose pivot - or die */
    if (fabsf(r3[0]) > fabsf(r2[0])) SWAP_ROWS(r3, r2);
    if (fabsf(r2[0]) > fabsf(r1[0])) SWAP_ROWS(r2, r1);
    if (fabsf(r1[0]) > fabsf(r0[0])) SWAP_ROWS(r1, r0);
    if (0.0F == r0[0]) return false;

    /* eliminate first variable     */
    m1 = r1[0] / r0[0];
    m2 = r2[0] / r0[0];
    m3 = r3[0] / r0[0];
    s = r0[1];
    r1[1] -= m1 * s;
    r2[1] -= m2 * s;
    r3[1] -= m3 * s;
    s = r0[2];
    r1[2] -= m1 * s;
    r2[2] -= m2 * s;
    r3[2] -= m3 * s;
    s = r0[3];
    r1[3] -= m1 * s;
    r2[3] -= m2 * s;
    r3[3] -= m3 * s;
    s = r0[4];
    if (s != 0.0F) {
        r1[4] -= m1 * s;
        r2[4] -= m2 * s;
        r3[4] -= m3 * s;
    }
    s = r0[5];
    if (s != 0.0F) {
        r1[5] -= m1 * s;
        r2[5] -= m2 * s;
        r3[5] -= m3 * s;
    }
    s = r0[6];
    if (s != 0.0F) {
        r1[6] -= m1 * s;
        r2[6] -= m2 * s;
        r3[6] -= m3 * s;
    }
    s = r0[7];
    if (s != 0.0F) {
        r1[7] -= m1 * s;
        r2[7] -= m2 * s;
        r3[7] -= m3 * s;
    }

    /* choose pivot - or die */
    if (fabsf(r3[1]) > fabsf(r2[1])) SWAP_ROWS(r3, r2);
    if (fabsf(r2[1]) > fabsf(r1[1])) SWAP_ROWS(r2, r1);
    if (0.0F == r1[1]) return false;

    /* eliminate second variable */
    m2 = r2[1] / r1[1];
    m3 = r3[1] / r1[1];
    r2[2] -= m2 * r1[2];
    r3[2] -= m3 * r1[2];
    r2[3] -= m2 * r1[3];
    r3[3] -= m3 * r1[3];
    s = r1[4];
    if (0.0F != s) {
        r2[4] -= m2 * s;
        r3[4] -= m3 * s;
    }
    s = r1[5];
    if (0.0F != s) {
        r2[5] -= m2 * s;
        r3[5] -= m3 * s;
    }
    s = r1[6];
    if (0.0F != s) {
        r2[6] -= m2 * s;
        r3[6] -= m3 * s;
    }
    s = r1[7];
    if (0.0F != s) {
        r2[7] -= m2 * s;
        r3[7] -= m3 * s;
    }

    /* choose pivot - or die */
    if (fabsf(r3[2]) > fabsf(r2[2])) SWAP_ROWS(r3, r2);
    if (0.0F == r2[2]) return false;

    /* eliminate third variable */
    m3 = r3[2] / r2[2];
    r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4], r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

    /* last check */
    if (0.0F == r3[3]) return false;

    s = 1.0F / r3[3]; /* now back substitute row 3 */
    r3[4] *= s;
    r3[5] *= s;
    r3[6] *= s;
    r3[7] *= s;

    m2 = r2[3]; /* now back substitute row 2 */
    s = 1.0F / r2[2];
    r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2), r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
    m1 = r1[3];
    r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1, r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
    m0 = r0[3];
    r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0, r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

    m1 = r1[2]; /* now back substitute row 1 */
    s = 1.0F / r1[1];
    r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1), r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
    m0 = r0[2];
    r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0, r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

    m0 = r0[1]; /* now back substitute row 0 */
    s = 1.0F / r0[0];
    r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0), r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

    MAT(out, 0, 0) = r0[4];
    MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
    MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
    MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
    MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
    MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
    MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
    MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
    MAT(out, 3, 3) = r3[7];

#undef SWAP_ROWS
#undef MAT

    return true;
}

/**
 * Transpose a GLfloat matrix.
 *
 * \param to destination array.
 * \param from source array.
 */
void DirectVolumeRenderer::_mesa_transposef(GLfloat to[16], const GLfloat from[16])
{
    to[0] = from[0];
    to[1] = from[4];
    to[2] = from[8];
    to[3] = from[12];
    to[4] = from[1];
    to[5] = from[5];
    to[6] = from[9];
    to[7] = from[13];
    to[8] = from[2];
    to[9] = from[6];
    to[10] = from[10];
    to[11] = from[14];
    to[12] = from[3];
    to[13] = from[7];
    to[14] = from[11];
    to[15] = from[15];
}

void DirectVolumeRenderer::_printMatrix(const GLfloat m[16])
{
    for (int i = 0; i < 4; i++) printf("\t%f %f %f %f\n", m[i], m[4 + i], m[8 + i], m[12 + i]);
}
