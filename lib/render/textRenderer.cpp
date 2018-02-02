//************************************************************************
//																	   *
//		   Copyright (C)  2004										 *
//	 University Corporation for Atmospheric Research				   *
//		   All Rights Reserved										 *
//																	   *
//************************************************************************/
//
//  File:		textRenderer.cpp
//
//  Author:	  Scott Pearse
//			   National Center for Atmospheric Research
//			   PO 3000, Boulder, Colorado
//
//  Date:		June 2014
//
//  Description: Implementation of text containers that render to
//			   QGLWidgets
//
#undef GLuint

//#include <vapor/glutil.h> // Must be included first!!!
//#include <gl/glew.h>
#include <cmath>
#include <array>
#include <iostream>
#include <FTGL/ftgl.h>

#include <vapor/glutil.h>
#ifdef Darwin
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
//#include <vapor/params.h>

//#include "glwindow.h"
#include <vapor/textRenderer.h>
//#include "viewpointparams.h"
//#include <vapor/MyBase.h>

using namespace VAPoR;
// struct GLWindow;		// just to retrieve window size

TextObject::TextObject()
{
    _pixmap = NULL;
    _vpParams = NULL;
    _orientationFlag = BOTTOMLEFT;
    _typeFlag = LABEL;
    /*_txtColor[0] = 1.f;
    _txtColor[1] = 1.f;
    _txtColor[2] = 1.f;
    _txtColor[3] = 1.f;
    _bgColor[0] = 0.f;
    _bgColor[1] = 0.f;
    _bgColor[2] = 0.f;
    _bgColor[3] = 1.f;*/
}

TextObject::~TextObject()
{
    if (_pixmap) {
        delete _pixmap;
        _pixmap = nullptr;
    }
    glDeleteTextures(1, &_fboTexture);
    glDeleteBuffers(1, &_fbo);
}

int TextObject::Initialize(string font, string text, int size, vector<double> txtColor, vector<double> bgColor, ViewpointParams *vpParams, TypeFlag type, OrientationFlag orientation)
{
    //	double txtColorArray[4] = {
    //		txtColor[0], txtColor[1], txtColor[2], txtColor[3]
    //	};
    //	double bgColorArray[4] = {
    //		bgColor[0], bgColor[1], bgColor[2], bgColor[3]
    //	};
    cout << "                                DOUBLE VEC CONSTRUCTOR" << endl;
    float txtColorArray[] = {(float)txtColor[0], (float)txtColor[1], (float)txtColor[2], (float)txtColor[3]};
    float bgColorArray[] = {(float)bgColor[0], (float)bgColor[1], (float)bgColor[2], (float)bgColor[3]};

    return Initialize(font, text, size, txtColorArray, bgColorArray, vpParams, type, orientation);
}

int TextObject::Initialize(string font, string text, int size, double txtColor[4], double bgColor[4], ViewpointParams *vpParams, TypeFlag type, OrientationFlag orientation)
{
    cout << "DOUBLE ARRAY CONSTRUCTOR" << endl;
    float txtColorArray[] = {(float)txtColor[0], (float)txtColor[1], (float)txtColor[2], (float)txtColor[3]};
    float bgColorArray[] = {(float)bgColor[0], (float)bgColor[1], (float)bgColor[2], (float)bgColor[3]};

    return Initialize(font, text, size, txtColorArray, bgColorArray, vpParams, type, orientation);
}

