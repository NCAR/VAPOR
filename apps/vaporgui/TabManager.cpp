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
#include <vapor/glutil.h> // Must be included first
#include <algorithm>
#include <qtabwidget.h>
#include <qwidget.h>
#include <QScrollArea>
#include <QSizePolicy>
#include "TabManager.h"
#include "ui_NavigationTab.h"
#include "RenderEventRouter.h"
#include "RenderHolder.h"

TabManager *TabManager::_tabManager = NULL;

using namespace VAPoR;
TabManager::TabManager(QWidget *parent, ControlExec *ce)
    : QTabWidget(parent) {
    _controlExec = ce;
    _renderHolder = new RenderHolder(this, _controlExec);

    //Initialize arrays of widgets and types
    //
    for (int i = 0; i < 3; i++) {
        _widgets[i].clear();
        _widgetTags[i].clear();
        _currentFrontPage[i] = -1;
        _prevFrontPage[i] = -1;
        _topWidgets[i] = 0;
    }

    _currentTopTab = -1;
    _prevTopTab = -1;

    setElideMode(Qt::ElideNone);

    _topName[0] = "Renderers";
    _topName[1] = "Navigation";
    _topName[2] = "Settings";

    connect(
        _renderHolder, SIGNAL(activeChanged(string, string, string)),
        this, SLOT(setActive(string, string, string)));
    connect(
        _renderHolder, SIGNAL(newRendererSignal(string, string, string)),
        this, SLOT(newRenderer(string, string, string)));

    show();
}

//Add a tabWidget to appropriate saved list of widgets:
//
void TabManager::AddWidget(QWidget *wid, string tag, int tagType) {
    _widgets[tagType].push_back(wid);
    _widgetTags[tagType].push_back(tag);
}

void TabManager::ShowRenderWidget(string tag) {

    MoveToFront(tag);
    for (int i = 0; i < _widgets[0].size(); i++) {
        if (_widgetTags[0][i] != tag) {
            _widgets[0][i]->hide();
        } else {
            _widgets[0][i]->show();
        }
    }
}
void TabManager::HideRenderWidgets() {
    for (int i = 0; i < _widgets[0].size(); i++) {
        _widgets[0][i]->hide();
    }
}

/*
 * Make the specified named panel move to front tab, using
 * a specific visualizer number.  
 */

int TabManager::MoveToFront(string widgetTag) {

    int posn = findWidget(widgetTag);
    if (posn < 0)
        return -1;

    int tabType = getTabType(widgetTag);
    assert(tabType >= 0 && tabType <= 2);

    _currentFrontPage[tabType] = posn;

    setCurrentIndex(tabType);
    if (tabType > 0) {
        QTabWidget *qtw = (QTabWidget *)_topWidgets[tabType];
        qtw->setCurrentIndex(posn);
    } else {
        _renderHolder->SetCurrentIndex(posn);
    }
    return posn;
}

/*
 *  Find the index of the widget in its subTabWidget that has the specified type
 */
int TabManager::findWidget(string widgetTag) {
    int tabType = getTabType(widgetTag);
    assert(tabType >= 0 && tabType <= 2);

    for (int i = 0; i < _widgets[tabType].size(); i++) {
        if (_widgetTags[tabType][i] == widgetTag)
            return i;
    }
    return -1;
}

void TabManager::SetActiveViz(const QString &vizNameQ) {

    // Make the renderHolder show either the active renderer or none.
    //
    _renderHolder->Update();
}

void TabManager::InstallWidgets() {

    clear();

    // Create top widgets.  Tab widgets exist but need to be
    // inserted as tabs, based on their type
    // Type 0 is for renderers, 1 for nav and 2 for settings.
    // Create top Tab Widgets to hold the nav and settings
    for (int i = 1; i < 3; i++)
        _topWidgets[i] = new QTabWidget(this);

    // The renderer tabs are put into a stackedWidget, managed by RenderHolder.
    // Insert the renderer 'tabs' but don't show them yet.
    //
    for (int i = 0; i < _widgets[0].size(); i++) {
        string tag = _widgetTags[0][i];
        _renderHolder->AddWidget(_widgets[0][i], tag.c_str(), tag);
        _widgets[0][i]->hide();
    }

    //Add the renderer tab to the top level TabWidget.
    addTab(_renderHolder, _topName[0]);
    _topWidgets[0] = _renderHolder;
    //Add the bottom widgets (eventrouter-based) to the nav and setting tabs:
    for (int topTab = 1; topTab < 3; topTab++) {
        //for (int j = 0; j< _widgets[topTab].size(); j++){

        // I'm eliminating the for loop and setting the tabs
        // explicitly, in order to have the 'VizFeatures' tab appear
        // as the default.
        // -Scott
        QScrollArea *myScrollArea = new QScrollArea(_topWidgets[topTab]);
        string tag = _widgetTags[topTab][1];
        QTabWidget *qtw = (QTabWidget *)_topWidgets[topTab];
        qtw->addTab(myScrollArea, QString::fromStdString(tag));
        myScrollArea->setWidget(_widgets[topTab][1]);

        myScrollArea = new QScrollArea(_topWidgets[topTab]);
        tag = _widgetTags[topTab][0];
        qtw = (QTabWidget *)_topWidgets[topTab];
        qtw->addTab(myScrollArea, QString::fromStdString(tag));
        myScrollArea->setWidget(_widgets[topTab][0]);

        myScrollArea = new QScrollArea(_topWidgets[topTab]);
        tag = _widgetTags[topTab][2];
        qtw = (QTabWidget *)_topWidgets[topTab];
        qtw->addTab(myScrollArea, QString::fromStdString(tag));
        myScrollArea->setWidget(_widgets[topTab][2]);
        //}
    }
    //Add all 3 top tabs to this
    for (int widType = 1; widType < 3; widType++) {
        addTab(_topWidgets[widType], _topName[widType]);
        QTabWidget *qtw = (QTabWidget *)_topWidgets[widType];
        qtw->setCurrentIndex(0);
    }

    //Start them with the renderer tab showing.

    _currentTopTab = 0;
    setCurrentIndex(0);

    for (int i = 0; i < 3; i++)
        _currentFrontPage[i] = 0;
    for (int toptab = 0; toptab < 3; toptab++) {
        for (int subtab = 0; subtab < _widgets[toptab].size(); subtab++) {
            EventRouter *ev = dynamic_cast<EventRouter *>(_widgets[toptab][subtab]);
#ifdef DEAD
            if (ev)
                ev->setTabIndices(toptab, subtab);
#endif
        }
    }

    //Hook up signals
    for (int topTab = 1; topTab < 3; topTab++) {
        connect(_topWidgets[topTab], SIGNAL(currentChanged(int)), this, SLOT(NewSubTab(int)));
    }
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(NewTopTab(int)));

    return;
}

