//-- ControlExec.cpp ----------------------------------------------------------------

// Implementation of ControlExec methods
//----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include <vapor/ParamsMgr.h>
#include <vapor/ControlExecutive.h>
#include <vapor/CalcEngineMgr.h>
#include <vapor/Visualizer.h>
#include <vapor/DataStatus.h>

#include <vapor/VolumeRenderer.h>
#include <vapor/VolumeIsoRenderer.h>


using namespace VAPoR;
using namespace std;



ControlExec::ControlExec(
	vector <string> appParamsNames, vector <string> appRenderParamNames,
	size_t cacheSizeMB, int nThreads
) : MyBase() {

	_paramsMgr = new ParamsMgr(appParamsNames, appRenderParamNames);
	_dataStatus = new DataStatus(cacheSizeMB, nThreads);
	_calcEngineMgr = new CalcEngineMgr(_dataStatus, _paramsMgr);
	_visualizers.clear();

}

ControlExec::~ControlExec() {

#ifdef  DEBUG
	cout << "Allocated XmlNode count before delete " <<
		XmlNode::GetAllocatedNodes().size() << endl;

	const vector <XmlNode *> &nodes = XmlNode::GetAllocatedNodes();
	for (int i=0; i<nodes.size(); i++) {
		cout << "   " << nodes[i]->GetTag() << " " << XmlNode::streamOut(cout, nodes[i]) << endl;
	}
#endif

	if (_paramsMgr) delete _paramsMgr;
	if (_dataStatus) delete _dataStatus;


#ifdef  DEBUG
	cout << "Allocated XmlNode count after delete " <<
		XmlNode::GetAllocatedNodes().size() << endl;

	for (int i=0; i<nodes.size(); i++) {
		cout << "   " << nodes[i]->GetTag() << " " << XmlNode::streamOut(cout, nodes[i]) << endl;
	}
#endif

}


int ControlExec::NewVisualizer(string winName) {

	_paramsMgr->BeginSaveStateGroup(GetNewVisualizerUndoTag());

	// Remove if already exists. Else no-op
	//
	RemoveVisualizer(winName);

	// Need to create a params instance for this visualizer
	//
	ViewpointParams *vpParams = _paramsMgr->CreateVisualizerParamsInstance(
		winName 
	);
	if (! vpParams) {
		SetErrMsg("Failed to create Visualizer parameters");
		_paramsMgr->EndSaveStateGroup();
		return -1;
	}

	Visualizer* viz = new Visualizer(_paramsMgr, _dataStatus, winName);
	_visualizers[winName] = viz;

	_paramsMgr->EndSaveStateGroup();

	return(0);
}

void ControlExec::RemoveVisualizer(string winName, bool hasOpenGLContext) {

	_paramsMgr->BeginSaveStateGroup(GetRemoveVisualizerUndoTag());

	RemoveAllRenderers(winName, hasOpenGLContext);

	std::map<string, Visualizer*>::iterator itr2 = _visualizers.find(winName);
	if (itr2 != _visualizers.end()) {
		delete itr2->second;
		_visualizers.erase(itr2);
		_paramsMgr->RemoveVisualizer(winName);
	}

	_paramsMgr->EndSaveStateGroup();
}
	
int ControlExec::InitializeViz(string winName, GLManager *glManager) {
	Visualizer* v = getVisualizer(winName);
	if (!v) {
		SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
		return -1;
	}

	if(v->InitializeGL(glManager) < 0) {
		SetErrMsg("InitializeGL failure");
		return -1;
	}
    
    _cachedVendor = glManager->GetVendor();

	return 0;
}

vector <string> ControlExec::GetVisualizerNames() const {
	vector <string> names; 

	std::map<string, Visualizer*>::const_iterator itr;
	for (itr=_visualizers.begin(); itr!=_visualizers.end(); ++itr) {
		names.push_back(itr->first);
	}
	return(names);
}