int TextObject::Initialize(string font, string text, int size, float txtColor[4], float bgColor[4], ViewpointParams *vpParams, TypeFlag type, OrientationFlag orientation)
{
    cout << "					txtColor " << txtColor[0] << " " << txtColor[1] << " " << txtColor[2] << " " << txtColor[3] << endl;
    cout << "					bgColor " << bgColor[0] << " " << bgColor[1] << " " << bgColor[2] << " " << bgColor[3] << endl;
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    _pixmap = new FTPixmapFont(font.c_str());
    _text = text;
    _size = size;
    _typeFlag = type;
    _orientationFlag = orientation;
    _font = font;
    _coords[0] = 0.;
    _coords[1] = 0.;
    _coords[2] = 0.;
    _bgColor[0] = 0.f;    // bgColor[0];
    _bgColor[1] = 1.f;    // bgColor[1];
    _bgColor[2] = 0.f;    // bgColor[2];
    _bgColor[3] = 1.f;    // bgColor[3];

    _txtColor[0] = 1.f;    // txtColor[0];
    _txtColor[1] = 0.f;    // txtColor[1];
    _txtColor[2] = 0.f;    // txtColor[2];
    _txtColor[3] = 1.f;    // txtColor[3];
    //_bgColor[0]   = (float)bgColor[0];
    //_bgColor[1]   = (float)bgColor[1];
    //_bgColor[2]   = (float)bgColor[2];
    _bgColor[3] = .9999f;    //(float)bgColor[3];
    //_txtColor[0]  = (float)txtColor[0];
    //_txtColor[1]  = (float)txtColor[1];
    //_txtColor[2]  = (float)txtColor[2];
    _txtColor[3] = .9999f;    //(float)txtColor[3];
    _fbo = 0;
    _fboTexture = 0;
    _vpParams = vpParams;

#ifdef NEEDSTRETCH
    if (inType == 1) {
        // Convert user to local/stretched coordinates in cube
        DataStatus *          ds = DataStatus::getInstance();
        const vector<double> &fullUsrExts = ds->getDataMgr()->GetExtents();
        double                sceneScaleFactor = 1. / ViewpointParams::getMaxStretchedCubeSide();
        const double *        scales = ds->getStretchFactors();
        for (int i = 0; i < 3; i++) {
            _3Dcoords[i] = _coords[i] - fullUsrExts[i];
            _3Dcoords[i] *= sceneScaleFactor;
            _3Dcoords[i] *= scales[i];
        }
    }
#endif

    _pixmap->FaceSize(_size);
    findBBoxSize();
    initFrameBufferTexture();
    initFrameBuffer();
    glPopAttrib();
    return 0;
}

void TextObject::initFrameBufferTexture(void)
{
    glGenTextures(1, &_fboTexture);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, _fboTexture);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);    // will not correct blending, but will be OK wrt other opaque geometry.

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Do write to the z buffer
    glDepthMask(GL_TRUE);

    processErrors("TextObject::initFrameBufferTexture()");
}

int TextObject::initFrameBuffer(void)
{
    glGenFramebuffersEXT(1, &_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _fboTexture, 0);

    // Check that status of our generated frame buffer
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        std::cout << "Couldn't create frame buffer" << std::endl;
        return -1;
    }

    // Save previous color state
    // Then render the background and text to our texture
    // Then finally reset colors to their previous state
    double txColors[4];
    double bgColors[4];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, bgColors);
    glGetDoublev(GL_CURRENT_COLOR, txColors);

    //	glColor4f(_txtColor[0],_txtColor[1],_txtColor[2],_txtColor[3]);
    //	glClearColor(_bgColor[0],_bgColor[1],_bgColor[2],_bgColor[3]);
    glClearColor(0.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1.f, 1.f, 0.f, 1.f);
    //	glGetDoublev(GL_COLOR_CLEAR_VALUE,bgColors);
    //	glGetDoublev(GL_CURRENT_COLOR,txColors);
    cout << "bg: " << _bgColor[0] << " " << _bgColor[1] << " " << _bgColor[2] << " " << _bgColor[3] << endl;
    //	cout << "bg: " << bgColors[0] << " " << bgColors[1] << " " << bgColors[2] << " " << bgColors[3] << endl << endl;
    cout << "tx: " << _txtColor[0] << " " << _txtColor[1] << " " << _txtColor[2] << " " << _txtColor[3] << endl;
    //	cout << "tx: " << txColors[0] << " " << txColors[1] << " " << txColors[2] << " " << txColors[3] << endl;

    // glColor4f(_txtColor[0],_txtColor[1],_txtColor[2],_txtColor[3]);
    //	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glWindowPos2f(0, 0);
    int xRenderOffset = _size / 30 + 1;                                                       //_size/24 + 1;
    int yRenderOffset = (int)(-_pixmap->BBox(_text.c_str()).Lower().Y() + _size / 16 + 1);    //+4
    if (_size <= 40) {
        xRenderOffset += 1;
        yRenderOffset += 1;
    } else
        xRenderOffset -= 1;
    FTPoint point;
    point.X(xRenderOffset);
    point.Y(yRenderOffset);
    _pixmap->Render(_text.c_str(), -1, point);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    GLenum glErr;
    glErr = glGetError();
    char *errString;
    if (glErr != GL_NO_ERROR) {
        while (glErr != GL_NO_ERROR) {
            errString = (char *)gluErrorString(glErr);
            Wasp::MyBase::SetErrMsg(errString);
            glErr = glGetError();
        }
    }

    //	double txtColor[4];
    //	double bgColor[4];
    //	glGetDoublev(GL_COLOR_CLEAR_VALUE,bgColor);
    //	glGetDoublev(GL_CURRENT_COLOR,txtColor);
    //	cout << "ag: " << bgColor[0] << " " << bgColor[1] << " " << bgColor[2] << " " << bgColor[3] << endl;
    //	cout << "ax: " << txtColor[0] << " " << txtColor[1] << " " << txtColor[2] << " " << txtColor[3] << endl;
    //	cout << endl;

    //	glColor4f(txColors[0],txColors[1],txColors[2],txColors[3]);
    //	glClearColor(bgColors[0],bgColors[1],bgColors[2],bgColors[3]);
    //	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    processErrors("TextObject::initFrameBuffer()");

    return 0;
}

