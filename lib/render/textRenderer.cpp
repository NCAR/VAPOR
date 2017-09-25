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
#include <iostream>
#include <FTGL/ftgl.h>
//#include <vapor/params.h>

//#include "glwindow.h"
#include <vapor/textRenderer.h>
//#include "viewpointparams.h"
//#include <vapor/MyBase.h>

using namespace VAPoR;
//struct GLWindow;		// just to retrieve window size

TextContainer::TextContainer(string fontFile,
                             int fontSize,
                             float txtColor[4],
                             float bgColor[4],
                             int type) {
    //						QGLWidget *myGivenWindow) {

    _font = fontFile;
    _size = fontSize;
    _type = type;
    _txtColor[0] = txtColor[0];
    _txtColor[1] = txtColor[1];
    _txtColor[2] = txtColor[2];
    _txtColor[3] = txtColor[3];
    _bgColor[0] = bgColor[0];
    _bgColor[1] = bgColor[1];
    _bgColor[2] = bgColor[2];
    _bgColor[3] = bgColor[3];
    //	_myWindow	= myGivenWindow;
}

int TextContainer::addText(string text, float coords[3]) {
    TextObject *to = new TextObject();
    to->Initialize(_font, text, _size, coords, _type,
                   _txtColor, _bgColor); //,_myWindow);
    TextObjects.push_back(to);

    return 0;
}

void TextContainer::drawText() {
    for (size_t i = 0; i < TextObjects.size(); i++) {
        TextObjects[i]->drawMe();
    }
}

TextContainer::~TextContainer() {
    for (size_t i = 0; i < TextObjects.size(); i++) {
        delete TextObjects[i];
    }
}

TextObject::TextObject() {
}

int TextObject::Initialize(string inFont,
                           string inText,
                           int inSize,
                           float inCoords[3],
                           int inType,
                           float txtColor[4],
                           float bgColor[4]) {
    //						QGLWidget *myGivenWindow = NULL) {

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    _pixmap = new FTPixmapFont(inFont.c_str());
    _text = inText;
    _size = inSize;
    _type = inType;
    _font = inFont;
    _coords[0] = inCoords[0];
    _coords[1] = inCoords[1];
    _coords[2] = inCoords[2];
    _bgColor[0] = bgColor[0];
    _bgColor[1] = bgColor[1];
    _bgColor[2] = bgColor[2];
    _bgColor[3] = bgColor[3];
    _txtColor[0] = txtColor[0];
    _txtColor[1] = txtColor[1];
    _txtColor[2] = txtColor[2];
    _txtColor[3] = txtColor[3];
    //	_myWindow	 = myGivenWindow;
    _fbo = 0;
    _fboTexture = 0;

    if (inType == 1) {
#ifdef NEEDSTRETCH
        //Convert user to local/stretched coordinates in cube
        DataStatus *ds = DataStatus::getInstance();
        const vector<double> &fullUsrExts = ds->getDataMgr()->GetExtents();
        float sceneScaleFactor = 1. / ViewpointParams::getMaxStretchedCubeSide();
        const float *scales = ds->getStretchFactors();
        for (int i = 0; i < 3; i++) {
            _3Dcoords[i] = _coords[i] - fullUsrExts[i];
            _3Dcoords[i] *= sceneScaleFactor;
            _3Dcoords[i] *= scales[i];
        }
#endif
    }

    _pixmap->FaceSize(_size);
    //	_myWindow->makeCurrent();
    findBBoxSize();
    initFrameBufferTexture();
    initFrameBuffer();
    glPopAttrib();
    return 0;
}

TextObject::~TextObject() {
    delete _pixmap;
    glDeleteTextures(1, &_fboTexture);
    glDeleteBuffers(1, &_fbo);
}

void TextObject::initFrameBufferTexture(void) {
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
    glEnable(GL_DEPTH_TEST); // will not correct blending, but will be OK wrt other opaque geometry.

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    //Do write to the z buffer
    glDepthMask(GL_TRUE);

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
}

