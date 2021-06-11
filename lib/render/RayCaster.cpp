#include "vapor/glutil.h"
#include "vapor/RayCaster.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_relational.hpp>

#ifdef WIN32
    #include <Windows.h>
#else
    #include <time.h>
#endif

#define OUTOFDATE   1
#define GLNOTREADY  2
#define GRIDERROR   -1
#define JUSTERROR   -2
#define PARAMSERROR -3
#define MEMERROR    -4
#define GLERROR     -5

using namespace VAPoR;

/*
GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
void glCheckError() { }
*/

// Constructor
RayCaster::RayCaster(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string paramsType, std::string classType, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, paramsType, classType, instName, dataMgr), _backFaceTexOffset(1), _frontFaceTexOffset(2), _volumeTexOffset(3), _colorMapTexOffset(4), _missingValueTexOffset(5),
  _vertCoordsTexOffset(6), _depthTexOffset(7), _2ndVarDataTexOffset(8), _2ndVarMaskTexOffset(9)
{
    _backFaceTextureId = 0;
    _frontFaceTextureId = 0;
    _volumeTextureId = 0;
    _missingValueTextureId = 0;
    _colorMapTextureId = 0;
    _vertCoordsTextureId = 0;
    _depthTextureId = 0;
    _frameBufferId = 0;
    _2ndVarDataTexId = 0;
    _2ndVarMaskTexId = 0;

    _vertexArrayId = 0;
    _vertexBufferId = 0;
    _indexBufferId = 0;
    _vertexAttribId = 0;

    _1stPassShader = nullptr;
    _2ndPassShader = nullptr;
    _3rdPassShader = nullptr;
    _3rdPassMode1Shader = nullptr;
    _3rdPassMode2Shader = nullptr;

    for (int i = 0; i < 4; i++) _currentViewport[i] = 0;

    _currentMV = glm::mat4(0.0f);

    // Detect if it's INTEL graphics card. If so, give a magic value to the params
    const unsigned char *vendorC = glGetString(GL_VENDOR);
    std::string          vendor((char *)vendorC);
    for (int i = 0; i < vendor.size(); i++) vendor[i] = std::tolower(vendor[i]);
    std::string::size_type n = vendor.find("intel");
    if (n == std::string::npos)
        _isIntel = false;
    else
        _isIntel = true;

    // Set the default ray casting method upon creation of the RayCaster.
    _selectDefaultCastingMethod();
}

