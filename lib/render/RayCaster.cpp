#include <vapor/glutil.h>
#include <vapor/RayCaster.h>
#include <iostream>
#include <sstream>

//
// OpenGL debug output
// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDebugMessageInsert.xhtml
//
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
        char buffer[2048];
        sprintf(buffer, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
        MyBase::SetErrMsg(buffer);
    }
}

using namespace VAPoR;

// Constructor
RayCaster::RayCaster(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string paramsType, std::string classType, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, paramsType, classType, instName, dataMgr)
{
    _backFaceTextureId = 0;
    _frontFaceTextureId = 0;
    _volumeTextureId = 0;
    _missingValueTextureId = 0;
    _colorMapTextureId = 0;
    _frameBufferId = 0;
    _depthBufferId = 0;

    _vertexArrayId = 0;
    _vertexBufferId = 0;
    _indexBufferId = 0;

    _1stPassShaderId = 0;
    _2ndPassShaderId = 0;
    _3rdPassShaderId = 0;

    _drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    _drawBuffers[1] = GL_COLOR_ATTACHMENT1;
}

// Destructor
RayCaster::~RayCaster()
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
    if (_vertexBufferId) {
        glDeleteBuffers(1, &_vertexBufferId);
        _vertexBufferId = 0;
    }
    if (_indexBufferId) {
        glDeleteBuffers(1, &_indexBufferId);
        _indexBufferId = 0;
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
RayCaster::UserCoordinates::UserCoordinates()
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
RayCaster::UserCoordinates::~UserCoordinates()
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

StructuredGrid *RayCaster::UserCoordinates::GetCurrentGrid(const RayCasterParams *params, DataMgr *dataMgr) const
{
    std::vector<double> extMin, extMax;
    params->GetBox()->GetExtents(extMin, extMax);
    StructuredGrid *grid = dynamic_cast<StructuredGrid *>(dataMgr->GetVariable(myCurrentTimeStep, myVariableName, myRefinementLevel, myCompressionLevel, extMin, extMax));
    if (grid == nullptr) {
        MyBase::SetErrMsg("UserCoordinates::GetCurrentGrid() isn't on a StructuredGrid; "
                          "the behavior is undefined in this case.");
    }
    return grid;
}

bool RayCaster::UserCoordinates::IsUpToDate(const RayCasterParams *params, DataMgr *dataMgr) const
{
    if ((myCurrentTimeStep != params->GetCurrentTimestep()) || (myVariableName != params->GetVariableName()) || (myRefinementLevel != params->GetRefinementLevel())
        || (myCompressionLevel != params->GetCompressionLevel())) {
        return false;
    }

    // compare grid boundaries and dimensions
    StructuredGrid *grid = this->GetCurrentGrid(params, dataMgr);
    if (!grid) {
        MyBase::SetErrMsg("Retrieving grid unsuccessful; "
                          "the behavior is then undefined!");
    }
    std::vector<double> extMin, extMax;
    grid->GetUserExtents(extMin, extMax);
    std::vector<size_t> gridDims = grid->GetDimensions();
    for (int i = 0; i < 3; i++) {
        if ((boxMin[i] != (float)extMin[i]) || (boxMax[i] != (float)extMax[i]) || (dims[i] != gridDims[i])) {
            delete grid;
            return false;
        }
    }

    // now we know it's up to date!
    delete grid;
    return true;
}

bool RayCaster::UserCoordinates::UpdateCoordinates(const RayCasterParams *params, DataMgr *dataMgr)
{
    myCurrentTimeStep = params->GetCurrentTimestep();
    myVariableName = params->GetVariableName();
    myRefinementLevel = params->GetRefinementLevel();
    myCompressionLevel = params->GetCompressionLevel();

    /* update member variables */
    StructuredGrid *    grid = this->GetCurrentGrid(params, dataMgr);
    std::vector<double> extMin, extMax;
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
    size_t numOfVertices = dims[0] * dims[1] * dims[2];
    if (dataField) delete[] dataField;
    dataField = new float[numOfVertices];
    if (!dataField) {
        delete grid;
        return false;
    }
    if (missingValueMask) {
        delete[] missingValueMask;
        missingValueMask = nullptr;
    }
    StructuredGrid::ConstIterator valItr = grid->cbegin();    // Iterator for data field values
    float                         valueRange1o = 1.0f / (valueRange[1] - valueRange[0]);

    if (grid->HasMissingData()) {
        float missingValue = grid->GetMissingValue();
        missingValueMask = new unsigned char[numOfVertices];
        if (!missingValueMask) {
            delete grid;
            return false;
        }
        float dataValue;
        for (size_t i = 0; i < numOfVertices; i++) {
            dataValue = float(*valItr);
            if (dataValue == missingValue) {
                dataField[i] = 0.0f;
                missingValueMask[i] = 127;
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
    }

    delete grid;
    return true;
}

int RayCaster::_initializeGL()
{
#ifdef Darwin
    return 0;
#endif
    // Enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    _loadShaders();

    // Create Vertex Array Object (VAO)
    glGenVertexArrays(1, &_vertexArrayId);
    glGenBuffers(1, &_vertexBufferId);
    glGenBuffers(1, &_indexBufferId);

    _initializeFramebufferTextures();

    return 0;
}

int RayCaster::_paintGL(bool fast)
{
#ifdef Darwin
    return 0;
#endif
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    if (!params) {
        MyBase::SetErrMsg("Not receiving RayCaster parameters; "
                          "the behavior becomes undefined!");
    }

    // Use our VAO
    glBindVertexArray(_vertexArrayId);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);

    /* Gather user coordinates */
    if (!_userCoordinates.IsUpToDate(params, _dataMgr)) {
        _userCoordinates.UpdateCoordinates(params, _dataMgr);

        /* Also attach the new data to 3D textures */
        glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, _userCoordinates.dims[0], _userCoordinates.dims[1], _userCoordinates.dims[2], 0, GL_RED, GL_FLOAT, _userCoordinates.dataField);

        // If there is missing value, upload the mask to texture. Otherwise, leave it empty.
        if (_userCoordinates.missingValueMask)    // Has missing value!
        {
            // Adjust alignment for GL_R8UI format. Stupid OpenGL parameter.
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, _userCoordinates.dims[0], _userCoordinates.dims[1], _userCoordinates.dims[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                         _userCoordinates.missingValueMask);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);    // Restore default alignment.
        }

        glBindTexture(GL_TEXTURE_3D, 0);
    }

    /* Gather the color map */
    params->GetMapperFunc()->makeLut(_colorMap);
    std::vector<double> range = params->GetMapperFunc()->getMinMaxMapValue();
    _colorMapRange[0] = float(range[0]);
    _colorMapRange[1] = float(range[1]);

    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());
    glBindTexture(GL_TEXTURE_1D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
    glViewport(0, 0, viewport[2], viewport[3]);

    _drawVolumeFaces(1);    // 1st pass, render back facing polygons to texture0 of the framebuffer

    /* Detect if we're inside the volume */
    GLfloat ModelView[16], InversedMV[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
    bool success = _mesa_invert_matrix_general(InversedMV, ModelView);
    if (!success) {
        MyBase::SetErrMsg("ModelView matrix is a singular matrix; "
                          "the behavior becomes undefined!");
    }
    std::vector<double> cameraUser(4, 1.0);    // camera position in user coordinates
    cameraUser[0] = InversedMV[12];
    cameraUser[1] = InversedMV[13];
    cameraUser[2] = InversedMV[14];
    std::vector<size_t> cameraCellIndices;    // camera position in which cell?
    StructuredGrid *    grid = _userCoordinates.GetCurrentGrid(params, _dataMgr);
    bool                insideACell = grid->GetIndicesCell(cameraUser, cameraCellIndices);

    if (insideACell) {
        GLfloat MVP[16], InversedMVP[16];
        _getMVPMatrix(MVP);
        _mesa_invert_matrix_general(InversedMVP, MVP);

        float topLeftNDC[4] = {-1.0f, 1.0f, -0.999f, 1.0f};
        float bottomLeftNDC[4] = {-1.0f, -1.0f, -0.999f, 1.0f};
        float topRightNDC[4] = {1.0f, 1.0f, -0.999f, 1.0f};
        float bottomRightNDC[4] = {1.0f, -1.0f, -0.999f, 1.0f};
        float near[16];
        _matMultiVec(InversedMVP, topLeftNDC, near);
        _matMultiVec(InversedMVP, bottomLeftNDC, near + 4);
        _matMultiVec(InversedMVP, topRightNDC, near + 8);
        _matMultiVec(InversedMVP, bottomRightNDC, near + 12);
        for (int i = 0; i < 4; i++) {
            _userCoordinates.nearCoords[i * 3] = near[i * 4] / near[i * 4 + 3];
            _userCoordinates.nearCoords[i * 3 + 1] = near[i * 4 + 1] / near[i * 4 + 3];
            _userCoordinates.nearCoords[i * 3 + 2] = near[i * 4 + 2] / near[i * 4 + 3];
        }
    }

    _drawVolumeFaces(2, insideACell);    // 2nd pass, render front facing polygons

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewport[2], viewport[3]);

    _drawVolumeFaces(3, insideACell, ModelView, InversedMV, fast);    // 3rd pass, perform ray casting

    delete grid;

    // Restore default VAO settings!
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return 0;
}

