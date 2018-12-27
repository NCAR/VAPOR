#include "vapor/glutil.h"
#include "vapor/RayCaster.h"
#include <iostream>
#include <fstream>
#include <sstream>

#define OUTOFDATE   1
#define GRIDERROR   -1
#define JUSTERROR   -2
#define PARAMSERROR -3
#define MEMERROR    -4
#define GLERROR     -5

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
/*
void glCheckError() { }
*/

// Constructor
RayCaster::RayCaster(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string paramsType, std::string classType, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, paramsType, classType, instName, dataMgr), _backFaceTexOffset(0), _frontFaceTexOffset(1), _volumeTexOffset(2), _colorMapTexOffset(3), _missingValueTexOffset(4),
  _zCoordsTexOffset(5), _xyCoordsTexOffset(6), _depthTexOffset(7)
{
    _backFaceTextureId = 0;
    _frontFaceTextureId = 0;
    _volumeTextureId = 0;
    _missingValueTextureId = 0;
    _colorMapTextureId = 0;
    _zCoordsTextureId = 0;
    _xyCoordsTextureId = 0;
    _depthTextureId = 0;
    _frameBufferId = 0;

    _vertexArrayId = 0;
    _vertexBufferId = 0;
    _indexBufferId = 0;
    _vertexAttribId = 0;
    _xyCoordsBufferId = 0;

    _1stPassShader = nullptr;
    _2ndPassShader = nullptr;
    _3rdPassShader = nullptr;
    _3rdPassMode1Shader = nullptr;
    _3rdPassMode2Shader = nullptr;

    _drawBuffers[0] = 0;
    _drawBuffers[1] = 0;

    GLint viewport[4] = {0, 0, 0, 0};
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
    if (_zCoordsTextureId) {
        glDeleteTextures(1, &_zCoordsTextureId);
        _zCoordsTextureId = 0;
    }
    if (_xyCoordsTextureId) {
        glDeleteTextures(1, &_xyCoordsTextureId);
        _xyCoordsTextureId = 0;
    }
    if (_depthTextureId) {
        glDeleteTextures(1, &_depthTextureId);
        _depthTextureId = 0;
    }

    // delete buffers
    if (_frameBufferId) {
        glDeleteFramebuffers(1, &_frameBufferId);
        _frameBufferId = 0;
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
        myGridMin[i] = 0;
        myGridMax[i] = 0;
        dims[i] = 0;
    }

    baseStepSize = 0.0f;
    myCurrentTimeStep = 0;
    myVariableName = "";
    myRefinementLevel = -1;
    myCompressionLevel = -1;
    myCastingMode = 1;
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

int RayCaster::UserCoordinates::GetCurrentGrid(const RayCasterParams *params, DataMgr *dataMgr, StructuredGrid **gridpp) const
{
    std::vector<double> extMin, extMax;
    params->GetBox()->GetExtents(extMin, extMax);
    StructuredGrid *grid =
        dynamic_cast<StructuredGrid *>(dataMgr->GetVariable(params->GetCurrentTimestep(), params->GetVariableName(), params->GetRefinementLevel(), params->GetCompressionLevel(), extMin, extMax));
    if (grid == nullptr) {
        MyBase::SetErrMsg("UserCoordinates::GetCurrentGrid() isn't on a StructuredGrid; "
                          "the behavior is undefined in this case.");
        return GRIDERROR;
    } else {
        *gridpp = grid;
        return 0;
    }
}

bool RayCaster::UserCoordinates::IsMetadataUpToDate(const RayCasterParams *params, const StructuredGrid *grid, DataMgr *dataMgr) const
{
    if ((myCurrentTimeStep != params->GetCurrentTimestep()) || (myVariableName != params->GetVariableName()) || (myRefinementLevel != params->GetRefinementLevel())
        || (myCastingMode != params->GetCastingMode()) || (myCompressionLevel != params->GetCompressionLevel())) {
        return false;
    }

    // compare grid extents
    std::vector<double> newMin, newMax;
    grid->GetUserExtents(newMin, newMax);
    assert(newMin.size() == 3 || newMax.size() == 3);
    for (int i = 0; i < 3; i++) {
        if ((myGridMin[i] != (float)newMin[i]) || (myGridMax[i] != (float)newMax[i])) return false;
    }

    // now we know it's up to date!
    return true;
}

int RayCaster::UserCoordinates::UpdateFaceAndData(const RayCasterParams *params, const StructuredGrid *grid, DataMgr *dataMgr)
{
    /* Update meta data */
    std::vector<double> newMin, newMax;
    grid->GetUserExtents(newMin, newMax);
    assert(newMin.size() == 3 || newMax.size() == 3);
    for (int i = 0; i < 3; i++) {
        myGridMin[i] = (float)newMin[i];
        myGridMax[i] = (float)newMax[i];
    }
    myCurrentTimeStep = params->GetCurrentTimestep();
    myVariableName = params->GetVariableName();
    myRefinementLevel = params->GetRefinementLevel();
    myCompressionLevel = params->GetCompressionLevel();
    myCastingMode = params->GetCastingMode();

    /* Update member variables */
    std::vector<size_t> gridDims = grid->GetDimensions();
    dims[0] = gridDims[0];
    dims[1] = gridDims[1];
    dims[2] = gridDims[2];

    // Save front face user coordinates ( z == dims[2] - 1 )
    if (frontFace) delete[] frontFace;
    frontFace = new float[dims[0] * dims[1] * 3];
    this->FillCoordsXYPlane(grid, dims[2] - 1, frontFace);

    // Save back face user coordinates ( z == 0 )
    if (backFace) delete[] backFace;
    backFace = new float[dims[0] * dims[1] * 3];
    this->FillCoordsXYPlane(grid, 0, backFace);

    // Save right face user coordinates ( x == dims[0] - 1 )
    if (rightFace) delete[] rightFace;
    rightFace = new float[dims[1] * dims[2] * 3];
    this->FillCoordsYZPlane(grid, dims[0] - 1, rightFace);

    // Save left face user coordinates ( x == 0 )
    if (leftFace) delete[] leftFace;
    leftFace = new float[dims[1] * dims[2] * 3];
    this->FillCoordsYZPlane(grid, 0, leftFace);

    // Save top face user coordinates ( y == dims[1] - 1 )
    if (topFace) delete[] topFace;
    topFace = new float[dims[0] * dims[2] * 3];
    this->FillCoordsXZPlane(grid, dims[1] - 1, topFace);

    // Save bottom face user coordinates ( y == 0 )
    if (bottomFace) delete[] bottomFace;
    bottomFace = new float[dims[0] * dims[2] * 3];
    this->FillCoordsXZPlane(grid, 0, bottomFace);

    // Save the data field values and missing values
    size_t numOfVertices = dims[0] * dims[1] * dims[2];
    if (dataField) {
        delete[] dataField;
        dataField = nullptr;
    }
    try {
        dataField = new float[numOfVertices];
    } catch (const std::bad_alloc &e) {
        MyBase::SetErrMsg(e.what());
        return MEMERROR;
    }
    if (missingValueMask) {
        delete[] missingValueMask;
        missingValueMask = nullptr;
    }

    StructuredGrid::ConstIterator valItr = grid->cbegin();    // Iterator for data field values

    if (grid->HasMissingData()) {
        float missingValue = grid->GetMissingValue();
        try {
            missingValueMask = new unsigned char[numOfVertices];
        } catch (const std::bad_alloc &e) {
            MyBase::SetErrMsg(e.what());
            return MEMERROR;
        }
        float dataValue;
        for (size_t i = 0; i < numOfVertices; i++) {
            dataValue = *valItr;
            if (dataValue == missingValue) {
                dataField[i] = 0.0f;
                missingValueMask[i] = 127u;
            } else {
                dataField[i] = dataValue;
                missingValueMask[i] = 0u;
            }
            ++valItr;
        }
    } else    // No missing value!
    {
        for (size_t i = 0; i < numOfVertices; i++) {
            dataField[i] = *valItr;
            ++valItr;
        }
    }

    // Calculate the base step size
    this->FindBaseStepSize(FixedStep);

    return 0;
}

void RayCaster::UserCoordinates::FillCoordsXYPlane(const StructuredGrid *grid, size_t planeIdx, float *coords)
{
    size_t idx = 0;
    double buf[3];
    for (size_t y = 0; y < dims[1]; y++)
        for (size_t x = 0; x < dims[0]; x++) {
            grid->GetUserCoordinates(x, y, planeIdx, buf[0], buf[1], buf[2]);
            coords[idx++] = (float)buf[0];
            coords[idx++] = (float)buf[1];
            coords[idx++] = (float)buf[2];
        }
}

void RayCaster::UserCoordinates::FillCoordsYZPlane(const StructuredGrid *grid, size_t planeIdx, float *coords)
{
    size_t idx = 0;
    double buf[3];
    for (size_t z = 0; z < dims[2]; z++)
        for (size_t y = 0; y < dims[1]; y++) {
            grid->GetUserCoordinates(planeIdx, y, z, buf[0], buf[1], buf[2]);
            coords[idx++] = (float)buf[0];
            coords[idx++] = (float)buf[1];
            coords[idx++] = (float)buf[2];
        }
}

void RayCaster::UserCoordinates::FillCoordsXZPlane(const StructuredGrid *grid, size_t planeIdx, float *coords)
{
    size_t idx = 0;
    double buf[3];
    for (size_t z = 0; z < dims[2]; z++)
        for (size_t x = 0; x < dims[0]; x++) {
            grid->GetUserCoordinates(x, planeIdx, z, buf[0], buf[1], buf[2]);
            coords[idx++] = (float)buf[0];
            coords[idx++] = (float)buf[1];
            coords[idx++] = (float)buf[2];
        }
}

int RayCaster::UserCoordinates::UpdateCurviCoords(const RayCasterParams *params, const StructuredGrid *grid, DataMgr *dataMgr)
{
    if (xyCoords) delete[] xyCoords;
    xyCoords = new float[dims[0] * dims[1] * 2];
    if (zCoords) delete[] zCoords;
    try {
        zCoords = new float[dims[0] * dims[1] * dims[2]];
    } catch (const std::bad_alloc &e) {
        MyBase::SetErrMsg(e.what());
        return MEMERROR;
    }

    // Gather the XY coordinate from frontFace buffer
    size_t xyIdx = 0, xyzIdx = 0;
    for (size_t y = 0; y < dims[1]; y++)
        for (size_t x = 0; x < dims[0]; x++) {
            xyCoords[xyIdx++] = frontFace[xyzIdx++];
            xyCoords[xyIdx++] = frontFace[xyzIdx++];
            xyzIdx++;
        }

    // Gather the Z coordinates from grid
    StructuredGrid::ConstCoordItr coordItr = grid->ConstCoordBegin();
    size_t                        numOfVertices = dims[0] * dims[1] * dims[2];
    for (xyzIdx = 0; xyzIdx < numOfVertices; xyzIdx++) {
        zCoords[xyzIdx] = float((*coordItr)[2]);
        ++coordItr;
    }

    // Calculate the base step size
    this->FindBaseStepSize(CellTraversal);

    return 0;
}

void RayCaster::UserCoordinates::FindBaseStepSize(int mode)
{
    if (mode == FixedStep) {
        float df[3] = {float(dims[0]), float(dims[1]), float(dims[2])};
        float numCells = std::sqrt(df[0] * df[0] + df[1] * df[1] + df[2] * df[2]);
        float span[3] = {myGridMax[0] - myGridMin[0], myGridMax[1] - myGridMin[1], myGridMax[2] - myGridMin[2]};
        if (numCells < 50.0f)    // Make sure at least 100 steps
            baseStepSize = std::sqrt(span[0] * span[0] + span[1] * span[1] + span[2] * span[2]) / 100.0f;
        else {    // Use Nyquist frequency
            baseStepSize = std::sqrt(span[0] * span[0] + span[1] * span[1] + span[2] * span[2]) / (numCells * 2.0f);
        }
    } else    // mode == CellTraversal
    {
        float dist, smallest, diffx, diffy, diffz;
        diffx = xyCoords[2] - xyCoords[0];
        diffy = xyCoords[3] - xyCoords[1];
        smallest = diffx * diffx + diffy * diffy;
        size_t idx1, idx2, x, y, z;

        // Find the smallest edge among the XY plane
        for (y = 0; y < dims[1]; y++)
            for (x = 0; x < dims[0]; x++) {
                // First test edge between vertex (x, y) and (x+1, y)
                if (x < dims[0] - 1) {
                    idx1 = (y * dims[0] + x) * 2;
                    idx2 = (y * dims[0] + x + 1) * 2;
                    diffx = xyCoords[idx2] - xyCoords[idx1];
                    diffy = xyCoords[idx2 + 1] - xyCoords[idx1 + 1];
                    dist = diffx * diffx + diffy * diffy;
                    smallest = smallest < dist ? smallest : dist;
                }

                // Second test edge between vertex (x, y) and (x, y+1)
                if (y < dims[1] - 1) {
                    idx1 = (y * dims[0] + x) * 2;
                    idx2 = ((y + 1) * dims[0] + x) * 2;
                    diffx = xyCoords[idx2] - xyCoords[idx1];
                    diffy = xyCoords[idx2 + 1] - xyCoords[idx1 + 1];
                    dist = diffx * diffx + diffy * diffy;
                    smallest = smallest < dist ? smallest : dist;
                }
            }

        // Find the smallest edge among the Z edges
        size_t planeSize = dims[0] * dims[1];
        for (z = 0; z < dims[2] - 1; z++)
            for (y = 0; y < dims[1]; y++)
                for (x = 0; x < dims[0]; x++) {
                    // Test edge between vertex (x, y, z) and (x, y, z+1)
                    idx1 = z * planeSize + y * dims[0] + x;
                    idx2 = (z + 1) * planeSize + y * dims[0] + x;
                    diffz = zCoords[idx2] - zCoords[idx1];
                    dist = diffz * diffz;
                    smallest = smallest < dist ? smallest : dist;
                }

        baseStepSize = std::sqrt(smallest);
    }
}

int RayCaster::_initializeGL()
{
    // Load 1st and 2nd pass shaders
    ShaderProgram *shader = nullptr;
    if ((shader = _glManager->shaderManager->GetShader("RayCaster1stPass")))
        _1stPassShader = shader;
    else
        return GLERROR;

    if ((shader = _glManager->shaderManager->GetShader("RayCaster2ndPass")))
        _2ndPassShader = shader;
    else
        return GLERROR;

    // Load 3rd pass shaders
    if (_load3rdPassShaders() != 0) {
        MyBase::SetErrMsg("Failed to load shaders!");
        return GLERROR;
    }

    // Get the current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    std::memcpy(_currentViewport, viewport, 4 * sizeof(GLint));

    int max_buffer_size, max_texture_size, max_3dtexture_size;
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &max_buffer_size);
    std::cout << "max buffer size = " << max_buffer_size << std::endl;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
    std::cout << "max texture size = " << max_texture_size << std::endl;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max_3dtexture_size);
    std::cout << "max 3D texture size = " << max_3dtexture_size << std::endl;

    // Create any textures, framebuffers, etc.
    if (_initializeFramebufferTextures() != 0) {
        MyBase::SetErrMsg("Failed to Create Framebuffer and Textures!");
        return GLERROR;
    }

    return 0;    // Success
}

