//************************************************************************
//									*
//		     Copyright (C)  2016				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		paramsmgr.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		October 2004
//
//	Description:	Implements the ParamsMgr class
//		This manages the collection of Params classes that are active in VAPOR
//
#include <iostream>
#include <sstream>
#include <fstream>

#include <vapor/ParamsMgr.h>
#include <vapor/ViewpointParams.h>
#include <vapor/AnimationParams.h>
#include <vapor/regionparams.h>

using namespace VAPoR;
//----------------------------------------------------------------------------
// Static member initialization.
//----------------------------------------------------------------------------
const string ParamsMgr::_rootTag = "VAPOR";
const string ParamsMgr::_globalTag = "Global";
const string ParamsMgr::_renderersTag = "Renderers";
const string ParamsMgr::_windowsTag = "Windows";

void ParamsMgr::_init(vector<string> appParams, XmlNode *node)
{
    _renderParamsMap.clear();

    if (node) {
        if (node->GetTag() != _rootTag) { node->SetTag(_rootTag); }
        _rootSeparator = new ParamsSeparator(&_ssave, node);
    } else {
        _rootSeparator = new ParamsSeparator(&_ssave, _rootTag);
        node = _rootSeparator->GetNode();
    }

    // State save class needs root node of state tree
    //
    _ssave.Reinit(node);

    XmlNode *child = node->GetChild(_globalTag);
    if (child) {
        _otherParams = new ParamsContainer(&_ssave, child);
    } else {
        _otherParams = new ParamsContainer(&_ssave, _globalTag);
        _otherParams->SetParent(_rootSeparator);
    }

    if (!_otherParams->GetParams(AnimationParams::GetClassType())) { _otherParams->Create(AnimationParams::GetClassType(), AnimationParams::GetClassType()); }

    if (!_otherParams->GetParams(RegionParams::GetClassType())) { _otherParams->Create(RegionParams::GetClassType(), RegionParams::GetClassType()); }

    if (!_otherParams->GetParams(VizFeatureParams::GetClassType())) { _otherParams->Create(VizFeatureParams::GetClassType(), VizFeatureParams::GetClassType()); }

    // Deal with any Params registered by the application
    //
    for (int i = 0; i < appParams.size(); i++) {
        if (!_otherParams->GetParams(appParams[i])) { _otherParams->Create(appParams[i], appParams[i]); }
    }
}

ParamsMgr::ParamsMgr()
{
    _dataStatus = NULL;
    _appParamNames.clear();

    _ssave.SetEnabled(false);
    _init(vector<string>(), NULL);
    _ssave.SetEnabled(true);
}

ParamsMgr::ParamsMgr(std::vector<string> appParamNames)
{
    _dataStatus = NULL;
    _appParamNames = appParamNames;

    _ssave.SetEnabled(false);
    _init(appParamNames, NULL);
    _ssave.SetEnabled(true);
}

void ParamsMgr::_destroy()
{
    // Delete all of the render containers
    //
    delete_ren_containers();

    // Now delete all of the viewpoint params
    //
    map<string, ViewpointParams *>::iterator vpitr;
    for (vpitr = _viewpointParamsMap.begin(); vpitr != _viewpointParamsMap.end(); ++vpitr) {
        if (vpitr->second) delete vpitr->second;
    }
    _viewpointParamsMap.clear();

    if (_otherParams) delete _otherParams;

    if (_rootSeparator) delete _rootSeparator;
}

ParamsMgr::~ParamsMgr() { _destroy(); }

void ParamsMgr::LoadState()
{
    _destroy();

    _init(_appParamNames, NULL);

    // If data loaded set up data dependent parameters from default state.
    //
    if (_dataStatus && _dataStatus->GetDataMgr()) { setDataStatusNew(_dataStatus); }
}

void ParamsMgr::LoadState(const XmlNode *node)
{
    _destroy();

    XmlNode *mynode = new XmlNode(*node);

    _init(_appParamNames, mynode);

    ParamsSeparator *windowsSep = new ParamsSeparator(_rootSeparator, _windowsTag);

    XmlNode *winNode = windowsSep->GetNode();
    for (int i = 0; i < winNode->GetNumChildren(); i++) {
        string winName = winNode->GetChild(i)->GetTag();

        (void)make_vp_params(winName);
    }

    delete windowsSep;

    // If data loaded set up data dependent parameters from new state.
    //
    if (_dataStatus && _dataStatus->GetDataMgr()) { setDataStatusMerge(_dataStatus); }
}