// Destructor
RayCaster::~RayCaster()
{
    // Delete framebuffers
    if (_frameBufferId) {
        glDeleteFramebuffers(1, &_frameBufferId);
        _frameBufferId = 0;
    }

    // Delete textures
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
    if (_vertCoordsTextureId) {
        glDeleteTextures(1, &_vertCoordsTextureId);
        _vertCoordsTextureId = 0;
    }
    if (_depthTextureId) {
        glDeleteTextures(1, &_depthTextureId);
        _depthTextureId = 0;
    }
    if (_2ndVarDataTexId) {
        glDeleteTextures(1, &_2ndVarDataTexId);
        _2ndVarDataTexId = 0;
    }
    if (_2ndVarMaskTexId) {
        glDeleteTextures(1, &_2ndVarMaskTexId);
        _2ndVarMaskTexId = 0;
    }

    // Delete vertex arrays
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
    vertCoords = nullptr;
    secondVarData = nullptr;
    missingValueMask = nullptr;
    secondVarMask = nullptr;
    for (int i = 0; i < 3; i++) {
        myGridMin[i] = 0;
        myGridMax[i] = 0;
        dims[i] = 0;
    }

    myCurrentTimeStep = 0;
    myVariableName = "";
    my2ndVarName = "";
    myRefinementLevel = -1;
    myCompressionLevel = -1;

    dataFieldUpToDate = false;
    vertCoordsUpToDate = false;
    secondVarUpToDate = false;
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
    if (vertCoords) {
        delete[] vertCoords;
        vertCoords = nullptr;
    }
    if (missingValueMask) {
        delete[] missingValueMask;
        missingValueMask = nullptr;
    }
    if (secondVarData) {
        delete[] secondVarData;
        secondVarData = nullptr;
    }
    if (secondVarMask) {
        delete[] secondVarMask;
        secondVarMask = nullptr;
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

void RayCaster::UserCoordinates::CheckUpToDateStatus(const RayCasterParams *params, const StructuredGrid *grid, DataMgr *dataMgr, bool use2ndVar)
{
    // First, if any of the metadata is changed, all data fields are not up-to-date
    if ((myCurrentTimeStep != params->GetCurrentTimestep()) || (myRefinementLevel != params->GetRefinementLevel()) || (myCompressionLevel != params->GetCompressionLevel())) {
        dataFieldUpToDate = false;
        vertCoordsUpToDate = false;
        secondVarUpToDate = false;
        return;
    }

    // Second, if the grid extents are changed, all data fields are not up-to-date
    std::vector<double> newMin, newMax;
    grid->GetUserExtents(newMin, newMax);
    VAssert(newMin.size() == 3 || newMax.size() == 3);
    for (int i = 0; i < 3; i++)
        if ((myGridMin[i] != (float)newMin[i]) || (myGridMax[i] != (float)newMax[i])) {
            dataFieldUpToDate = false;
            vertCoordsUpToDate = false;
            secondVarUpToDate = false;
            return;
        }

    // Third, let's compare the primary variable name
    if (myVariableName != params->GetVariableName()) { dataFieldUpToDate = false; }

    // Fourth, let's check the vertex coordinates.
    // Actually, the only way vertex coordinates go out of date is changing the metadata
    //   and user extents, which is already checked. We don't need to do anything here!

    // Fifth, let check if second variable data is up to date
    if (use2ndVar && (my2ndVarName != params->GetColorMapVariableName())) { secondVarUpToDate = false; }
}

int RayCaster::UserCoordinates::UpdateFaceAndData(const RayCasterParams *params, const StructuredGrid *grid, DataMgr *dataMgr)
{
    /* Update meta data */
    std::vector<double> newMin, newMax;
    grid->GetUserExtents(newMin, newMax);
    VAssert(newMin.size() == 3 || newMax.size() == 3);
    for (int i = 0; i < 3; i++) {
        myGridMin[i] = (float)newMin[i];
        myGridMax[i] = (float)newMax[i];
    }
    myCurrentTimeStep = params->GetCurrentTimestep();
    myVariableName = params->GetVariableName();
    myRefinementLevel = params->GetRefinementLevel();
    myCompressionLevel = params->GetCompressionLevel();

    /* Update member variables */
    auto gridDims = grid->GetDimensions();
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
    if (grid->HasMissingData()) {
        try {
            missingValueMask = new unsigned char[numOfVertices];
        } catch (const std::bad_alloc &e) {
            MyBase::SetErrMsg(e.what());
            return MEMERROR;
        }
    }

    // Now iterate the current grid
    this->IterateAGrid(grid, numOfVertices, dataField, missingValueMask);

    dataFieldUpToDate = true;

    return 0;
}

int RayCaster::UserCoordinates::Update2ndVariable(const RayCasterParams *params, DataMgr *dataMgr)
{
    VAssert(dataFieldUpToDate);

    // Update 2nd variable name
    my2ndVarName = params->GetColorMapVariableName();

    // Retrieve grid for the 2nd variable
    std::vector<double> extMin, extMax;
    params->GetBox()->GetExtents(extMin, extMax);
    StructuredGrid *grid = dynamic_cast<StructuredGrid *>(dataMgr->GetVariable(myCurrentTimeStep, my2ndVarName, myRefinementLevel, myCompressionLevel, extMin, extMax));
    if (grid == nullptr) {
        MyBase::SetErrMsg("The secondary variable isn't on a StructuredGrid; "
                          "the behavior is undefined in this case.");
        return GRIDERROR;
    }

    // Make sure the secondary grid shares the same dimention as the primary grid
    auto seconDims = grid->GetDimensions();
    for (int i = 0; i < 3; i++)
        if (seconDims[i] != dims[i]) {
            MyBase::SetErrMsg("The secondary and primary variable grids have different dimensions; "
                              "the behavior is undefined in this case.");
            delete grid;
            return GRIDERROR;
        }

    // Allocate memory
    size_t numOfVertices = dims[0] * dims[1] * dims[2];
    if (secondVarData) {
        delete[] secondVarData;
        secondVarData = nullptr;
    }
    try {
        secondVarData = new float[numOfVertices];
    } catch (const std::bad_alloc &e) {
        MyBase::SetErrMsg(e.what());
        delete grid;
        return MEMERROR;
    }
    if (secondVarMask) {
        delete[] secondVarMask;
        secondVarMask = nullptr;
    }
    if (grid->HasMissingData()) {
        try {
            secondVarMask = new unsigned char[numOfVertices];
        } catch (const std::bad_alloc &e) {
            MyBase::SetErrMsg(e.what());
            delete grid;
            return MEMERROR;
        }
    }

    // Now iterate the current grid
    this->IterateAGrid(grid, numOfVertices, secondVarData, secondVarMask);

    delete grid;
    secondVarUpToDate = true;

    return 0;
}

void RayCaster::UserCoordinates::IterateAGrid(const StructuredGrid *grid, size_t numOfVert, float *dataBuf, unsigned char *maskBuf)
{
    StructuredGrid::ConstIterator valItr = grid->cbegin();

    if (grid->HasMissingData()) {
        float missingValue = grid->GetMissingValue();
        float dataValue;
        for (size_t i = 0; i < numOfVert; i++) {
            dataValue = *valItr;
            if (dataValue == missingValue) {
                dataBuf[i] = 0.0f;
                maskBuf[i] = 127u;
            } else {
                dataBuf[i] = dataValue;
                maskBuf[i] = 0u;
            }
            ++valItr;
        }
    } else {
        for (size_t i = 0; i < numOfVert; i++) {
            dataBuf[i] = *valItr;
            ++valItr;
        }
    }
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

int RayCaster::UserCoordinates::UpdateVertCoords(const RayCasterParams *params, const StructuredGrid *grid, DataMgr *dataMgr)
{
    VAssert(dataFieldUpToDate);

    size_t numOfVertices = dims[0] * dims[1] * dims[2];
    if (vertCoords) delete[] vertCoords;
    try {
        vertCoords = new float[3 * numOfVertices];
    } catch (const std::bad_alloc &e) {
        MyBase::SetErrMsg(e.what());
        return MEMERROR;
    }

    // Gather the vertex coordinates from grid
    StructuredGrid::ConstCoordItr coordItr = grid->ConstCoordBegin();
    for (int i = 0; i < numOfVertices; i++) {
        vertCoords[3 * i] = float((*coordItr)[0]);
        vertCoords[3 * i + 1] = float((*coordItr)[1]);
        vertCoords[3 * i + 2] = float((*coordItr)[2]);
        ++coordItr;
    }

    vertCoordsUpToDate = true;

    return 0;
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
    //
    // Retrieved viewport may contain zero width and height sometimes.
    //   Need to make these dimensions positive, so the initialization routine,
    //   including the step of attaching textures to framebuffers, could complete.
    //   Later, paintGL() will have another chance to set the correct dimensions.
    //   The bottom line is, the rest of the class can safely assume that
    //   _currentViewport[4] always contains non-zero dimensions.
    //
    for (int i = 2; i < 4; i++)
        if (_currentViewport[i] < 1) _currentViewport[i] = 8;

    // Create any textures, framebuffers, etc.
    if (_initializeFramebufferTextures() != 0) {
        MyBase::SetErrMsg("Failed to Create Framebuffer and Textures!");
        return GLERROR;
    }

    return 0;    // Success
}

int RayCaster::_paintGL(bool fast)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // When viewport has zero dimensions, bail immediately.
    //   This happens when undo/redo is issued.
    if (viewport[2] < 1 || viewport[3] < 1) return 0;
    _updateViewportWhenNecessary(viewport);

    glDisable(GL_POLYGON_SMOOTH);

    // Collect params and grid that will be used repeatedly
    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    if (!params) {
        MyBase::SetErrMsg("Error occured during retrieving RayCaster parameters!");
        return PARAMSERROR;
    }

    // Return when there's no variable selected.
    if (params->GetVariableName().empty()) {
        MyBase::SetErrMsg("Please select a valid 3D variable for operation!");
        return PARAMSERROR;
    }

    // Do not perform any fast rendering in cell traverse mode
    int castingMode = int(params->GetCastingMode());
    if (castingMode == CellTraversal && fast) return 0;

    // Force casting mode to be FixedStep if on Intel GPU.
    if (_isIntel) {
        castingMode = FixedStep;
        params->SetCastingMode(FixedStep);
    }

    StructuredGrid *grid = nullptr;
    if (_userCoordinates.GetCurrentGrid(params, _dataMgr, &grid) != 0) {
        MyBase::SetErrMsg("Failed to retrieve a StructuredGrid");
        return GRIDERROR;
    }

    if (_load3rdPassShaders() != 0) {
        MyBase::SetErrMsg("Failed to load shaders");
        delete grid;
        return GLERROR;
    }

    // Use the correct shader for 3rd pass rendering
    if (castingMode == FixedStep)
        _3rdPassShader = _3rdPassMode1Shader;
    else if (castingMode == CellTraversal)
        _3rdPassShader = _3rdPassMode2Shader;
    else {
        MyBase::SetErrMsg("RayCasting Mode not supported!");
        delete grid;
        return JUSTERROR;
    }

    // Retrieve if we're using secondary variable
    bool use2ndVar = _use2ndVariable(params);

    // Check if there is an update event
    _userCoordinates.CheckUpToDateStatus(params, grid, _dataMgr, use2ndVar);

    // Update primary variable data field
    if (!_userCoordinates.dataFieldUpToDate) {
        int success = _userCoordinates.UpdateFaceAndData(params, grid, _dataMgr);
        if (success != 0) {
            MyBase::SetErrMsg("Error occured during updating face and volume data!");
            delete grid;
            return JUSTERROR;
        }

        // Texture for primary variable data is updated only when data changes
        _updateDataTextures();
    }

    // Update vertex coordinates field only when using CellTraversal method.
    glm::mat4 ModelView = Renderer::_glManager->matrixManager->GetModelViewMatrix();
    if (castingMode == CellTraversal) {
        if (!_userCoordinates.vertCoordsUpToDate) {
            int success = _userCoordinates.UpdateVertCoords(params, grid, _dataMgr);
            if (success != 0) {
                MyBase::SetErrMsg("Error occured during updating curvilinear coordinates!");
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_1D, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_3D, 0);
                delete grid;
                return JUSTERROR;
            }
        }

        // Transform vertex coordinate data to eye space, and then send to GPU.
        // This step takes place at every loop.
        if (_updateVertCoordsTexture(ModelView) != 0) {
            MyBase::SetErrMsg("Error occured during calculating eye coordinates!");
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_1D, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_3D, 0);
            delete grid;
            return MEMERROR;
        }
    }

    // Update secondary variable
    if (use2ndVar && !_userCoordinates.secondVarUpToDate) {
        int success = _userCoordinates.Update2ndVariable(params, _dataMgr);
        if (success != 0) {
            MyBase::SetErrMsg("Error occured during updating secondary variable!");
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_1D, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_3D, 0);
            delete grid;
            return JUSTERROR;
        }
    }

    // This function has no effect for DVR. It's only usable for IsoSurface
    //   with 2nd variable enabled.
    _update2ndVarTextures();

    glBindVertexArray(_vertexArrayId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
    glViewport(0, 0, _currentViewport[2], _currentViewport[3]);

    // 1st pass: render back facing polygons to texture0 of the framebuffer
    std::vector<size_t> cameraCellIdx(0);
    _drawVolumeFaces(1, castingMode, cameraCellIdx);

    // Detect if we're inside the volume
    glm::mat4           InversedMV = glm::inverse(ModelView);
    std::vector<double> cameraUser(4, 1.0);    // current camera position in user coordinates
    cameraUser[0] = InversedMV[3][0];
    cameraUser[1] = InversedMV[3][1];
    cameraUser[2] = InversedMV[3][2];
    bool insideACell = grid->GetIndicesCell(cameraUser, cameraCellIdx);
    if (!insideACell) cameraCellIdx.clear();    // Make sure size 0 to indicate outside of the volume

    // 2nd pass, render front facing polygons
    _drawVolumeFaces(2, castingMode, cameraCellIdx);

    // Update color map texture
    _updateColormap(params);
    glActiveTexture(GL_TEXTURE0 + _colorMapTexOffset);
    glBindTexture(GL_TEXTURE_1D, _colorMapTextureId);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, _colorMap.size() / 4, 0, GL_RGBA, GL_FLOAT, _colorMap.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _currentViewport[2], _currentViewport[3]);

    // 3rd pass, perform ray casting
    _drawVolumeFaces(3, castingMode, cameraCellIdx, InversedMV, fast);

    // Restore OpenGL values changed in this function.
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, 0);

    delete grid;

    return 0;
}

