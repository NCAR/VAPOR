#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/DirectVolumeRenderer.h>
#include <iostream>

//
// OpenGL debug output
//
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
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
    _baskFaceTextureId = 0;
}

DirectVolumeRenderer::UserCoordinates::UserCoordinates()
{
    frontFace = NULL;
    backFace = NULL;
    rightFace = NULL;
    leftFace = NULL;
    topFace = NULL;
    bottomFace = NULL;
    dims[0] = 0;
    dims[1] = 0;
    dims[2] = 0;
}

DirectVolumeRenderer::UserCoordinates::~UserCoordinates()
{
    if (frontFace) {
        delete[] frontFace;
        frontFace = NULL;
    }
    if (backFace) {
        delete[] backFace;
        backFace = NULL;
    }
    if (rightFace) {
        delete[] rightFace;
        rightFace = NULL;
    }
    if (leftFace) {
        delete[] leftFace;
        leftFace = NULL;
    }
    if (topFace) {
        delete[] topFace;
        topFace = NULL;
    }
    if (bottomFace) {
        delete[] bottomFace;
        bottomFace = NULL;
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
}

int DirectVolumeRenderer::_initializeGL()
{
    // Enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    int maxTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    std::cout << "    **** System Info ****" << std::endl;
    std::cout << "    OpenGL version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL version   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "    Vendor         : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "    Renderer       : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "    Number of texture units: " << maxTextureUnits << std::endl;
    std::cout << "    **** System Info ****" << std::endl;

    _initializeTextures();

    if (!_shaderMgr) {
        std::cerr << "Programmable shading not available" << std::endl;
        SetErrMsg("Programmable shading not available");
        return (-1);
    }

    if (!_shaderMgr->EffectExists(_effectNameStr)) {
        int rc = _shaderMgr->DefineEffect(_effectNameStr, "", _effectNameStr);
        if (rc < 0) {
            std::cerr << "DefineEffect() failed" << std::endl;
            SetErrMsg("DefineEffect() failed");
            return -1;
        }
    }

    /* good texture tutorial:
       https://open.gl/textures */
    // glGenTextures(1, &_colormapTextureUnit );
    // std::cout << "_colormapTextureUnit = " << _colormapTextureUnit << std::endl;

    // glActiveTexture( GL_TEXTURE1 );
    // glBindTexture(   GL_TEXTURE_1D, _colormapTextureUnit );
    ////glTexImage1D(    GL_TEXTURE_1D, 0, 4, _colormapSize, 0, GL_RGBA, GL_FLOAT, _colormap );
    // glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    return 0;
}

int DirectVolumeRenderer::_paintGL()
{
    std::cout << "_paintGL() called" << std::endl;

    if (_isCacheDirty()) _saveCacheParams(true);

    int rc = _shaderMgr->EnableEffect(_effectNameStr);
    if (rc < 0) {
        std::cerr << "EnableEffect() failed!" << std::endl;
        return (-1);
    }

    /*glColor3f(1.0f,1.0f,1.0f);
    glBegin(GL_LINES);
        glVertex3dv( _cacheParams.boxMin.data() );
        glVertex3dv( _cacheParams.boxMax.data() );
    glEnd();*/

    _drawVolumeFaces(_cacheParams.userCoords.frontFace, _cacheParams.userCoords.backFace, _cacheParams.userCoords.rightFace, _cacheParams.userCoords.leftFace, _cacheParams.userCoords.topFace,
                     _cacheParams.userCoords.bottomFace, _cacheParams.boxMin.data(), _cacheParams.boxMax.data(), _cacheParams.userCoords.dims, true);

    _shaderMgr->DisableEffect();

    return 0;
}

void DirectVolumeRenderer::_saveCacheParams(bool considerUserCoordinates)
{
    DVRParams *params = dynamic_cast<DVRParams *>(GetActiveParams());
    _cacheParams.varName = params->GetVariableName();
    _cacheParams.ts = params->GetCurrentTimestep();
    _cacheParams.level = params->GetRefinementLevel();
    _cacheParams.lod = params->GetCompressionLevel();
    params->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    MapperFunction *mapper = params->GetMapperFunc(_cacheParams.varName);
    _cacheParams.colormap.resize(mapper->getNumEntries() * 4, 0.0f);
    // colormap values aren't filled yet!

    if (considerUserCoordinates) {
        VAPoR::StructuredGrid *grid =
            dynamic_cast<VAPoR::StructuredGrid *>(_dataMgr->GetVariable(_cacheParams.ts, _cacheParams.varName, _cacheParams.level, _cacheParams.lod, _cacheParams.boxMin, _cacheParams.boxMax));
        if (grid != NULL) {
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

    vector<double> min, max;
    params->GetBox()->GetExtents(min, max);
    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

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
    /*GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport ); */

    /* Generate backfacing texture */
    /*glGenTextures(1, &_baskFaceTextureId);
    glBindTexture(GL_TEXTURE_2D, _baskFaceTextureId); */
    // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // from vapor2
    /*glTexImage2D(GL_TEXTURE_2D, 0, 4, viewport[2], viewport[3],
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _baskFaceTextureId, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "_openGLInitialization(): Framebuffer failed!!" << std::endl;
    }*/
}

void DirectVolumeRenderer::_drawVolumeFaces(const float *frontFace, const float *backFace, const float *rightFace, const float *leftFace, const float *topFace, const float *bottomFace,
                                            const double *volumeMin, const double *volumeMax, const size_t *dims, bool frontFacing)
{
    float        gridCoord[3];         // normalized grid coordinates, for drawing front-facing facets
    float        realWorldCoord[3];    // normalized real world coordinates
    size_t       bx = dims[0];
    size_t       by = dims[1];
    size_t       bz = dims[2];
    float        deltaX = 1.0f / (bx - 1);
    float        deltaY = 1.0f / (by - 1);
    float        deltaZ = 1.0f / (bz - 1);
    float        volumeSpanInverse[3] = {1.0f / (volumeMax[0] - volumeMin[0]), 1.0f / (volumeMax[1] - volumeMin[1]), 1.0f / (volumeMax[2] - volumeMin[2])};
    const float *ptr = NULL;

    // Render front face:
    /*
  gridCoord[2] = 1.0;
  for( int y = 0; y < by - 1; y++ )   // strip by strip
  {
    glBegin( GL_TRIANGLE_STRIP );
    for( int x = 0; x < bx; x++ )
    {
      ptr = frontFace + ((y + 1) * bx + x) * 3;
      */
    /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[0] =   x      * deltaX;
        gridCoord[1] =  (y + 1) * deltaY;
        glTexCoord3fv(  gridCoord );
      }*/
    // glVertex3fv(      ptr );

    // ptr = frontFace + (y * bx + x) * 3;
    /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[1] =  y * deltaY;
        glTexCoord3fv(  gridCoord );
      }*/
    /*
      glVertex3fv(      ptr );
    }
    glEnd();
  }
    */

    // Render back face:
    gridCoord[2] = 0.0;
    for (int y = 0; y < by - 1; y++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < bx; x++) {
            ptr = backFace + (y * bx + x) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[0] =  x * deltaX;
        gridCoord[1] =  y * deltaY;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);

            ptr = backFace + ((y + 1) * bx + x) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[1] =  (y + 1) * deltaY;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);
        }
        glEnd();
    }

    // Render right face:
    /*
  gridCoord[0] = 1.0;
  for( int z = 0; z < bz - 1; z++ )
  {
    glBegin( GL_TRIANGLE_STRIP );
    for( int y = 0; y < by; y++ )
    {
      ptr = rightFace + ((z + 1) * by + y) * 3;
*/
    /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[1] =   y      * deltaY;
        gridCoord[2] =  (z + 1) * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
    //     glVertex3fv(      ptr );

    //    ptr = rightFace + (z * by + y) * 3;
    /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[2] =  z * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
    /*
      glVertex3fv(      ptr );
    }
    glEnd();
  }
*/

    // Render left face:
    gridCoord[0] = 0.0;
    for (int z = 0; z < bz - 1; z++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int y = 0; y < by; y++) {
            ptr = leftFace + (z * by + y) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[1] =  y * deltaY;
        gridCoord[2] =  z * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);

            ptr = leftFace + ((z + 1) * by + y) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[2] =  (z + 1) * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);
        }
        glEnd();
    }

    // Render top face:
    gridCoord[1] = 1.0;
    for (int z = 0; z < bz - 1; z++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < bx; x++) {
            ptr = topFace + (z * bx + x) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[0] =  x * deltaX;
        gridCoord[2] =  z * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);

            ptr = topFace + ((z + 1) * bx + x) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[2] =  (z + 1) * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);
        }
        glEnd();
    }

    // Render bottom face:
    gridCoord[1] = 0.0;
    for (int z = 0; z < bz - 1; z++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < bx; x++) {
            ptr = bottomFace + ((z + 1) * bx + x) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[0]  =   x * deltaX;
        gridCoord[2]  =  (z + 1) * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);

            ptr = bottomFace + (z * bx + x) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[2]  =  z * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);
        }
        glEnd();
    }
}
