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
#include "AnnotationEventRouter.h"
#include "NavigationEventRouter.h"
#include "RenderEventRouter.h"
#include "RenderHolder.h"
#include "TabManager.h"
#include <vapor/DataStatus.h>

using namespace VAPoR;

const string TabManager::_renderersTabName = "Renderers";
const string TabManager::_navigationTabName = "Scene";
const string TabManager::_settingsTabName = "Settings";

TabManager::TabManager(QWidget *parent, ControlExec *ce) : QTabWidget(parent)
{
    _controlExec = ce;

    // order of vector is order of display
    //
    _tabNames = {_renderersTabName, _navigationTabName};

    // Initialize arrays of widgets and types
    //
    for (int i = 0; i < _tabNames.size(); i++) {
        _tabWidgets[_tabNames[i]] = NULL;
        _subTabWidgets[_tabNames[i]] = vector<QWidget *>();
        _subTabNames[_tabNames[i]] = vector<string>();
    }

    _currentFrontTab = "";
    _prevFrontTab = "";

    _initialized = false;
    _animationEventRouter = NULL;
    _navigationEventRouter = NULL;

    setElideMode(Qt::ElideNone);

    _createAllDefaultTabs();

    show();

    _initialized = true;
}

void TabManager::SetActiveRenderer(string activeViz, string renderClass, string renderInst)
{
    if (renderClass.empty() || renderInst.empty()) {
        HideRenderWidgets();
        return;
    }

    ShowRenderWidget(renderClass);

    RenderEventRouter *eRouter = _getRenderEventRouter(activeViz, renderClass, renderInst);
    eRouter->SetActive(renderInst);
    eRouter->updateTab();

    emit ActiveEventRouterChanged(eRouter->GetType());
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
    VAssert(tabIndex >= 0);

    int subTabIndex = _getSubTabIndex(tabName, subTabName);
    if (subTabIndex < 0) return;

    _currentFrontSubTab[tabName] = subTabName;

    setCurrentIndex(tabIndex);
    if (tabName != _renderersTabName) {
        QTabWidget *qtw = (QTabWidget *)_getTabWidget(tabName);
        qtw->setCurrentIndex(subTabIndex);
    } else {
        _renderHolder->SetCurrentWidget(subTabName);
    }
}