int ControlExec::ResizeViz(string winName, int width, int height){
	Visualizer* v = getVisualizer(winName);
	if (!v) {
		SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
		return -1;
	}
	if(v->resizeGL(width, height)<0) return -1;
	
	return 0;
}

GLManager::Vendor ControlExec::GetGPUVendor() const
{
    return _cachedVendor;
}

int ControlExec::Paint(string winName, bool fast){
	Visualizer* v = getVisualizer(winName);
	if (!v) {
		SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
		return -1;
	}

	// Disable state saving when generating the transfer function
	//
	bool enabled = _paramsMgr->GetSaveStateEnabled();
	_paramsMgr->SetSaveStateEnabled(false);

	int rc =  v->paintEvent(fast);

	_paramsMgr->SetSaveStateEnabled(enabled);


	if(rc) SetErrMsg("Error performing paint event");
	return rc;
}

int ControlExec::ActivateRender(
	string winName, string dataSetName, string renderType, 
	string renderName, bool on
) {
	if (! _dataStatus->GetDataMgrNames().size()) {
		SetErrMsg("Invalid state : no data");
		return -1;
	}

	Visualizer* v = getVisualizer(winName);
	if (!v) {
		SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
		return -1;
	}

	_paramsMgr->BeginSaveStateGroup(GetActivateRendererUndoTag());

	// Create new renderer if one does not exist
	//
	string paramsType = RendererFactory::Instance()->
			GetParamsClassFromRenderClass(renderType);

	if (! v->HasRenderer(renderType, renderName)) { 


		VAssert(! paramsType.empty());

		// Need to create a params instance for this renderer
		//
		RenderParams *rp = _paramsMgr->CreateRenderParamsInstance(
			winName, dataSetName, paramsType, renderName
		);
		if (! rp) {
			SetErrMsg("Invalid renderer of type \"%s\"",renderType.c_str());
			_paramsMgr->EndSaveStateGroup();
			return -1;
		}

		int rc = rp->Initialize();
		if (rc < 0) {
			SetErrMsg("Failed to initialize of type \"%s\"",renderType.c_str());
			_paramsMgr->EndSaveStateGroup();
			return(-1);
		}

		rc = v->CreateRenderer(dataSetName, renderType, renderName);
		if (rc<0) {
			SetErrMsg("Invalid renderer of type \"%s\"",renderType.c_str());
			_paramsMgr->EndSaveStateGroup();
			return(-1);
		}
	}


	// Get newly created (or existing) render params for this renderer
	//
	RenderParams *rp = _paramsMgr->GetRenderParams(
		winName, dataSetName, paramsType, renderName
	);
	VAssert(rp);

	rp->SetEnabled(on);
	v->MoveRendererToFront(renderType, renderName);
//    v->MoveRenderersOfTypeToFront(VolumeIsoRenderer::GetClassType());
    v->MoveRenderersOfTypeToFront(VolumeRenderer::GetClassType());

	_paramsMgr->EndSaveStateGroup();

	return 0;
}