int ParamsMgr::LoadState(string stateFile)
{
    XmlParser parser;
    XmlNode   node;

    int rc = parser.LoadFromFile(&node, stateFile);
    if (rc < 0) {
        SetErrMsg("Invalid session file : %s", stateFile.c_str());
        return (-1);
    }

    LoadState(&node);
    return (0);
}

void ParamsMgr::setDataStatusNew(DataStatus *dataStatus)
{
    assert(dataStatus != NULL);

    _dataStatus = dataStatus;

    // Delete all of the renderer containers
    //
    delete_ren_containers();
}

void ParamsMgr::setDataStatusMerge(DataStatus *dataStatus)
{
    assert(dataStatus != NULL);

    _dataStatus = dataStatus;

    ParamsSeparator *windowsSep = new ParamsSeparator(_rootSeparator, _windowsTag);

    XmlNode *winSepNode = windowsSep->GetNode();
    for (int i = 0; i < winSepNode->GetNumChildren(); i++) {
        XmlNode *winNode = winSepNode->GetChild(i);
        string   winName = winNode->GetTag();

        if (winNode->HasChild(_renderersTag)) {
            XmlNode *renderersNode = winNode->GetChild(_renderersTag);

            for (int j = 0; j < renderersNode->GetNumChildren(); j++) { (void)make_ren_container(winName, renderersNode->GetChild(j)->GetTag()); }
        }
    }

    delete windowsSep;
}

void ParamsMgr::SetDataStatus(DataStatus *dataStatus)
{
    // If state already exist use it. Otherwise, create new, default
    // state
    //
    if (_rootSeparator->HasChild(_windowsTag)) {
        setDataStatusMerge(dataStatus);
    } else {
        setDataStatusNew(dataStatus);
    }
}

ViewpointParams *ParamsMgr::CreateVisualizerParamsInstance(string winName)
{
    if (!_dataStatus) {
        SetErrMsg("Invalid state : no DataStatus");
        return (NULL);
    }

    _ssave.BeginGroup("CreateVisualizerParamsInstance");

    ViewpointParams *vpParams = get_vp_params(winName);
    if (!vpParams) { vpParams = make_vp_params(winName); }
    assert(vpParams != NULL);

    _ssave.EndGroup();

    return (vpParams);
}

RenderParams *ParamsMgr::CreateRenderParamsInstance(string winName, string className, string instName)
{
    if (!_dataStatus || !_dataStatus->GetDataMgr()) {
        SetErrMsg("Invalid state : no DataStatus");
        return (NULL);
    }

    // Create ViewpointParams if we don't have one
    //
    ViewpointParams *vpParams = get_vp_params(winName);
    if (!vpParams) { vpParams = CreateVisualizerParamsInstance(winName); }
    assert(vpParams != NULL);

    _ssave.BeginGroup("CreateRenderParamsInstance");

    RenParamsContainer *container = get_ren_container(winName, className);
    if (!container) { container = make_ren_container(winName, className); }
    assert(container != NULL);

    RenderParams *rp = container->GetParams(instName);
    if (!rp) { rp = container->Create(className, instName); }

    _ssave.EndGroup();

    if (!rp) {
        SetErrMsg("Invalid derived RenderParams  class name %s", className.c_str());
        return (NULL);
    }

    return (rp);
}

RenderParams *ParamsMgr::CreateRenderParamsInstance(string winName, string instName, const RenderParams *rp)
{
    assert(rp);

    if (!_dataStatus || !_dataStatus->GetDataMgr()) {
        SetErrMsg("Invalid state : no DataStatus");
        return (NULL);
    }

    string className = rp->GetName();

    // Create ViewpointParams if we don't have one
    //
    ViewpointParams *vpParams = get_vp_params(winName);
    if (!vpParams) { vpParams = CreateVisualizerParamsInstance(winName); }
    assert(vpParams != NULL);

    _ssave.BeginGroup("InsertRenderParamsInstance");

    RenParamsContainer *container = get_ren_container(winName, className);
    if (!container) { container = make_ren_container(winName, className); }
    assert(container != NULL);

    RenderParams *newRP = container->Insert(rp, instName);

    _ssave.EndGroup();

    if (!newRP) {
        SetErrMsg("Invalid derived RenderParams  class name %s", className.c_str());
        return (NULL);
    }

    return (newRP);
}