int RayCaster::_initializeFramebufferTextures()
{
    /* Create Vertex Array Object (VAO) */
    glGenVertexArrays(1, &_vertexArrayId);
    glGenBuffers(1, &_vertexBufferId);
    glGenBuffers(1, &_indexBufferId);
    if (!_isIntel) glGenBuffers(1, &_vertexAttribId);

    /* Generate and configure 2D back-facing texture */
    glGenTextures(1, &_backFaceTextureId);
    glActiveTexture(GL_TEXTURE0 + _backFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _currentViewport[2], _currentViewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);
    this->_configure2DTextureLinearInterpolation();

    /* Generate and configure 2D front-facing texture */
    glGenTextures(1, &_frontFaceTextureId);
    glActiveTexture(GL_TEXTURE0 + _frontFaceTexOffset);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _currentViewport[2], _currentViewport[3], 0, GL_RGBA, GL_FLOAT, nullptr);
    this->_configure2DTextureLinearInterpolation();

    /* Create an Frame Buffer Object for the front and back side of the volume. */
    glGenFramebuffers(1, &_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);

    /* Set "_backFaceTextureId"  as color attachement #0,
       and "_frontFaceTextureId" as color attachement #1.  */
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _backFaceTextureId, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _frontFaceTextureId, 0);
    GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

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
    this->_configure3DTextureLinearInterpolation();

    /* Generate and configure 3D texture: _2ndVarDataTexId */
    glGenTextures(1, &_2ndVarDataTexId);
    glActiveTexture(GL_TEXTURE0 + _2ndVarDataTexOffset);
    glBindTexture(GL_TEXTURE_3D, _2ndVarDataTexId);
    this->_configure3DTextureLinearInterpolation();

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
    this->_configure3DTextureNearestInterpolation();

    /* Generate and configure 3D texture: _2ndVarMaskTexId */
    glGenTextures(1, &_2ndVarMaskTexId);
    glActiveTexture(GL_TEXTURE0 + _2ndVarMaskTexOffset);
    glBindTexture(GL_TEXTURE_3D, _2ndVarMaskTexId);
    this->_configure3DTextureNearestInterpolation();

    if (!_isIntel) {
        /* Generate 3D texture: _vertCoordsTextureId */
        glGenTextures(1, &_vertCoordsTextureId);
        glActiveTexture(GL_TEXTURE0 + _vertCoordsTexOffset);
        glBindTexture(GL_TEXTURE_3D, _vertCoordsTextureId);
        this->_configure3DTextureNearestInterpolation();
    }

    /* Generate and configure 2D depth texture */
    glGenTextures(1, &_depthTextureId);
    glActiveTexture(GL_TEXTURE0 + _depthTexOffset);
    glBindTexture(GL_TEXTURE_2D, _depthTextureId);
    this->_configure2DTextureLinearInterpolation();

    return 0;
}

