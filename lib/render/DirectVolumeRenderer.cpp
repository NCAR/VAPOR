#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/DirectVolumeRenderer.h>
#include <iostream>
#include <sstream>

//
// OpenGL debug output
//
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDebugMessageInsert.xhtml
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
    _volumeTextureUnit = 0;
    _colormapTextureUnit = 0;
    _volumeCoordinateTextureUnit = 0;
    _frameBufferId = 0;
    _backFaceTextureId = 0;
    _frontFaceTextureId = 0;
    _depthBufferId = 0;

    _vertexArrayId = 0;
    _1stPassShaderId = 0;
    _2ndPassShaderId = 0;
    _quadShaderId = 0;
}

DirectVolumeRenderer::UserCoordinates::UserCoordinates()
{
    frontFace = nullptr;
    backFace = nullptr;
    rightFace = nullptr;
    leftFace = nullptr;
    topFace = nullptr;
    bottomFace = nullptr;
    dims[0] = 0;
    dims[1] = 0;
    dims[2] = 0;
}

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
}

void DirectVolumeRenderer::UserCoordinates::Fill(const VAPoR::StructuredGrid *grid)
{
    std::vector<size_t> gridDims = grid->GetDimensions();
    dims[0] = gridDims[0];
    dims[1] = gridDims[1];
    dims[2] = gridDims[2];
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
}

