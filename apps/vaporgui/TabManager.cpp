//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		TabManager.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2004
//
//	Description:	Implements the TabManager class.  This is the QTabWidget
//		that contains a tab for each of the Params classes
//
#include <vapor/glutil.h>    // Must be included first
#include <algorithm>
#include <qtabwidget.h>
#include <qwidget.h>
#include <QScrollArea>
#include <QSizePolicy>
#include "TabManager.h"
#include "ui_NavigationTab.h"
#include "AnimationEventRouter.h"
#include "NavigationEventRouter.h"
#include "SettingsEventRouter.h"
#include "HelloEventRouter.h"
#include "RenderEventRouter.h"
#include "RenderHolder.h"
#include "AppSettingsParams.h"

// Extension tabs also included (until we find a nicer way to separate extensions)
#include "TwoDDataEventRouter.h"
#include "ImageEventRouter.h"
#include "BarbEventRouter.h"
#include "ContourEventRouter.h"

TabManager *TabManager::_tabManager = NULL;

using namespace VAPoR;
TabManager::TabManager(QWidget *parent, ControlExec *ce, VizWinMgr *vizWinMgr) : QTabWidget(parent)
{
    _controlExec = ce;
    _renderHolder = new RenderHolder(this, _controlExec);

    // UGGGGH! Need to get VizWinMgr out of here and out NavigationEventRouter
    //
    _vizWinMgr = vizWinMgr;

    // Initialize arrays of widgets and types
    //
    for (int i = 0; i < 3; i++) {
        _widgets[i].clear();
        _widgetTags[i].clear();
        _currentFrontPage[i] = -1;
        _prevFrontPage[i] = -1;
        _topWidgets.push_back(NULL);
    }

    _currentTopTab = -1;
    _prevTopTab = -1;

    setElideMode(Qt::ElideNone);

    _topName[0] = "Renderers";
    _topName[1] = "Navigation";
    _topName[2] = "Settings";

    _initialized = false;

    connect(_renderHolder, SIGNAL(activeChanged(string, string, string)), this, SLOT(setActive(string, string, string)));
    connect(_renderHolder, SIGNAL(newRendererSignal(string, string, string)), this, SLOT(newRenderer(string, string, string)));

    _createAllDefaultTabs();

    show();

    _initialized = true;
}

void TabManager::ShowRenderWidget(string tag)
{
    MoveToFront(tag);
    for (int i = 0; i < _widgets[0].size(); i++) {
        if (_widgetTags[0][i] != tag) {
            _widgets[0][i]->hide();
        } else {
            _widgets[0][i]->show();
        }
    }
}
void TabManager::HideRenderWidgets()
{
    for (int i = 0; i < _widgets[0].size(); i++) { _widgets[0][i]->hide(); }
}

/*
 * Make the specified named panel move to front tab, using
 * a specific visualizer number.
 */

int TabManager::MoveToFront(string widgetTag)
{
    int posn = findWidget(widgetTag);
    if (posn < 0) return -1;

    int tabType = getTabType(widgetTag);
    assert(tabType >= 0 && tabType <= 2);

    _currentFrontPage[tabType] = posn;

    setCurrentIndex(tabType);
    if (tabType > 0) {
        QTabWidget *qtw = (QTabWidget *)_getSubTabWidget(tabType);
        qtw->setCurrentIndex(posn);
    } else {
        _renderHolder->SetCurrentIndex(posn);
    }
    return posn;
}

/*
 *  Find the index of the widget in its subTabWidget that has the specified type
 */
int TabManager::findWidget(string widgetTag)
{
    int tabType = getTabType(widgetTag);
    assert(tabType >= 0 && tabType <= 2);

    for (int i = 0; i < _widgets[tabType].size(); i++) {
        if (_widgetTags[tabType][i] == widgetTag) return i;
    }
    return -1;
}