int TextObject::initFrameBuffer(void) {
    glGenFramebuffersEXT(1, &_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D, _fboTexture, 0);

    // Check that status of our generated frame buffer
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        std::cout << "Couldn't create frame buffer" << std::endl;
        return -1;
    }

    // Save previous color state
    // Then render the background and text to our texture
    // Then finally reset colors to their previous state
    float txColors[4];
    float bgColors[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, bgColors);
    glGetFloatv(GL_CURRENT_COLOR, txColors);
    glClearColor(_bgColor[0], _bgColor[1], _bgColor[2], _bgColor[3]);
    glColor4f(_txtColor[0], _txtColor[1], _txtColor[2], _txtColor[3]);
    glWindowPos2f(0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int xRenderOffset = _size / 30 + 1;                                                    //_size/24 + 1;
    int yRenderOffset = (int)(-_pixmap->BBox(_text.c_str()).Lower().Y() + _size / 16 + 1); //+4
    if (_size <= 40) {
        xRenderOffset += 1;
        yRenderOffset += 1;
    } else
        xRenderOffset -= 1;
    FTPoint point;
    point.X(xRenderOffset);
    point.Y(yRenderOffset);
    _pixmap->Render(_text.c_str(), -1, point);
    glColor4f(txColors[0], txColors[1], txColors[2], txColors[3]);
    glClearColor(bgColors[0], bgColors[1], bgColors[2], bgColors[3]);

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
    return 0;
}

void TextObject::findBBoxSize() {
    FTBBox box = _pixmap->BBox(_text.c_str());
    FTPoint up = box.Upper();
    FTPoint lo = box.Lower();

    int xPadding = _size / 12 + 4;
    int yPadding = _size / 12 + 4;

    _height = (int)(up.Y() - lo.Y() + yPadding);
    _width = (int)(up.X() - lo.X() + xPadding);
}

int TextObject::applyViewerMatrix(float coords[3]) {
    if ((_type == 0) || (_type == 1)) {
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
        float newCoords[3] = {0.f, 0.f, 0.f};

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
        float *viewDir;
        viewDir = myViewParams->getViewDir();
        //glPushMatrix();
        //glRotatef(-90.0,1.0,0.0,0.0);
    }
#endif
    glEnable(GL_TEXTURE_2D);
    return 1;
}

