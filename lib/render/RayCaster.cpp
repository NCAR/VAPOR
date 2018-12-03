#include "vapor/glutil.h"
#include "vapor/RayCaster.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace VAPoR;

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW: error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// Constructor
RayCaster::RayCaster(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string paramsType, std::string classType, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, paramsType, classType, instName, dataMgr), _backFaceTexOffset(0), _frontFaceTexOffset(1), _volumeTexOffset(2), _colorMapTexOffset(3), _missingValueTexOffset(4),
  _xyCoordsTexOffset(5), _zCoordsTexOffset(6)
{
    _backFaceTextureId = 0;
    _frontFaceTextureId = 0;
    _volumeTextureId = 0;
    _missingValueTextureId = 0;
    _colorMapTextureId = 0;
    _xyCoordsTextureId = 0;
    _zCoordsTextureId = 0;
    _frameBufferId = 0;
    _depthBufferId = 0;

    _vertexArrayId = 0;
    _vertexBufferId = 0;
    _indexBufferId = 0;
    _vertexAttribId = 0;
    _xyCoordsBufferId = 0;
    _zCoordsBufferId = 0;

    _1stPassShaderId = 0;
    _2ndPassShaderId = 0;
    _3rdPassShaderId = 0;
    _3rdPassMode1ShaderId = 0;
    _3rdPassMode2ShaderId = 0;

    _drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    _drawBuffers[1] = GL_COLOR_ATTACHMENT1;

    /* Get viewport dimensions */
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    std::memcpy(_currentViewport, viewport, 4 * sizeof(GLint));
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
    if (_xyCoordsTextureId) {
        glDeleteTextures(1, &_xyCoordsTextureId);
        _xyCoordsTextureId = 0;
    }
    if (_zCoordsTextureId) {
        glDeleteTextures(1, &_zCoordsTextureId);
        _zCoordsTextureId = 0;
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
    if (_vertexAttribId) {
        glDeleteBuffers(1, &_vertexAttribId);
        _vertexAttribId = 0;
    }
    if (_xyCoordsBufferId) {
        glDeleteBuffers(1, &_xyCoordsBufferId);
        _xyCoordsBufferId = 0;
    }
    if (_zCoordsBufferId) {
        glDeleteBuffers(1, &_zCoordsBufferId);
        _zCoordsBufferId = 0;
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
    xyCoords = nullptr;
    zCoords = nullptr;
    missingValueMask = nullptr;
    for (int i = 0; i < 3; i++) {
        boxMin[i] = 0;
        boxMax[i] = 0;
    }
    for (int i = 0; i < 4; i++) { dims[i] = 0; }

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
    if (xyCoords) {
        delete[] xyCoords;
        xyCoords = nullptr;
    }
    if (zCoords) {
        delete[] zCoords;
        zCoords = nullptr;
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

bool RayCaster::UserCoordinates::IsMetadataUpToDate(const RayCasterParams *params, DataMgr *dataMgr) const
{
    if ((myCurrentTimeStep != params->GetCurrentTimestep()) || (myVariableName != params->GetVariableName()) || (myRefinementLevel != params->GetRefinementLevel())
        || (myCompressionLevel != params->GetCompressionLevel())) {
        return false;
    }

    // compare grid boundaries and dimensions
    StructuredGrid *    grid = this->GetCurrentGrid(params, dataMgr);
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

bool RayCaster::UserCoordinates::UpdateFaceAndData(const RayCasterParams *params, DataMgr *dataMgr)
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
    float df[3] = {float(dims[0]), float(dims[1]), float(dims[2])};
    dims[3] = size_t(std::sqrt(df[0] * df[0] + df[1] * df[1] + df[2] * df[2])) + 1;
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
    if (dataField) {
        delete[] dataField;
        dataField = nullptr;
    }
    dataField = new float[numOfVertices];
    if (!dataField)    // Test if allocation successful for 3D buffers.
    {
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
                missingValueMask[i] = 127u;
            } else {
                dataField[i] = (dataValue - valueRange[0]) * valueRange1o;
                missingValueMask[i] = 0u;
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

bool RayCaster::UserCoordinates::UpdateCurviCoords(const RayCasterParams *params, DataMgr *dataMgr)
{
    assert(params->GetCastingMode() == 2);

    if (xyCoords) delete[] xyCoords;
    xyCoords = new float[dims[0] * dims[1] * 2];
    if (zCoords) delete[] zCoords;
    zCoords = new float[dims[0] * dims[1] * dims[2]];
    if (!zCoords)    // Test if allocation successful for 3D buffers.
        return false;

    // Gather the XY coordinate from frontFace buffer
    size_t xyIdx = 0, xyzIdx = 0;
    for (size_t y = 0; y < dims[1]; y++)
        for (size_t x = 0; x < dims[0]; x++) {
            xyCoords[xyIdx++] = frontFace[xyzIdx++];
            xyCoords[xyIdx++] = frontFace[xyzIdx++];
            xyzIdx++;
        }

    // Gather the Z coordinates from grid
    StructuredGrid *              grid = this->GetCurrentGrid(params, dataMgr);
    StructuredGrid::ConstCoordItr coordItr = grid->ConstCoordBegin();
    size_t                        numOfVertices = dims[0] * dims[1] * dims[2];
    for (xyzIdx = 0; xyzIdx < numOfVertices; xyzIdx++) {
        zCoords[xyzIdx] = float((*coordItr)[2]);
        ++coordItr;
    }

    return true;
}

int RayCaster::_initializeGL()
{
    _loadShaders();
    _initializeFramebufferTextures();
    return 0;
}

int RayCaster::_paintGL(bool fast)
{
    const MatrixManager *mm = Renderer::_glManager->matrixManager;
    // Visualizer dimensions would change if window is resized
    GLint newViewport[4];
    glGetIntegerv(GL_VIEWPORT, newViewport);
    if (std::memcmp(newViewport, _currentViewport, 4 * sizeof(GLint)) != 0) {
        std::memcpy(_currentViewport, newViewport, 4 * sizeof(GLint));

        // Re-size 2D textures
        glActiveTexture(GL_TEXTURE0 + _backFaceTexOffset);
        glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _currentViewport[2], _currentViewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);

        glActiveTexture(GL_TEXTURE0 + _frontFaceTexOffset);
        glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _currentViewport[2], _currentViewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);

        glBindRenderbuffer(GL_RENDERBUFFER, _depthBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _currentViewport[2], _currentViewport[3]);
    }

    glBindVertexArray(_vertexArrayId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);

    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    if (!params) {
        MyBase::SetErrMsg("Not receiving RayCaster parameters; "
                          "the behavior becomes undefined!");
    }
    long castingMode = params->GetCastingMode();

    // If there an update event
    if (!_userCoordinates.IsMetadataUpToDate(params, _dataMgr)) {
        if (!_userCoordinates.UpdateFaceAndData(params, _dataMgr)) {
            MyBase::SetErrMsg("Memory allocation failed!");
            return 1;
        }

        if (castingMode == 2 && !_userCoordinates.UpdateCurviCoords(params, _dataMgr)) {
            MyBase::SetErrMsg("Memory allocation failed!");
            return 1;
        }

        // Also attach the new data to 3D textures
        glActiveTexture(GL_TEXTURE0 + _volumeTexOffset);
        glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
#ifdef Darwin
        //
        // Intel driver on MacOS seems to not able to correctly update the texture content
        //   when the texture is moderately big. This workaround of loading a dummy texture
        //   to force it to update this texture seems to resolve the issue.
        //
        float dummyVolume[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 2, 2, 2, 0, GL_RED, GL_FLOAT, dummyVolume);
#endif
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, _userCoordinates.dims[0], _userCoordinates.dims[1], _userCoordinates.dims[2], 0, GL_RED, GL_FLOAT, _userCoordinates.dataField);

        // Now we HAVE TO attach a missing value mask texture, because
        //   Intel driver on Mac doesn't like leaving the texture empty...
        unsigned char dummyMask[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
        glActiveTexture(GL_TEXTURE0 + _missingValueTexOffset);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    // Alignment adjustment. Stupid.
        glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
        if (_userCoordinates.missingValueMask)    // There is missing value.
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, _userCoordinates.dims[0], _userCoordinates.dims[1], _userCoordinates.dims[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                         _userCoordinates.missingValueMask);
        else
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, 2, 2, 2, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, dummyMask);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);    // Restore default alignment.

        // If using cell traverse ray casting, we need to upload user coordinates
        if (castingMode == 2) {
            const size_t *dims = _userCoordinates.dims;

            // Fill data to buffer object _xyCoordsBufferId
            glBindBuffer(GL_TEXTURE_BUFFER, _xyCoordsBufferId);
            glBufferData(GL_TEXTURE_BUFFER, 2 * sizeof(float) * dims[0] * dims[1], _userCoordinates.xyCoords, GL_STATIC_READ);
            // Pass data to the buffer texture: _xyCoordsTextureId
            glActiveTexture(GL_TEXTURE0 + _xyCoordsTexOffset);
            glBindTexture(GL_TEXTURE_BUFFER, _xyCoordsTextureId);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, _xyCoordsBufferId);

            // Repeat for the next buffer texture: _zCoordsBufferId
            glBindBuffer(GL_TEXTURE_BUFFER, _zCoordsBufferId);
            glBufferData(GL_TEXTURE_BUFFER, sizeof(float) * dims[0] * dims[1] * dims[2], _userCoordinates.zCoords, GL_STATIC_READ);
            glActiveTexture(GL_TEXTURE0 + _zCoordsTexOffset);
            glBindTexture(GL_TEXTURE_BUFFER, _zCoordsTextureId);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, _zCoordsBufferId);

            glBindBuffer(GL_TEXTURE_BUFFER, 0);
            glBindTexture(GL_TEXTURE_BUFFER, 0);
        }

        glBindTexture(GL_TEXTURE_3D, 0);
    }

    /* Gather the color map */
    if (params->UseSingleColor()) {
        float singleColor[4];
        params->GetConstantColor(singleColor);
        singleColor[3] = 1.0f;    // 1.0 in alpha channel
        _colorMap.resize(8);      // _colorMap will have 2 RGBA values
        for (int i = 0; i < 8; i++) _colorMap[i] = singleColor[i % 4];
        _colorMapRange[0] = 0.0f;
        _colorMapRange[1] = 0.0f;
    } else {
        params->GetMapperFunc()->makeLut(_colorMap);
        std::vector<double> range = params->GetMapperFunc()->getMinMaxMapValue();
        _colorMapRange[0] = float(range[0]);
        _colorMapRange[1] = float(range[1]);
    }

    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());
    glBindTexture(GL_TEXTURE_1D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
    glViewport(0, 0, _currentViewport[2], _currentViewport[3]);

    glm::mat4 ModelView = mm->GetModelViewMatrix();

    // 1st pass: render back facing polygons to texture0 of the framebuffer
    _drawVolumeFaces(1, castingMode, false);
    glCheckError();

    /* Detect if we're inside the volume */
    glm::mat4           InversedMV = glm::inverse(ModelView);
    std::vector<double> cameraUser(4, 1.0);    // camera position in user coordinates
    cameraUser[0] = InversedMV[3][0];
    cameraUser[1] = InversedMV[3][1];
    cameraUser[2] = InversedMV[3][2];
    std::vector<size_t> cameraCellIndices;    // camera position in which cell?
    StructuredGrid *    grid = _userCoordinates.GetCurrentGrid(params, _dataMgr);
    bool                insideACell = grid->GetIndicesCell(cameraUser, cameraCellIndices);

    if (insideACell) {
        glm::mat4 MVP = mm->GetModelViewProjectionMatrix();
        glm::mat4 InversedMVP = glm::inverse(MVP);
        glm::vec4 topLeftNDC(-1.0f, 1.0f, -0.9999f, 1.0f);
        glm::vec4 bottomLeftNDC(-1.0f, -1.0f, -0.9999f, 1.0f);
        glm::vec4 topRightNDC(1.0f, 1.0f, -0.9999f, 1.0f);
        glm::vec4 bottomRightNDC(1.0f, -1.0f, -0.9999f, 1.0f);
        glm::vec4 near[4];
        near[0] = InversedMVP * topLeftNDC;
        near[1] = InversedMVP * bottomLeftNDC;
        near[2] = InversedMVP * topRightNDC;
        near[3] = InversedMVP * bottomRightNDC;
        for (int i = 0; i < 4; i++) {
            near[i] /= near[i].w;
            std::memcpy(_userCoordinates.nearCoords + i * 3, glm::value_ptr(near[i]), 3 * sizeof(float));
        }
    }

    // 2nd pass, render front facing polygons
    _drawVolumeFaces(2, castingMode, insideACell);
    glCheckError();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _currentViewport[2], _currentViewport[3]);

    // 3rd pass, perform ray casting
    if (castingMode == 1)
        _3rdPassShaderId = _3rdPassMode1ShaderId;
    else if (castingMode == 2)
        _3rdPassShaderId = _3rdPassMode2ShaderId;
    else {
        MyBase::SetErrMsg("RayCasting Mode not supported!");
        return 1;
    }
    _drawVolumeFaces(3, castingMode, insideACell, InversedMV, fast);
    glCheckError();

    delete grid;

    // Restore default VAO settings!
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return 0;
}

