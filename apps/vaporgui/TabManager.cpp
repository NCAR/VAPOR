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
#include "AnimationEventRouter.h"
#include "VizFeatureEventRouter.h"
#include "AppSettingsEventRouter.h"
#include "StartupEventRouter.h"
#include "NavigationEventRouter.h"
#include "HelloEventRouter.h"
#include "RenderEventRouter.h"
#include "RenderHolder.h"
#include "AppSettingsParams.h"
#include "StartupParams.h"

// Extension tabs also included
// (until we find a nicer way to separate extensions)
//
#include "TwoDDataEventRouter.h"
#include "ImageEventRouter.h"
#include "BarbEventRouter.h"
#include "ContourEventRouter.h"

#include "TabManager.h"

using namespace VAPoR;

const string TabManager::_renderersTabName = "Renderers";
const string TabManager::_navigationTabName = "Navigation";
const string TabManager::_settingsTabName = "Settings";

TabManager::TabManager(QWidget *parent, ControlExec *ce) : QTabWidget(parent)
{
    _controlExec = ce;
    _renderHolder = new RenderHolder(this, _controlExec);

    // order of vector is order of display
    //
    _tabNames = {_renderersTabName, _navigationTabName, _settingsTabName};

    // Initialize arrays of widgets and types
    //
    for (int i = 0; i < _tabNames.size(); i++) {
        _tabWidgets[_tabNames[i]] = NULL;
        _subTabWidgets[_tabNames[i]] = vector<QWidget *>();
        _subTabNames[_tabNames[i]] = vector<string>();
    }

    _currentFrontTab = "";
    _prevFrontTab = "";

    setElideMode(Qt::ElideNone);

    _initialized = false;

    connect(_renderHolder, SIGNAL(activeChanged(string, string, string)), this, SLOT(_setActive(string, string, string)));
    connect(_renderHolder, SIGNAL(newRendererSignal(string, string, string)), this, SLOT(_newRenderer(string, string, string)));

    _createAllDefaultTabs();

    show();

    _initialized = true;
}

void TabManager::ShowRenderWidget(string subTabName)
{
    MoveToFront(subTabName);
    for (int i = 0; i < _subTabWidgets[_renderersTabName].size(); i++) {
        if (_subTabNames[_renderersTabName][i] != subTabName) {
            _subTabWidgets[_renderersTabName][i]->hide();
        } else {
            _subTabWidgets[_renderersTabName][i]->show();
        }
    }
}
void TabManager::HideRenderWidgets()
{
    for (int i = 0; i < _subTabWidgets[_renderersTabName].size(); i++) { _subTabWidgets[_renderersTabName][i]->hide(); }
}

//
// Make the specified named panel move to front tab, using
// a specific visualizer number.
//
void TabManager::MoveToFront(string subTabName)
{
    string tabName = _getTabForSubTab(subTabName);
    if (tabName.empty()) return;

    int tabIndex = _getTabIndex(tabName);
    assert(tabIndex >= 0);

    int subTabIndex = _getSubTabIndex(tabName, subTabName);
    if (subTabIndex < 0) return;

    _currentFrontSubTab[tabName] = subTabName;

    setCurrentIndex(tabIndex);
    if (tabName != _renderersTabName) {
        QTabWidget *qtw = (QTabWidget *)_getTabWidget(tabName);
        qtw->setCurrentIndex(subTabIndex);
    } else {
        _renderHolder->SetCurrentIndex(subTabIndex);
    }
}

//
//  Find the index of the widget in its subTabWidget that has the specified type
//
int TabManager::_getSubTabIndex(string tabName, string subTabName) const
{
    map<string, vector<string>>::const_iterator itr;

    itr = _subTabNames.find(tabName);
    if (itr == _subTabNames.end()) return (-1);

    const vector<string> &vref = itr->second;
    for (int i = 0; i < vref.size(); i++) {
        if (vref[i] == subTabName) return (i);
    }
    return -1;
}

int TabManager::_getSubTabIndex(string subTabName) const
{
    string tabName = _getTabForSubTab(subTabName);

    return (_getSubTabIndex(tabName, subTabName));
}

int TabManager::_getTabIndex(string tabName) const
{
    for (int i = 0; i < _tabNames.size(); i++) {
        if (_tabNames[i] == tabName) return (i);
    }
    return (-1);
}

void TabManager::SetActiveViz(const QString &vizNameQ)
{
    show();
    // Make the renderHolder show either the active renderer or none.
    //
    _renderHolder->Update();
}

