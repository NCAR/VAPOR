//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//					
//	File:		VizWinMgr.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Sept 2004
//
//	Description:  Implementation of VizWinMgr class	
//		This class manages the VizWin visualizers
//		Its main function is to catch events from the visualizers and
//		to route them to the appropriate params class, and in reverse,
//		to route events from tab panels to the appropriate visualizer.
//
#ifdef WIN32
#pragma warning(disable : 4251 4100)
#endif
#include <vapor/glutil.h>	// Must be included first!!!
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>
#include <cassert>
#include "GL/glew.h"
#include <qapplication.h>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include <vapor/ViewpointParams.h>
#include <vapor/regionparams.h>

#include "AnimationParams.h"
#include "MainForm.h"
#include "MouseModeParams.h"
#include "AnimationEventRouter.h"
#include "regioneventrouter.h"
#include "VizFeatureEventRouter.h"
#include "AppSettingsEventRouter.h"
#include "StartupEventRouter.h"
#include "ViewpointEventRouter.h"
#include "HelloEventRouter.h"
#include "TrackBall.h"
#include "VizWin.h"
#include "VizWinMgr.h"

//Extension tabs also included (until we find a nicer way to separate extensions)
#include "TwoDDataEventRouter.h"
#include "ImageEventRouter.h"
#include "BarbEventRouter.h"
#include "ContourEventRouter.h"
#ifdef	DEAD
#include "imageeventrouter.h"
//#include "arroweventrouter.h"
#include "isolineeventrouter.h"
#endif

using namespace VAPoR;
VizWinMgr* VizWinMgr::_vizWinMgr = NULL;

namespace {
string make_viz_name(vector <string> currentNames) {

	int index = 0;
	bool found = false;
	string name;
	while (! found) {
		std::stringstream out;
		out << index;
		name = "Visualizer_No._" + out.str(); 

		found = true;
		for (int i=0; i<currentNames.size(); i++) {
			if (currentNames[i] == name) found = false;
		}
		index++;
	}
	return(name);
}
};


VizWinMgr::VizWinMgr(ControlExec *ce)
{
	_controlExec = ce;
	_mainForm = MainForm::getInstance();
    _mdiArea = _mainForm->getMDIArea();
	_tabManager = TabManager::getInstance();
	setActiveViz("");
   
	_vizWindow.clear();
	_vizMdiWin.clear();
	
	m_trackBall = new Trackball();

	m_initialized = false;

	connect(
		this, SIGNAL(activateViz(const QString &)),
		_tabManager, SLOT(SetActiveViz(const QString &))
	);
	
}
//Create the global params and the default renderer params:
void VizWinMgr::createAllDefaultTabs() {

	QWidget *parent;

	// Install built-in tabs
	//
	parent = TabManager::getInstance()->GetSubTabWidget(2);
	EventRouter *er = new StartupEventRouter(parent, _controlExec);
	installTab(er->GetType(), 2, er);
	
	parent = TabManager::getInstance()->GetSubTabWidget(1);
	er = new AnimationEventRouter(parent, _controlExec);
	installTab(er->GetType(), 1, er);

	parent = TabManager::getInstance()->GetSubTabWidget(1);
	er = new ViewpointEventRouter(parent, this, _controlExec);
	installTab(er->GetType(), 1, er);

	parent = TabManager::getInstance()->GetSubTabWidget(1);
	er = new RegionEventRouter(parent, _controlExec);
	installTab(er->GetType(), 1, er);

	parent = TabManager::getInstance()->GetSubTabWidget(2);
	er = new VizFeatureEventRouter(parent, _controlExec);
	installTab(er->GetType(), 2, er);

	parent = TabManager::getInstance()->GetSubTabWidget(2);
	er = new AppSettingsEventRouter(parent, _controlExec);
	installTab(er->GetType(), 2, er);

	// Renderer tabs
	//
	parent = TabManager::getInstance()->GetSubTabWidget(0);
	er = new TwoDDataEventRouter(parent, _controlExec);
	installTab(er->GetType(), 0, er);

	parent = TabManager::getInstance()->GetSubTabWidget(0);
	er = new HelloEventRouter(parent, _controlExec);
	installTab(er->GetType(), 0, er);

	parent = TabManager::getInstance()->GetSubTabWidget(0);
	er = new BarbEventRouter(parent, this, _controlExec);
	installTab(er->GetType(), 0, er);

	parent = TabManager::getInstance()->GetSubTabWidget(0);
	er = new ImageEventRouter(parent, _controlExec);
	installTab(er->GetType(), 0, er);

	parent = TabManager::getInstance()->GetSubTabWidget(0);
	er = new ContourEventRouter(parent, this, _controlExec);
	installTab(er->GetType(), 0, er);

	//set up widgets in tabs:
	_tabManager->InstallWidgets();
}