void ParamsMgr::RemoveRenderParamsInstance(string winName, string className, string instName)
{
    RenParamsContainer *container = get_ren_container(winName, className);
    if (!container) return;

    container->Remove(instName);
}

RenderParams *ParamsMgr::GetRenderParams(string winName, string className, string instName) const
{
    RenParamsContainer *container = get_ren_container(winName, className);
    if (!container) { return (NULL); }

    return (container->GetParams(instName));
}

vector<string> ParamsMgr::GetVisualizerNames() const
{
    vector<string> vizNames;

    map<string, ViewpointParams *>::const_iterator itr;
    for (itr = _viewpointParamsMap.begin(); itr != _viewpointParamsMap.end(); ++itr) { vizNames.push_back(itr->first); }

    return (vizNames);
}

vector<string> ParamsMgr::GetRenderParamsClassNames(string winName) const
{
    vector<string> rClassNames;

    map<string, map<string, RenParamsContainer *>>::const_iterator itr1;
    itr1 = _renderParamsMap.find(winName);
    if (itr1 == _renderParamsMap.end()) return (rClassNames);

    const map<string, RenParamsContainer *> &         ref = itr1->second;
    map<string, RenParamsContainer *>::const_iterator itr2;
    for (itr2 = ref.begin(); itr2 != ref.end(); ++itr2) { rClassNames.push_back(itr2->first); }

    return (rClassNames);
}

vector<string> ParamsMgr::GetRenderParamInstances(string winName, string className) const
{
    vector<string>      instances;
    RenParamsContainer *rpc = get_ren_container(winName, className);
    if (rpc) { instances = rpc->GetNames(); }
    return (instances);
}

ViewpointParams *ParamsMgr::GetViewpointParams(string winName) const { return (get_vp_params(winName)); }

#ifdef DEAD
void ParamsMgr::InsertRenderParamsInstance(RenderParams *rp, string winName, string instName)
{
    string className = rp->GetName();

    RenParamsContainer *container = get_ren_container(winName, className);
    if (!container) { container = make_ren_container(className, winName); }
    assert(container != NULL);

    container->Insert(rp, instName);
}
#endif

RenParamsContainer *ParamsMgr::get_ren_container(string winName, string renderName) const
{
    map<string, map<string, RenParamsContainer *>>::const_iterator itr1;
    itr1 = _renderParamsMap.find(winName);
    if (itr1 == _renderParamsMap.end()) return (NULL);

    const map<string, RenParamsContainer *> &         ref = itr1->second;
    map<string, RenParamsContainer *>::const_iterator itr2;

    itr2 = ref.find(renderName);
    if (itr2 == ref.end()) return (NULL);

    return (itr2->second);
}

void ParamsMgr::delete_ren_container(string winName, string renderName)
{
    map<string, map<string, RenParamsContainer *>>::iterator itr1;
    itr1 = _renderParamsMap.find(winName);
    if (itr1 == _renderParamsMap.end()) return;

    map<string, RenParamsContainer *> &         ref = itr1->second;
    map<string, RenParamsContainer *>::iterator itr2;
    itr2 = ref.find(renderName);
    if (itr2 == ref.end()) return;

    RenParamsContainer *rpc = itr2->second;
    ref.erase(itr2);

    if (!rpc) return;

    // Set parent to root so  Xml representation will be deleted
    //
    rpc->GetNode()->SetParent(NULL);
    delete rpc;
}

void ParamsMgr::delete_ren_containers(string winName)
{
    map<string, map<string, RenParamsContainer *>>::iterator itr1;
    itr1 = _renderParamsMap.find(winName);

    if (itr1 == _renderParamsMap.end()) return;
    map<string, RenParamsContainer *> &ref = itr1->second;

    // Delete each render container associated with this window name
    //
    map<string, RenParamsContainer *>::iterator itr2;
    while ((itr2 = ref.begin()) != ref.end()) { delete_ren_container(winName, itr2->first); }

    _renderParamsMap.erase(itr1);
}