bool TabManager::IsFrontTab(QWidget *wid) {
    int topTab = _currentTopTab;

    return (wid == _widgets[topTab][_currentFrontPage[topTab]]);
}

int TabManager::getTabType(string t) {

    for (int tabType = 0; tabType < 3; tabType++) {
        for (int i = 0; i < _widgetTags[tabType].size(); i++) {
            if (_widgetTags[tabType][i] == t)
                return (tabType);
        }
    }
    return (-1);
}

void TabManager::Update() {
    _renderHolder->Update();
}

//////////////////////////////////////////////////////////////////////////////
//
// SLOTS
//
//////////////////////////////////////////////////////////////////////////////

void TabManager::newFrontTab(int topTab, int newSubPosn) {
    _prevTopTab = _currentTopTab;
    if (_prevTopTab >= 0)
        _prevFrontPage[_prevTopTab] = _currentFrontPage[_prevTopTab];
    _currentTopTab = topTab;
    _currentFrontPage[topTab] = newSubPosn;
    if (_prevTopTab != _currentTopTab ||
        (_prevTopTab >= 0 && _prevFrontPage[_prevTopTab] != _currentFrontPage[_prevTopTab])) {
        emit tabLeft(_prevTopTab, _prevFrontPage[_prevTopTab]);
    }
    EventRouter *eRouter = VizWinMgr::getInstance()->GetEventRouter(_widgetTags[topTab][newSubPosn]);
    eRouter->updateTab();

    emit ActiveEventRouterChanged(eRouter->GetType());
}

void TabManager::NewSubTab(int posn) {
    int topTab = currentIndex();
    if (topTab < 0)
        return;
    newFrontTab(topTab, posn);
}

// Catch any change in the top tab, update the eventRouter of the sub tab.
// Also provide signal indicating tab changed
//
void TabManager::NewTopTab(int newFrontPosn) {

    if (newFrontPosn < 0)
        return;
    if (_prevTopTab >= 0)
        _prevFrontPage[_prevTopTab] = _currentFrontPage[_prevTopTab];
    int subTabIndex = _currentFrontPage[newFrontPosn];
    if (subTabIndex < 0)
        return;
    _prevTopTab = _currentTopTab;
    _currentTopTab = newFrontPosn;

    string tag = _widgetTags[newFrontPosn][subTabIndex];
    if (_prevTopTab != _currentTopTab ||
        (_prevTopTab >= 0 && _prevFrontPage[_prevTopTab] != _currentFrontPage[_prevTopTab])) {
        emit tabLeft(_prevTopTab, _prevFrontPage[_prevTopTab]);
    }
    EventRouter *eRouter = VizWinMgr::getInstance()->GetEventRouter(tag);

    QWidget *wid = dynamic_cast<QWidget *>(eRouter);
    if (wid && wid->isVisible()) {
        eRouter->updateTab();
        emit ActiveEventRouterChanged(eRouter->GetType());
    }
}

void TabManager::setActive(
    string activeViz, string renderClass, string renderInst) {

    if (renderClass.empty() || renderInst.empty()) {
        HideRenderWidgets();
        return;
    }

    ShowRenderWidget(renderClass);

    RenderEventRouter *eRouter = VizWinMgr::getInstance()->GetRenderEventRouter(
        activeViz, renderClass, renderInst);

    eRouter->SetActive(renderInst);

    eRouter->updateTab();

    emit ActiveEventRouterChanged(eRouter->GetType());
}

void TabManager::newRenderer(
    string activeViz, string renderClass, string renderInst) {
    if (renderClass.empty() || renderInst.empty()) {
        HideRenderWidgets();
        return;
    }

    ShowRenderWidget(renderClass);

    RenderEventRouter *er = VizWinMgr::getInstance()->GetRenderEventRouter(
        activeViz, renderClass, renderInst);

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    string winName, dataSetName, paramsType;
    bool status = paramsMgr->RenderParamsLookup(
        renderInst, winName, dataSetName, paramsType);

    AnimationParams *aParams = (AnimationParams *)paramsMgr->GetParams(
        AnimationParams::GetClassType());
    size_t ts = aParams->GetCurrentTimestep();

    DataStatus *dataStatus = _controlExec->GetDataStatus();

    RenderParams *rParams = er->GetActiveParams();
    size_t local_ts = dataStatus->MapGlobalToLocalTimeStep(dataSetName, ts);
    rParams->SetCurrentTimestep(local_ts);

    er->SetActive(renderInst);

    QWidget *w = dynamic_cast<QWidget *>(er);
    assert(w);
    w->setEnabled(true);

    er->updateTab();
}