/***********************************************************************
 *  Destroys the object and frees any allocated resources
 ***********************************************************************/
VizWinMgr::~VizWinMgr()
{
	
	std::map<string, VizWin*>::iterator it;
	for(it = _vizWindow.begin(); it != _vizWindow.end(); it++){
		VizWin* win = it->second;
		string winName= it->first;
		assert(win != 0);
		delete win;
		delete _vizMdiWin[winName];
		
	}
	if (m_trackBall) delete m_trackBall;
    
	
}

void VizWinMgr::vizAboutToDisappear(string vizName)  {

	std::map<string, VizWin*>::iterator itr = _vizWindow.find(vizName);
	if (itr == _vizWindow.end()) return;

	std::map<string,QMdiSubWindow*>::iterator itr2 = _vizMdiWin.find(vizName);
	assert(itr2 != _vizMdiWin.end());

	string activeViz = GetActiveVizName();

	
#ifdef	DEAD
	// Stop animation capture in this visualizer
	//
	MainForm::getInstance()->stopAnimCapture(vizName);
#endif

	// Remove the vizwin and the vizmdiwin
	//
	if (_vizWindow[vizName]) delete _vizWindow[vizName];
	_vizWindow.erase(vizName);

	if (_vizMdiWin[vizName]) delete _vizMdiWin[vizName];
	_vizMdiWin.erase(vizName);

	// If we are deleting the active viz, pick a new active viz
	//
	if (activeViz == vizName && _vizWindow.size()) {
		map <string, VizWin *>::iterator itr = _vizWindow.begin();
		setActiveViz(itr->first);
	}
	
	//Remove the visualizer
	_controlExec->RemoveVisualizer(vizName);

	//Remove the visualizer from the vizSelectCombo
	emit (removeViz(QString::fromStdString(vizName)));
	
	// When the number is reduced to 1, disable multiviz options.
	//
	if (_vizWindow.size() == 1) emit enableMultiViz(false);
	
}

void VizWinMgr::attachVisualizer(string vizName) {

	if (_vizWindow.find(vizName) != _vizWindow.end()) return;

    QPoint* topLeft = new QPoint(0,0);
    QSize* minSize = new QSize(400, 400);
    QRect* newRect = new QRect (*topLeft, *minSize);

    //Don't record events generated by window activation here: 
	
	QString qvizname = QString::fromStdString(vizName);
	_vizWindow[vizName] = new VizWin (
		MainForm::getInstance(), qvizname, this, newRect, vizName,
		_controlExec, m_trackBall
	
	);
	
	QMdiSubWindow* qsbw = MainForm::getInstance()->getMDIArea()->addSubWindow(_vizWindow[vizName]);
	_vizMdiWin[vizName]=qsbw;
	_vizWindow[vizName]->setFocusPolicy(Qt::ClickFocus);
	_vizWindow[vizName]->setWindowTitle(qvizname);

	setActiveViz(vizName);

	_vizWindow[vizName]->showMaximized();
	
	int numWins = _controlExec->GetNumVisualizers();
	//Tile if more than one visualizer:
	if(numWins > 1) fitSpace();

	//Set non-renderer tabbed panels to use global parameters:
	
#ifdef	DEAD
	emit activateViz(QString::fromStdString(vizName));
#endif
	_vizWindow[vizName]->show();

	//notify the window selector:
	emit (newViz(qvizname));

	//When we go from 1 to 2 windows, need to enable multiple viz panels and signals.
	if (numWins > 1){
		emit enableMultiViz(true);
	}

	//Initially, don't show any renderers
	//_tabManager->hideRenderWidgets();
}