void TextObject::findBBoxSize()
{
    FTBBox  box = _pixmap->BBox(_text.c_str());
    FTPoint up = box.Upper();
    FTPoint lo = box.Lower();

    int xPadding = _size / 12 + 4;
    int yPadding = _size / 12 + 4;

    _height = (int)(up.Y() - lo.Y() + yPadding);
    _width = (int)(up.X() - lo.X() + xPadding);
}

int TextObject::applyViewerMatrix(double coords[3])
{
    if ((_typeFlag == LABEL) || (_typeFlag == BILLBOARD)) {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_TEXTURE_2D);
    }
#ifdef NEEDSTRETCH
    if (_type == 1) {
        double newCoords[3] = {0.f, 0.f, 0.f};

        GLWindow *castWin;
        castWin = dynamic_cast<GLWindow *>(_myWindow);
        castWin->projectPointToWin(coords, newCoords);
        coords[0] = newCoords[0];
        coords[1] = newCoords[1];
    }
    if (_type == 3) {
        GLWindow *castWin;
        castWin = dynamic_cast<GLWindow *>(_myWindow);
        ViewpointParams *myViewParams = castWin->getActiveViewpointParams();
        double *         viewDir;
        viewDir = myViewParams->getViewDir();
        // glPushMatrix();
        // glRotatef(-90.0,1.0,0.0,0.0);
    }
#endif
    glEnable(GL_TEXTURE_2D);
    return 1;
}