int ControlExec::ActivateRender(
	string winName, string dataSetName, const RenderParams *rp, 
	string renderName, bool on
) {
	VAssert (rp);

	if (! _dataStatus->GetDataMgrNames().size()) {
		SetErrMsg("Invalid state : no data");
		return -1;
	}

	Visualizer* v = getVisualizer(winName);
	if (!v) {
		SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
		return -1;
	}

	string renderType = RendererFactory::Instance()->
		GetRenderClassFromParamsClass(rp->GetName());

	string paramsType = rp->GetName();

	_paramsMgr->BeginSaveStateGroup(GetActivateRendererUndoTag());

	// Create new renderer if one does not exist
	//
	if (! v->HasRenderer(renderType, renderName)) { 

		// Need to create a params instance for this renderer
		//
		RenderParams *newRP = _paramsMgr->CreateRenderParamsInstance(
			winName, dataSetName, renderName, rp
		);
		if (! newRP) {
			SetErrMsg("Invalid renderer of type \"%s\"",renderType.c_str());
			_paramsMgr->EndSaveStateGroup();
			return -1;
		}

		int rc = v->CreateRenderer(dataSetName, renderType, renderName);
		if (rc<0) {
			SetErrMsg("Invalid renderer of type \"%s\"",renderType.c_str());
			_paramsMgr->EndSaveStateGroup();
			return(-1);
		}
	}

	RenderParams *newRP = _paramsMgr->GetRenderParams(
		winName, dataSetName, paramsType, renderName
	);
	VAssert(newRP);

	newRP->SetEnabled(on);
	v->MoveRendererToFront(renderType, renderName);
//    v->MoveRenderersOfTypeToFront(VolumeIsoRenderer::GetClassType());
    v->MoveRenderersOfTypeToFront(VolumeRenderer::GetClassType());

	_paramsMgr->EndSaveStateGroup();

	return 0;
}

void ControlExec::_removeRendererHelper(
	string winName, string dataSetName, string pClassName, string renderName,
	bool removeFromParamsFlag, bool hasOpenGLContext
) {

	// No-op if tuple of winName, dataSetName, renderType, and
	// renderName is unknown
	//

	// Convert from params render type to render type. Sigh
	//
	string rClassName = RendererFactory::Instance()->GetRenderClassFromParamsClass(pClassName);

	RenderParams *rParams = _paramsMgr->GetRenderParams(
		winName, dataSetName, pClassName, renderName
	);
	if (! rParams) return;

	Visualizer* v = getVisualizer(winName);
	if (! v)  return;

	v->DestroyRenderer(rClassName, renderName, hasOpenGLContext);

	if (removeFromParamsFlag) {
		_paramsMgr->RemoveRenderParamsInstance(
				winName, dataSetName, pClassName, renderName
		);
	}
}

void ControlExec::RemoveRenderer(
	string winName, string dataSetName, string renderType, string renderName,
	bool hasOpenGLContext
) {

	string pClassName = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);

	RenderParams *rParams = _paramsMgr->GetRenderParams(
		winName, dataSetName, pClassName, renderName
	);
	if (! rParams) return;

	_paramsMgr->BeginSaveStateGroup(GetRemoveRendererUndoTag());

	rParams->SetEnabled(false);

	_removeRendererHelper(
		winName, dataSetName, pClassName, renderName, true, hasOpenGLContext
	);

	_paramsMgr->EndSaveStateGroup();
}

void ControlExec::RemoveAllRenderers(
	string winName, bool hasOpenGLContext
) {
	vector <string> dataSetNames = _paramsMgr->GetDataMgrNames();
	for (int k=0; k<dataSetNames.size(); k++) {

		vector <string> pClassNames = _paramsMgr->
			GetRenderParamsClassNames(winName, dataSetNames[k]);

		for (int j=0; j<pClassNames.size(); j++) {
			vector <string> instNames = _paramsMgr->GetRenderParamInstances(
				winName, dataSetNames[k], pClassNames[j]
			);

			for (int i=0; i<instNames.size(); i++) {

				_removeRendererHelper(
					winName, dataSetNames[k], pClassNames[j],
					instNames[i], true, hasOpenGLContext
				);
			}
		}
	}
}

void ControlExec::LoadState() {

	_paramsMgr->BeginSaveStateGroup("Load state");

	vector <string> vizNames = GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		RemoveVisualizer(vizNames[i]);
	}

	_paramsMgr->LoadState();
	_paramsMgr->EndSaveStateGroup();


}


void ControlExec::LoadState(const XmlNode *rootNode) {
	_paramsMgr->BeginSaveStateGroup("Load state");

	// Destroy current visualizers
	//
	vector <string> vizNames = GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		RemoveVisualizer(vizNames[i]);
	}

	// Set up param state
	//
	_paramsMgr->LoadState(rootNode);

	// Create new visualizers based on new parameter state
	//
	vizNames = _paramsMgr->GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		string winName = vizNames[i];
		Visualizer* viz = new Visualizer(_paramsMgr, _dataStatus, winName);
		_visualizers[winName] = viz;
	}
	_paramsMgr->EndSaveStateGroup();
}