void TabManager::GetWebHelp(string tabName, std::vector<std::pair<string, string>> &help) const
{
    help.clear();

    EventRouter *er = _getEventRouter(tabName);
    if (!er) return;

    return (er->GetWebHelp(help));
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
    this->show();
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

//////////////////////////////////////////////////////////////////////////////
//
// SLOTS
//
//////////////////////////////////////////////////////////////////////////////

void TabManager::_setSubTab(int posn)
{
    int tabIndex = currentIndex();

    if (posn < 0) return;

    _prevFrontTab = _currentFrontTab;

    if (!_prevFrontTab.empty()) { _prevFrontSubTab[_prevFrontTab] = _currentFrontSubTab[_prevFrontTab]; }

    _currentFrontTab = _tabNames[tabIndex];
    _currentFrontSubTab[_currentFrontTab] = _subTabNames[_currentFrontTab][posn];

    EventRouter *eRouter = _getEventRouter(_subTabNames[_currentFrontTab][posn]);
    eRouter->updateTab();

    emit ActiveEventRouterChanged(eRouter->GetType());
}

// Catch any change in the top tab, update the eventRouter of the sub tab.
// Also provide signal indicating tab changed
//
void TabManager::_setFrontTab(int newFrontPosn)
{
    if (newFrontPosn < 0 || newFrontPosn > _tabNames.size()) return;

    if (!_prevFrontTab.empty()) { _prevFrontSubTab[_prevFrontTab] = _currentFrontSubTab[_prevFrontTab]; }

    _prevFrontTab = _currentFrontTab;
    _currentFrontTab = _tabNames[newFrontPosn];

    string subTab = _currentFrontSubTab[_tabNames[newFrontPosn]];
    if (subTab.empty()) { subTab = _subTabNames[_tabNames[newFrontPosn]][0]; }

    EventRouter *eRouter = _getEventRouter(subTab);

    QWidget *wid = dynamic_cast<QWidget *>(eRouter);
    if (wid && wid->isVisible()) {
        eRouter->updateTab();
        emit ActiveEventRouterChanged(eRouter->GetType());
    }
}

void TabManager::_setActive(string activeViz, string renderClass, string renderInst) { SetActiveRenderer(activeViz, renderClass, renderInst); }

void TabManager::_newRenderer(string activeViz, string renderClass, string renderInst)
{
    if (renderClass.empty() || renderInst.empty()) {
        HideRenderWidgets();
        return;
    }

    ShowRenderWidget(renderClass);

    RenderEventRouter *er = _getRenderEventRouter(activeViz, renderClass, renderInst);

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    string winName, dataSetName, className;
    bool   ok = paramsMgr->RenderParamsLookup(renderInst, winName, dataSetName, className);
    VAssert(ok);

    AnimationParams *aParams = (AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType());
    size_t           ts = aParams->GetCurrentTimestep();

    DataStatus *dataStatus = _controlExec->GetDataStatus();

    RenderParams *rParams = er->GetActiveParams();
    size_t        local_ts = dataStatus->MapGlobalToLocalTimeStep(dataSetName, ts);
    rParams->SetCurrentTimestep(local_ts);

    er->SetActive(renderInst);

    QWidget *w = dynamic_cast<QWidget *>(er);
    VAssert(w);
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
        VAssert(w);
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

    Update();
}

void TabManager::Reinit() { EnableRouters(true); }

void TabManager::LoadDataNotify(string dataSetName)
{
    map<string, EventRouter *>::iterator itr;
    for (itr = _eventRouterMap.begin(); itr != _eventRouterMap.end(); ++itr) { itr->second->LoadDataNotify(dataSetName); }
}

EventRouter *TabManager::_getEventRouter(string erType) const
{
    map<string, EventRouter *>::const_iterator itr;
    itr = _eventRouterMap.find(erType);
    if (itr == _eventRouterMap.end()) {
        VAssert(0);
        return 0;
    }
    return itr->second;
}

RenderEventRouter *TabManager::_getRenderEventRouter(string winName, string renderType, string instName) const
{
    map<string, EventRouter *>::const_iterator itr;
    itr = _eventRouterMap.find(renderType);
    if (itr == _eventRouterMap.end()) {
        VAssert(0);
        return 0;
    }
    RenderEventRouter *er = dynamic_cast<RenderEventRouter *>(itr->second);
    VAssert(er);

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
    QWidget *    parent;
    EventRouter *er;

    // Install built-in tabs
    //
    parent = _getTabWidget(_navigationTabName);
    er = new AnnotationEventRouter(parent, _controlExec);
    _installTab(_navigationTabName, er->GetType(), er);

    _animationEventRouter = new AnimationEventRouter(parent, _controlExec);

    connect(_animationEventRouter, SIGNAL(AnimationOnOffSignal(bool)), this, SLOT(_setAnimationOnOff(bool)));
    connect(_animationEventRouter, SIGNAL(AnimationDrawSignal()), this, SLOT(_setAnimationDraw()));

    er = _animationEventRouter;
    _installTab(_navigationTabName, er->GetType(), er);

    _navigationEventRouter = new NavigationEventRouter(parent, _controlExec);
    er = _navigationEventRouter;
    _installTab(_navigationTabName, er->GetType(), er);

    connect((NavigationEventRouter *)er, SIGNAL(Proj4StringChanged(string)), this, SLOT(_setProj4String(string)));

    connect((NavigationEventRouter *)er, SIGNAL(ProjectionTypeChanged(int)), this, SLOT(_projectionTypeChanged(int)));

    // Create renderers from render factory
    //
    vector<string> renderNames = RenderEventRouterFactory::Instance()->GetFactoryNames();

    parent = _getTabWidget(_renderersTabName);
    for (int i = 0; i < renderNames.size(); i++) {
        RenderEventRouter *er = RenderEventRouterFactory::Instance()->CreateInstance(renderNames[i], parent, _controlExec);

        VAssert(er);

        _installTab(_renderersTabName, er->GetType(), er);
    }

    // set up widgets in tabs:
    _installWidgets();
}

void TabManager::_projectionTypeChanged(int itype)
{
    ViewpointParams::ProjectionType type = (ViewpointParams::ProjectionType)itype;
    if (type == ViewpointParams::MapOrthographic) {
        ViewAll();
        SetHomeViewpoint();
    }
}

void TabManager::_installTab(string tabName, string subTabName, EventRouter *eRouter)
{
    _registerEventRouter(subTabName, eRouter);
    eRouter->hookUpTab();
    QWidget *tabWidget = dynamic_cast<QWidget *>(eRouter);
    VAssert(tabWidget);
    _addSubTabWidget(tabWidget, subTabName, tabName);
}

void TabManager::_registerEventRouter(const std::string subTabName, EventRouter *router) { _eventRouterMap[subTabName] = router; }

void TabManager::_installWidgets()
{
    clear();

    _currentFrontTab = _tabNames[0];
    for (int i = 0; i < _tabNames.size(); i++) {
        string tabName = _tabNames[i];
        _currentFrontSubTab[tabName] = _subTabNames[tabName][0];
    }

    // Create top widgets.  Tab widgets exist but need to be
    // inserted as tabs, based on their type
    // Type RENDERERS is for renderers, NAVIGATION for nav
    // Create top Tab Widgets to hold the nav
    for (int i = 1; i < _tabNames.size(); i++) { _tabWidgets[_tabNames[i]] = new QTabWidget(this); }

    for (int i = 0; i < _subTabWidgets[_renderersTabName].size(); i++) {
        string tag = _subTabNames[_renderersTabName][i];
        _subTabWidgets[_renderersTabName][i]->hide();
    }

    _initRenderHolder();

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
            myScrollArea->setWidgetResizable(true);

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

    setCurrentIndex(0);

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

        EventRouter *      eRouter = _getEventRouter(tab);
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
        if (_currentFrontTab == _renderersTabName) SetActiveRenderer(activeViz, renderClass, instName);

        EventRouter *eRouter = _getRenderEventRouter(activeViz, renderClass, instName);

        eRouter->updateTab();
    }
}