void RayCaster::_configure3DTextureNearestInterpolation() const
{
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void RayCaster::_configure3DTextureLinearInterpolation() const
{
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void RayCaster::_configure2DTextureLinearInterpolation() const
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void RayCaster::_drawVolumeFaces(int whichPass, int castingMode, const std::vector<size_t> &cameraCellIdx, const glm::mat4 &InversedMV, bool fast) const
{
    VAssert(cameraCellIdx.size() == 0 || cameraCellIdx.size() == 3);
    bool insideVolume = (cameraCellIdx.size() == 3);

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

        // Render the back side of the volume.
        _renderTriangleStrips(1, castingMode);
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

        // Render the front side of the volume if not inside it.
        // Do nothing if inside the volume.
        if (!insideVolume) _renderTriangleStrips(2, castingMode);
    } else    // 3rd pass
    {
        _3rdPassShader->Bind();
        _3rdPassShader->SetUniform("MV", modelview);
        _3rdPassShader->SetUniform("Projection", projection);
        _3rdPassShader->SetUniform("inversedMV", InversedMV);
        if (castingMode == CellTraversal) {
            // Upload entryCellIdx, no matter inside or outside of the volume
            glm::ivec3 entryCellIdx(0);
            if (insideVolume) {
                entryCellIdx.x = int(cameraCellIdx[0]);
                entryCellIdx.y = int(cameraCellIdx[1]);
                entryCellIdx.z = int(cameraCellIdx[2]);
            }
            _3rdPassShader->SetUniform("entryCellIdx", entryCellIdx);
        }
        _load3rdPassUniforms(castingMode, fast, insideVolume);
        _3rdPassSpecialHandling(fast, castingMode);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        // When DVR is rendered the last, it blends with previous rendering.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Renders the near clipping plane if inside the volume.
        //   Otherwise, render the front side of the volume.
        if (insideVolume) {
            // Transform the near clipping plane to model coordinate
            //             0---------2
            //              |       |
            //              |       |
            //              |       |
            //             1|_______|3
            GLfloat   nearCoords[12];
            glm::mat4 MVP = _glManager->matrixManager->GetModelViewProjectionMatrix();
            glm::mat4 InversedMVP = glm::inverse(MVP);
            glm::vec4 topLeftNDC(-1.0f, 1.0f, -0.999f, 1.0f);
            glm::vec4 bottomLeftNDC(-1.0f, -1.0f, -0.999f, 1.0f);
            glm::vec4 topRightNDC(1.0f, 1.0f, -0.999f, 1.0f);
            glm::vec4 bottomRightNDC(1.0f, -1.0f, -0.999f, 1.0f);
            glm::vec4 nearP[4];
            nearP[0] = InversedMVP * topLeftNDC;
            nearP[1] = InversedMVP * bottomLeftNDC;
            nearP[2] = InversedMVP * topRightNDC;
            nearP[3] = InversedMVP * bottomRightNDC;
            for (int i = 0; i < 4; i++) {
                nearP[i] /= nearP[i].w;
                std::memcpy(nearCoords + i * 3, glm::value_ptr(nearP[i]), 3 * sizeof(GLfloat));
            }

            glEnableVertexAttribArray(0);    // attribute 0 is vertex coordinates
            glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
            glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), nearCoords, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDisableVertexAttribArray(0);
        } else {
            _renderTriangleStrips(3, castingMode);
        }
    }

    // Let's also do some clean up.
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0);
}