void RayCaster::_initializeFramebufferTextures()
{
    /* Create an Frame Buffer Object for the back side of the volume. */
    glGenFramebuffers(1, &_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);

    /* Get viewport dimensions */
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    /* Generate back-facing texture */
    GLuint textureUnit = 0;
    glGenTextures(1, &_backFaceTextureId);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport[2], viewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);

    /* Configure the back-facing texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Generate front-facing texture */
    textureUnit = 1;
    glGenTextures(1, &_frontFaceTextureId);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
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
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        MyBase::SetErrMsg("_openGLInitialization(): Framebuffer failed; "
                          "the behavior is then undefined.");
    }

    /* Bind the default frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* Generate and configure 3D texture: _volumeTextureId */
    textureUnit = 2;
    glGenTextures(1, &_volumeTextureId);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);

    /* Configure _volumeTextureId */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate and configure 1D texture: _colorMapTextureId */
    textureUnit = 3;
    glGenTextures(1, &_colorMapTextureId);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);

    /* Configure _colorMapTextureId */
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    /* Generate and configure 3D texture: _missingValueTextureId */
    textureUnit = 4;
    glGenTextures(1, &_missingValueTextureId);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);

    /* Configure _missingValueTextureId */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Bind the default textures */
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void RayCaster::_drawVolumeFaces(int whichPass, bool insideACell, const GLfloat *ModelView, const GLfloat *InversedMV, bool fast)
{
    GLint   uniformLocation;
    GLfloat MVP[16];
    _getMVPMatrix(MVP);

    if (whichPass == 1) {
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
    } else if (whichPass == 2) {
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
    } else {
        _load3rdPassUniforms(MVP, ModelView, InversedMV, fast);

        _3rdPassSpecialHandling(fast);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }

    // Let's use our VAO here
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    if (insideACell) {
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), _userCoordinates.nearCoords, GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } else {
        _renderTriangleStrips();
    }

    glDisableVertexAttribArray(0);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glDepthMask(GL_TRUE);

    glUseProgram(0);
}