int ControlExec::LoadState(string stateFile) {
	_paramsMgr->BeginSaveStateGroup("Load state");

	vector <string> vizNames = GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		RemoveVisualizer(vizNames[i]);
	}

	int rc = _paramsMgr->LoadState(stateFile);
	if (rc < 0) {
		_paramsMgr->EndSaveStateGroup();
		return(-1);
	}

	vizNames = _paramsMgr->GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		string winName = vizNames[i];
		Visualizer* viz = new Visualizer(_paramsMgr, _dataStatus, winName);
		_visualizers[winName] = viz;
	}

	_paramsMgr->EndSaveStateGroup();
	return(0);
}

void ControlExec::SetNumThreads(size_t nthreads) {
    _dataStatus->SetNumThreads(nthreads);
}

size_t ControlExec::GetNumThreads() const {
    return (_dataStatus->GetNumThreads());
}

void ControlExec::SetCacheSize(size_t sizeMB) {
    _dataStatus->SetCacheSize(sizeMB);
}

int ControlExec::activateClassRenderers(
	string vizName, string dataSetName, string pClassName, 
	vector <string> instNames, bool reportErrs
) {
	bool errEnabled = MyBase::GetEnableErrMsg();

	for (int i=0; i<instNames.size(); i++) {
		RenderParams *rp = _paramsMgr->GetRenderParams(
			vizName, dataSetName, pClassName, instNames[i]
		);
		VAssert(rp);

		// Convert from params render type to render type. Sigh
		//
		string rClassName = RendererFactory::Instance()->GetRenderClassFromParamsClass(pClassName);

		if (! reportErrs) {
			EnableErrMsg(false);
		}
			
		int rc = ActivateRender(
			vizName, dataSetName, rClassName, instNames[i], rp->IsEnabled()
		);
		if (rc<0) {
			SetErrMsg(
				"Failed to activate render: %s", instNames[i].c_str()
			);
			EnableErrMsg(errEnabled);
			return(rc);
		}
	}

	if (! reportErrs) {
		EnableErrMsg(errEnabled);
	}

	return(0);
}

int ControlExec::openDataHelper(bool reportErrs) {

	// Activate/Create renderers as needed. This is a no-op if renderers
	// already exist
	//
	vector <string> dataSetNames = _paramsMgr->GetDataMgrNames();
	vector <string> vizNames = _paramsMgr->GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {

		for (int j=0; j<dataSetNames.size(); j++) {
			vector <string> pClassNames = _paramsMgr->
				GetRenderParamsClassNames(vizNames[i], dataSetNames[j]);

			for (int k=0; k<pClassNames.size(); k++) {
				vector <string> instNames = 
					_paramsMgr->GetRenderParamInstances(
						vizNames[i], dataSetNames[j], pClassNames[k]
				);

				int rc = activateClassRenderers(
					vizNames[i], dataSetNames[j], pClassNames[k], instNames, 
					reportErrs
				);
				if (rc<0) return(rc);

			}
		}
	}

	// Rebuild from params database
	//
	_calcEngineMgr->ReinitFromState();
	return(0);
}

