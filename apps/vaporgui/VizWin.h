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

#include <vapor/glutil.h>
#include <QGLWidget>
#include "vapor/Transform.h"
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
class Visualizer;
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

    // Render the scene
    //
    // If \p fast is true try to render the scene quickly
    //
    void Render(bool fast);
    bool paintOnResize = true;

signals:
    void EndNavigation(const string &winName);

public slots:
    virtual void setFocus();

private:
    void _renderHelper(bool fast);
    void _preRender();
    void _postRender();
    void updateManip(bool initialize = false);
    void _updateOriginGlyph();
    void _drawContourSliceQuad();

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

    virtual void _mousePressEventManip(QMouseEvent *);
    virtual void _mouseReleaseEventManip(QMouseEvent *);
    virtual void _mouseMoveEventManip(QMouseEvent *);

    // QGLWidget reimplementations
    virtual void resizeGL(int width, int height);
    void _resizeGL(int width, int height);
    virtual void initializeGL();
    virtual void paintGL();

    void safeSwapBuffers();

    string              _winName;
    VAPoR::ControlExec *_controlExec;
    VAPoR::GLManager *  _glManager;
    double              _strHandleMid[3];
    bool                _insideRender = false;

    bool       _mouseClicked;    // Indicates mouse has been clicked but not move
    int        _buttonNum;       // currently pressed button (0=none, 1=left,2=mid, 3=right)
    bool       _navigateFlag;
    bool       _manipFlag;
    bool       _manipFlowSeedFlag = false;
    bool       _manipFlowIntegrationFlag = false;
    Trackball *_trackBall;
    bool _navigationPendingChanges;

    std::vector<double> _getScreenCoords(QMouseEvent *e) const;
    string              _getCurrentMouseMode() const;
    void                _setNewExtents();
    void                _getActiveExtents(VAPoR::CoordType &minExts, VAPoR::CoordType &maxExts);
    void                _getUnionOfFieldVarExtents(VAPoR::RenderParams *rParams, VAPoR::DataMgr *dataMgr, int timestep, int refLevel, int lod, VAPoR::CoordType &minExts, VAPoR::CoordType &maxExts);
    string _getCurrentDataMgrName() const;
    VAPoR::Transform *_getDataMgrTransform() const;

    void                 _getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar) const;
    VAPoR::RenderParams *_getRenderParams();
    VAPoR::RenderParams *_getRenderParams(string &classType);

    void _setUpProjMatrix();
    void _setUpModelViewMatrix();

    VAPoR::TranslateStretchManip *_manip;

    bool _openGLInitFlag;
    bool _swapBufferQueued = false;

    friend class VizWinMgr;
};

#endif    // VIZWIN_H