void TabManager::SetActiveViz(const QString &vizNameQ)
{
    _tabManager->show();
    // Make the renderHolder show either the active renderer or none.
    //
    _renderHolder->Update();
}

bool TabManager::IsFrontTab(QWidget *wid)
{
    int topTab = _currentTopTab;

    return (wid == _widgets[topTab][_currentFrontPage[topTab]]);
}

int TabManager::getTabType(string t)
{
    for (int tabType = 0; tabType < 3; tabType++) {
        for (int i = 0; i < _widgetTags[tabType].size(); i++) {
            if (_widgetTags[tabType][i] == t) return (tabType);
        }
    }
    return (-1);
}

void TabManager::Update()
{
    _renderHolder->Update();

    _updateRouters();
}

//////////////////////////////////////////////////////////////////////////////
//
// SLOTS
//
//////////////////////////////////////////////////////////////////////////////

void TabManager::newFrontTab(int topTab, int newSubPosn)
{
    _prevTopTab = _currentTopTab;
    if (_prevTopTab >= 0) _prevFrontPage[_prevTopTab] = _currentFrontPage[_prevTopTab];
    _currentTopTab = topTab;
    _currentFrontPage[topTab] = newSubPosn;
    if (_prevTopTab != _currentTopTab || (_prevTopTab >= 0 && _prevFrontPage[_prevTopTab] != _currentFrontPage[_prevTopTab])) { emit tabLeft(_prevTopTab, _prevFrontPage[_prevTopTab]); }
    EventRouter *eRouter = GetEventRouter(_widgetTags[topTab][newSubPosn]);
    eRouter->updateTab();

    emit ActiveEventRouterChanged(eRouter->GetType());
}

void TabManager::NewSubTab(int posn)
{
    int topTab = currentIndex();
    if (topTab < 0) return;
    newFrontTab(topTab, posn);
}

// Catch any change in the top tab, update the eventRouter of the sub tab.
// Also provide signal indicating tab changed
//
void TabManager::NewTopTab(int newFrontPosn)
{
    if (newFrontPosn < 0) return;
    if (_prevTopTab >= 0) _prevFrontPage[_prevTopTab] = _currentFrontPage[_prevTopTab];
    int subTabIndex = _currentFrontPage[newFrontPosn];
    if (subTabIndex < 0) return;
    _prevTopTab = _currentTopTab;
    _currentTopTab = newFrontPosn;

    string tag = _widgetTags[newFrontPosn][subTabIndex];
    if (_prevTopTab != _currentTopTab || (_prevTopTab >= 0 && _prevFrontPage[_prevTopTab] != _currentFrontPage[_prevTopTab])) { emit tabLeft(_prevTopTab, _prevFrontPage[_prevTopTab]); }
    EventRouter *eRouter = GetEventRouter(tag);

    QWidget *wid = dynamic_cast<QWidget *>(eRouter);
    if (wid && wid->isVisible()) {
        eRouter->updateTab();
        emit ActiveEventRouterChanged(eRouter->GetType());
    }
}

void TabManager::setActive(string activeViz, string renderClass, string renderInst)
{
    if (renderClass.empty() || renderInst.empty()) {
        HideRenderWidgets();
        return;
    }

    ShowRenderWidget(renderClass);

    RenderEventRouter *eRouter = GetRenderEventRouter(activeViz, renderClass, renderInst);

    eRouter->SetActive(renderInst);

    eRouter->updateTab();

    emit ActiveEventRouterChanged(eRouter->GetType());
}

