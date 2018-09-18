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
#include "vapor/Transform.h"
#include "vapor/Visualizer.h"
#include "Manip.h"
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
struct GLManager;
};    // namespace VAPoR

//! \class VizWin
//! \ingroup Public_GUI
/*! \brief A QGLWidget that supports display based on GL methods invoked in a
 *    Visualizer
 */
//! \author Alan Norton
//! \version 3.0
//! \date October 2013

//!	The VizWin class is a QGLWidget that supports the rendering by the VAPOR
//! Visualizer class.
//! The standard rendering methods (resize, initialize, paint) are passed to the
//! Visualizer.
//! In addition this is the class that responds to mouse events, resulting in
//! scene navigation
//! or manipulator changes.
//!
class VizWin : public QGLWidget {
    Q_OBJECT

public:
    VizWin(const QGLFormat &format, QWidget *parent, const QString &name, string winName, VAPoR::ControlExec *ce, Trackball *trackBall);
    ~VizWin();

    //! Identify the visualizer index
    //! \retval visualizer index.
    string getWindowName() { return _winName; }

    // void makeCurrent();

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

    void updateManip(bool initialize = false);

    // Event handling
    // Virtual overrides:
    virtual void wheelEvent(QWheelEvent *e) { e->accept(); }

    // QWidget reimplementations
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    virtual void _mousePressEventNavigate(QMouseEvent *);
    virtual void _mouseReleaseEventNavigate(QMouseEvent *);
    virtual void _mouseMoveEventNavigate(QMouseEvent *);

    virtual void focusInEvent(QFocusEvent *e);
    virtual void closeEvent(QCloseEvent *);

    // QGLWidget reimplementations
    virtual void resizeGL(int width, int height);
    virtual void initializeGL();
    void         paintGL();

    string              _winName;
    VAPoR::ControlExec *_controlExec;
    VAPoR::GLManager *  _glManager;
    double              _strHandleMid[3];

    bool       _mouseClicked;    // Indicates mouse has been clicked but not move
    int        _buttonNum;       // currently pressed button (0=none, 1=left,2=mid, 3=right)
    bool       _navigating;
    Trackball *_trackBall;

    std::vector<double> _getScreenCoords(QMouseEvent *e) const;
    string              _getCurrentMouseMode() const;
    void                _setNewExtents();
    void                _getActiveExtents(std::vector<double> &minExts, std::vector<double> &maxExts);
    void                _getUnionOfFieldVarExtents(VAPoR::RenderParams *rParams, VAPoR::DataMgr *dataMgr, int timestep, int refLevel, std::vector<double> &minExts, std::vector<double> &maxExts);
    void                _getCenterAndCamPos(std::vector<double> &rotationCenter, std::vector<double> &cameraPos);
    void                _getWindowSize(std::vector<int> &windowSize);
    string              _getCurrentDataMgrName() const;
    VAPoR::Transform *  _getDataMgrTransform() const;

    void                 _getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar) const;
    VAPoR::RenderParams *_getRenderParams();
    VAPoR::RenderParams *_getRenderParams(string &classType);

    void _setUpProjMatrix();
    void _setUpModelViewMatrix();

    VAPoR::TranslateStretchManip *_manip;

    void _setMatrixFromModeParams();

    double _center[3], _posvec[3], _dirvec[3], _upvec[3];
};

#endif    // VIZWIN_H
