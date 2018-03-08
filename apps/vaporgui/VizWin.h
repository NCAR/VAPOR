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

#include <GL/glew.h>
#include <QGLWidget>
#include "vapor/Visualizer.h"
#include <QWheelEvent>

class QCloseEvent;
class QRect;
class QMouseEvent;
class QFocusEvent;

class Visualizer;
class Viewpoint;
class Trackball;

namespace VAPoR {
class ControlExec;
};

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
    VizWin(QWidget *parent, const QString &name, string winName, VAPoR::ControlExec *ce, Trackball *trackBall);
    ~VizWin();

    //! Identify the visualizer index
    //! \retval visualizer index.
    string getWindowName() { return _winName; }

signals:
    // Sent prior to closing window - after receiving Qt closeEvent()
    //
    void Closing(const string &winName);

    // Sent when window gains focus - after receiving Qt focusInEvent()
    //
    void HasFocus(const string &winName);

public slots:
    virtual void setFocus();

private:
    VizWin() {}

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

    string              _winName;
    VAPoR::ControlExec *_controlExec;

    bool       _mouseClicked;    // Indicates mouse has been clicked but not move
    int        _buttonNum;       // currently pressed button (0=none, 1=left,2=mid, 3=right)
    Trackball *_trackBall;

    void getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar) const;

    void setUpProjMatrix();
    void setUpModelViewMatrix();
};

#endif    // VIZWIN_H
