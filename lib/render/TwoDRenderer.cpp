//---------------------------------------------------------------------------
//
//                   Copyright (C)  2011
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------

#include <vapor/glutil.h> // Must be included first!!!
#include <cstdlib>
#include <cstdio>

#include <vapor/ViewpointParams.h>
#include <vapor/MyBase.h>
#include <vapor/TwoDRenderer.h>

using namespace VAPoR;
using namespace Wasp;

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TwoDRenderer::TwoDRenderer(
    const ParamsMgr *pm, string winName, string dataSetName, string paramsType,
    string classType, string instName, DataMgr *dataMgr) : Renderer(pm, winName, dataSetName, paramsType, classType, instName, dataMgr) {
    _textureID = 0;
    _texture = NULL;
    _texCoords = NULL;
    _texWidth = 0;
    _texHeight = 0;
    _texInternalFormat = 0;
    _texFormat = GL_RGBA;
    _texType = GL_UNSIGNED_BYTE;
    _texelSize = 0;
    _verts = NULL;
    _normals = NULL;
    _meshWidth = 0;
    _meshHeight = 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TwoDRenderer::~TwoDRenderer() {
    if (_textureID)
        glDeleteTextures(1, &_textureID);
}

int TwoDRenderer::_initializeGL() {
    glGenTextures(1, &_textureID);
    return (0);
}

int TwoDRenderer::_paintGL() {

    // Get the 2D texture
    //
    _texture = _GetTexture(
        _dataMgr, _texWidth, _texHeight, _texInternalFormat,
        _texFormat, _texType, _texelSize);
    if (!_texture) {
        return (-1);
    }
    assert(_texWidth >= 2);
    assert(_texHeight >= 2);

    // Get the proxy geometry used to render the 2D surface (vertices and
    // normals)
    //
    int rc = _GetMesh(_dataMgr, &_verts, &_normals, _meshWidth, _meshHeight);
    if (rc < 0) {
        return (-1);
    }
    assert(_meshWidth >= 2);
    assert(_meshHeight >= 2);

    _texCoords = (GLfloat *)_sb_texCoords.Alloc(
        _meshWidth * _meshHeight * 2 * sizeof(*_texCoords));

    _computeTexCoords(_texCoords, _meshWidth, _meshHeight);

    // Render the 2D surface
    //
    _renderMesh();

    return (0);
}

// Setup OpenGL state for rendering
//
void TwoDRenderer::_openGLInit() {

    RenderParams *myParams = (RenderParams *)GetActiveParams();
    float opacity = myParams->GetConstantOpacity();

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _textureID);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D, 0, _texInternalFormat, _texWidth, _texHeight, 0,
        _texFormat, _texType, _texture);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);

    // LIGHTING IS NOT ENABLED
    //
    int nLights = 0;
    if (nLights > 0) {
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        //		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, elevGridColor);
    } else {
        glDisable(GL_LIGHTING);
        //		glColor3fv(elevGridColor);
    }

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_COLOR_MATERIAL);
    glColor4f(1.0, 1.0, 1.0, opacity);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // will not correct blending, but will be OK wrt other opaque geometry.
    //
    glAlphaFunc(GL_GREATER, .1);

    // Do write to the z buffer
    //
    glDepthMask(GL_TRUE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
}

// Restore OpenGL settings to OpenGL defaults
//
void TwoDRenderer::_openGLRestore() {

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void TwoDRenderer::_renderMesh() {

    _openGLInit();

    GLuint *indeces;

    indeces = new GLuint[_meshWidth * 2];

    // Construct indeces for a triangle strip covering one row
    // of the mesh
    //
    for (int i = 0; i < _meshWidth; i++)
        indeces[2 * i] = i;
    for (int i = 0; i < _meshWidth; i++)
        indeces[2 * i + 1] = i + _meshWidth;

    // Draw triangle strips one row at a time
    //
    for (int j = 0; j < _meshHeight - 1; j++) {
        glVertexPointer(3, GL_FLOAT, 0, &_verts[j * _meshWidth * 3]);
        glTexCoordPointer(2, GL_FLOAT, 0, &_texCoords[j * _meshWidth * 2]);
        glNormalPointer(GL_FLOAT, 0, &_normals[j * _meshWidth * 3]);

        glDrawElements(
            GL_TRIANGLE_STRIP, 2 * _meshWidth, GL_UNSIGNED_INT, indeces);
    }

    delete[] indeces;

    _openGLRestore();
}

void TwoDRenderer::_ComputeNormals(
    const GLfloat *verts,
    GLsizei w, GLsizei h,
    GLfloat *normals) {
    // Go over the grid of vertices, calculating normals
    // by looking at adjacent x,y,z coords.
    //
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            const GLfloat *point = verts + 3 * (i + w * j);
            GLfloat *norm = normals + 3 * (i + w * j);
            //do differences of right point vs left point,
            //except at edges of grid just do differences
            //between current point and adjacent point:
            float dx = 0.f, dy = 0.f, dzx = 0.f, dzy = 0.f;
            if (i > 0 && i < w - 1) {
                dx = *(point + 3) - *(point - 3);
                dzx = *(point + 5) - *(point - 1);
            } else if (i == 0) {
                dx = *(point + 3) - *(point);
                dzx = *(point + 5) - *(point + 2);
            } else if (i == w - 1) {
                dx = *(point) - *(point - 3);
                dzx = *(point + 2) - *(point - 1);
            }
            if (j > 0 && j < h - 1) {
                dy = *(point + 1 + 3 * w) - *(point + 1 - 3 * w);
                dzy = *(point + 2 + 3 * w) - *(point + 2 - 3 * w);
            } else if (j == 0) {
                dy = *(point + 1 + 3 * w) - *(point + 1);
                dzy = *(point + 2 + 3 * w) - *(point + 2);
            } else if (j == h - 1) {
                dy = *(point + 1) - *(point + 1 - 3 * w);
                dzy = *(point + 2) - *(point + 2 - 3 * w);
            }
            norm[0] = dy * dzx;
            norm[1] = dx * dzy;
            norm[2] = 1.0;

            //vnormal(norm);
        }
    }
}

void TwoDRenderer::_computeTexCoords(
    GLfloat *tcoords,
    size_t w,
    size_t h) const {
    assert(_meshWidth >= 2);
    assert(_meshHeight >= 2);

    double deltax = 1.0 / (_meshWidth - 1);
    double deltay = 1.0 / (_meshHeight - 1);

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            tcoords[(2 * j * w) + (2 * i + 0)] = i * deltax;
            tcoords[(2 * j * w) + (2 * i + 1)] = j * deltay;
        }
    }
}
