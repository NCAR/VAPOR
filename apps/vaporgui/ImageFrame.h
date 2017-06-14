//************************************************************************
//									*
//		     Copyright (C)  2014				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		ImageFrame.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		March 2014
//
//	Description:	Defines the ImageFrame class.  This provides
//		a frame in which various QGLWidgets sit, for displaying an image in the GUI tab.
//		Principally involved in drawing and responding to mouse events.
//		This class is used as a custom dialog for Qt Designer, so that
//		it can be embedded in various tabs
//		It's *not* in the Vapor namespace, so that designer can use it.
//
#ifndef IMAGEFRAME_H
#define IMAGEFRAME_H
#include <QFrame>
#include <qwidget.h>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

#include <vapor/RenderParams.h>
#include "EventRouter.h"

class QWidget;

class ImageFrame : public QFrame {
    Q_OBJECT
public:
    ImageFrame(QWidget *parent = 0, Qt::WFlags f = 0);
#ifdef DEAD
    virtual ~ImageFrame();

    void       setGLImageWindow(QGLWidget *wid) { _glImageWindow = wid; }
    QGLWidget *getGLImageWindow() { return _glImageWindow; }

    void          setParams(RenderParams *p) { _renderParams = p; }
    RenderParams *getParams() { return _renderParams; }
    void          attachRenderWindow(QGLWidget *wid, EventRouter *evr);
    void          setRect(float rLeft, float rTop)
    {
        _rectLeft = rLeft;
        _rectTop = rTop;
    }

private:
    float                _rectLeft, _rectTop;
    void                 mousePressEvent(QMouseEvent *);
    void                 mouseReleaseEvent(QMouseEvent *);
    void                 mouseMoveEvent(QMouseEvent *);
    void                 mapPixelToFrameCoords(int ix, int iy, float *x, float *y);
    QGLWidget *          _glImageWindow;
    VAPoR::RenderParams *_renderParams;
    EventRouter *        _eventRouter;
#endif
};

#endif    // IMAGEFRAME_H