string TabManager::_getTabForSubTab(string subTabName) const
{
    map<string, vector<string>>::const_iterator itr;
    for (itr = _subTabNames.begin(); itr != _subTabNames.end(); ++itr) {
        string                tabName = itr->first;
        const vector<string> &vref = itr->second;
        for (int i = 0; i < vref.size(); i++) {
            if (vref[i] == subTabName) return (tabName);
        }
    }
    return ("");
}

void TabManager::Update()
{
    _renderHolder->Update();

    _updateRouters();
}

void TabManager::_newFrontTab(int tabIndex, int newSubPosn)
{
    _prevFrontTab = _currentFrontTab;

    if (!_prevFrontTab.empty()) { _prevFrontSubTab[_prevFrontTab] = _currentFrontSubTab[_prevFrontTab]; }

    _currentFrontTab = _tabNames[tabIndex];
    _currentFrontSubTab[_currentFrontTab] = _subTabNames[_currentFrontTab][newSubPosn];

    EventRouter *eRouter = GetEventRouter(_subTabNames[_currentFrontTab][newSubPosn]);
    eRouter->updateTab();

    emit ActiveEventRouterChanged(eRouter->GetType());
}

//////////////////////////////////////////////////////////////////////////////
//
// SLOTS
//
//////////////////////////////////////////////////////////////////////////////

void TabManager::_setSubTab(int posn)
{
    int tabIndex = currentIndex();
    if (tabIndex < 0) return;

    _newFrontTab(tabIndex, posn);
}

// Catch any change in the top tab, update the eventRouter of the sub tab.
// Also provide signal indicating tab changed
//
void TabManager::_setFrontTab(int newFrontPosn)
{
    if (newFrontPosn < 0 || newFrontPosn > _tabNames.size()) return;

    if (!_prevFrontTab.empty()) { _prevFrontSubTab[_prevFrontTab] = _currentFrontSubTab[_prevFrontTab]; }

    string subTab = _currentFrontSubTab[_tabNames[newFrontPosn]];
    if (subTab.empty()) return;
    _prevFrontTab = _currentFrontTab;
    _currentFrontTab = _tabNames[newFrontPosn];

    EventRouter *eRouter = GetEventRouter(subTab);

    QWidget *wid = dynamic_cast<QWidget *>(eRouter);
    if (wid && wid->isVisible()) {
        eRouter->updateTab();
        emit ActiveEventRouterChanged(eRouter->GetType());
    }
}

void TabManager::_setActive(string activeViz, string renderClass, string renderInst)
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

void TabManager::_newRenderer(string activeViz, string renderClass, string renderInst)
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

    DataStatus *dataStatus = _controlExec->getDataStatus();

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

QWidget *TabManager::_getSubTabWidget(string subTabName) const
{
    int    subTabIndex = _getSubTabIndex(subTabName);
    string tabName = _getTabForSubTab(subTabName);

    std::map<string, vector<QWidget *>>::const_iterator itr;
    itr = _subTabWidgets.find(tabName);
    if (itr == _subTabWidgets.end()) return (NULL);

    return (itr->second[subTabIndex]);
}

QWidget *TabManager::_getTabWidget(string tabName) const
{
    std::map<string, QWidget *>::const_iterator itr;
    itr = _tabWidgets.find(tabName);
    if (itr == _tabWidgets.end()) return (NULL);

    return (itr->second);
}

// Create the global params and the default renderer params:
void TabManager::_createAllDefaultTabs()
{
    QWidget *parent;

    // Install built-in tabs
    //
    parent = _getTabWidget(_settingsTabName);
    EventRouter *er = new StartupEventRouter(parent, _controlExec);
    _installTab(_settingsTabName, er->GetType(), er);

    parent = _getTabWidget(_navigationTabName);
    er = new AnimationEventRouter(parent, _controlExec);
    _installTab(_navigationTabName, er->GetType(), er);

    parent = _getTabWidget(_navigationTabName);
    er = new NavigationEventRouter(parent, _controlExec);
    _installTab(_navigationTabName, er->GetType(), er);

    parent = _getTabWidget(_settingsTabName);
    er = new VizFeatureEventRouter(parent, _controlExec);
    _installTab(_settingsTabName, er->GetType(), er);

    parent = _getTabWidget(_settingsTabName);
    er = new AppSettingsEventRouter(parent, _controlExec);
    _installTab(_settingsTabName, er->GetType(), er);

    // Renderer tabs
    //
    parent = _getTabWidget(_renderersTabName);
    er = new TwoDDataEventRouter(parent, _controlExec);
    _installTab(_renderersTabName, er->GetType(), er);

#ifndef HELLO_RENDERER
    parent = _getTabWidget(_renderersTabName);
    er = new HelloEventRouter(parent, _controlExec);
    _installTab(_renderersTabName, er->GetType(), er);
#endif

    parent = _getTabWidget(_renderersTabName);
    er = new BarbEventRouter(parent, _controlExec);
    _installTab(_renderersTabName, er->GetType(), er);

    parent = _getTabWidget(_renderersTabName);
    er = new ImageEventRouter(parent, _controlExec);
    _installTab(_renderersTabName, er->GetType(), er);

    parent = _getTabWidget(_renderersTabName);
    er = new ContourEventRouter(parent, _controlExec);
    _installTab(_renderersTabName, er->GetType(), er);

    // set up widgets in tabs:
    _installWidgets();
}