void TabManager::newRenderer(string activeViz, string renderClass, string renderInst)
{
    if (renderClass.empty() || renderInst.empty()) {
        HideRenderWidgets();
        return;
    }

    ShowRenderWidget(renderClass);

    RenderEventRouter *er = GetRenderEventRouter(activeViz, renderClass, renderInst);

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    string     winName, dataSetName, paramsType;
    bool       status = paramsMgr->RenderParamsLookup(renderInst, winName, dataSetName, paramsType);

    AnimationParams *aParams = (AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType());
    size_t           ts = aParams->GetCurrentTimestep();

    DataStatus *dataStatus = _controlExec->GetDataStatus();

    RenderParams *rParams = er->GetActiveParams();
    size_t        local_ts = dataStatus->MapGlobalToLocalTimeStep(dataSetName, ts);
    rParams->SetCurrentTimestep(local_ts);

    er->SetActive(renderInst);

    QWidget *w = dynamic_cast<QWidget *>(er);
    assert(w);
    w->setEnabled(true);

    er->updateTab();
}

vector<string> TabManager::GetInstalledTabNames(bool renderOnly) const
{
    vector<string>                             keys;
    map<string, EventRouter *>::const_iterator itr;
    for (itr = _eventRouterMap.begin(); itr != _eventRouterMap.end(); ++itr) {
        RenderEventRouter *er = dynamic_cast<RenderEventRouter *>(itr->second);

        if (renderOnly && !er) continue;

        keys.push_back(itr->first);
    }
    return (keys);
}

void TabManager::EnableRouters(bool onOff)
{
    map<string, EventRouter *>::iterator itr;
    for (itr = _eventRouterMap.begin(); itr != _eventRouterMap.end(); ++itr) {
        // Ugh! EventRouter base class does not inheret from QWidget, but
        // derived classes *should*. No way to enforce this.
        //

        QWidget *w = dynamic_cast<QWidget *>(itr->second);
        assert(w);
        w->setEnabled(onOff);
    }
}

void TabManager::Shutdown()
{
    EnableRouters(false);

    _initialized = false;
}

void TabManager::Restart()
{
    // Must be shutdown before restarting
    //
    if (_initialized) return;

    Reinit();

    _initialized = true;
}

void TabManager::Reinit() { EnableRouters(true); }

EventRouter *TabManager::GetEventRouter(string erType) const
{
    map<string, EventRouter *>::const_iterator itr;
    itr = _eventRouterMap.find(erType);
    if (itr == _eventRouterMap.end()) {
        assert(0);
        return 0;
    }
    return itr->second;
}

RenderEventRouter *TabManager::GetRenderEventRouter(string winName, string renderType, string instName) const
{
    map<string, EventRouter *>::const_iterator itr;
    itr = _eventRouterMap.find(renderType);
    if (itr == _eventRouterMap.end()) {
        assert(0);
        return 0;
    }
    RenderEventRouter *er = dynamic_cast<RenderEventRouter *>(itr->second);
    assert(er);

    er->SetActive(instName);

    return er;
}

// Create the global params and the default renderer params:
void TabManager::_createAllDefaultTabs()
{
    QWidget *    parent;
    EventRouter *er;

    // Install built-in tabs
    //
    parent = _getSubTabWidget(1);
    er = new AnimationEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 1, er);

    parent = _getSubTabWidget(1);
    er = new NavigationEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 1, er);

    parent = _getSubTabWidget(2);
    er = new SettingsEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 2, er);

    // Renderer tabs
    //
    parent = _getSubTabWidget(0);
    er = new TwoDDataEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 0, er);

#ifndef HELLO_RENDERER
    parent = _getSubTabWidget(0);
    er = new HelloEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 0, er);
#endif

    parent = _getSubTabWidget(0);
    er = new BarbEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 0, er);

    parent = _getSubTabWidget(0);
    er = new ImageEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 0, er);

    parent = _getSubTabWidget(0);
    er = new ContourEventRouter(parent, _controlExec);
    _installTab(er->GetType(), 0, er);

    // set up widgets in tabs:
    _installWidgets();
}

void TabManager::_installTab(const std::string tag, int tabType, EventRouter *eRouter)
{
    _registerEventRouter(tag, eRouter);
    eRouter->hookUpTab();
    QWidget *tabWidget = dynamic_cast<QWidget *>(eRouter);
    assert(tabWidget);
    _addWidget(tabWidget, tag, tabType);
}