void RayCaster::_load3rdPassUniforms(int castingMode, bool fast, bool insideVolume) const
{
    ShaderProgram *shader = _3rdPassShader;

    shader->SetUniform("colorMapRange", glm::make_vec3(_colorMapRange));
    shader->SetUniform("viewportDims", glm::ivec2(_currentViewport[2], _currentViewport[3]));
    const size_t *cdims = _userCoordinates.dims;
    glm::ivec3    volumeDims((int)cdims[0], (int)cdims[1], (int)cdims[2]);
    shader->SetUniform("volumeDims", volumeDims);
    float planes[24];    // 6 planes, each with 4 elements
    Renderer::GetClippingPlanes(planes);
    shader->SetUniformArray("clipPlanes", 6, (glm::vec4 *)planes);

    glm::vec3 gridMin, gridMax;
    for (int i = 0; i < 3; i++) {
        gridMin[i] = _userCoordinates.myGridMin[i];
        gridMax[i] = _userCoordinates.myGridMax[i];
    }
    if (castingMode == FixedStep) {
        shader->SetUniform("boxMin", gridMin);
        shader->SetUniform("boxMax", gridMax);
    }

    // Get light settings from params.
    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    bool             lighting = params->GetLighting();
    if (lighting) {
        std::vector<double> coeffsD = params->GetLightingCoeffs();
        float               coeffsF[4] = {float(coeffsD[0]), float(coeffsD[1]), float(coeffsD[2]), float(coeffsD[3])};
        shader->SetUniformArray("lightingCoeffs", 4, coeffsF);
    }

    // Pack four booleans together, so there's one data transmission
    //   to the GPU, instead of four.
    int flags[4] = {int(fast), int(lighting), int(insideVolume), int(_userCoordinates.missingValueMask != nullptr)};
    shader->SetUniformArray("flags", 4, flags);

    // Calculate the step size with sample rate multiplier taken into account.
    float stepSize1D, multiplier = 1.0f;
    if (castingMode == FixedStep) switch (params->GetSampleRateMultiplier()) {
        case 0: multiplier = 1.0f; break;    // These values need to be in sync with
        case 1: multiplier = 2.0f; break;    //   the multiplier values in the GUI.
        case 2: multiplier = 4.0f; break;
        case 3: multiplier = 0.5f; break;
        case 4: multiplier = 0.25f; break;
        case 5: multiplier = 0.125f; break;
        default: multiplier = 1.0f; break;
        }
    glm::vec3 dimsf((float)cdims[0], (float)cdims[1], (float)cdims[2]);
    float     numCells = glm::length(dimsf);
    glm::mat4 modelview = _glManager->matrixManager->GetModelViewMatrix();
    glm::vec4 tmpVec4 = modelview * glm::vec4(gridMin, 1.0);
    glm::vec3 gridMinEye(tmpVec4.x, tmpVec4.y, tmpVec4.z);
    tmpVec4 = modelview * glm::vec4(gridMax, 1.0);
    glm::vec3 gridMaxEye(tmpVec4.x, tmpVec4.y, tmpVec4.z);
    glm::vec3 diagonal = gridMaxEye - gridMinEye;
    if (numCells < 50.0f)    // Make sure at least 100 steps
        stepSize1D = glm::length(diagonal) / 100.0f * multiplier;
    else    // Use Nyquist frequency
        stepSize1D = glm::length(diagonal) / (numCells * 2.0f) * multiplier;

    if (fast && castingMode == FixedStep) stepSize1D *= 8.0f;    //  Increase step size, when fast rendering
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
        glActiveTexture(GL_TEXTURE0 + _vertCoordsTexOffset);
        glBindTexture(GL_TEXTURE_3D, _vertCoordsTextureId);
        shader->SetUniform("vertCoordsTexture", _vertCoordsTexOffset);
    }
}