void RayCaster::_initializeFramebufferTextures()
{
    /* Create Vertex Array Object (VAO) */
    glGenVertexArrays(1, &_vertexArrayId);
    glGenBuffers(1, &_vertexBufferId);
    glGenBuffers(1, &_indexBufferId);
    glGenBuffers(1, &_vertexAttribId);

    /* Create an Frame Buffer Object for the back side of the volume. */
    glGenFramebuffers(1, &_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);

    /* Generate back-facing texture */
    glGenTextures(1, &_backFaceTextureId);
    glActiveTexture(GL_TEXTURE0 + _backFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _currentViewport[2], _currentViewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);

    /* Configure the back-facing texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Generate front-facing texture */
    glGenTextures(1, &_frontFaceTextureId);
    glActiveTexture(GL_TEXTURE0 + _frontFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _currentViewport[2], _currentViewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);

    /* Configure the front-faceing texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Depth buffer */
    glGenRenderbuffers(1, &_depthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _currentViewport[2], _currentViewport[3]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBufferId);

    /* Set "_backFaceTextureId" as colour attachement #0,
       and "_frontFaceTextureId" as attachement #1       */
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
    glGenTextures(1, &_volumeTextureId);
    glActiveTexture(GL_TEXTURE0 + _volumeTexOffset);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);

    /* Configure _volumeTextureId */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate and configure 1D texture: _colorMapTextureId */
    glGenTextures(1, &_colorMapTextureId);
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);

    /* Configure _colorMapTextureId */
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    /* Generate and configure 3D texture: _missingValueTextureId */
    glGenTextures(1, &_missingValueTextureId);
    glActiveTexture(GL_TEXTURE0 + _missingValueTexOffset);
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);

    /* Configure _missingValueTextureId */
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate buffer texture for 2D X-Y and 3D Z coordinates */
    glGenTextures(1, &_xyCoordsTextureId);
    glGenTextures(1, &_zCoordsTextureId);
    glGenBuffers(1, &_xyCoordsBufferId);
    glGenBuffers(1, &_zCoordsBufferId);

    /* Bind the default textures */
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void RayCaster::_drawVolumeFaces(int whichPass, long castingMode, bool insideACell, const glm::mat4 &InversedMV, bool fast)
{
    glm::mat4 modelview = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();

    if (whichPass == 1) {
        glUseProgram(_1stPassShaderId);
        GLint uniformLocation = glGetUniformLocation(_1stPassShaderId, "MV");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(modelview));
        uniformLocation = glGetUniformLocation(_1stPassShaderId, "Projection");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(projection));

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
        GLint uniformLocation = glGetUniformLocation(_1stPassShaderId, "MV");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(modelview));
        uniformLocation = glGetUniformLocation(_1stPassShaderId, "Projection");
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(projection));

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL);
        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 1, black);    // clear GL_COLOR_ATTACHMENT1
    } else                                      // 3rd pass
    {
        _load3rdPassUniforms(castingMode, InversedMV, fast);
        glCheckError();
        _3rdPassSpecialHandling(fast, castingMode);
        glCheckError();

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }

    if (insideACell)    // only when 1st or 2nd pass
    {
        glEnableVertexAttribArray(0);    // attribute 0 is vertex coordinates
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), _userCoordinates.nearCoords, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(0);
    } else    // could be all 3 passes
    {
        _renderTriangleStrips(whichPass, castingMode);
        glCheckError();
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glDepthMask(GL_TRUE);

    glUseProgram(0);
}