// Destructor
DirectVolumeRenderer::~DirectVolumeRenderer()
{
    /* A few useful files from VAPOR2:
         DVRRayCaster.h/cpp
         DVRTexture3d.h/cpp
         TextureBrick.h/cpp
         DVRShader.h/cpp
    */
    if (_volumeTextureUnit) glDeleteTextures(1, &_volumeTextureUnit);
    if (_colormapTextureUnit) glDeleteTextures(1, &_colormapTextureUnit);
    if (_volumeCoordinateTextureUnit) glDeleteTextures(1, &_volumeCoordinateTextureUnit);

    if (_vertexArrayId) glDeleteVertexArrays(1, &_vertexArrayId);
    if (_1stPassShaderId) glDeleteProgram(_1stPassShaderId);
    if (_2ndPassShaderId) glDeleteProgram(_2ndPassShaderId);
    if (_quadShaderId) glDeleteProgram(_quadShaderId);

    // Need to look up what resources to destroy in OpenGL.
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

    const char vglQuad[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVRQuad.vgl";
    const char fglQuad[] = "/home/shaomeng/Git/VAPOR-new-DVR-src/share/shaders/main/DVRQuad.fgl";
    _quadShaderId = _loadShaders(vglQuad, fglQuad);

    /* good texture tutorial:
       https://open.gl/textures */

    // Create Vertex Array Object (VAO)
    glGenVertexArrays(1, &_vertexArrayId);
    glBindVertexArray(_vertexArrayId);

    _initializeTextures();

    _printGLInfo();

    return 0;
}

int DirectVolumeRenderer::_paintGL()
{
    if (_isCacheDirty()) _saveCacheParams(true);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    /* 1st pass, render back facing polygons to texture0 of the framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);
    glViewport(0, 0, viewport[2], viewport[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _drawVolumeFaces(_cacheParams.userCoords.frontFace, _cacheParams.userCoords.backFace, _cacheParams.userCoords.rightFace, _cacheParams.userCoords.leftFace, _cacheParams.userCoords.topFace,
                     _cacheParams.userCoords.bottomFace, _cacheParams.boxMin, _cacheParams.boxMax, _cacheParams.userCoords.dims,
                     1);    // The 1st pass!!!

    /* 2nd pass, render front facing polygons to texture1 of the framebuffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear the framebuffer
    _drawVolumeFaces(_cacheParams.userCoords.frontFace, _cacheParams.userCoords.backFace, _cacheParams.userCoords.rightFace, _cacheParams.userCoords.leftFace, _cacheParams.userCoords.topFace,
                     _cacheParams.userCoords.bottomFace, _cacheParams.boxMin, _cacheParams.boxMax, _cacheParams.userCoords.dims,
                     2);    // The 2nd pass!!!

    // put the framebuffer texture to a quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewport[2], viewport[3]);

    _drawQuad();

    return 0;
}

void DirectVolumeRenderer::_saveCacheParams(bool considerUserCoordinates)
{
    DVRParams *params = dynamic_cast<DVRParams *>(GetActiveParams());
    _cacheParams.varName = params->GetVariableName();
    _cacheParams.ts = params->GetCurrentTimestep();
    _cacheParams.level = params->GetRefinementLevel();
    _cacheParams.lod = params->GetCompressionLevel();
    std::vector<double> extMin, extMax;
    params->GetBox()->GetExtents(extMin, extMax);
    assert(extMin.size() == 3);
    assert(extMax.size() == 3);
    for (int i = 0; i < 3; i++) {
        _cacheParams.boxMin[i] = (float)extMin[i];
        _cacheParams.boxMax[i] = (float)extMax[i];
    }

    MapperFunction *mapper = params->GetMapperFunc(_cacheParams.varName);
    _cacheParams.colormap.resize(mapper->getNumEntries() * 4, 0.0f);
    // colormap values aren't filled yet!

    if (considerUserCoordinates) {
        VAPoR::StructuredGrid *grid = dynamic_cast<VAPoR::StructuredGrid *>(_dataMgr->GetVariable(_cacheParams.ts, _cacheParams.varName, _cacheParams.level, _cacheParams.lod, extMin, extMax));
        if (grid != nullptr) {
            _cacheParams.userCoords.Fill(grid);
        } else {
            std::cerr << "_saveCacheParams() grid isn't StructuredGrid" << std::endl;
        }

        delete grid;
    }
}

bool DirectVolumeRenderer::_isCacheDirty() const
{
    DVRParams *params = dynamic_cast<DVRParams *>(GetActiveParams());
    if (_cacheParams.varName != params->GetVariableName()) return true;
    if (_cacheParams.ts != params->GetCurrentTimestep()) return true;
    if (_cacheParams.level != params->GetRefinementLevel()) return true;
    if (_cacheParams.lod != params->GetCompressionLevel()) return true;

    vector<double> extMin, extMax;
    params->GetBox()->GetExtents(extMin, extMax);
    assert(extMin.size() == 3);
    assert(extMax.size() == 3);
    for (int i = 0; i < 3; i++) {
        if (_cacheParams.boxMin[i] != (float)extMin[i]) return true;
        if (_cacheParams.boxMax[i] != (float)extMax[i]) return true;
    }

    MapperFunction *mapper = params->GetMapperFunc(_cacheParams.varName);
    // colormap values aren't compared yet!!

    return false;
}

void DirectVolumeRenderer::_twoPassDVR()
{
    /* Now try Render to Texture                                                            */
    /* http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/ */
}

void DirectVolumeRenderer::_initializeTextures()
{
    /* Create an Frame Buffer Object for the back side of the volume.                       */
    glGenFramebuffers(1, &_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId);

    /* Get viewport dimensions */
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    /* Generate back-facing texture */
    glGenTextures(1, &_backFaceTextureId);
    glBindTexture(GL_TEXTURE_2D, _backFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, viewport[2], viewport[3], 0, GL_RGB, GL_FLOAT, nullptr);

    /* Configure the texture: _backFaceTextureId */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    /* Generate front-facing texture */
    glGenTextures(1, &_frontFaceTextureId);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, viewport[2], viewport[3], 0, GL_RGB, GL_FLOAT, nullptr);

    /* Configure the texture: _frontFaceTextureId */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    /* Depth buffer */
    glGenRenderbuffers(1, &_depthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewport[2], viewport[3]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBufferId);

    /* Set "_backFaceTextureId" as colour attachement #0, and "_frontFaceTextureId" as attachement #1 */
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _backFaceTextureId, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _frontFaceTextureId, 0);
    GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    /* Check if framebuffer is complete */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { std::cerr << "_openGLInitialization(): Framebuffer failed!!" << std::endl; }

    /* Bind the default frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DirectVolumeRenderer::_printGLInfo() const
{
    std::cout << "    **** System Info ****" << std::endl;
    std::cout << "    OpenGL version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL version   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "    Vendor         : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "    Renderer       : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "    **** System Info ****" << std::endl;
}

void DirectVolumeRenderer::_drawVolumeFaces(const float *frontFace, const float *backFace, const float *rightFace, const float *leftFace, const float *topFace, const float *bottomFace,
                                            const float *boxMin, const float *boxMax, const size_t *dims, int whichPass)
{
    assert(whichPass == 3 || whichPass == 2 || whichPass == 1);

    size_t       bx = dims[0];
    size_t       by = dims[1];
    size_t       bz = dims[2];
    const float *ptr = nullptr;

    size_t idx;
    size_t numOfVertices;

    /* Set up shader uniforms, OpenGL states, etc. */
    if (whichPass == 1)    // render back-facing polygons
    {
        glUseProgram(_1stPassShaderId);
        GLfloat MVP[16];
        _getMVPMatrix(MVP);
        GLuint MVPId = glGetUniformLocation(_1stPassShaderId, "MVP");
        glUniformMatrix4fv(MVPId, 1, GL_FALSE, MVP);
        GLuint boxMinId = glGetUniformLocation(_1stPassShaderId, "boxMin");
        glUniform3fv(boxMinId, 1, boxMin);
        GLuint boxMaxId = glGetUniformLocation(_1stPassShaderId, "boxMax");
        glUniform3fv(boxMaxId, 1, boxMax);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(0.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_GEQUAL);
    } else if (whichPass == 2)    // render front-facing polygons
    {
        glUseProgram(_2ndPassShaderId);
        GLfloat MVP[16];
        _getMVPMatrix(MVP);
        GLuint MVPId = glGetUniformLocation(_2ndPassShaderId, "MVP");
        glUniformMatrix4fv(MVPId, 1, GL_FALSE, MVP);
        GLuint boxMinId = glGetUniformLocation(_2ndPassShaderId, "boxMin");
        glUniform3fv(boxMinId, 1, boxMin);
        GLuint boxMaxId = glGetUniformLocation(_2ndPassShaderId, "boxMax");
        glUniform3fv(boxMaxId, 1, boxMax);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL);
    } else    // perform ray-casting
    {
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
    glClearDepth(1.0);
    glDisable(GL_DEPTH_TEST);

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

void DirectVolumeRenderer::_drawQuad()
{
    const GLfloat quadVertices[] = {_cacheParams.boxMin[0], _cacheParams.boxMax[1], _cacheParams.boxMin[2], _cacheParams.boxMin[0], _cacheParams.boxMin[1], _cacheParams.boxMin[2],
                                    _cacheParams.boxMax[0], _cacheParams.boxMax[1], _cacheParams.boxMin[2], _cacheParams.boxMax[0], _cacheParams.boxMin[1], _cacheParams.boxMin[2]};

    glUseProgram(_quadShaderId);

    GLfloat MVP[16];
    _getMVPMatrix(MVP);

    GLuint MVPId = glGetUniformLocation(_quadShaderId, "MVP");
    glUniformMatrix4fv(MVPId, 1, GL_FALSE, MVP);

    GLuint boxminId = glGetUniformLocation(_quadShaderId, "boxmin");
    glUniform3fv(boxminId, 1, _cacheParams.boxMin);

    GLuint boxmaxId = glGetUniformLocation(_quadShaderId, "boxmax");
    glUniform3fv(boxmaxId, 1, _cacheParams.boxMax);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _frontFaceTextureId);
    GLuint texId = glGetUniformLocation(_quadShaderId, "backFaceTexture");
    glUniform1i(texId, 0);

    glEnableVertexAttribArray(0);
    GLuint vertexBufferId = 0;
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * 4, quadVertices, GL_STREAM_DRAW);
    glVertexAttribPointer(0,    // need to match attribute 0 in the shader
                          3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDeleteBuffers(1, &vertexBufferId);
    glDisableVertexAttribArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}