void RayCaster::_3rdPassSpecialHandling(bool fast, int castingMode) const
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
    std::memset(indexBuffer, 0, numOfVertices * sizeof(unsigned int));
    // Create buffer object to keep indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_DYNAMIC_DRAW);

    bool attrib1Enabled = false;    // Attribute to hold provoking index of each triangle.
    int *attrib1Buffer = nullptr;
    if (castingMode == CellTraversal && whichPass == 3) {
        attrib1Enabled = true;
        attrib1Buffer = new int[bx * by * 4];    // Buffer for front and back face
        std::memset(attrib1Buffer, 0, bx * by * 4 * sizeof(int));
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
        // Create buffer object to keep attributes
        glBufferData(GL_ARRAY_BUFFER, bx * by * 4 * sizeof(int), attrib1Buffer, GL_DYNAMIC_DRAW);
    }

    //
    // Render front face:
    //
    // Attribute 0 keeps all the vertex indices of the entire front face.
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * by * 3 * sizeof(float), _userCoordinates.frontFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    for (unsigned int y = 0; y < by - 1; y++)    // Looping over every TriangleStrip
    {
        idx = 0;
        //
        // This loop controls rendering order of the triangle strip. It
        // provides the indices for each vertex in the current strip.
        //
        for (unsigned int x = 0; x < bx; x++)    // Filling indices for vertices of this TriangleStrip
        {
            indexBuffer[idx++] = (y + 1) * bx + x;
            indexBuffer[idx++] = y * bx + x;
        }
        //
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
            // Update attribute 1
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
            glBufferSubData(GL_ARRAY_BUFFER, y * bx * 4 * sizeof(int), 2 * bx * 4 * sizeof(int), (attrib1Buffer + y * bx * 4));
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        // Update indices buffer
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numOfVertices * sizeof(unsigned int), indexBuffer);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render back face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * by * 3 * sizeof(float), _userCoordinates.backFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
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
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
            glBufferSubData(GL_ARRAY_BUFFER, y * bx * 4 * sizeof(int), 2 * bx * 4 * sizeof(int), (attrib1Buffer + y * bx * 4));
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numOfVertices * sizeof(unsigned int), indexBuffer);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    if (attrib1Enabled) {
        delete[] attrib1Buffer;
        attrib1Buffer = new int[bx * bz * 4];    // For top and bottom faces
        std::memset(attrib1Buffer, 0, bx * bz * 4 * sizeof(int));
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
        // Create a new buffer object with the new size.
        glBufferData(GL_ARRAY_BUFFER, bx * bz * 4 * sizeof(int), attrib1Buffer, GL_DYNAMIC_DRAW);
    }

    //
    // Render top face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * bz * 3 * sizeof(float), _userCoordinates.topFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
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
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
            glBufferSubData(GL_ARRAY_BUFFER, z * bx * 4 * sizeof(int), 2 * bx * 4 * sizeof(int), (attrib1Buffer + z * bx * 4));
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numOfVertices * sizeof(unsigned int), indexBuffer);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render bottom face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, bx * bz * 3 * sizeof(float), _userCoordinates.bottomFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
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
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
            glBufferSubData(GL_ARRAY_BUFFER, z * bx * 4 * sizeof(int), 2 * bx * 4 * sizeof(int), (attrib1Buffer + z * bx * 4));
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numOfVertices * sizeof(unsigned int), indexBuffer);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    // Each strip will have the same numOfVertices for the rest 2 faces.
    numOfVertices = by * 2;
    delete[] indexBuffer;
    indexBuffer = new unsigned int[numOfVertices];
    std::memset(indexBuffer, 0, numOfVertices * sizeof(unsigned int));
    // Re-create the index buffer object with the new size.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfVertices * sizeof(unsigned int), indexBuffer, GL_DYNAMIC_DRAW);
    if (attrib1Enabled) {
        delete[] attrib1Buffer;
        attrib1Buffer = new int[by * bz * 4];    // For right and left faces
        std::memset(attrib1Buffer, 0, by * bz * 4 * sizeof(int));
        glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
        glBufferData(GL_ARRAY_BUFFER, by * bz * 4 * sizeof(int), attrib1Buffer, GL_DYNAMIC_DRAW);
    }

    //
    // Render right face:
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, by * bz * 3 * sizeof(float), _userCoordinates.rightFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
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
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
            glBufferSubData(GL_ARRAY_BUFFER, z * by * 4 * sizeof(int), 2 * by * 4 * sizeof(int), (attrib1Buffer + z * by * 4));
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numOfVertices * sizeof(unsigned int), indexBuffer);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    //
    // Render left face
    //
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, by * bz * 3 * sizeof(float), _userCoordinates.leftFace, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
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
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexAttribId);
            glBufferSubData(GL_ARRAY_BUFFER, z * by * 4 * sizeof(int), 2 * by * 4 * sizeof(int), (attrib1Buffer + z * by * 4));
            glVertexAttribIPointer(1, 4, GL_INT, 0, (void *)0);
        }
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numOfVertices * sizeof(unsigned int), indexBuffer);
        glDrawElements(GL_TRIANGLE_STRIP, numOfVertices, GL_UNSIGNED_INT, (void *)0);
    }

    if (attrib1Enabled) delete[] attrib1Buffer;
    delete[] indexBuffer;
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#ifndef WIN32
double RayCaster::_getElapsedSeconds(const struct timeval *begin, const struct timeval *end) const { return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec) / 1000000.0); }
#endif

