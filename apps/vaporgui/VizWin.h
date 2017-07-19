//************************************************************************
//									*
//		     Copyright (C)  2013				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		VizWin.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		October 2013
//
//

#ifndef VIZWIN_H
#define VIZWIN_H

#include <QGLWidget>
#include "VizWinMgr.h"
#include "vapor/Visualizer.h"
#include <QWheelEvent>

class QCloseEvent;
class QRect;
class QMouseEvent;
class QFocusEvent;

class MainForm;
class VizWinMgr;
class Visualizer;
class Viewpoint;
class Trackball;

//! \class VizWin
//! \ingroup Public_GUI
//! \brief A QGLWidget that supports display based on GL methods invoked in a Visualizer
//! \author Alan Norton
//! \version 3.0
//! \date October 2013

//!	The VizWin class is a QGLWidget that supports the rendering by the VAPOR Visualizer class.
//! The standard rendering methods (resize, initialize, paint) are passed to the Visualizer.
//! In addition this is the class that responds to mouse events, resulting in scene navigation
//! or manipulator changes.
//!
class VizWin : public QGLWidget {
    Q_OBJECT

public:
    VizWin(MainForm *parent, const QString &name, VizWinMgr *myMgr, QRect *location, string winName, VAPoR::ControlExec *ce, Trackball *trackBall);
    ~VizWin();

    //! Identify the visualizer index
    //! \retval visualizer index.
    string getWindowName() { return _winName; }

    //! Force the window to update, even if nothing has changed.
    void reallyUpdate();

    void SetTrackBall(const double posvec[3], const double dirvec[3], const double upvec[3], const double centerRot[3], bool perspective);

#ifndef DOXYGEN_SKIP_THIS
public slots:
    virtual void setFocus();

private:
    VizWin() {}

    MainForm *_mainForm;

    virtual QSize minimumSizeHint() const { return QSize(400, 400); }

    QPoint _mouseDownPosition;
    bool   _mouseDownHere;
    // Event handling
    // Virtual overrides:
    virtual void wheelEvent(QWheelEvent *e) { e->accept(); }

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    virtual void mousePressEventNavigate(QMouseEvent *);
    virtual void mouseReleaseEventNavigate(QMouseEvent *);
    virtual void mouseMoveEventNavigate(QMouseEvent *);

    virtual void focusInEvent(QFocusEvent *e);
    virtual void closeEvent(QCloseEvent *);

    virtual void resizeGL(int width, int height);
    virtual void initializeGL();
    void         paintGL();
    bool         mouseIsDown() { return _mouseDownHere; }

    string              _winName;
    VizWinMgr *         _vizWinMgr;
    VAPoR::ControlExec *_controlExec;
    // Variables to control spin animation:
    QTime *_spinTimer;

    int        _moveCount;          // number of mouse move events since mouse press
    int        _moveCoords[2];      // position at last move event during rotation navigation
    int        _moveDist;           // Distance between last two mouse move events
    int        _latestMoveTime;     // most recent time of move
    int        _olderMoveTime;      // time of move before the latest
    bool       _mouseClicked;       // Indicates mouse has been clicked but not move
    double     _strHandleMid[3];    // Stretched coordinates of middle of selected handle
    int        _buttonNum;          // currently pressed button (0=none, 1=left,2=mid, 3=right)
    Trackball *_trackBall;

    void getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar) const;

    void setUpProjMatrix();
    void setUpModelViewMatrix();

#endif    // DOXYGEN_SKIP_THIS
};

#endif    // VIZWIN_H