void ParamsMgr::delete_ren_containers()
{
    // For each window name delete all of the windows render containers
    //
    map<string, map<string, RenParamsContainer *>>::iterator itr;
    while ((itr = _renderParamsMap.begin()) != _renderParamsMap.end()) { delete_ren_containers(itr->first); }
}

RenParamsContainer *ParamsMgr::make_ren_container(string winName, string renderName)
{
    assert(_dataStatus != NULL);

    ParamsSeparator *windowsSep = new ParamsSeparator(_rootSeparator, _windowsTag);

    ParamsSeparator *windowSep = new ParamsSeparator(windowsSep, winName);

    ParamsSeparator *renderSep = new ParamsSeparator(windowSep, _renderersTag);

    // Delete any existing occurences with the same name
    //
    RenParamsContainer *rpc = get_ren_container(winName, renderName);
    if (rpc) delete rpc;

    if (renderSep->HasChild(renderName)) {
        XmlNode *node = renderSep->GetNode()->GetChild(renderName);
        assert(node);

        rpc = new RenParamsContainer(_dataStatus->GetDataMgr(), &_ssave, node);
    } else {
        rpc = new RenParamsContainer(_dataStatus->GetDataMgr(), &_ssave, renderName);
        rpc->GetSeparator()->SetParent(renderSep);
    }

    _renderParamsMap[winName][renderName] = rpc;

    delete windowsSep;
    delete windowSep;
    delete renderSep;

    return (rpc);
}

ViewpointParams *ParamsMgr::get_vp_params(string winName) const
{
    map<string, ViewpointParams *>::const_iterator itr;
    itr = _viewpointParamsMap.find(winName);
    if (itr == _viewpointParamsMap.end()) return (NULL);

    return (itr->second);
}

ViewpointParams *ParamsMgr::make_vp_params(string winName)
{
    assert(_dataStatus != NULL);

    ParamsSeparator *windowsSep = new ParamsSeparator(_rootSeparator, _windowsTag);

    ParamsSeparator *windowSep = new ParamsSeparator(windowsSep, winName);

    // Delete any existing occurences with the same name
    //
    ViewpointParams *vpParams = get_vp_params(winName);
    if (vpParams) delete vpParams;

    if (windowSep->HasChild(ViewpointParams::GetClassType())) {
        XmlNode *node = windowSep->GetNode()->GetChild(ViewpointParams::GetClassType());
        assert(node);

        vpParams = new ViewpointParams(&_ssave, node);
    } else {
        vpParams = new ViewpointParams(&_ssave);
        vpParams->SetParent(windowSep);
    }

    _viewpointParamsMap[winName] = vpParams;

    delete windowsSep;
    delete windowSep;

    return (vpParams);
}

#ifdef DEAD
//----------------------------------------------------------------------------
// Create a transfer function by parsing a file.
//----------------------------------------------------------------------------
int ParamsMgr::loadFromFile(string path)
{
    XmlParser xmlparser;

    XmlNode *node = GetNode();

    int rc = xmlparser.LoadFromFile(node, path);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to read file %s : %M", path.c_str());
        return (-1);
    }
    return (0);
}

#endif

//----------------------------------------------------------------------------
// Save the transfer function to a file.
//----------------------------------------------------------------------------
int ParamsMgr::SaveToFile(string path) const
{
    ofstream out(path);
    if (!out) {
        MyBase::SetErrMsg("Failed to open file %s : %M", path.c_str());
        return (-1);
    }

    out << *(_rootSeparator->GetNode());

    if (out.bad()) {
        MyBase::SetErrMsg("Failed to write file %s : %M", path.c_str());
        return (-1);
    }
    out.close();

    return (0);
}

bool ParamsMgr::undoRedoHelper()
{
    string description;

    // Get top of **undo** stack
    //
    const XmlNode *newNode = _ssave.GetTop(description);
    if (!newNode) return (false);    // nothing to undo

    // Need to disable state saving so the undo itself doesn't trigger
    // saving of intermediate state
    //
    bool saveState = GetSaveStateEnabled();
    SetSaveStateEnabled(false);

    // Load the new Xml tree (which destroys the old one)
    //
    LoadState(newNode);

    // Restore state saving
    //
    SetSaveStateEnabled(saveState);

    return (true);
}

bool ParamsMgr::Undo()
{
    (void)_ssave.Undo();

    bool status = undoRedoHelper();
    if (!status) return (status);

    return (true);
}