void RayCaster::_updateViewportWhenNecessary(const GLint *viewport)
{
    if ((std::memcmp(viewport, _currentViewport, 4 * sizeof(GLint)) != 0)) {
        std::memcpy(_currentViewport, viewport, 4 * sizeof(GLint));

        // Re-size 1st and 2nd pass rendering 2D textures
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
        _colorMapRange[0] = 0.0f;     // min value of the color map
        _colorMapRange[1] = 0.0f;     // max value of the color map
        _colorMapRange[2] = 1e-5f;    // diff of color map. Has to be non-zero though.
    } else {
        // Subclasses will have a chance here to use their own colormaps.
        _colormapSpecialHandling();
    }
}

void RayCaster::_colormapSpecialHandling()
{
    // Left empty intentionally.
    // Subclasses, e.g., IsoSurfaceRenderer and DVRenderer, feel free to implement it.
}

bool RayCaster::_use2ndVariable(const RayCasterParams *params) const
{
    // By default a ray caster does not use a secondary variable.
    // Subclasses can take advantage of it, for example, an IsoSurface Renderer.
    return false;
}

void RayCaster::_update2ndVarTextures()
{
    // Intentionally left empty
}

void RayCaster::_updateDataTextures()
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
    glBindTexture(GL_TEXTURE_3D, _missingValueTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    // Alignment adjustment. Stupid OpenGL thing.
    if (_userCoordinates.missingValueMask) {
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dims[0], dims[1], dims[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, _userCoordinates.missingValueMask);
    } else {
        unsigned char dummyMask[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, 2, 2, 2, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, dummyMask);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);    // Restore default alignment.
}