void RayCaster::_load3rdPassUniforms(long castingMode, const glm::mat4 &inversedMV, bool fast) const
{
    glm::mat4 modelview = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();

    glUseProgram(_3rdPassShaderId);
    GLint uniformLocation = glGetUniformLocation(_3rdPassShaderId, "MV");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(modelview));

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "Projection");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(projection));

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "inversedMV");
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(inversedMV));

    float dataRanges[4] = {_userCoordinates.valueRange[0], _userCoordinates.valueRange[1], _colorMapRange[0], _colorMapRange[1]};
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "dataRanges");
    glUniform2fv(uniformLocation, 2, dataRanges);

    float boxExtents[6];
    std::memcpy(boxExtents, _userCoordinates.boxMin, sizeof(float) * 3);
    std::memcpy(boxExtents + 3, _userCoordinates.boxMax, sizeof(float) * 3);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "boxExtents");
    glUniform3fv(uniformLocation, 2, boxExtents);

    int volumeDims[3] = {int(_userCoordinates.dims[0]), int(_userCoordinates.dims[1]), int(_userCoordinates.dims[2])};
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "volumeDims");
    glUniform3iv(uniformLocation, 1, volumeDims);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "viewportDims");
    glUniform2iv(uniformLocation, 1, _currentViewport + 2);

    float planes[24];    // 6 planes, each with 4 elements
    Renderer::GetClippingPlanes(planes);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "clipPlanes");
    glUniform4fv(uniformLocation, 6, planes);

    // Get light settings from params.
    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    bool             lighting = params->GetLighting();
    if (lighting) {
        std::vector<double> coeffsD = params->GetLightingCoeffs();
        float               coeffsF[4] = {float(coeffsD[0]), float(coeffsD[1]), float(coeffsD[2]), float(coeffsD[3])};
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "lightingCoeffs");
        glUniform1fv(uniformLocation, (GLsizei)4, coeffsF);
    }

    // Pack in fast mode, lighting, and missing value booleans together
    int flags[3] = {int(fast), int(lighting), int(_userCoordinates.missingValueMask != nullptr)};
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "flags");
    glUniform1iv(uniformLocation, (GLsizei)3, flags);

    // Calculate the step size
    glm::vec4 boxMin(boxExtents[0], boxExtents[1], boxExtents[2], 1.0f);
    glm::vec4 boxMax(boxExtents[3], boxExtents[4], boxExtents[5], 1.0f);
    glm::vec4 boxminEye = modelview * boxMin;
    glm::vec4 boxmaxEye = modelview * boxMax;
    float     span[3] = {boxmaxEye[0] - boxminEye[0], boxmaxEye[1] - boxminEye[1], boxmaxEye[2] - boxminEye[2]};
    float     stepSize1D;
    if (_userCoordinates.dims[3] < 50)    // Make sure at least 100 steps
        stepSize1D = std::sqrt(span[0] * span[0] + span[1] * span[1] + span[2] * span[2]) / 100.0f;
    else
        stepSize1D = std::sqrt(span[0] * span[0] + span[1] * span[1] + span[2] * span[2]) / float(_userCoordinates.dims[3] * 4);
    if (fast) stepSize1D *= 4.0;    // Quadruple step size when fast rendering
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "stepSize1D");
    glUniform1f(uniformLocation, stepSize1D);

    // Pass in textures
    glActiveTexture(GL_TEXTURE0 + _backFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "backFaceTexture");
    glUniform1i(uniformLocation, _backFaceTexOffset);

    glActiveTexture(GL_TEXTURE0 + _frontFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "frontFaceTexture");
    glUniform1i(uniformLocation, _frontFaceTexOffset);

    glActiveTexture(GL_TEXTURE0 + _volumeTexOffset);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "volumeTexture");
    glUniform1i(uniformLocation, _volumeTexOffset);

    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "colorMapTexture");
    glUniform1i(uniformLocation, _colorMapTexOffset);

    glActiveTexture(GL_TEXTURE0 + _missingValueTexOffset);
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "missingValueMaskTexture");
    glUniform1i(uniformLocation, _missingValueTexOffset);

    if (castingMode == 2) {
        glActiveTexture(GL_TEXTURE0 + _xyCoordsTexOffset);
        glBindTexture(GL_TEXTURE_BUFFER, _xyCoordsTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "xyCoordsTexture");
        glUniform1i(uniformLocation, _xyCoordsTexOffset);

        glActiveTexture(GL_TEXTURE0 + _zCoordsTexOffset);
        glBindTexture(GL_TEXTURE_BUFFER, _zCoordsTextureId);
        uniformLocation = glGetUniformLocation(_3rdPassShaderId, "zCoordsTexture");
        glUniform1i(uniformLocation, _zCoordsTexOffset);
    }
}