void RayCaster::_load3rdPassUniforms(const GLfloat *MVP, const GLfloat *ModelView, const GLfloat *InversedMV, bool fast) const
{
    glUseProgram(_3rdPassShaderId);
    GLint uniformLocation = glGetUniformLocation(_3rdPassShaderId, "MVP");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, MVP);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "ModelView");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, ModelView);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "transposedInverseMV");
    glUniformMatrix4fv(uniformLocation, 1, GL_TRUE, InversedMV);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "valueRange");
    glUniform2fv(uniformLocation, 1, _userCoordinates.valueRange);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "colorMapRange");
    glUniform2fv(uniformLocation, 1, _colorMapRange);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "boxMin");
    glUniform3fv(uniformLocation, 1, _userCoordinates.boxMin);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "boxMax");
    glUniform3fv(uniformLocation, 1, _userCoordinates.boxMax);

    float volumeDimensions[3] = {float(_userCoordinates.dims[0]), float(_userCoordinates.dims[1]), float(_userCoordinates.dims[2])};
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "volumeDimensions");
    glUniform3fv(uniformLocation, 1, volumeDimensions);

    float stepSize1D = 0.005f;    // This is like ~200 samples
    if (!fast)                    // Calculate a better step size if in fast rendering mode
    {
        float maxVolumeDim = volumeDimensions[0] > volumeDimensions[1] ? volumeDimensions[0] : volumeDimensions[1];
        maxVolumeDim = maxVolumeDim > volumeDimensions[2] ? maxVolumeDim : volumeDimensions[2];
        maxVolumeDim = maxVolumeDim > 200 ? maxVolumeDim : 200;    // Make sure at least 400 smaples
        stepSize1D = 0.5f / maxVolumeDim;                          // Approximately 2 samples per cell
    }
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "stepSize1D");
    glUniform1f(uniformLocation, stepSize1D);

    float planes[24];    // 6 planes, each with 4 elements
    Renderer::GetClippingPlanes(planes);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "clipPlanes");
    glUniform4fv(uniformLocation, 6, planes);

    // Pass in textures
    GLuint textureUnit = 0;
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "backFaceTexture");
    glUniform1i(uniformLocation, textureUnit);

    textureUnit = 1;
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "frontFaceTexture");
    glUniform1i(uniformLocation, textureUnit);

    textureUnit = 2;
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "volumeTexture");
    glUniform1i(uniformLocation, textureUnit);

    textureUnit = 3;
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "colorMapTexture");
    glUniform1i(uniformLocation, textureUnit);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "hasMissingValue");
    // If there is missing value, pass in missingValueMaskTexture as well.
    //   Otherwise, leave it empty.
    if (_userCoordinates.missingValueMask == nullptr)
        glUniform1i(uniformLocation, 0);    // Set to false
    else {
        glUniform1i(uniformLocation, 1);    // Set to true
        textureUnit = 4;
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "missingValueMaskTexture");
        glUniform1i(uniformLocation, textureUnit);
    }
}

