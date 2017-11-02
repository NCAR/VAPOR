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
class QDesktopWidget;
class QMainWidget;
class QTimer;
class QDockWidget;

#include <vapor/ControlExecutive.h>
#include <qobject.h>
#include <vapor/common.h>
#include "GUIStateParams.h"

namespace VAPoR {
class ViewpointParams;
class RegionParams;
} // namespace VAPoR

class EventRouter;
class RenderEventRouter;
class TabManager;
class MainForm;

class VizWin;
class AnimationEventRouter;
class RegionEventRouter;
class ViewpointEventRouter;
class Trackball;

//! \class VizWinMgr
//! \ingroup Public_GUI
//! \brief A class for managing all visualizers
//! \author Alan Norton
//! \version 3.0
//! \date October 2013

//!	This class manages the VAPOR visualizers in the GUI.
//!	It provides various methods relating the active visualizers and the
//! corresponding EventRouter and Params classes.
//! Methods are also provided for setting up the Qt OpenGL context of a QGLWidget and
//! associating the corresponding Visualizer instance.
//
class VizWinMgr : public QObject {
    Q_OBJECT

  public:
    static VizWinMgr *Create(VAPoR::ControlExec *ce) {
        if (_vizWinMgr)
            return (_vizWinMgr);

        _vizWinMgr = new VizWinMgr(ce); // private default constructor
        return (_vizWinMgr);
    }

    //! Obtain the (unique) VizWinMgr instance
    //! \retval VizWinMgr*
    static VizWinMgr *getInstance() {
        assert(_vizWinMgr);
        return _vizWinMgr;
    }

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

    //! method that creates an eventRouter, and installs it as one of the tabs.
    //! All extension EventRouter classes must call this during the InstallExtensions() method.
    //! \param[in] const std::string tag : XML tag identifying the Params class.
    void installTab(const std::string tag, int tagType, EventRouter *eRouter);

    //! Get list of Installed Tab names
    //!
    //! \param[in] renderOnly If true only return render event routers.
    //! See GetRenderEventRouter()
    //!
    //! \sa installTab()
    //
    vector<string> GetInstalledTabNames(bool renderOnly = false);

    //! method obtains the EventRouter instance associated with a
    //! particular tag.
    //! \param[in] erType
    //! \retval EventRouter* pointer to the associated EventRouter instance
    EventRouter *GetEventRouter(string erType) const;

    RenderEventRouter *GetRenderEventRouter(
        string winName, string renderType, string instName) const;

    //! Reset the GUI to its default state, either due to New Session, or in preparation for loading a session file.
    void SetToDefaults();

    //! Static helper method invoked during Undo and Redo visualizer creation and destruction, as well
    //! setting the current viz window.
    //! This function must be passed in Command::CaptureStart()
    //! \sa UndoRedoHelpCB_T
    //! \param[in] isUndo indicates whether an Undo or Redo is being performed
    //! \param[in] instance is not used here
    //! \param[in] beforeP is a copy of the VizWinParams* at the start of the Command
    //! \param[in] afterP is a copy of the VizWinParams* at the end of the Command

    //! Determine the total number of visualizers.
    //! \retval number of visualizers.
    int getNumVisualizers();

    //! Obtain the VizWin object associated with the current active visualizer.
    //! \retval VizWin* current active VizWin object
    VizWin *getActiveVizWin() {
        GUIStateParams *p = (GUIStateParams *)
                                _controlExec->GetParamsMgr()
                                    ->GetParams(GUIStateParams::GetClassType());
        assert(p);
        string activeViz = p->GetActiveVizName();
        assert(!activeViz.empty());
        //Use an iterator so we don't insert if the viz is not there
        std::map<string, VizWin *>::iterator it = _vizWindow.find(activeViz);
        if (it == _vizWindow.end())
            return 0;
        return _vizWindow[activeViz];
    }

    //! Obtain the (unique) RegionEventRouter in the GUI
    //! \retval RegionEventRouter*
    RegionEventRouter *getRegionRouter();

    //! Obtain the (unique) AnimationEventRouter in the GUI
    //! \retval AnimationEventRouter*
    AnimationEventRouter *getAnimationRouter();

    //! Obtain the (unique) ViewpointEventRouter in the GUI
    //! \retval ViewpointEventRouter*
    ViewpointEventRouter *getViewpointRouter();

    //! @name Internal
    //! Internal methods not intended for general use
    //!
    ///@{
    //! Method setting up the tabs in their default state.
    //! This is invoked once at the time vaporgui is started.
    //! This method includes a line for each built-in tab.
    void createAllDefaultTabs();

    //! Method that responds to user destruction of a visualizer.
    //! Relevant params, renderers, etc. are removed.
    void vizAboutToDisappear(string winName);

    //! Method to delete a visualizer under program control, e.g. if
    //! the user performs Undo after creating a visualizer
    //! \param[in] viznum Visualizer index.
    void removeVisualizer(string vizName);

    //! Set a Visualizer to be the active (selected) Visualizer
    //! \param[in] vizNum index of Visualizer to be activated.
    void setActiveViz(string winName);

    string GetActiveVizName() const;

    vector<string> GetVisualizerNames() const;

    //! Force all renderers to re-obtain their data,
    //! for example when a new session is opened.
    void refreshRenderData();

    ~VizWinMgr();

  public slots:

    //! Method launches a new visualizer, sets up appropriate
    //! Params etc.  It returns the visualizer index as
    //! returned by ControlExec::NewVisualizer()
    //! \retval visualizer index
    void LaunchVisualizer();

    //! Arrange the Visualizers in a cascading sequence
    void cascade();

    //! Arrange the visualizers to tile the available space.
    void fitSpace();

    //! Respond to user request to activate a window:
    void winActivated(const QString &vizName);

    //! Close the VizWin associated with a Visualizer index
    void killViz(string vizName);

    //Slots that set viewpoint:

    void SetTrackBall(
        const double posvec[3], const double dirvec[3],
        const double upvec[3], const double centerRot[3],
        bool perspective);

    //! Invoke updateGL on all the visualizers that have dirty bit set.
    void updateDirtyWindows();

    void setEnabled(bool flag);

    void ReinitRouters();

    //! Enable or disable widgets associated with all event routers
    //
    void EnableRouters(bool onOff);

    //! Force all the EventRouters to update based on the
    //! state of the Params for the active window.
    void UpdateRouters();

///@}
#ifndef DOXYGEN_SKIP_THIS
  signals:
    //Turn on/off multiple viz options:
    void enableMultiViz(bool onOff);
    //Respond to user setting the vizselectorcombo:
    void newViz(const QString &);
    void removeViz(const QString &);
    void activateViz(const QString &);

  private:
    void registerEventRouter(const std::string tag, EventRouter *router);

    // map tags to eventrouters
    std::map<string, EventRouter *> _eventRouterMap;

    // Can't call default constructor
    //
    VizWinMgr(VAPoR::ControlExec *ce);
    VizWinMgr();

    static VizWinMgr *_vizWinMgr;
    //Map visualizer id to vizwin
    std::map<string, VizWin *> _vizWindow;
    std::map<string, QMdiSubWindow *> _vizMdiWin;
    TabManager *_tabManager;

    MainForm *_mainForm;
    QMdiArea *_mdiArea;

    VAPoR::ControlExec *_controlExec;
    Trackball *m_trackBall;
    bool m_initialized;

    void attachVisualizer(string vizName);

#endif //DOXYGEN_SKIP_THIS
};
#endif // VIZWINMGR_H
