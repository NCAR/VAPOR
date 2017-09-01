//************************************************************************
//														*
//		     Copyright (C)  2014									*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		ImageFrame.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2005
//
//	Description:	Implements the ImageFrame class.  This provides
//		a frame in which various images can be displayed and mouse-picked.
//		Principally involved in drawing and responding to mouse events.
//
#include "ImageFrame.h"

#include <QFrame>
#include <qwidget.h>
#include <qgl.h>
#include <qlayout.h>

#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QHBoxLayout>

ImageFrame::ImageFrame(QWidget *parent, Qt::WFlags f) : QFrame(parent, f)
{
#ifdef CRAP
    setFocusPolicy(Qt::StrongFocus);
    _glImageWindow = 0;
    _renderParams = 0;
#endif
}

#ifdef CRAP

void ImageFrame::attachRenderWindow(QGLWidget *wid, EventRouter *evr)
{
    QGLFormat fmt = wid->format();
    if (!(fmt.directRendering() && fmt.rgba() && fmt.alpha() && fmt.doubleBuffer())) {
        QString strng("Unable to obtain adequate OpenGL rendering format.\n");
        strng += "Ensure your graphics card is properly configured, and/or \n";
        strng += "Be sure to use 'vglrun' if you are in a VirtualGL session.";
    #ifdef DEAD
        Params::BailOut((const char *)strng.toAscii(), __FILE__, __LINE__);
    #endif
    }
    _eventRouter = evr;
    QBoxLayout *flayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    flayout->addWidget(wid, 0);
    setGLImageWindow(wid);
}
ImageFrame::~ImageFrame() {}

void ImageFrame::mousePressEvent(QMouseEvent *e)
{
    #ifdef DEAD

    RenderParams *pm = getParams();
    if (!pm) return;
    float coords[2];
    mapPixelToFrameCoords(e->x(), e->y(), &coords[0], &coords[1]);
    _eventRouter->StartCursorMove();
    pm->SetCursorCoords(coords);
    update();
    #endif
}
void ImageFrame::mouseMoveEvent(QMouseEvent *e)
{
    #ifdef DEAD

    RenderParams *pm = getParams();
    if (!pm) return;
    float coords[2];
    mapPixelToFrameCoords(e->x(), e->y(), &coords[0], &coords[1]);
    pm->SetCursorCoords(coords);
    update();
    #endif
}
void ImageFrame::mouseReleaseEvent(QMouseEvent *e)
{
    #ifdef DEAD

    RenderParams *pm = getParams();
    if (!pm) return;
    float coords[2];
    mapPixelToFrameCoords(e->x(), e->y(), &coords[0], &coords[1]);
    pm->SetCursorCoords(coords);
    _eventRouter->EndCursorMove();
    update();
    #endif
}

//
// To map the window coords, first map the device coords (0..width-1)
// and (0..height-1) to
// float (-1,1).  Then stretch according to _rectLeft, _rectRight
//
void ImageFrame::mapPixelToFrameCoords(int ix, int iy, float *x, float *y)
{
    float xcoord = 1. - 2.f * ((float)ix) / ((float)(width() - 1));
    float ycoord = 1.f - 2.f * ((float)iy) / ((float)(height() - 1));
    *x = xcoord / _rectLeft;
    *y = ycoord / _rectTop;
}
#endif