void VizWinMgr::LaunchVisualizer()
{

	string vizName = make_viz_name(_controlExec->GetVisualizerNames());

	int rc = _controlExec->NewVisualizer(vizName);
	if (rc<0) {
		cerr << "ERROR MESSAGE" << endl;
#ifdef	DEAD
		emit remove vis
#endif
		return;
	}

		

	attachVisualizer(vizName);

}



/**************************************************************
 * Methods that arrange the viz windows:
 **************************************************************/
void 
VizWinMgr::cascade(){
   	_mdiArea->cascadeSubWindows(); 
	//Now size them up to a reasonable size:
	map<string,VizWin*>::iterator it;
	for (it = _vizWindow.begin(); it != _vizWindow.end(); it++){
		(it->second)->resize(400,400);
	}
}

void 
VizWinMgr::fitSpace(){
	
    _mdiArea->tileSubWindows();
	
}


int VizWinMgr::
getNumVisualizers(){
    return _vizWindow.size();
}

/**********************************************************************
 * Method called when the user makes a visualizer active:
 **********************************************************************/
void VizWinMgr::setActiveViz(string vizName){
	if (vizName.empty()) return;


	GUIStateParams *p = _mainForm->GetStateParams();
	string currentVizName = p->GetActiveVizName();
	if (currentVizName != vizName){

		p->SetActiveVizName(vizName);
		emit(activateViz(QString::fromStdString(vizName)));
		
		// Determine if the local viewpoint dialog applies, update the 
		// tab dialog appropriately
		//
		UpdateRouters();

		//Set the animation toolbar to the correct frame number:
		//
		ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
		int currentTS = _mainForm->GetAnimationParams()->GetCurrentTimestep();

		_tabManager->show();
		//Add to history if this is not during initial creation.
		
		//Need to cause a redraw in all windows if we are not in navigate mode,
		//So that the manips will change where they are drawn:

		MouseModeParams *p = _mainForm->GetStateParams()->GetMouseModeParams();
		if (p->GetCurrentMouseMode() != MouseModeParams::GetNavigateModeName()){
			map<string,VizWin*>::iterator it;
			for (it = _vizWindow.begin(); it != _vizWindow.end(); it++){
				(it->second)->updateGL();
			}
		}
	}
}

string VizWinMgr::GetActiveVizName() const {
	GUIStateParams *p = _mainForm->GetStateParams();
	return(p->GetActiveVizName());
}

vector <string> VizWinMgr::GetVisualizerNames() const {
	vector <string> names;
	std::map<string, VizWin*>::const_iterator itr = _vizWindow.begin();
	for (; itr != _vizWindow.end(); ++itr) {
		names.push_back(itr->first);
	}
	return(names);
}

//Method to enable closing of a vizWin
void VizWinMgr::killViz(string vizName){
	
	assert(_vizWindow.find(vizName) != _vizWindow.end());

	MainForm::getInstance()->getMDIArea()->removeSubWindow(_vizMdiWin[vizName]);

	// This will trigger a closeEvent on VizWin, which will in turn
	// call vizAboutToDisappear
	//
	_vizWindow[vizName]->setEnabled(false);
	_vizWindow[vizName]->close();


}



/*
 * Tell the parameter panels when there are or are not multiple viz's
 */

/********************************************************************
 *					SLOTS
 ********************************************************************/
/*
 * Slot that responds to user setting of VizSelectCombo
 */
void VizWinMgr::winActivated(const QString &qS){
	string vizName = qS.toStdString();
	//Truly make specified visualizer active:
	
	_vizWindow[vizName]->setFocus();
}

/*******************************************************************
 *	Slots associated with VizTab:
 ********************************************************************/