bool ParamsMgr::Redo()
{
    bool status = _ssave.Redo();

    if (!status) return (status);

    return (undoRedoHelper());
}

void ParamsMgr::UndoRedoClear() { _ssave.Clear(); }

ParamsMgr::PMgrStateSave::PMgrStateSave(int stackSize) : StateSave()
{
    _enabled = true;
    _stackSize = stackSize;
    _rootNode = NULL;
    _undoStack.clear();
    _redoStack.clear();
}

ParamsMgr::PMgrStateSave::~PMgrStateSave()
{
    cleanStack(0, _undoStack);
    cleanStack(0, _redoStack);
}

void ParamsMgr::PMgrStateSave::Save(const XmlNode *node, string description)
{
    assert(_rootNode);
    assert(node);

    if (!GetEnabled()) return;

    // Only save state if the node is a branch of the tree rooted at
    // the node named by _rootTag
    //
    vector<string> pathvec = node->GetPathVec();
    if ((!pathvec.size()) || (pathvec[0] != _rootTag)) { return; }

    const XmlNode *topNode = NULL;
    string         s;
    topNode = GetTop(s);
    if (topNode && (*topNode == *_rootNode)) {
        // Don't save tree if no changes
        //
        return;
    }

    // Set state change flags
    //
    for (int i = 0; i < _stateChangeFlags.size(); i++) { *(_stateChangeFlags[i]) = true; }

    if (!_groups.empty()) { return; }

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _undoStack);

    // It not inside a group push this element onto the stack
    //
    _undoStack.push_back(make_pair(description, new XmlNode(*_rootNode)));
//#define DEBUG
#ifdef DEBUG
    cout << "ParamsMgr::PMgrStateSave::Save() : saving node " << node->GetTag() << " : " << description << endl;
#endif
}

void ParamsMgr::PMgrStateSave::BeginGroup(string description)
{
    assert(_rootNode);
    assert(!description.empty());
    if (!GetEnabled()) return;

    _groups.push(description);
}

void ParamsMgr::PMgrStateSave::EndGroup()
{
    assert(_rootNode);

    if (!GetEnabled()) return;

    if (!_groups.size()) return;    // BeginGroup() not called

    string desc = _groups.top();
    _groups.pop();

    // Don't do anything until _groups is empty
    //
    if (_groups.size()) return;

    const XmlNode *topNode = NULL;
    string         s;
    topNode = GetTop(s);

    if (topNode && (*topNode == *_rootNode)) {
        // Don't save tree if no changes
        //
        return;
    }

#ifdef DEBUG
    cout << "ParamsMgr::PMgrStateSave::EndGroup() : saving "
         << " : " << desc << endl;
#endif

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _undoStack);

    _undoStack.push_back(make_pair(desc, new XmlNode(*_rootNode)));
}

const XmlNode *ParamsMgr::PMgrStateSave::GetTop(string &description) const
{
    assert(_rootNode);
    description.clear();

    if (!_undoStack.size()) return (NULL);

    const pair<string, XmlNode *> &p1 = _undoStack.back();

    description = p1.first;
    return (p1.second);
}

bool ParamsMgr::PMgrStateSave::Undo()
{
    assert(_rootNode);

    if (!_undoStack.size()) return (false);

    pair<string, XmlNode *> &p1 = _undoStack.back();

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _redoStack);

    _redoStack.push_back(p1);

    _undoStack.pop_back();

    return (true);
}

bool ParamsMgr::PMgrStateSave::Redo()
{
    assert(_rootNode);

    if (!_redoStack.size()) return (false);

    pair<string, XmlNode *> &p1 = _redoStack.back();

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _undoStack);

    _undoStack.push_back(p1);

    _redoStack.pop_back();

    return (true);
}

void ParamsMgr::PMgrStateSave::Clear()
{
    assert(_rootNode);

    cleanStack(0, _undoStack);
    cleanStack(0, _redoStack);
    while (_groups.size()) _groups.pop();
}

void ParamsMgr::PMgrStateSave::cleanStack(int maxN, std::deque<std::pair<string, XmlNode *>> &s)
{
    // Delete oldest elements if needed
    //
    while (s.size() > maxN) {
        pair<string, XmlNode *> &p1 = s.front();

        if (p1.second) { delete p1.second; }

        s.pop_front();
    }
}