void TabManager::_installTab(string tabName, string subTabName, EventRouter *eRouter)
{
    _registerEventRouter(subTabName, eRouter);
    eRouter->hookUpTab();
    QWidget *tabWidget = dynamic_cast<QWidget *>(eRouter);
    assert(tabWidget);
    if (subTabName != AppSettingsParams::GetClassType() && subTabName != StartupParams::GetClassType()) { tabWidget->setEnabled(false); }
    _addSubTabWidget(tabWidget, subTabName, tabName);
}

void TabManager::_registerEventRouter(const std::string subTabName, EventRouter *router) { _eventRouterMap[subTabName] = router; }

void TabManager::_installWidgets()
{
    clear();

    // Create top widgets.  Tab widgets exist but need to be
    // inserted as tabs, based on their type
    // Type RENDERERS is for renderers, NAVIGATION for nav and SETTINGS for settings.
    // Create top Tab Widgets to hold the nav and settings
    for (int i = 1; i < _tabNames.size(); i++) { _tabWidgets[_tabNames[i]] = new QTabWidget(this); }

    // The renderer tabs are put into a stackedWidget, managed by RenderHolder.
    // Insert the renderer 'tabs' but don't show them yet.
    //
    for (int i = 0; i < _subTabWidgets[_renderersTabName].size(); i++) {
        string tag = _subTabNames[_renderersTabName][i];
        _renderHolder->AddWidget(_subTabWidgets[_renderersTabName][i], tag.c_str(), tag);
        _subTabWidgets[_renderersTabName][i]->hide();
    }

    // Add the renderer tab to the top level TabWidget.
    //
    addTab(_renderHolder, QString::fromStdString(_renderersTabName));
    _tabWidgets[_renderersTabName] = _renderHolder;

    // Add the bottom widgets (eventrouter-based) to the nav and setting tabs:
    //
    for (int i = 1; i < _tabNames.size(); i++) {
        for (int j = 0; j < _subTabWidgets[_tabNames[i]].size(); j++) {
            string tab = _tabNames[i];

            QScrollArea *myScrollArea = new QScrollArea(_tabWidgets[tab]);

            string      subTabName = _subTabNames[tab][j];
            QTabWidget *qtw = (QTabWidget *)_tabWidgets[tab];
            qtw->addTab(myScrollArea, QString::fromStdString(subTabName));
            myScrollArea->setWidget(_subTabWidgets[tab][j]);
        }
    }

    // Add the remaning top tabs to this
    //
    for (int i = 1; i < _tabNames.size(); i++) {
        string tab = _tabNames[i];

        addTab(_tabWidgets[tab], QString::fromStdString(_tabNames[i]));

        QTabWidget *qtw = (QTabWidget *)_tabWidgets[_tabNames[i]];
        qtw->setCurrentIndex(0);
    }

    // Start them with the renderer tab showing.

    _currentFrontTab = _tabNames[0];
    setCurrentIndex(0);

    for (int j = 0; j < _tabNames.size(); j++) {
        string tab = _tabNames[j];

        for (int subidx = 0; subidx < _subTabWidgets[tab].size(); subidx++) { EventRouter *ev = dynamic_cast<EventRouter *>(_subTabWidgets[tab][subidx]); }
    }

    // Hook up signals
    for (int i = 1; i < _tabNames.size(); i++) {
        string tab = _tabNames[i];
        connect(_tabWidgets[tab], SIGNAL(currentChanged(int)), this, SLOT(_setSubTab(int)));
    }
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(_setFrontTab(int)));

    return;
}

// Add a tabWidget to appropriate saved list of widgets:
//
void TabManager::_addSubTabWidget(QWidget *wid, string tag, string tagType)
{
    _subTabWidgets[tagType].push_back(wid);
    _subTabNames[tagType].push_back(tag);
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
