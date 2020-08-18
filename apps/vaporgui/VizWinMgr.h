//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		VizWinMgr.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Sept 2004
//

#ifndef VIZWINMGR_H
#define VIZWINMGR_H

class QMdiArea;
class QMdiSubWindow;
class QWidget;

#include <vapor/ControlExecutive.h>
#include <qobject.h>
#include <vapor/common.h>
#include <vapor/ParamsMgr.h>
#include <vapor/ViewpointParams.h>
#include "AnimationParams.h"
#include "GUIStateParams.h"

class VizWin;
class Trackball;

//! \class VizWinMgr
//! \ingroup Public_GUI
//! \brief A class for managing all visualizers
//! \author Alan Norton
//! \version 3.0
//! \date October 2013

//!	This class manages the VAPOR visualizers in the GUI.
//!	It provides various methods relating the active visualizers and the
//! corresponding and Params classes.
//! Methods are also provided for setting up the Qt OpenGL context of a QGLWidget and
//! associating the corresponding Visualizer instance.
//
class VizWinMgr : public QObject {
    Q_OBJECT

public:
    VizWinMgr(QWidget *parent, QMdiArea *mdiArea, VAPoR::ControlExec *ce);

    ~VizWinMgr();

    //! Shutdown all visualizers
    //!
    //! Reset to default state, killing all active visualizers
    //
    void Shutdown();

    //! Restart all visualizers
    //!
    //! Restart all known visualizers
    //
    void Restart();

    void SetTrackBall(const double posvec[3], const double dirvec[3], const double upvec[3], const double centerRot[3], bool perspective);

    //! Invoke updateGL on all the visualizers that have dirty bit set.
    void Update(bool fast);

    //! \copydoc VAPoR::ControlExec::EnableImageCapture()
    int EnableImageCapture(string filename, string winName);

public slots:

    //! Method launches a new visualizer, sets up appropriate
    //! Params etc.  It returns the visualizer index as
    //! returned by ControlExec::NewVisualizer()
    //! \retval visualizer index
    void LaunchVisualizer();

    //! Arrange the Visualizers in a cascading sequence
    void Cascade();

    //! Arrange the visualizers to tile the available space.
    void FitSpace();

    //! Respond to user request to activate a window:
    void SetWinActive(const QString &vizName);

    void Reinit();

private slots:

    // Set a Visualizer to be the active (selected) Visualizer
    // \param[in] vizNum index of Visualizer to be activated.
    void _setActiveViz(string winName);

    //! Method that responds to user destruction of a visualizer.
    //! Relevant params, renderers, etc. are removed.
    void _vizAboutToDisappear(string winName);

    // Method that responds to completion of window navigation
    //
    void _syncViewpoints(string winName);

signals:
    // Turn on/off multiple viz options:
    //
    void enableMultiViz(bool onOff);

    // Respond to user setting the vizselectorcombo:
    //
    void newViz(const QString &);

    void removeViz(const QString &);

    void activateViz(const QString &);

private:
    // Can't call default constructor
    //
    VizWinMgr();

    std::map<string, VizWin *>        _vizWindow;
    std::map<string, QMdiSubWindow *> _vizMdiWin;

    QMdiArea *_mdiArea;

    QWidget *           _parent;
    VAPoR::ControlExec *_controlExec;
    Trackball *         _trackBall;
    bool                _initialized;
    bool                _insideRender = false;

    void _attachVisualizer(string vizName);

    GUIStateParams *_getStateParams() const
    {
        VAssert(_controlExec != NULL);
        VAPoR::ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
        return ((GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType()));
    }

    AnimationParams *_getAnimationParams() const
    {
        VAssert(_controlExec != NULL);
        VAPoR::ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
        return ((AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType()));
    }

    VAPoR::ViewpointParams *_getViewpointParams(string winName) const
    {
        VAssert(_controlExec != NULL);
        VAPoR::ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
        return (paramsMgr->GetViewpointParams(winName));
    }

    vector<string> _getVisualizerNames() const;
    void           _killViz(string vizName);
};
#endif    // VIZWINMGR_H