void VizWinMgr::SetTrackBall(
	const double posvec[3], const double dirvec[3],
	const double upvec[3], const double centerRot[3],
	bool perspective
) {
	ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
	paramsMgr->BeginSaveStateGroup("Navigate scene");

	std::map<string, VizWin*>::iterator itr;
	for(itr = _vizWindow.begin(); itr != _vizWindow.end(); itr++){
		VizWin *vw = itr->second;
		vw->SetTrackBall(posvec, dirvec, upvec, centerRot, true);
	}

	paramsMgr->EndSaveStateGroup();
}

#ifdef	DEAD
//Reset the near/far distances for all the windows that
//share a viewpoint, based on region in specified regionparams
//
void VizWinMgr::
resetViews(ViewpointParams* vp){
	int vizNum = vp->GetVizNum();
	if (vizNum>=0) {
		Visualizer* glw = _vizWindow[vizNum]->getVisualizer();
		if(glw) glw->resetNearFar(vp);
	}
	if(vp->IsLocal()) return;
	map<int,VizWin*>::iterator it;
	for (it = _vizWindow.begin(); it != _vizWindow.end(); it++){
		int i = it->first;
		Params* vpParams = (ViewpointParams*)_paramsMgr->GetParamsInstance(Params::_viewpointParamsTag,i, -1);
		if  ( (i != vizNum)  && ((!vpParams)||!vpParams->IsLocal())){
			Visualizer* glw = (it->second)->getVisualizer();
			if(glw) glw->resetNearFar(vp);
		}
	}
}
#endif


//Local global selector for all panels are similar.  First, Animation panel:
//
void VizWinMgr::
setAnimationLocalGlobal(int val){
#ifdef	DEAD
	//If changes to global, revert to global panel.
	//If changes to local, may need to create a new local panel
	int activeViz = _controlExec->GetActiveVizIndex();
	if (val == 0){//toGlobal.  
		//First set the global status, 
		//then put  values in tab based on global settings.
		//Note that updateDialog will trigger events changing values
		//on the current dialog
		AnimationParams* ap = (AnimationParams*)_paramsMgr->GetParamsInstance(Params::_animationParamsTag,activeViz,-1);
		assert(ap);
		getAnimationRouter()->SetLocal(ap,false);
		getAnimationRouter()->updateTab();
		_tabManager->show();
	} else { //Local: Do we need to create new parameters?
		 //need to revert to existing local settings:
		AnimationParams* ap = (AnimationParams*)_paramsMgr->GetParamsInstance(Params::_animationParamsTag,activeViz,-1);
		assert(ap);
		getAnimationRouter()->SetLocal(ap,true);
		getAnimationRouter()->updateTab();
			
	}
#endif
		
	//and then refresh the panel:
	_tabManager->show();
	
}



EventRouter* VizWinMgr::GetEventRouter(string erType) const {
	map <string, EventRouter*> :: const_iterator itr;
    itr = _eventRouterMap.find(erType);
	if (itr == _eventRouterMap.end()) {
		assert(0);
		return 0;
	}
	return itr->second;
}

RenderEventRouter* VizWinMgr::GetRenderEventRouter(
	string winName,  string renderType, string instName
)  const {


	map <string, EventRouter*> :: const_iterator itr;
    itr = _eventRouterMap.find(renderType);
	if (itr == _eventRouterMap.end()) {
		assert(0);
		return 0;
	}
	RenderEventRouter *er = dynamic_cast<RenderEventRouter*> (itr->second);
	assert(er);

	er->SetActive(instName);

	return er;
}

	
void VizWinMgr::registerEventRouter(const std::string tag, EventRouter* router){
	_eventRouterMap[tag] = router;
}


void VizWinMgr::installTab(
	const std::string tag, int tabType, EventRouter *eRouter
) {
	registerEventRouter(tag, eRouter);
	eRouter->hookUpTab();
	QWidget* tabWidget = dynamic_cast<QWidget*> (eRouter);
	assert(tabWidget);
	if (tag != AppSettingsParams::GetClassType() && tag != StartupParams::GetClassType()) {
		tabWidget->setEnabled(false);
	}
	_tabManager->AddWidget(tabWidget, tag, tabType);
	
}