void TabManager::_registerEventRouter(const std::string tag, EventRouter *router) { _eventRouterMap[tag] = router; }

void TabManager::_installWidgets()
{
    clear();

    // Create top widgets.  Tab widgets exist but need to be
    // inserted as tabs, based on their type
    // Type 0 is for renderers, 1 for nav and 2 for settings.
    // Create top Tab Widgets to hold the nav and settings
    for (int i = 1; i < 3; i++) _topWidgets[i] = new QTabWidget(this);

    // The renderer tabs are put into a stackedWidget, managed by RenderHolder.
    // Insert the renderer 'tabs' but don't show them yet.
    //
    for (int i = 0; i < _widgets[0].size(); i++) {
        string tag = _widgetTags[0][i];
        _renderHolder->AddWidget(_widgets[0][i], tag.c_str(), tag);
        _widgets[0][i]->hide();
    }

    // Add the renderer tab to the top level TabWidget.
    addTab(_renderHolder, _topName[0]);
    _topWidgets[0] = _renderHolder;
    // Add the bottom widgets (eventrouter-based) to the nav and setting tabs:
    for (int topTab = 1; topTab < 3; topTab++) {
        for (int j = 0; j < _widgets[topTab].size(); j++) {
            QScrollArea *myScrollArea = new QScrollArea(_topWidgets[topTab]);
            string       tag = _widgetTags[topTab][j];
            QTabWidget * qtw = (QTabWidget *)_topWidgets[topTab];
            qtw->addTab(myScrollArea, QString::fromStdString(tag));
            myScrollArea->setWidget(_widgets[topTab][j]);
        }
    }
    // Add all 3 top tabs to this
    for (int widType = 1; widType < 3; widType++) {
        addTab(_topWidgets[widType], _topName[widType]);
        QTabWidget *qtw = (QTabWidget *)_topWidgets[widType];
        qtw->setCurrentIndex(0);
    }

    // Start them with the renderer tab showing.

    _currentTopTab = 0;
    setCurrentIndex(0);

    for (int i = 0; i < 3; i++) _currentFrontPage[i] = 0;
    for (int toptab = 0; toptab < 3; toptab++) {
        for (int subtab = 0; subtab < _widgets[toptab].size(); subtab++) {
            EventRouter *ev = dynamic_cast<EventRouter *>(_widgets[toptab][subtab]);
#ifdef DEAD
            if (ev) ev->setTabIndices(toptab, subtab);
#endif
        }
    }

    // Hook up signals
    for (int topTab = 1; topTab < 3; topTab++) { connect(_topWidgets[topTab], SIGNAL(currentChanged(int)), this, SLOT(NewSubTab(int))); }
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(NewTopTab(int)));

    return;
}

// Add a tabWidget to appropriate saved list of widgets:
//
void TabManager::_addWidget(QWidget *wid, string tag, int tagType)
{
    _widgets[tagType].push_back(wid);
    _widgetTags[tagType].push_back(tag);
}

void TabManager::_updateRouters()
{
    if (!_initialized) return;

    // First handle non-render event routers
    //
    vector<string> tabNames = GetInstalledTabNames(false);

    for (int i = 0; i < tabNames.size(); i++) {
        string tab = tabNames[i];

        EventRouter *      eRouter = GetEventRouter(tab);
        RenderEventRouter *reRouter = dynamic_cast<RenderEventRouter *>(eRouter);

        if (reRouter) continue;    // Skip render event routers

        eRouter->updateTab();
    }

    // Now handle the active render event router.
    //
    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();

    string renderClass, instName;
    p->GetActiveRenderer(activeViz, renderClass, instName);

    if (activeViz.size() && renderClass.size() && instName.size()) {
        EventRouter *eRouter = GetRenderEventRouter(activeViz, renderClass, instName);

        eRouter->updateTab();
    }
}