int RayCaster::_paintGL(bool fast)
{
    // Collect params and grid that will be used repeatedly
    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    if (!params) {
        MyBase::SetErrMsg("Error occured during retrieving RayCaster parameters!");
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return PARAMSERROR;
    }
    // Do not perform any fast rendering in cell traverse mode
    int castingMode = int(params->GetCastingMode());
    if (castingMode == CellTraversal && fast) return 0;

    StructuredGrid *grid = nullptr;
    if (_userCoordinates.GetCurrentGrid(params, _dataMgr, &grid) != 0) {
        MyBase::SetErrMsg("Failed to retrieve a StructuredGrid");
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return GRIDERROR;
    }

#ifndef NDEBUG
    // Do NOT try to reload shaders in Release mode due to
    //   large latencies on parallel filesystems
    if (_load3rdPassShaders() != 0) {
        MyBase::SetErrMsg("Failed to load shaders");
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return GLERROR;
    }
#endif

    const MatrixManager *mm = Renderer::_glManager->matrixManager;

    _updateViewportWhenNecessary();
    glDisable(GL_POLYGON_SMOOTH);

    // Collect existing depth value of the scene
    glBindTexture(GL_TEXTURE_2D, _depthTextureId);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _currentViewport[0], _currentViewport[1], _currentViewport[2], _currentViewport[3], 0);

    glBindVertexArray(_vertexArrayId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);

    // Use the correct shader for 3rd pass rendering
    if (castingMode == FixedStep)
        _3rdPassShader = _3rdPassMode1Shader;
    else if (castingMode == CellTraversal)
        _3rdPassShader = _3rdPassMode2Shader;
    else {
        MyBase::SetErrMsg("RayCasting Mode not supported!");
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete grid;
        return JUSTERROR;
    }

    // If there is an update event
    if (!_userCoordinates.IsMetadataUpToDate(params, grid, _dataMgr)) {
        int success = _userCoordinates.UpdateFaceAndData(params, grid, _dataMgr);
        if (success != 0) {
            MyBase::SetErrMsg("Error occured during updating face and volume data!");
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            delete grid;
            return JUSTERROR;
        }

        if (castingMode == CellTraversal && _userCoordinates.UpdateCurviCoords(params, grid, _dataMgr) != 0) {
            MyBase::SetErrMsg("Error occured during updating curvilinear coordinates!");
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            delete grid;
            return JUSTERROR;
        }

        _updateDataTextures(castingMode);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
    glViewport(0, 0, _currentViewport[2], _currentViewport[3]);

    glm::mat4 ModelView = mm->GetModelViewMatrix();

    // 1st pass: render back facing polygons to texture0 of the framebuffer
    _drawVolumeFaces(1, castingMode, false);

    // Detect if we're inside the volume
    glm::mat4           InversedMV = glm::inverse(ModelView);
    std::vector<double> cameraUser(4, 1.0);    // camera position in user coordinates
    cameraUser[0] = InversedMV[3][0];
    cameraUser[1] = InversedMV[3][1];
    cameraUser[2] = InversedMV[3][2];
    std::vector<size_t> cameraCellIndices;    // camera position in which cell?
    bool                insideACell = grid->GetIndicesCell(cameraUser, cameraCellIndices);
    if (insideACell) { _updateNearClippingPlane(); }

    // 2nd pass, render front facing polygons
    _drawVolumeFaces(2, castingMode, insideACell);

    // Collect the color map for the 3rd pass rendering, which is the actual ray casting.
    _updateColormap(params);
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());
    glBindTexture(GL_TEXTURE_1D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _currentViewport[2], _currentViewport[3]);

    glFinish();
    struct timeval start, finish;
    gettimeofday(&start, NULL);
    // 3rd pass, perform ray casting
    _drawVolumeFaces(3, castingMode, insideACell, InversedMV, fast);
    glFinish();
    gettimeofday(&finish, NULL);
    std::cout << _getElapsedSeconds(&start, &finish) << std::endl;

    // Restore default VAO settings!
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete grid;

    return 0;
}

