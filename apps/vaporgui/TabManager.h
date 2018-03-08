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
#include "VizWinMgr.h"

namespace VAPoR {
class ControlExec;
}

class RenderHolder;
class EventRouter;
class RenderEventRouter;
class VizWinMgr;

//! \class TabManager
//! \ingroup Public_GUI
//! \brief A class that manages the contents of the parameter tabs in VAPOR GUI
//! \author Alan Norton
//! \version 3.0
//! \date May 2015

//! The parameters in VAPOR GUI are displayed using a hierarchy of tabs.  The TabManager is a QTabWidget that
//! handles the arrangement of these tabs.  Generally this layout is automatically set up and maintained by
//! the MainForm and VizWinMgr classes, however users may occasionally need to query the status of a tab.
//!
//! At the top level there are three "Top" tabs:  The \p Renderer, the \p Navigation, and the \p Settings.
//!
//! When the \p Renderer tab is selected, the RenderHolder is displayed.  This allows creation, deletion, enablement, and selection of
//! the renderer that is to be edited.  The currently selected renderer has its parameters displayed in a QStackedWidget that is
//! managed by the RenderHolder class.  There is one EventRouter instance whose contents are displayed as tabs in the QStackedWidget.
//!
//! When the \p Navigation tab is selected, A QTabWidget is displayed, with one tab for each of the "navigation" EventRouters: Viewpoint/Lights,
//! Region, and Animation.
//!
//! When the \p Settings tab is selected, another QTabWidget is displayed, with one tab for User Preferences, and another tab for Visualizer Features.
//!

//! @name Internal
//! Internal methods not intended for general use
//!
///@{
class TabManager : public QTabWidget {
    Q_OBJECT

public:
    //! Constructor:  Invoked by the MainForm during the set up of the
    //! main window.
    //
    static TabManager *Create(QWidget *parent, VAPoR::ControlExec *ce, VizWinMgr *vizWinMgr)
    {
        if (_tabManager) return (_tabManager);

        _tabManager = new TabManager(parent, ce, vizWinMgr);
        return (_tabManager);
    };

    //! Obtain the (unique) TabManager instance
    //! \retval TabManager*
    static TabManager *getInstance()
    {
        assert(_tabManager);
        return _tabManager;
    }

    //! In order to display the parameters for the selected renderer, QWidget::show() is invoked for the selected EventRouter, and
    //! QWidget::hide() is invoked for all other renderer EventRouters.  This is performed in the RenderHolder class.
    //! \param[in] tag is the tag associated with the renderer.
    void ShowRenderWidget(string tag);

    //! All the render EventRouter widgets are hidden until one is selected, using this method.
    void HideRenderWidgets();

    //! Update from current state
    //
    void Update();

    //! Make the specified EventRouter be displayed in front.
    //! \param[in] widgetTag is the Params tag that should be moved to front.
    //! \return  the tab position in the associated sub tab.
    int MoveToFront(string widgetTag);

    //! Determine if a widget is currently the front tab in its subtab
    //! \param[in] is the QWidget* to be checked
    //! \return bool true if it is in front.
    bool IsFrontTab(QWidget *wid);
    //! Get list of Installed Tab names
    //!
    //! \param[in] renderOnly If true only return render event routers.
    //! See GetRenderEventRouter()
    //!
    //
    vector<string> GetInstalledTabNames(bool renderOnly) const;

    EventRouter *GetEventRouter(string erType) const;

    RenderEventRouter *GetRenderEventRouter(string winName, string renderType, string instName) const;

    //! Enable or disable widgets associated with all event routers
    //
    void EnableRouters(bool onOff);
    void Shutdown();
    void Restart();
    void Reinit();

signals:
    void tabLeft(int, int);

    //! Triggered when currently active event router changes
    //!
    //! \param[in] type Type of event router as returned by
    //! EventRouter::GetType()
    //!
    void ActiveEventRouterChanged(string type);

protected slots:
    //! Slot that responds to selecting a tab to be in front
    //! \param[in] tabnum is 0,1, or 2 for the selected top tab
    void NewTopTab(int tabnum);

    //! Slot that responds to user selecting a 2nd level tab, i.e.
    //! a tab that corresponds to an EventRouter.
    //! \param[in] tabid ID of selected subtab.
    void NewSubTab(int tabid);

    void SetActiveViz(const QString &vizNameQ);

    void setActive(string activeViz, string renderClass, string renderInst);

    void newRenderer(string activeViz, string renderClass, string renderInst);

    ///@} //End of internal methods

#ifndef DOXYGEN_SKIP_THIS
private:
    // Find the position of the specified widget in subTab, or -1 if it isn't there.
    //
    int findWidget(string widgetTag);

    virtual QSize sizeHint() const { return QSize(460, 800); }

    // This prevents a "beep" from occuring when you press enter on the Mac.
    virtual void keyPressEvent(QKeyEvent *e) { e->accept(); }

    int  getTabType(string tag);
    void newFrontTab(int topType, int subPosn);

private:
    static TabManager *_tabManager;
    // map tags to eventrouters
    std::map<string, EventRouter *> _eventRouterMap;

    VAPoR::ControlExec *_controlExec;
    RenderHolder *      _renderHolder;
    VizWinMgr *         _vizWinMgr;

    // Data structures to store widget info
    vector<QWidget *> _widgets[3];

    vector<string>         _widgetTags[3];
    std::vector<QWidget *> _topWidgets;
    QString                _topName[3];
    int                    _currentFrontPage[3];
    int                    _prevFrontPage[3];
    int                    _currentTopTab;
    int                    _prevTopTab;

    bool _initialized;

    TabManager() {}
    TabManager(QWidget *, VAPoR::ControlExec *ce, VizWinMgr *vizWinMgr);

    QWidget *_getSubTabWidget(int widType)
    {
        assert(widType >= 0 && widType < _topWidgets.size());
        return _topWidgets[widType];
    }

    GUIStateParams *_getStateParams() const
    {
        assert(_controlExec != NULL);
        VAPoR::ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
        return ((GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType()));
    }

    void _createAllDefaultTabs();

    // method that creates an eventRouter, and installs it as one of the tabs.
    // All extension EventRouter classes must call this during the
    // InstallExtensions() method.
    //
    // const std::string tag : XML tag identifying the Params class.
    //
    void _installTab(const std::string tag, int tagType, EventRouter *eRouter);

    void _registerEventRouter(const std::string tag, EventRouter *router);

    // During initialization, after all the EventRouters have been created
    // (and identified via addWidget()), the installWidgets
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
    void _addWidget(QWidget *evWid, string Tag, int tagType);

    void _updateRouters();

#endif    // DOXYGEN_SKIP_THIS
};

#endif    // TABMANAGER_H