void RayCaster::_3rdPassSpecialHandling(bool fast, long castingMode)
{
    // Left empty intentially.
    // Derived classes feel free to put stuff here.
}

void RayCaster::_renderTriangleStrips(int whichPass, long castingMode) const
{
    /* Give bx, by, bz type of "unsigned int" for indexBuffer */
    unsigned int bx = (unsigned int)_userCoordinates.dims[0];
    unsigned int by = (unsigned int)_userCoordinates.dims[1];
    unsigned int bz = (unsigned int)_userCoordinates.dims[2];
    size_t       idx;

    // Each strip will have the same numOfVertices for the first 4 faces
    size_t        numOfVertices = bx * 2;
    unsigned int *indexBuffer = new unsigned int[numOfVertices];

    bool attrib1 = false;
    int *attrib1Buffer = nullptr;
    if (castingMode == 2 && whichPass == 3) {
        attrib1 = true;
        unsigned int big1 = bx > by ? bx : by;
        unsigned int small = bx < by ? bx : by;
        unsigned int big2 = bz > small ? bz : small;
        attrib1Buffer = new int[big1 * big2 * 4];    // enough length for all faces
    }

    //
    // Render front face:
    //
    glEnableVertexAttribArray(0);    // attribute 0 is vertex coordinates
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * by * 3 * sizeof(float), _userCoordinates.frontFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    if (attrib1)    // specify shader input: vertexLogicalIdx
    {
        glEnableVertexAttribArray(1);    // attribute 1 is the logical indices
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
    }
    for (unsigned int y = 0; y < by - 1; y++)    // strip by strip
    {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = (y + 1) * bx + x;
            indexBuffer[idx++] = y * bx + x;
        }
        if (attrib1) {
            for (unsigned int x = 0; x < bx; x++) {
                unsigned int attribIdx = ((y + 1) * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = int(y);
                attrib1Buffer[attribIdx + 2] = int(bz) - 2;
                attrib1Buffer[attribIdx + 3] = 0;
                attribIdx = (y * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = int(y);
                attrib1Buffer[attribIdx + 2] = int(bz) - 2;
                attrib1Buffer[attribIdx + 3] = 0;
            }
            glBufferData(GL_ARRAY_BUFFER, bx * by * 4 * sizeof(int), attrib1Buffer, GL_STREAM_READ);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_READ);
        glCheckError();
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
        glCheckError();
    }

    //
    // Render back face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * by * 3 * sizeof(float), _userCoordinates.backFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    if (attrib1) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
    }
    for (unsigned int y = 0; y < by - 1; y++)    // strip by strip
    {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = y * bx + x;
            indexBuffer[idx++] = (y + 1) * bx + x;
        }
        if (attrib1) {
            for (unsigned int x = 0; x < bx; x++) {
                unsigned int attribIdx = (y * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = int(y);
                attrib1Buffer[attribIdx + 2] = 0;
                attrib1Buffer[attribIdx + 3] = 1;
                attribIdx = ((y + 1) * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = int(y);
                attrib1Buffer[attribIdx + 2] = 0;
                attrib1Buffer[attribIdx + 3] = 1;
            }
            glBufferData(GL_ARRAY_BUFFER, bx * by * 4 * sizeof(int), attrib1Buffer, GL_STREAM_READ);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_READ);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render top face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * bz * 3 * sizeof(float), _userCoordinates.topFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    if (attrib1) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
    }
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = z * bx + x;
            indexBuffer[idx++] = (z + 1) * bx + x;
        }
        if (attrib1) {
            for (unsigned int x = 0; x < bx; x++) {
                unsigned int attribIdx = (z * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = int(by) - 2;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 2;
                attribIdx = ((z + 1) * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = int(by) - 2;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 2;
            }
            glBufferData(GL_ARRAY_BUFFER, bx * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_READ);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_READ);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render bottom face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * bz * 3 * sizeof(float), _userCoordinates.bottomFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    if (attrib1) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
    }
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = (z + 1) * bx + x;
            indexBuffer[idx++] = z * bx + x;
        }
        if (attrib1) {
            for (unsigned int x = 0; x < bx; x++) {
                unsigned int attribIdx = ((z + 1) * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = 0;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 3;
                attribIdx = (z * bx + x) * 4;
                attrib1Buffer[attribIdx] = int(x) - 1;
                attrib1Buffer[attribIdx + 1] = 0;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 3;
            }
            glBufferData(GL_ARRAY_BUFFER, bx * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_READ);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_READ);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Each strip will have the same numOfVertices for the rest 2 faces.
    numOfVertices = by * 2;
    delete[] indexBuffer;
    indexBuffer = new unsigned int[numOfVertices];

    //
    // Render right face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, by * bz * 3 * sizeof(float), _userCoordinates.rightFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    if (attrib1) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
    }
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int y = 0; y < by; y++) {
            indexBuffer[idx++] = (z + 1) * by + y;
            indexBuffer[idx++] = z * by + y;
        }
        if (attrib1) {
            for (unsigned int y = 0; y < by; y++) {
                unsigned int attribIdx = ((z + 1) * by + y) * 4;
                attrib1Buffer[attribIdx] = int(bx) - 2;
                attrib1Buffer[attribIdx + 1] = int(y) - 1;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 4;
                attribIdx = (z * by + y) * 4;
                attrib1Buffer[attribIdx] = int(bx) - 2;
                attrib1Buffer[attribIdx + 1] = int(y) - 1;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 4;
            }
            glBufferData(GL_ARRAY_BUFFER, by * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_READ);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_READ);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render left face
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, by * bz * 3 * sizeof(float), _userCoordinates.leftFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    if (attrib1) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
    }
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int y = 0; y < by; y++) {
            indexBuffer[idx++] = z * by + y;
            indexBuffer[idx++] = (z + 1) * by + y;
        }
        if (attrib1) {
            for (unsigned int y = 0; y < by; y++) {
                unsigned int attribIdx = (z * by + y) * 4;
                attrib1Buffer[attribIdx] = 0;
                attrib1Buffer[attribIdx + 1] = int(y) - 1;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 5;
                attribIdx = ((z + 1) * by + y) * 4;
                attrib1Buffer[attribIdx] = 0;
                attrib1Buffer[attribIdx + 1] = int(y) - 1;
                attrib1Buffer[attribIdx + 2] = int(z);
                attrib1Buffer[attribIdx + 3] = 5;
            }
            glBufferData(GL_ARRAY_BUFFER, by * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_READ);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_READ);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    if (attrib1) delete[] attrib1Buffer;
    delete[] indexBuffer;
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

double RayCaster::_getElapsedSeconds(const struct timeval *begin, const struct timeval *end) const { return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec) / 1000000.0); }
