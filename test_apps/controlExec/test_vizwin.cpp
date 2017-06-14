//************************************************************************
//									*
//		     Copyright (C)  2016				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//	File:		test_vizwin.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		October 2013
//
//	Description:	Implements the VizWin class
//		This is the QGLWidget that performs OpenGL rendering (using associated Visualizer)
//		Plus supports mouse event reporting
//
#include <vapor/glutil.h>    // Must be included first!!!
#include <cassert>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QCloseEvent>

#include "vapor/ControlExecutive.h"
#include "test_vizwin.h"

using namespace VAPoR;

/*
 *  Constructs a VizWindow as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
Test_VizWin::Test_VizWin(QWidget *parent, ControlExec *ce, string winName) : QGLWidget(parent)
{
    _windowName = winName;
    _controlExec = ce;
    m_initialized = false;
    setAutoBufferSwap(false);

    return;
}

// React to a user-change in window size/position (or possibly max/min)
// Either the window is minimized, maximized, restored, or just resized.
void Test_VizWin::resizeGL(int width, int height)
{
    int rc1 = printOpenGLErrorMsg("GLVizWindowResizeEvent");

    int rc2 = _controlExec->ResizeViz(_windowName, width, height);
    if (!rc1 && !rc2) reallyUpdate();
    return;
}
void Test_VizWin::initializeGL()
{
    int rc1 = printOpenGLErrorMsg("GLVizWindowInitializeEvent");
    makeCurrent();
    int rc2 = _controlExec->InitializeViz(_windowName);
    int rc3 = printOpenGLErrorMsg("GLVizWindowInitializeEvent");

    if (!(rc2 < 0)) { m_initialized = true; }
}

void Test_VizWin::paintGL()
{
    if (!m_initialized) return;

    // only paint if necessary
    // Note that makeCurrent is needed when here we have multiple windows.
    int rc0 = printOpenGLErrorMsg("VizWindowPaintGL");
    int rc1 = 0, rc2 = 0;
    if (!rc0) {
        makeCurrent();

        rc1 = _controlExec->Paint(_windowName, false);
        if (!rc1) swapBuffers();
        rc2 = printOpenGLErrorMsg("VizWindowPaintGL");
    }

    return;
}
void Test_VizWin::reallyUpdate()
{
    makeCurrent();
    int rc = _controlExec->Paint(_windowName, true);

    swapBuffers();
    return;
}
void Test_VizWin::closeEvent(QCloseEvent *e) { QGLWidget::closeEvent(e); }