int ControlExec::OpenData(
	const std::vector <string> &files, const std::vector <string> &options,
	string dataSetName, string typ
) {

	// If re-opening an exising data set we need to close all of the
	// visualizers to force the renderers to be recreated with the
	// new data manager
	//
	if (_dataStatus->GetDataMgr(dataSetName)) {
		_dataStatus->Close(dataSetName);

		vector <string> vizNames = _paramsMgr->GetVisualizerNames();
		for (int k=0; k<vizNames.size(); k++) {

			vector <string> pClassNames = _paramsMgr->
				GetRenderParamsClassNames(vizNames[k], dataSetName);

			for (int j=0; j<pClassNames.size(); j++) {
				vector <string> instNames = _paramsMgr->GetRenderParamInstances(
					vizNames[k], dataSetName, pClassNames[j]
				);

				for (int i=0; i<instNames.size(); i++) {

					_removeRendererHelper(
						vizNames[k], dataSetName, pClassNames[j],
						instNames[i], false, false
					);
				}
			}
		}
	}

	int rc = _dataStatus->Open(files, options, dataSetName, typ);
	if (rc < 0) {
		SetErrMsg("Failure to open data set of type \"%s\"", typ.c_str());
		UndoRedoClear();
		return -1;
	}

	_paramsMgr->AddDataMgr(
		dataSetName, _dataStatus->GetDataMgr(dataSetName)
	);

	// Need to call initializers for any registered application renderers
	//
	vector <RenderParams *> appRenderParams;
	_paramsMgr->GetAppRenderParams(dataSetName, appRenderParams);
	for (int i=0; i<appRenderParams.size(); i++) {
		int rc = appRenderParams[i]->Initialize();
		if (rc<0) {
			_dataStatus->Close(dataSetName);
			_paramsMgr->RemoveDataMgr(dataSetName);
			SetErrMsg(
				"Failure to initialize application renderer \"%s\"",
				appRenderParams[i]->GetName().c_str()
			);
			UndoRedoClear();
			return(-1);
		}
	}

	// Re-initialize the ControlExec to match the new state
	//
	rc = openDataHelper(true);

	UndoRedoClear();
	return(rc);
}

void ControlExec::CloseData(string dataSetName) {


	if (! _dataStatus->GetDataMgr(dataSetName)) return; 


	// Remove any renderers associated with this data set
	//
	vector <string> winNames = GetVisualizerNames();
	for (int i=0; i<winNames.size(); i++) {
		vector <string> renderTypes = GetRenderClassNames(winNames[i]);
		for (int j=0; j<renderTypes.size(); j++) {
			vector <string> renderNames = GetRenderInstances(
				winNames[i], renderTypes[j]
			);
			for (int k=0; k<renderNames.size(); k++) {

				// This is a no-op if the tuple of window, data set,
				// render type, and render instance does not exist
				//
				RemoveRenderer(
					winNames[i], dataSetName, renderTypes[j], renderNames[k], false
				);
			}
		}
	}
		
	_paramsMgr->RemoveDataMgr(dataSetName);

	// Rebuild the calculation engine from params database after removing 
	// the data set from the params database.
	//
	_calcEngineMgr->ReinitFromState();

	_dataStatus->Close(dataSetName);
	

	UndoRedoClear();
}


int ControlExec::EnableImageCapture(string filename, string winName)
{
	Visualizer* v = getVisualizer(winName);
	if (!v) {
		SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
		return -1;
	}
	if(v->SetImageCaptureEnabled(true, filename)) 
    {
		SetErrMsg("Visualizer (%s) failed to enable capturing  image.", winName.c_str());
        return -1;
    }

	// Disable state saving when capturing an image
	//
	bool enabled = _paramsMgr->GetSaveStateEnabled();
	_paramsMgr->SetSaveStateEnabled(false);
    int rc =  v->paintEvent(false);   // paint with image capture enabled
	_paramsMgr->SetSaveStateEnabled(enabled);
    if( rc != 0 )
    {
		SetErrMsg("Visualizer (%s) failed to paint and thus not capturing image.", winName.c_str());
        return -1;
    }
	return 0;
}

int ControlExec::EnableAnimationCapture(
	string winName, bool onOff, string filename 
) {
		
	Visualizer* v = getVisualizer(winName);
	if (!v) {
		SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
		return -1;
	}

	if(v->SetAnimationCaptureEnabled(onOff, filename)) return -1;
	return 0;
}