void TextObject::removeViewerMatrix() {
    glEnable(GL_BLEND);
    if ((_type == 0) || (_type == 1)) {
        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

int TextObject::drawMe(float coords[3]) {
    _coords[0] = coords[0];
    _coords[1] = coords[1];
    _coords[2] = coords[2];
    return drawMe();
}

int TextObject::drawMe(float coords[3], int timestep) {

#ifdef NEEDSTRETCH
    //Convert user to local coordinates
    DataStatus *ds = DataStatus::getInstance();
    const vector<double> &fullUsrExts = ds->getDataMgr()->GetExtents((size_t)timestep);
    float sceneScaleFactor = 1. / ViewpointParams::getMaxStretchedCubeSide();
    const float *scales = ds->getStretchFactors();
    for (int i = 0; i < 3; i++) {
        coords[i] = coords[i] - fullUsrExts[i];
        coords[i] *= sceneScaleFactor;
        coords[i] *= scales[i];
    }
#endif

    applyViewerMatrix(coords);
    //else applyViewerMatrix();

    glBindTexture(GL_TEXTURE_2D, _fboTexture);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#ifdef NEEDSTRETCH
    float fltTxtWidth, fltTxtHeight, llx, lly, urx, ury;
    if (_type == 1) {
        fltTxtWidth = (float)_width / (float)_myWindow->width();
        fltTxtHeight = (float)_height / (float)_myWindow->height();
        llx = 2. * coords[0] / (float)_myWindow->width() - 1.f;
        lly = 2. * coords[1] / (float)_myWindow->height() - 1.f;
        urx = llx + 2. * fltTxtWidth;
        ury = lly + 2. * fltTxtHeight;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(llx, lly, .0f); //coords[2]);//.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(llx, ury, .0f); //coords[2]);//.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(urx, ury, .0f); //coords[2]);//.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(urx, lly, .0f); //coords[2]);//.0f);
        glEnd();
    }

    if (_type == 2) {
        fltTxtWidth = (float)_width / (float)_myWindow->width();
        fltTxtHeight = (float)_height / (float)_myWindow->height();
        llx = coords[0];
        lly = coords[1];
        urx = llx + (float)_width / (float)_myWindow->width();
        ury = lly + (float)_height / (float)_myWindow->height();

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(llx, lly, coords[2]); //coords[2]);//.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(llx, ury, coords[2]); //coords[2]);//.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(urx, ury, coords[2]); //.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(urx, lly, coords[2]); //.0f);
        glEnd();
    }

    if (_type == 3) {
        fltTxtWidth = (float)_width / (float)_myWindow->width();
        fltTxtHeight = (float)_height / (float)_myWindow->height();
        llx = coords[0];
        lly = coords[1];

        GLWindow *castWin;
        castWin = dynamic_cast<GLWindow *>(_myWindow);
        ViewpointParams *myViewParams = castWin->getActiveViewpointParams();
        float *viewDir;
        float *upDir;
        float rightDir[3];
        viewDir = myViewParams->getViewDir();
        upDir = myViewParams->getUpVec();
        rightDir[0] = viewDir[1] * upDir[2] - viewDir[2] * upDir[1];
        rightDir[1] = viewDir[2] * upDir[0] - viewDir[0] * upDir[2];
        rightDir[2] = viewDir[0] * upDir[1] - viewDir[1] * upDir[0];

        float llz;
        float ulx, uly, ulz;
        float urx, ury, urz;
        float lrx, lry, lrz;

        llx = coords[0];
        lly = coords[1];
        llz = coords[2];

        ulx = llx + upDir[0] * fltTxtHeight;
        uly = lly + upDir[1] * fltTxtHeight;
        ulz = llz + upDir[2] * fltTxtHeight;

        urx = ulx + rightDir[0] * fltTxtWidth;
        ury = uly + rightDir[1] * fltTxtWidth;
        urz = ulz + rightDir[2] * fltTxtWidth;

        lrx = urx - upDir[0] * fltTxtHeight;
        lry = ury - upDir[1] * fltTxtHeight;
        lrz = urz - upDir[2] * fltTxtHeight;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(llx, lly, llz);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(ulx, uly, ulz);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(urx, ury, urz);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(lrx, lry, lrz);
        glEnd();
    }
#endif

    glBindTexture(GL_TEXTURE_2D, 0);

    removeViewerMatrix();

    GLenum glErr;
    glErr = glGetError();
    char *errString;
    if (glErr != GL_NO_ERROR) {
        errString = (char *)gluErrorString(glErr);
        Wasp::MyBase::SetErrMsg(errString);

        // Loop through remaining errors, if any
        glErr = glGetError();
        while (glErr != GL_NO_ERROR) {
            errString = (char *)gluErrorString(glErr);
            Wasp::MyBase::SetErrMsg(errString);
        }
        return -1;
    }
    return 0;
}

int TextObject::drawMe() {
    applyViewerMatrix();

    glBindTexture(GL_TEXTURE_2D, _fboTexture);

    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint fbWidth = dims[2];
    GLint fbHeight = dims[3];

    float fltTxtWidth = (float)_width / (float)fbWidth;
    float fltTxtHeight = (float)_height / (float)fbHeight;

    //	float llx = 2.*_coords[0]/(float)_myWindow->width() - 1.f;
    //	float lly = 2.*_coords[1]/(float)_myWindow->height() - 1.f;
    float llx = 2. * _coords[0] / (float)fbWidth - 1.f;
    float lly = 2. * _coords[1] / (float)fbHeight - 1.f;
    float urx = llx + 2. * fltTxtWidth;
    float ury = lly + 2. * fltTxtHeight;

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

    glBindTexture(GL_TEXTURE_2D, 0);

    removeViewerMatrix();

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
    return 0;
}
