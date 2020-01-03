//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		TabManager.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2015
//
//	Description:	Defines the TabManager class.  This is the QTabWidget
//		that contains a tab for each of the Params classes
//
#ifndef TABMANAGER_H
#define TABMANAGER_H
#include <qtabwidget.h>
#include <qwidget.h>
#include <QKeyEvent>

#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include <AnimationEventRouter.h>
#include <NavigationEventRouter.h>
#include "GUIStateParams.h"
class RenderHolder;
class EventRouter;
class RenderEventRouter;

//! \class TabManager
//! \ingroup Public_GUI
//! \brief A class that manages the contents of the parameter tabs in VAPOR GUI
//! \author Alan Norton
//! \version 3.0
//! \date May 2015

//! The parameters in VAPOR GUI are displayed using a hierarchy of tabs.
//! The TabManager is a QTabWidget that
//! handles the arrangement of these tabs.  Generally this layout is
//! automatically set up and maintained by
//! the MainForm and VizWinMgr classes, however users may occasionally need
//! to query the status of a tab.
//!
//! At the top level there are three "Top" tabs:  The \p Renderer,
//! the \p Navigation, and the \p Settings.
//!
//! When the \p Renderer tab is selected, the RenderHolder is displayed.
//! This allows creation, deletion, enablement, and selection of
//! the renderer that is to be edited.  The currently selected
//! renderer has its parameters displayed in a QStackedWidget that is
//! managed by the RenderHolder class.  There is one EventRouter
//! instance whose contents are displayed as tabs in the QStackedWidget.
//!
//! When the \p Navigation tab is selected, A QTabWidget is displayed,
//! with one tab for each of the "navigation" EventRouters: Viewpoint/Lights,
//! Region, and Animation.
//!
//! When the \p Settings tab is selected, another QTabWidget is
//! displayed, with one tab for User Preferences, and another tab
//! for Visualizer Features.
//!

//! @name Internal
//! Internal methods not intended for general use
//!
//
class TabManager : public QTabWidget {
    Q_OBJECT

public:
    TabManager(QWidget *, VAPoR::ControlExec *ce);

    void SetActiveRenderer(string activeViz, string renderClass, string renderInst);

    //! In order to display the parameters for the selected renderer,
    //! QWidget::show() is invoked for the selected EventRouter, and
    //! QWidget::hide() is invoked for all other renderer EventRouters.
    //! This is performed in the RenderHolder class.
    //! \param[in] tag is the tag associated with the renderer.
    //
    void ShowRenderWidget(string tag);

    //! All the render EventRouter widgets are hidden until one
    //! is selected, using this method.
    //
    void HideRenderWidgets();

    //! Update from current state
    //
    void Update();

    //! Make the specified EventRouter be displayed in front.
    //! \param[in] widgetTag is the Params tag that should be moved to front.
    //! \return  the tab position in the associated sub tab.
    //
    void MoveToFront(string subTabName);

    //! Get list of Installed Tab names
    //!
    //! \param[in] renderOnly If true only return render event routers.
    //! See GetRenderEventRouter()
    //!
    //
    vector<string> GetInstalledTabNames(bool renderOnly) const;

    virtual void GetWebHelp(string tabName, std::vector<std::pair<string, string>> &help) const;

    //! Enable or disable widgets associated with all event routers
    //
    void EnableRouters(bool onOff);
    void Shutdown();
    void Restart();
    void Reinit();

    //! Notify TabManager that  a new data set has been loaded
    //!
    void LoadDataNotify(string dataSetName);

signals:

    //! Triggered when currently active event router changes
    //!
    //! \param[in] type Type of event router as returned by
    //! EventRouter::GetType()
    //!
    void ActiveEventRouterChanged(string type);

    //! Proj4 string changed
    //
    void Proj4StringChanged(string proj4String);

    void AnimationOnOffSignal(bool);
    void AnimationDrawSignal();

public slots:
    void UseHomeViewpoint() { _navigationEventRouter->UseHomeViewpoint(); }
    void ViewAll() { _navigationEventRouter->ViewAll(); }
    void SetHomeViewpoint() { _navigationEventRouter->SetHomeViewpoint(); }
    void AlignView(int axis)
    {
        _navigationEventRouter->AlignView(axis);

        QComboBox *cb = (QComboBox *)sender();
        cb->blockSignals(true);
        cb->setCurrentIndex(0);
        cb->blockSignals(false);
    }
    void CenterSubRegion() { _navigationEventRouter->CenterSubRegion(); }