vector <string> VizWinMgr::GetInstalledTabNames(bool renderOnly) {
	vector <string> keys;
	map <string, EventRouter *>::const_iterator itr;
	for (itr = _eventRouterMap.begin(); itr != _eventRouterMap.end(); ++itr) {
		RenderEventRouter *er = dynamic_cast<RenderEventRouter*> (itr->second);

		if (renderOnly && ! er) continue;

		keys.push_back(itr->first);
	}
	return(keys);
}

AnimationEventRouter* VizWinMgr::getAnimationRouter() {
	return ((AnimationEventRouter*)
		GetEventRouter(AnimationEventRouter::GetClassType())
	);
}

ViewpointEventRouter* VizWinMgr::getViewpointRouter() {
	return ((ViewpointEventRouter*)
		GetEventRouter(ViewpointEventRouter::GetClassType())
	);
}

RegionEventRouter* VizWinMgr::getRegionRouter() {
	return ((RegionEventRouter*)
		GetEventRouter(RegionEventRouter::GetClassType())
	);
}

void VizWinMgr::updateDirtyWindows(){
	map<string, VizWin*>::const_iterator it;
	for (it = _vizWindow.begin(); it != _vizWindow.end(); it++){
		(it->second)->updateGL();
	}
}

void VizWinMgr::setEnabled(bool onOff){
	_tabManager->setEnabled(onOff);
}

void VizWinMgr::Shutdown() {

	vector <string> vizNames = GetVisualizerNames();

	for (int i=0; i<vizNames.size(); i++) {
		killViz(vizNames[i]);
	}
	assert(_vizMdiWin.empty());
	assert(_vizWindow.empty());

	m_initialized = false;
}

void VizWinMgr::Restart() {

	// Must be shutdown before restarting
	//
	if (m_initialized) return;

	GUIStateParams *p = _mainForm->GetStateParams();
	p->SetActiveVizName("");

	vector <string> vizNames = _controlExec->GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		attachVisualizer(vizNames[i]);
	}
}

void VizWinMgr::removeVisualizer(string vizName){
	killViz(vizName);
	fitSpace();
}
#ifdef	DEAD
void VizWinMgr::resetTrackball(){
	vector <string> vizNames = _controlExec->GetVisualizerNames();
	for (int i = 0; i<vizNames.size(); i++){
		Visualizer* viz = _controlExec->GetVisualizer(vizNames[i]);
		viz->resetTrackball();
	}
}
#endif

void VizWinMgr::ReinitRouters() {

	DataStatus *dataStatus = _controlExec->getDataStatus();
	ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
	size_t ts = _mainForm->GetAnimationParams()->GetCurrentTimestep();

	vector <double> minExts, maxExts;
	dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);
	assert(minExts.size() == 3);
	assert(maxExts.size() == 3);

	double scale[3];
	scale[0] = scale[1] = scale[2] = max(
		maxExts[0]-minExts[0], (maxExts[1]-minExts[1])
	);
	m_trackBall->SetScale(scale);
	

	UpdateRouters();

	m_initialized = true;
}

void VizWinMgr::UpdateRouters() {
	if (! m_initialized) return;

	// First handle non-render event routers
	//
	vector <string> tabNames = GetInstalledTabNames();

	for (int i=0; i<tabNames.size(); i++) {
		string tab = tabNames[i];

		EventRouter* eRouter = GetEventRouter(tab);
		RenderEventRouter *reRouter = dynamic_cast<RenderEventRouter*>(eRouter);

		if (reRouter) continue;	// Skip render event routers

		QWidget* w = dynamic_cast<QWidget*> (eRouter);
		assert(w);
		w->setEnabled(true);

		eRouter->updateTab();
	}

	// Now handle the active render event router. 
	//
	GUIStateParams *p = _mainForm->GetStateParams();
	string activeViz = p->GetActiveVizName();

	string renderClass, instName;
    p->GetActiveRenderer(activeViz, renderClass, instName);

	if (activeViz.size() && renderClass.size() && instName.size()) {
		EventRouter* eRouter = GetRenderEventRouter(
			activeViz, renderClass, instName
		);

		QWidget* w = dynamic_cast<QWidget*> (eRouter);
		assert(w);
		w->setEnabled(true);

		eRouter->updateTab();
	}

}