void TextObject::removeViewerMatrix()
{
    glEnable(GL_BLEND);
    if ((_typeFlag == LABEL) || (_typeFlag == BILLBOARD)) {
        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

void TextObject::applyTransform(Transform *t)
{
    vector<double> scale = t->GetScales();
    vector<double> origin = t->GetOrigin();
    vector<double> translate = t->GetTranslations();
    vector<double> rotate = t->GetRotations();
    assert(translate.size() == 3);
    assert(rotate.size() == 3);
    assert(scale.size() == 3);
    assert(origin.size() == 3);

    glTranslatef(origin[0], origin[1], origin[2]);
    glScalef(scale[0], scale[1], scale[2]);
    glRotatef(rotate[0], 1, 0, 0);
    glRotatef(rotate[1], 0, 1, 0);
    glRotatef(rotate[2], 0, 0, 1);
    glTranslatef(-origin[0], -origin[1], -origin[2]);

    glTranslatef(translate[0], translate[1], translate[2]);
    processErrors("TextObject::applyTransform()");
}

void TextObject::processErrors(string functionName)
{
    GLenum glErr;
    glErr = glGetError();
    string      cppErrString;
    const char *errString;
    if (glErr != GL_NO_ERROR) {
        while (glErr != GL_NO_ERROR) {
            cppErrString.append((char *)(gluErrorString(glErr)));
            cppErrString += " " + functionName;
            errString = cppErrString.c_str();
            Wasp::MyBase::SetErrMsg(errString);
            glErr = glGetError();
            cout << " ERROR " << errString << endl;
        }
    }
}

int TextObject::drawMe(double coords[3])
{
    _coords[0] = coords[0];
    _coords[1] = coords[1];
    _coords[2] = coords[2];

    double bgColors[4];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, bgColors);
    cout << "drawME " << bgColors[0] << " " << bgColors[1] << " " << bgColors[2] << " " << bgColors[3] << endl;

    applyViewerMatrix();

    glBindTexture(GL_TEXTURE_2D, _fboTexture);

    if ((_typeFlag & LABEL) || (_typeFlag & BILLBOARD)) drawOnScreen();

    glBindTexture(GL_TEXTURE_2D, 0);

    removeViewerMatrix();

    processErrors("TextObject::drawMe()");

    return 0;
}

void TextObject::applyOffset(double &llx, double &lly, double &urx, double &ury)
{
    double width = urx - llx;
    double height = ury - lly;
    double xOffset = 0.;
    double yOffset = 0.;

    if (_orientationFlag & BOTTOMLEFT)
        return;
    else if (_orientationFlag & CENTERLEFT)
        yOffset -= height / 2.;
    else if (_orientationFlag & TOPLEFT)
        yOffset -= height;
    else if (_orientationFlag & CENTERTOP) {
        xOffset -= width / 2.;
        yOffset -= height;
    } else if (_orientationFlag & TOPRIGHT) {
        xOffset -= width;
        yOffset -= height;
    } else if (_orientationFlag & CENTERRIGHT) {
        xOffset -= width;
        yOffset -= height / 2.;
    } else if (_orientationFlag & BOTTOMRIGHT)
        xOffset -= width;
    else if (_orientationFlag & CENTERBOTTOM)
        xOffset -= width / 2.;
    else if (_orientationFlag & DEADCENTER) {
        xOffset -= width / 2.;
        yOffset -= height / 2.;
    }

    llx += xOffset;
    urx += xOffset;
    lly += yOffset;
    ury += yOffset;
}

bool TextObject::projectPointToWin(double cubeCoords[3])
{
    double   depth;
    GLdouble wCoords[2];
    GLdouble cbCoords[3];
    for (int i = 0; i < 3; i++) cbCoords[i] = (double)cubeCoords[i];

    double mvMatrix[16];
    double pMatrix[16];
    _vpParams->GetModelViewMatrix(mvMatrix);
    _vpParams->GetProjectionMatrix(pMatrix);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    bool success = (0 != gluProject(cbCoords[0], cbCoords[1], cbCoords[2], mvMatrix, pMatrix, viewport, wCoords, (wCoords + 1), (GLdouble *)(&depth)));
    if (!success) return false;
    cubeCoords[0] = wCoords[0];
    cubeCoords[1] = wCoords[1];
    return (depth > 0.0);
}

void TextObject::drawLabel()
{
    GLint dims[4];
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint fbWidth = dims[2];
    GLint fbHeight = dims[3];

    double fltTxtWidth = (double)_width / (double)fbWidth;
    double fltTxtHeight = (double)_height / (double)fbHeight;

    double llx = 2. * _coords[0] / (double)fbWidth - 1.f;
    double lly = 2. * _coords[1] / (double)fbHeight - 1.f;
    double urx = llx + 2. * fltTxtWidth;
    double ury = lly + 2. * fltTxtHeight;

    applyOffset(llx, lly, urx, ury);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(llx, lly, .0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(llx, ury, .0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(urx, ury, .0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(urx, lly, .0f);
    glEnd();
}

void TextObject::drawOnScreen()
{
    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint fbWidth = dims[2];
    GLint fbHeight = dims[3];

    double txtWidth = (double)_width / (double)fbWidth;
    double txtHeight = (double)_height / (double)fbHeight;

    if (_typeFlag & BILLBOARD) projectPointToWin(_coords);

    // double llx = 2.*newCoords[0]/(double)fbWidth - 1.f;
    // double lly = 2.*newCoords[1]/(double)fbHeight - 1.f;
    double llx = 2. * _coords[0] / (double)fbWidth - 1.f;
    double lly = 2. * _coords[1] / (double)fbHeight - 1.f;
    double urx = llx + 2. * txtWidth;
    double ury = lly + 2. * txtHeight;

    applyOffset(llx, lly, urx, ury);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(llx, lly, .0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(llx, ury, .0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(urx, ury, .0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(urx, lly, .0f);
    glEnd();
}