void TabManager::_initRenderHolder()
{
    vector<QWidget *> widgets;
    vector<string>    widgetNames;
    vector<string>    descriptions;
    vector<string>    iconPaths;
    vector<string>    smallIconPaths;
    vector<bool>      dim2dSupport;
    vector<bool>      dim3dSupport;

    for (int i = 0; i < _subTabWidgets[_renderersTabName].size(); i++) {
        string tag = _subTabNames[_renderersTabName][i];

        widgets.push_back(_subTabWidgets[_renderersTabName][i]);
        widgetNames.push_back(tag);

        RenderEventRouter *re = dynamic_cast<RenderEventRouter *>(_subTabWidgets[_renderersTabName][i]);

        descriptions.push_back(re->GetDescription());
        iconPaths.push_back(re->GetIconImagePath());
        smallIconPaths.push_back(re->GetSmallIconImagePath());

        dim2dSupport.push_back(re->Supports2DVariables());
        dim3dSupport.push_back(re->Supports3DVariables());
    }

    _renderHolder = new RenderHolder(this, _controlExec, widgets, widgetNames, descriptions, iconPaths, smallIconPaths, dim2dSupport, dim3dSupport);

    connect(_renderHolder, SIGNAL(activeChanged(string, string, string)), this, SLOT(_setActive(string, string, string)));
    connect(_renderHolder, SIGNAL(newRendererSignal(string, string, string)), this, SLOT(_newRenderer(string, string, string)));
}