string ControlExec::MakeStringConformant(string s) {
	if (s.empty()) s += "_";

	if (! (isalpha(s[0]) || s[0] == '_')) {
		s = "_" + s;
	}

	for (string::iterator itr = s.begin(); itr != s.end(); ++itr) {
		if (!
			(isalnum(*itr) ||
			 isdigit(*itr) ||
			 *itr == '-'   ||
			 *itr == '_'   ||
			 *itr == '.')) {

			*itr = '_';
		}
		if (isspace(*itr)) {
			*itr = '_';
		}
	}
	return(s);
}




bool ControlExec::_undoRedoRenderer(bool undoFlag) {

	// Get renderer parameters *before* undo/redo event
	//
	vector <string> beforeNames;
	map <string, vector <string> > beforeMap;
	_paramsMgr->GetRenderParamNames(beforeNames);
	std::sort(beforeNames.begin(), beforeNames.end());
	for (const auto &name: beforeNames) {
		string winName, dataSetName, className;
		bool status = _paramsMgr->RenderParamsLookup(name, winName, dataSetName, className);
		VAssert(status);
		beforeMap[name] = vector <string> {winName, dataSetName, className};
	}

	// Undo/Redo the event
	//
	bool status = true;
	if (undoFlag) {
		status = _paramsMgr->Undo();
	}
	else {
		status = _paramsMgr->Redo();
	}
	if (! status) return (status);

	// Get renderer parameters *after* undo/redo event
	//
	vector <string> afterNames;
	map <string, vector <string> > afterMap;
	_paramsMgr->GetRenderParamNames(afterNames);
	std::sort(afterNames.begin(), afterNames.end());
	for (const auto &name: afterNames) {
		string winName, dataSetName, className;
		bool status = _paramsMgr->RenderParamsLookup(name, winName, dataSetName, className);
		VAssert(status);
		afterMap[name] = vector <string> {winName, dataSetName, className};
	}

	// Generate list of renderers we need to create
	//
	map <string, vector <string> > createMap;
	std::set_difference(
		afterMap.begin(), afterMap.end(),
		beforeMap.begin(), beforeMap.end(),
		std::inserter(createMap, createMap.end())
	);


	// Generate list of renderers we need to destroy
	//
	map <string, vector <string> > destroyMap;
	std::set_difference(
		beforeMap.begin(), beforeMap.end(),
		afterMap.begin(), afterMap.end(),
		std::inserter(destroyMap, destroyMap.end())
	);

	// Disable state saving. This is probably not necessary as code below should not be
	// changing params state
	//
	bool enabled = GetSaveStateEnabled();
	SetSaveStateEnabled(false);

	// Create renderers
	//
	for (const auto &itr : createMap) {
        string renderType = RendererFactory::Instance()->GetRenderClassFromParamsClass(
			itr.second[2]
		);

		Visualizer* v = getVisualizer(itr.second[0]);
		VAssert(v);
        int rc = v->CreateRenderer(itr.second[1], renderType, itr.first);
		VAssert(rc >=0);

	}

	// Destroy renderers
	//
	for (const auto &itr : destroyMap) {

        string renderType = RendererFactory::Instance()->GetRenderClassFromParamsClass(
			itr.second[2]
		);

		Visualizer* v = getVisualizer(itr.second[0]);
		VAssert(v);

		v->DestroyRenderer(renderType, itr.first, false);
	}
	SetSaveStateEnabled(enabled);

	return(true);
}