int RayCaster::_updateVertCoordsTexture(const glm::mat4 &MV)
{
    // Step zero: see if MV is the same as it was from the last iteration.
    //   If so, return directly without updating these coordinates.
    glm::bvec4 columeEqual = glm::equal(_currentMV, MV);
    if (glm::all(columeEqual)) return 0;

    // Now we need to calculate and upload the new vertex coordinates
    // First, transform every vertex coordinate to the eye space
    size_t numOfVertices = _userCoordinates.dims[0] * _userCoordinates.dims[1] * _userCoordinates.dims[2];
    float *coordEye = nullptr;
    try {
        coordEye = new float[3 * numOfVertices];
    } catch (const std::bad_alloc &e) {
        MyBase::SetErrMsg(e.what());
        return MEMERROR;
    }

    glm::vec4 posModel(1.0f);
    float *   posModelPtr = glm::value_ptr(posModel);
    for (size_t i = 0; i < numOfVertices; i++) {
        std::memcpy(posModelPtr, _userCoordinates.vertCoords + 3 * i, 3 * sizeof(float));
        glm::vec4 posEye = MV * posModel;
        std::memcpy(coordEye + 3 * i, glm::value_ptr(posEye), 3 * sizeof(float));
    }
    posModelPtr = nullptr;

    // Second, send these eye coordinates to the GPU
    glActiveTexture(GL_TEXTURE0 + _vertCoordsTexOffset);
    glBindTexture(GL_TEXTURE_3D, _vertCoordsTextureId);

#ifdef Darwin
    // Apply a dummy texture
    float dummyVolume[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 2, 2, 2, 0, GL_RED, GL_FLOAT, dummyVolume);
#endif

    // Test if the existing texture has the same dimensions.
    //   If so, simply substitute its content.
    //   If not, create a new object.
    int width, height, depth;
    glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
    if ((size_t)width == _userCoordinates.dims[0] && (size_t)height == _userCoordinates.dims[1] && (size_t)depth == _userCoordinates.dims[2]) {
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RGB, GL_FLOAT, coordEye);
    } else {
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, _userCoordinates.dims[0], _userCoordinates.dims[1], _userCoordinates.dims[2], 0, GL_RGB, GL_FLOAT, coordEye);
    }

    // Don't forget to update the cached model view matrix
    _currentMV = MV;

    delete[] coordEye;

    return 0;
}

int RayCaster::_selectDefaultCastingMethod() const
{
    RayCasterParams *params = dynamic_cast<RayCasterParams *>(GetActiveParams());
    if (!params) {
        MyBase::SetErrMsg("Error occured during retrieving RayCaster parameters!");
        return PARAMSERROR;
    }

    if (_isIntel) {
        params->SetCastingMode(FixedStep);
        return 0;
    }

    // If params already contain a value of mode 1 or 2, then do nothing.
    //   This case happens when loading params from a session file.
    int castingMode = int(params->GetCastingMode());
    if (castingMode == FixedStep || castingMode == CellTraversal) return 0;

    // castingMode == 0 if not initialized before. Let's figure out what value it should have.
    StructuredGrid *grid = nullptr;
    if (_userCoordinates.GetCurrentGrid(params, _dataMgr, &grid) != 0) {
        MyBase::SetErrMsg("Failed to retrieve a StructuredGrid");
        return GRIDERROR;
    }

    //
    // In case of a regular grid, use "fixed step" ray casting.
    // In other cases, use "cell traversal" ray casting.
    //
    RegularGrid *regular = dynamic_cast<RegularGrid *>(grid);
    if (regular)
        params->SetCastingMode(FixedStep);
    else
        params->SetCastingMode(CellTraversal);

    delete grid;

    return 0;
}

void RayCaster::_sleepAWhile() const
{
#ifdef WIN32
    glFinish();
    Sleep(1);    // 1 milliseconds
#else
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = 1000000L;    // 1 milliseconds
    glFinish();
    nanosleep(&req, &rem);
#endif
}
