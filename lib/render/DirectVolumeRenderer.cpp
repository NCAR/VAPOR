#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/DirectVolumeRenderer.h>
#include <iostream>

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
}

DirectVolumeRenderer::UserCoordinates::UserCoordinates()
{
    frontFace = NULL;
    backFace = NULL;
    rightFace = NULL;
    leftFace = NULL;
    topFace = NULL;
    bottomFace = NULL;
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
    int maxTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    std::cout << "    **** System Info ****" << std::endl;
    std::cout << "    OpenGL version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL version   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "    Vendor         : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "    Renderer       : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "    Max number of texture units: " << maxTextureUnits << std::endl;
    std::cout << "    **** System Info ****" << std::endl;

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
    glGenTextures(1, &_colormapTextureUnit);
    std::cout << "_colormapTextureUnit = " << _colormapTextureUnit << std::endl;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _colormapTextureUnit);
    // glTexImage1D(    GL_TEXTURE_1D, 0, 4, _colormapSize, 0, GL_RGBA, GL_FLOAT, _colormap );
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    return 0;
}

int DirectVolumeRenderer::_paintGL()
{
    std::cout << "_paintGL() called" << std::endl;

    if (_isCacheDirty()) _saveCacheParams();

    int rc = _shaderMgr->EnableEffect(_effectNameStr);
    if (rc < 0) {
        std::cerr << "EnableEffect() failed!" << std::endl;
        return (-1);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3dv(_cacheParams.boxMin.data());
    glVertex3dv(_cacheParams.boxMax.data());
    glEnd();

    _shaderMgr->DisableEffect();

    return 0;
}

void DirectVolumeRenderer::_saveCacheParams()
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

void DirectVolumeRenderer::_drawVolumeFaces(const float *frontFace, const float *backFace, const float *rightFace, const float *leftFace, const float *topFace, const float *bottomFace,
                                            const float *volumeMin, const float *volumeMax, int bx, int by, int bz, bool frontFacing)
{
    float        gridCoord[3];         // normalized grid coordinates, for drawing front-facing facets
    float        realWorldCoord[3];    // normalized real world coordinates
    float        deltaX = 1.0f / (bx - 1);
    float        deltaY = 1.0f / (by - 1);
    float        deltaZ = 1.0f / (bz - 1);
    float        volumeSpanInverse[3] = {1.0f / (volumeMax[0] - volumeMin[0]), 1.0f / (volumeMax[1] - volumeMin[1]), 1.0f / (volumeMax[2] - volumeMin[2])};
    const float *ptr = NULL;

    // Render front face:
    gridCoord[2] = 1.0;
    for (int y = 0; y < by - 1; y++)    // strip by strip
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < bx; x++) {
            ptr = frontFace + ((y + 1) * bx + x) * 3;
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
            glVertex3fv(ptr);

            ptr = frontFace + (y * bx + x) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[1] =  y * deltaY;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);
        }
        glEnd();
    }

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
    gridCoord[0] = 1.0;
    for (int z = 0; z < bz - 1; z++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int y = 0; y < by; y++) {
            ptr = rightFace + ((z + 1) * by + y) * 3;
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
            glVertex3fv(ptr);

            ptr = rightFace + (z * by + y) * 3;
            /*realWorldCoord[0] = (*ptr        - volumeMin[0]) * volumeSpanInverse[0];
      realWorldCoord[1] = (*(ptr + 1)  - volumeMin[1]) * volumeSpanInverse[1];
      realWorldCoord[2] = (*(ptr + 2)  - volumeMin[2]) * volumeSpanInverse[2];
      glColor3fv(       realWorldCoord );
      if( frontFacing )
      {
        gridCoord[2] =  z * deltaZ;
        glTexCoord3fv(  gridCoord );
      }*/
            glVertex3fv(ptr);
        }
        glEnd();
    }

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