bool ControlExec::_undoRedoVisualizer(bool undoFlag) {

	// Undo/Redo the event
	//
	bool status = true;
	if (undoFlag) {
		status = _paramsMgr->Undo();
	}
	else {
		status = _paramsMgr->Redo();
	}
	if (! status) return (status);

	bool enabled = GetSaveStateEnabled();
	SetSaveStateEnabled(false);

	// Destroy current visualizers
	//
	vector <string> vizNames = GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		RemoveVisualizer(vizNames[i]);
	}

	// Create new visualizers based on new parameter state
	//
	vizNames = _paramsMgr->GetVisualizerNames();
	for (int i=0; i<vizNames.size(); i++) {
		string winName = vizNames[i];
		Visualizer* viz = new Visualizer(_paramsMgr, _dataStatus, winName);
		_visualizers[winName] = viz;
	}

	// Data-dependent re-initialization
	//
	int rc = openDataHelper(false);
	VAssert(rc>=0);

	SetSaveStateEnabled(enabled);

	return(true);
}

bool ControlExec::Undo() {

	// Render and Visualizer Create/Destroy events require special handing for undo/redo
	// because renderer and visualizer creation is done explicity (not implicitly via 
	// Update(). The only way to determine if an undo/redo was for renderer of visualizer
	// event is to examine the comment associated with the state change.
	//
	bool renderEvent = (_paramsMgr->GetTopUndoDesc() == GetActivateRendererUndoTag()) ||
        (_paramsMgr->GetTopUndoDesc() == GetRemoveRendererUndoTag());

	bool visualizerEvent = ((_paramsMgr->GetTopUndoDesc() == GetRemoveVisualizerUndoTag()) ||
        (_paramsMgr->GetTopUndoDesc() == GetNewVisualizerUndoTag()));

	if (renderEvent)  {
		return(_undoRedoRenderer(true));
	}
	else if (visualizerEvent) {
		return(_undoRedoVisualizer(true));
	}
	else {
		return(_paramsMgr->Undo());
	}
}

bool ControlExec::Redo() {

	bool renderEvent = (_paramsMgr->GetTopRedoDesc() == GetActivateRendererUndoTag()) ||
        (_paramsMgr->GetTopRedoDesc() == GetRemoveRendererUndoTag());

	bool visualizerEvent = ((_paramsMgr->GetTopRedoDesc() == GetRemoveVisualizerUndoTag()) ||
        (_paramsMgr->GetTopRedoDesc() == GetNewVisualizerUndoTag()));

	if (renderEvent)  {
		return(_undoRedoRenderer(false));
	}
	else if (visualizerEvent) {
		return(_undoRedoVisualizer(false));
	}
	else {
		return(_paramsMgr->Redo());
	}
}

void ControlExec::UndoRedoClear() {
	_paramsMgr->UndoRedoClear();
}



int ControlExec::SaveSession(string filename)
{
	
	ofstream fileout;
	string s;
	
	fileout.open(filename.c_str());
	if (! fileout) {
		SetErrMsg("Unable to open output session file : %M");
		return(-1);
	}

	const XmlNode *node = _paramsMgr->GetXMLRoot();
	XmlNode::streamOut(fileout, *node);
	if (fileout.bad()) {
		SetErrMsg("Unable to write output session file : %M");
		return(-1);
	}

	fileout.close();
	return(0);
	
}


RenderParams *ControlExec::GetRenderParams(
    string winName, string dataSetName, string renderType,  string instName
) const {
	
	string paramsType = RendererFactory::Instance()->
		GetParamsClassFromRenderClass(renderType);

	RenderParams *rParams = _paramsMgr->GetRenderParams(
		winName, dataSetName, paramsType, instName
	);

	if (! rParams) {
		SetErrMsg("Invalid window name, render type, or instance name");
		return(NULL);
	}

	return(rParams);
}

vector<string> ControlExec::GetRenderClassNames(string winName) const {

	vector<string> v = _paramsMgr->GetRenderParamsClassNames(winName);

	for (int i=0; i<v.size(); i++) {
		v[i] = RendererFactory::Instance()->GetRenderClassFromParamsClass(v[i]);
	}

	return(v);
}

vector<string> ControlExec::GetRenderInstances(
	string winName, string renderType
) const {

	string paramsType = RendererFactory::Instance()->
		GetParamsClassFromRenderClass(renderType);

	return(_paramsMgr->GetRenderParamInstances(winName, paramsType));

}