int RayCaster::_initializeFramebufferTextures()
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

    /* Set "_backFaceTextureId" as colour attachement #0,
       and "_frontFaceTextureId" as attachement #1       */
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _backFaceTextureId, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _frontFaceTextureId, 0);
    _drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    _drawBuffers[1] = GL_COLOR_ATTACHMENT1;
    glDrawBuffers(2, _drawBuffers);

    /* Check if framebuffer is complete */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        MyBase::SetErrMsg("_openGLInitialization(): Framebuffer failed; "
                          "the behavior is then undefined.");
        return GLERROR;
    }

    /* Bind the default frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* Generate and configure 3D texture: _volumeTextureId */
    glGenTextures(1, &_volumeTextureId);
    glActiveTexture(GL_TEXTURE0 + _volumeTexOffset);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate and configure 1D texture: _colorMapTextureId */
    glGenTextures(1, &_colorMapTextureId);
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    /* Generate and configure 3D texture: _missingValueTextureId */
    glGenTextures(1, &_missingValueTextureId);
    glActiveTexture(GL_TEXTURE0 + _missingValueTexOffset);
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate 3D texture: _zCoordsTextureId */
    glGenTextures(1, &_zCoordsTextureId);
    glActiveTexture(GL_TEXTURE0 + _zCoordsTexOffset);
    glBindTexture(GL_TEXTURE_3D, _zCoordsTextureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    /* Generate buffer texture for 2D X-Y coordinates */
    /*   Note: the max size of buffer texture is actually much smaller than
         that of a regular 3D texture, making it not suitable for a 3D volume.
         (It is tested to be 512^3 on GeForce 1060 and Quadro GP100 cards.)  */
    glGenTextures(1, &_xyCoordsTextureId);
    glGenBuffers(1, &_xyCoordsBufferId);

    /* Generate and configure depth texture */
    glGenTextures(1, &_depthTextureId);
    glActiveTexture(GL_TEXTURE0 + _depthTexOffset);
    glBindTexture(GL_TEXTURE_2D, _depthTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Bind the default textures */
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, 0);

    return 0;
}