    void AnimationPlayForward() { _animationEventRouter->AnimationPlayForward(); }
    void AnimationPlayBackward() { _animationEventRouter->AnimationPlayReverse(); }
    void AnimationPause() { _animationEventRouter->AnimationPause(); }
    void AnimationStepBackward() { _animationEventRouter->AnimationStepReverse(); }
    void AnimationStepForward() { _animationEventRouter->AnimationStepForward(); }
    void AnimationSetTimestep(int ts) { _animationEventRouter->SetTimeStep(ts); }

protected slots:

private slots:
    void _setProj4String(string proj4String) { emit Proj4StringChanged(proj4String); }

    void _projectionTypeChanged(int);

    void _setAnimationOnOff(bool onOff) { emit AnimationOnOffSignal(onOff); }

    void _setAnimationDraw() { emit AnimationDrawSignal(); }

    //! Slot that responds to user selecting a 2nd level tab, i.e.
    //! a tab that corresponds to an EventRouter.
    //! \param[in] tabid ID of selected subtab.
    void _setSubTab(int tabid);

    //! Slot that responds to selecting a tab to be in front
    //! \param[in] tabnum is 0,1, or 2 for the selected top tab
    //
    void _setFrontTab(int tabnum);

    void SetActiveViz(const QString &vizNameQ);

    void _setActive(string activeViz, string renderClass, string renderInst);

    void _newRenderer(string activeViz, string renderClass, string renderInst);

private:
    static const string _renderersTabName;
    static const string _navigationTabName;
    static const string _settingsTabName;

    VAPoR::ControlExec *_controlExec;
    RenderHolder *      _renderHolder;

    // ordered list of all top level tabs
    //
    std::vector<string> _tabNames;

    // Top level tabs (widgets), one for each string in _tabNames
    //
    std::map<string, QWidget *> _tabWidgets;

    // sub tabs - a vector of widgets for each top level tab
    //
    std::map<string, vector<QWidget *>> _subTabWidgets;
    std::map<string, vector<string>>    _subTabNames;

    // Map top level widget name to current or previous subwidget name
    //
    std::map<string, string> _currentFrontSubTab;
    std::map<string, string> _prevFrontSubTab;

    string _currentFrontTab;
    string _prevFrontTab;

    // map tags to eventrouters
    std::map<string, EventRouter *> _eventRouterMap;

    bool                   _initialized;
    AnimationEventRouter * _animationEventRouter;
    NavigationEventRouter *_navigationEventRouter;

    TabManager() {}

    virtual QSize sizeHint() const { return QSize(460, 800); }

    // This prevents a "beep" from occuring when you press enter on the Mac.
    virtual void keyPressEvent(QKeyEvent *e) { e->accept(); }

    EventRouter *_getEventRouter(string erType) const;

    RenderEventRouter *_getRenderEventRouter(string winName, string renderType, string instName) const;

    // Find the position of the specified widget in subTab, or -1 if it isn't there.
    //
    int _getSubTabIndex(string tabName, string subTabName) const;
    int _getSubTabIndex(string subTabName) const;
    int _getTabIndex(string tabName) const;

    string _getTabForSubTab(string subTabName) const;

    QWidget *_getSubTabWidget(string subTabName) const;
    QWidget *_getTabWidget(string tabName) const;

    GUIStateParams *_getStateParams() const
    {
        VAssert(_controlExec != NULL);
        VAPoR::ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
        return ((GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType()));
    }

    void _createAllDefaultTabs();

    // method that creates an eventRouter, and installs it as one of the tabs.
    // All extension EventRouter classes must call this during the
    // InstallExtensions() method.
    //
    //
    void _installTab(string tabName, string subTabName, EventRouter *eRouter);

    void _registerEventRouter(const std::string subTabName, EventRouter *router);

    // During initialization, after all the EventRouters have been created
    // (and identified via _addSubTabWidget()), the installWidgets
    // method must be called to set up the various widgets in each tab.
    //
    void _installWidgets();

    // Method invoked by the VizWinMgr at the time all the EventRouters
    // are created
    // Each eventRouter and its associated tag are saved in arrays for
    // subsequent use
    //
    // \param[in] evWid indicates the QWidget that is associated with
    // an EventRouter
    // \param[in] tag is the Params Tag associated with the EventRouter.
    //
    void _addSubTabWidget(QWidget *evWid, string Tag, string tagType);

    void _updateRouters();

    void _initRenderHolder();
};

#endif    // TABMANAGER_H