vector <string> ControlExec::GetAllRenderClasses() {
    return(RendererFactory::Instance()->GetFactoryNames());
}

bool ControlExec::RenderLookup(
    string instName, string &winName, string &dataSetName, string &renderType
) const {

	string paramsType;
	bool ok = _paramsMgr->RenderParamsLookup(instName, winName, dataSetName, paramsType);
	if (! ok) return (ok);

	renderType = RendererFactory::Instance()->GetRenderClassFromParamsClass(paramsType);
	return(ok);

}

int ControlExec::DrawText(string winName, string text, 
						int x, int y, int size,	float color[3], int type) {
	Visualizer* v = getVisualizer(winName);
	if (v == NULL) {
		string msg = "Could not get Visualizer " + winName;
		SetErrMsg(msg.c_str());
		return -1;
	}

	v->DrawText(text, x, y, size, color, type);
	
	return 0;
}

int ControlExec::DrawText(string text, int x, 
						int y, int size, float color[3], int type) {
	vector<string> visNames = GetVisualizerNames();
	for (int i=0; i<visNames.size(); i++) {
		DrawText(visNames[i], text, x, y, size, color, type);
	}

	return 0;
}

int ControlExec::ClearText(string winName) {
	Visualizer* v = getVisualizer(winName);
	if (v == NULL) {
		string msg = "Could not get Visualizer " + winName;
		SetErrMsg(msg.c_str());
		return -1;
	}

	v->ClearText();
	
	return 0;
}

int ControlExec::ClearText() {
	vector<string> visNames = GetVisualizerNames();
	for (int i=0; i<visNames.size(); i++) {
		ClearText(visNames[i]);
	}

	return 0;
}

int ControlExec::AddFunction(
	string scriptType,
	string dataSetName,
	string scriptName,
	string script,
	const vector <string> &inputVarNames,
	const vector <string> &outputVarNames,
	const vector <string> &outputVarMeshes,
	bool coordFlag
) {

	// Ugh. Need to force each renderer to clear any cached data because
	// if we redefine a variable the variable's data will change but
	// the variable's name will not. Hence, if we don't clear the data
	// the renderer may continue using old data
	//
	std::map<string, Visualizer*>::const_iterator itr;
	for (itr=_visualizers.begin(); itr!=_visualizers.end(); ++itr) {
		itr->second->ClearRenderCache();
	}

	return(
		_calcEngineMgr->AddFunction(
			scriptType, dataSetName, scriptName, script,
			inputVarNames, outputVarNames, outputVarMeshes, coordFlag
		)
	);
}

void ControlExec::RemoveFunction(
	string scriptType,
	string dataSetName,
	string scriptName
) {
	return(
		_calcEngineMgr->RemoveFunction(
			scriptType, dataSetName, scriptName
		)
	);
}

bool ControlExec::GetFunction(
	string scriptType,
	string dataSetName,
	string scriptName,
	string &script,
	vector <string> &inputVarNames,
	vector <string> &outputVarNames,
	vector <string> &outputVarMeshes,
	bool &coordFlag
) const {
	script.clear();
	inputVarNames.clear();
	outputVarNames.clear();
	outputVarMeshes.clear();

	return(
		_calcEngineMgr->GetFunctionScript(
			scriptType, dataSetName, scriptName, script,
			inputVarNames, outputVarNames, outputVarMeshes,
			coordFlag
		)
	);
}

string ControlExec::GetFunctionStdout(
	string scriptType,
	string dataSetName,
	string scriptName
) const {

	return(_calcEngineMgr->GetFunctionStdout(
		scriptType, dataSetName, scriptName
	));
}


std::vector<string> ControlExec::GetFunctionNames(
	string scriptType,
	string dataSetName
) const {

	return(
		_calcEngineMgr->GetFunctionNames(scriptType, dataSetName)
	);
}