void RayCaster::_drawVolumeFaces(int whichPass, int castingMode, bool insideACell, const glm::mat4 &InversedMV, bool fast)
{
    glm::mat4 modelview = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();

    if (whichPass == 1) {
        _1stPassShader->Bind();
        _1stPassShader->SetUniform("MV", modelview);
        _1stPassShader->SetUniform("Projection", projection);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(0.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_GEQUAL);
        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 0, black);    // clear GL_COLOR_ATTACHMENT0
        glDisable(GL_BLEND);
    } else if (whichPass == 2) {
        _2ndPassShader->Bind();
        _2ndPassShader->SetUniform("MV", modelview);
        _2ndPassShader->SetUniform("Projection", projection);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL);
        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 1, black);    // clear GL_COLOR_ATTACHMENT1
        glDisable(GL_BLEND);
    } else    // 3rd pass
    {
        _3rdPassShader->Bind();
        _load3rdPassUniforms(castingMode, InversedMV, fast);
        _3rdPassSpecialHandling(fast, castingMode);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (insideACell)    // Only enters this section when 1st or 2nd pass
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
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(0);
}

void RayCaster::_load3rdPassUniforms(int castingMode, const glm::mat4 &inversedMV, bool fast) const
{
    ShaderProgram *shader = _3rdPassShader;

    glm::mat4 modelview = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();

    shader->SetUniform("MV", modelview);
    shader->SetUniform("Projection", projection);
    // shader->SetUniform("transposedInverseMV", glm::transpose(inversedMV));

    const float *cboxMin = _userCoordinates.myGridMin;
    const float *cboxMax = _userCoordinates.myGridMax;
    shader->SetUniform("boxMin", (glm::vec3 &)*cboxMin);
    shader->SetUniform("boxMax", (glm::vec3 &)*cboxMax);
    shader->SetUniform("colorMapRange", (glm::vec3 &)*_colorMapRange);
    shader->SetUniform("viewportDims", glm::ivec2(_currentViewport[2], _currentViewport[3]));

    glm::ivec3 volumeDims(int(_userCoordinates.dims[0]), int(_userCoordinates.dims[1]), int(_userCoordinates.dims[2]));
    shader->SetUniform("volumeDims", volumeDims);

    float planes[24];    // 6 planes, each with 4 elements
    Renderer::GetClippingPlanes(planes);
    shader->SetUniformArray("clipPlanes", 6, (glm::vec4 *)planes);

    // Get light settings from params.
    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    bool             lighting = params->GetLighting();
    if (lighting) {
        std::vector<double> coeffsD = params->GetLightingCoeffs();
        float               coeffsF[4] = {float(coeffsD[0]), float(coeffsD[1]), float(coeffsD[2]), float(coeffsD[3])};
        shader->SetUniformArray("lightingCoeffs", 4, coeffsF);
    }

    // Pack in fast mode, lighting, and missing value booleans together
    int flags[3] = {int(fast), int(lighting), int(_userCoordinates.missingValueMask != nullptr)};
    shader->SetUniformArray("flags", 3, flags);

    // Calculate the step size with sample rate multiplier taken into account.
    float stepSize1D;
    if (castingMode == FixedStep) {
        float multiplier;
        switch (params->GetSampleRateMultiplier()) {
        case 0: multiplier = 1.0f; break;    // These values need to be in sync with
        case 1: multiplier = 2.0f; break;    //   the multiplier values in the GUI.
        case 2: multiplier = 4.0f; break;
        case 3: multiplier = 0.5f; break;
        case 4: multiplier = 0.25f; break;
        case 5: multiplier = 0.125f; break;
        default: multiplier = 1.0f; break;
        }
        stepSize1D = _userCoordinates.baseStepSize / multiplier;
        if (fast) stepSize1D *= 8.0f;    //  Increase step size, thus fewer steps, when fast rendering
    } else
        stepSize1D = _userCoordinates.baseStepSize * 0.95f;
    shader->SetUniform("stepSize1D", stepSize1D);

    // Pass in textures
    glActiveTexture(GL_TEXTURE0 + _backFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    shader->SetUniform("backFaceTexture", _backFaceTexOffset);

    glActiveTexture(GL_TEXTURE0 + _frontFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    shader->SetUniform("frontFaceTexture", _frontFaceTexOffset);

    glActiveTexture(GL_TEXTURE0 + _volumeTexOffset);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
    shader->SetUniform("volumeTexture", _volumeTexOffset);

    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    shader->SetUniform("colorMapTexture", _colorMapTexOffset);

    glActiveTexture(GL_TEXTURE0 + _missingValueTexOffset);
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
    shader->SetUniform("missingValueMaskTexture", _missingValueTexOffset);

    if (castingMode == CellTraversal) {
        glActiveTexture(GL_TEXTURE0 + _zCoordsTexOffset);
        glBindTexture(GL_TEXTURE_3D, _zCoordsTextureId);
        shader->SetUniform("zCoordsTexture", _zCoordsTexOffset);

        glActiveTexture(GL_TEXTURE0 + _xyCoordsTexOffset);
        glBindTexture(GL_TEXTURE_BUFFER, _xyCoordsTextureId);
        shader->SetUniform("xyCoordsTexture", _xyCoordsTexOffset);
    }
}

void RayCaster::_3rdPassSpecialHandling(bool fast, int castingMode)
{
    // Left empty intentially.
    // Derived classes feel free to put stuff here.
}

void RayCaster::_renderTriangleStrips(int whichPass, int castingMode) const
{
    /* Give bx, by, bz type of "unsigned int" for indexBuffer */
    unsigned int bx = (unsigned int)_userCoordinates.dims[0];
    unsigned int by = (unsigned int)_userCoordinates.dims[1];
    unsigned int bz = (unsigned int)_userCoordinates.dims[2];
    size_t       idx;

    // Each strip will have the same numOfVertices for the first 4 faces
    size_t        numOfVertices = bx * 2;
    unsigned int *indexBuffer = new unsigned int[numOfVertices];

    bool attrib1Enabled = false;    // Attribute to hold provoking index of each triangle.
    int *attrib1Buffer = nullptr;
    if (castingMode == CellTraversal && whichPass == 3) {
        attrib1Enabled = true;
        unsigned int big1 = bx > by ? bx : by;
        unsigned int small = bx < by ? bx : by;
        unsigned int big2 = bz > small ? bz : small;
        attrib1Buffer = new int[big1 * big2 * 4];    // Enough length for all faces
    }

    //
    // Render front face:
    //
    _enableVertexAttribute(_userCoordinates.frontFace, bx * by * 3, attrib1Enabled);
    for (unsigned int y = 0; y < by - 1; y++)    // Looping over every TriangleStrip
    {
        idx = 0;
        // This loop controls rendering order of the triangle strip. It
        // provides the indices for each vertex in a strip. Strips are
        // created one row at a time.
        //
        for (unsigned int x = 0; x < bx; x++)    // Filling indices for vertices of this TriangleStrip
        {
            indexBuffer[idx++] = (y + 1) * bx + x;
            indexBuffer[idx++] = y * bx + x;
        }

        // In cell-traverse ray casting mode we need a cell index for the
        // two triangles forming the face of a cell. Use the OpenGL "provoking"
        // vertex to provide this information.
        //
        if (attrib1Enabled)    // Also specify attrib1 values
        {
            for (unsigned int x = 0; x < bx; x++)    // Fill attrib1 value for each vertex
            {
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
            glBufferData(GL_ARRAY_BUFFER, bx * by * 4 * sizeof(int), attrib1Buffer, GL_STREAM_DRAW);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render back face:
    //
    _enableVertexAttribute(_userCoordinates.backFace, bx * by * 3, attrib1Enabled);
    for (unsigned int y = 0; y < by - 1; y++)    // strip by strip
    {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = y * bx + x;
            indexBuffer[idx++] = (y + 1) * bx + x;
        }
        if (attrib1Enabled) {
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
            glBufferData(GL_ARRAY_BUFFER, bx * by * 4 * sizeof(int), attrib1Buffer, GL_STREAM_DRAW);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render top face:
    //
    _enableVertexAttribute(_userCoordinates.topFace, bx * bz * 3, attrib1Enabled);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = z * bx + x;
            indexBuffer[idx++] = (z + 1) * bx + x;
        }
        if (attrib1Enabled) {
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
            glBufferData(GL_ARRAY_BUFFER, bx * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_DRAW);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render bottom face:
    //
    _enableVertexAttribute(_userCoordinates.bottomFace, bx * bz * 3, attrib1Enabled);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int x = 0; x < bx; x++) {
            indexBuffer[idx++] = (z + 1) * bx + x;
            indexBuffer[idx++] = z * bx + x;
        }
        if (attrib1Enabled) {
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
            glBufferData(GL_ARRAY_BUFFER, bx * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_DRAW);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Each strip will have the same numOfVertices for the rest 2 faces.
    numOfVertices = by * 2;
    delete[] indexBuffer;
    indexBuffer = new unsigned int[numOfVertices];

    //
    // Render right face:
    //
    _enableVertexAttribute(_userCoordinates.rightFace, by * bz * 3, attrib1Enabled);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int y = 0; y < by; y++) {
            indexBuffer[idx++] = (z + 1) * by + y;
            indexBuffer[idx++] = z * by + y;
        }
        if (attrib1Enabled) {
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
            glBufferData(GL_ARRAY_BUFFER, by * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_DRAW);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render left face
    //
    _enableVertexAttribute(_userCoordinates.leftFace, by * bz * 3, attrib1Enabled);
    for (unsigned int z = 0; z < bz - 1; z++) {
        idx = 0;
        for (unsigned int y = 0; y < by; y++) {
            indexBuffer[idx++] = z * by + y;
            indexBuffer[idx++] = (z + 1) * by + y;
        }
        if (attrib1Enabled) {
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
            glBufferData(GL_ARRAY_BUFFER, by * bz * 4 * sizeof(int), attrib1Buffer, GL_STREAM_DRAW);
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_STREAM_DRAW);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    if (attrib1Enabled) delete[] attrib1Buffer;
    delete[] indexBuffer;
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RayCaster::_enableVertexAttribute(const float *buf, size_t length, bool attrib1Enabled) const
{
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), buf, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    if (attrib1Enabled) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
    }
}

double RayCaster::_getElapsedSeconds(const struct timeval *begin, const struct timeval *end) const
{
#ifdef WIN32
    return 0.0;
#else
    return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec) / 1000000.0);
#endif
}

void RayCaster::_updateViewportWhenNecessary()
{
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
    }
}

void RayCaster::_updateColormap(RayCasterParams *params)
{
    if (params->UseSingleColor()) {
        float singleColor[4];
        params->GetConstantColor(singleColor);
        singleColor[3] = 1.0f;    // 1.0 in alpha channel
        _colorMap.resize(8);      // _colorMap will have 2 RGBA values
        for (int i = 0; i < 8; i++) _colorMap[i] = singleColor[i % 4];
        _colorMapRange[0] = 0.0f;
        _colorMapRange[1] = 0.0f;
        _colorMapRange[2] = 1e-5f;
    } else {
        params->GetMapperFunc()->makeLut(_colorMap);
        assert(_colorMap.size() % 4 == 0);
        std::vector<double> range = params->GetMapperFunc()->getMinMaxMapValue();
        _colorMapRange[0] = float(range[0]);
        _colorMapRange[1] = float(range[1]);
        _colorMapRange[2] = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ? (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f;
    }
}

void RayCaster::_updateDataTextures(int castingMode)
{
    const size_t *dims = _userCoordinates.dims;

    glActiveTexture(GL_TEXTURE0 + _volumeTexOffset);
    glBindTexture(GL_TEXTURE_3D, _volumeTextureId);
#ifdef Darwin
    //
    // Intel driver on MacOS seems to not able to correctly update the texture content
    //   when the texture is moderately big. This workaround of loading a dummy texture
    //   to force it to update seems to resolve this issue.
    //
    float dummyVolume[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 2, 2, 2, 0, GL_RED, GL_FLOAT, dummyVolume);
#endif
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, _userCoordinates.dataField);

    // Now we HAVE TO attach a missing value mask texture, because
    //   Intel driver on Mac doesn't like leaving the texture empty...
    glActiveTexture(GL_TEXTURE0 + _missingValueTexOffset);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    // Alignment adjustment. Stupid OpenGL thing.
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
    if (_userCoordinates.missingValueMask) {
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dims[0], dims[1], dims[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, _userCoordinates.missingValueMask);
    } else {
        unsigned char dummyMask[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, 2, 2, 2, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, dummyMask);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);    // Restore default alignment.

    // If using cell traverse ray casting, we need to upload user coordinates
    if (castingMode == CellTraversal) {
        glActiveTexture(GL_TEXTURE0 + _zCoordsTexOffset);
        glBindTexture(GL_TEXTURE_3D, _zCoordsTextureId);
#ifdef Darwin
        // Apply the same trick as noted above.
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 2, 2, 2, 0, GL_RED, GL_FLOAT, dummyVolume);
#endif
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, _userCoordinates.zCoords);

        // Fill data to buffer object _xyCoordsBufferId
        glBindBuffer(GL_TEXTURE_BUFFER, _xyCoordsBufferId);
        glBufferData(GL_TEXTURE_BUFFER, 2 * sizeof(float) * dims[0] * dims[1], _userCoordinates.xyCoords, GL_STATIC_READ);
        // Pass this buffer object to the buffer texture: _xyCoordsTextureId
        glActiveTexture(GL_TEXTURE0 + _xyCoordsTexOffset);
        glBindTexture(GL_TEXTURE_BUFFER, _xyCoordsTextureId);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, _xyCoordsBufferId);

        glBindBuffer(GL_TEXTURE_BUFFER, 0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }

    glBindTexture(GL_TEXTURE_3D, 0);
}

void RayCaster::_updateNearClippingPlane()
{
    glm::mat4 MVP = Renderer::_glManager->matrixManager->GetModelViewProjectionMatrix();
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