void RayCaster::_3rdPassSpecialHandling(bool fast)
{
    // Left empty intentially.
    // Derived classes feel free to put stuff here.
}

void RayCaster::_renderTriangleStrips() const
{
    unsigned int bx = (unsigned int)_userCoordinates.dims[0];
    unsigned int by = (unsigned int)_userCoordinates.dims[1];
    unsigned int bz = (unsigned int)_userCoordinates.dims[2];
    unsigned int idx;

    // Each strip will have the same numOfVertices for the first 4 faces
    size_t        numOfVertices = bx * 2;
    unsigned int *indexBuffer = new unsigned int[numOfVertices];

    // Render front face:
    glBufferData(GL_ARRAY_BUFFER, bx * by * 3 * sizeof(float), _userCoordinates.frontFace, GL_STATIC_DRAW);
    for (unsigned int y = 0; y < by - 1; y++)    // strip by strip
    {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = (y + 1) * bx + x;
            indexBuffer[idx++] = y * bx + x;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Render back face:
    glBufferData(GL_ARRAY_BUFFER, bx * by * 3 * sizeof(float), _userCoordinates.backFace, GL_STATIC_DRAW);
    for (unsigned int y = 0; y < by - 1; y++)    // strip by strip
    {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = y * bx + x;
            indexBuffer[idx++] = (y + 1) * bx + x;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Render top face:
    glBufferData(GL_ARRAY_BUFFER, bx * bz * 3 * sizeof(float), _userCoordinates.topFace, GL_STATIC_DRAW);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = z * bx + x;
            indexBuffer[idx++] = (z + 1) * bx + x;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Render bottom face:
    glBufferData(GL_ARRAY_BUFFER, bx * bz * 3 * sizeof(float), _userCoordinates.bottomFace, GL_STATIC_DRAW);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = (z + 1) * bx + x;
            indexBuffer[idx++] = z * bx + x;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Each strip will have the same numOfVertices for the rest 2 faces.
    numOfVertices = by * 2;
    delete[] indexBuffer;
    indexBuffer = new unsigned int[numOfVertices];

    // Render right face:
    glBufferData(GL_ARRAY_BUFFER, by * bz * 3 * sizeof(float), _userCoordinates.rightFace, GL_STATIC_DRAW);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int y = 0; y < by; y++) {
            indexBuffer[idx++] = (z + 1) * by + y;
            indexBuffer[idx++] = z * by + y;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Render left face
    glBufferData(GL_ARRAY_BUFFER, by * bz * 3 * sizeof(float), _userCoordinates.leftFace, GL_STATIC_DRAW);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int y = 0; y < by; y++) {
            indexBuffer[idx++] = z * by + y;
            indexBuffer[idx++] = (z + 1) * by + y;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    delete[] indexBuffer;
}

GLuint RayCaster::_compileShaders(const char *vertex_file_path, const char *fragment_file_path)
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

void RayCaster::_getMVPMatrix(GLfloat *MVP) const
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

void RayCaster::_matMultiVec(const GLfloat *mat, const GLfloat *in, GLfloat *out) const
{
#define MAT(m, r, c) (m)[(c)*4 + (r)]
    out[0] = MAT(mat, 0, 0) * in[0] + MAT(mat, 0, 1) * in[1] + MAT(mat, 0, 2) * in[2] + MAT(mat, 0, 3) * in[3];
    out[1] = MAT(mat, 1, 0) * in[0] + MAT(mat, 1, 1) * in[1] + MAT(mat, 1, 2) * in[2] + MAT(mat, 1, 3) * in[3];
    out[2] = MAT(mat, 2, 0) * in[0] + MAT(mat, 2, 1) * in[1] + MAT(mat, 2, 2) * in[2] + MAT(mat, 2, 3) * in[3];
    out[3] = MAT(mat, 3, 0) * in[0] + MAT(mat, 3, 1) * in[1] + MAT(mat, 3, 2) * in[2] + MAT(mat, 3, 3) * in[3];
#undef MAT
}

double RayCaster::_getElapsedSeconds(const struct timeval *begin, const struct timeval *end) const { return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec) / 1000000.0); }

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
bool RayCaster::_mesa_invert_matrix_general(GLfloat out[16], const GLfloat in[16])
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
void RayCaster::_mesa_transposef(GLfloat to[16], const GLfloat from[16])
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

void RayCaster::_printMatrix(const GLfloat m[16])
{
    for (int i = 0; i < 4; i++) printf("\t%f %f %f %f\n", m[i], m[4 + i], m[8 + i], m[12 + i]);
}
