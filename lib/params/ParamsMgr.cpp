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
#include <algorithm>

#include <vapor/ParamsMgr.h>
#include <vapor/ViewpointParams.h>
#include <vapor/regionparams.h>

using namespace VAPoR;
//----------------------------------------------------------------------------
// Static member initialization.
//----------------------------------------------------------------------------
const string ParamsMgr::_rootTag = "VAPOR";
const string ParamsMgr::_globalTag = "Global";
const string ParamsMgr::_windowsTag = "Windows";
const string ParamsMgr::_renderersTag = "Renderers";
const string ParamsMgr::_appRenderersTag = "AppRenderers";

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

    if (!_otherParams->GetParams(RegionParams::GetClassType())) { _otherParams->Create(RegionParams::GetClassType(), RegionParams::GetClassType()); }

    if (!_otherParams->GetParams(AnnotationParams::GetClassType())) { _otherParams->Create(AnnotationParams::GetClassType(), AnnotationParams::GetClassType()); }

    if (!_otherParams->GetParams(DatasetsParams::GetClassType())) { _otherParams->Create(DatasetsParams::GetClassType(), DatasetsParams::GetClassType()); }

    // Deal with any Params registered by the application
    //
    for (int i = 0; i < appParams.size(); i++) {
        if (!_otherParams->GetParams(appParams[i])) { _otherParams->Create(appParams[i], appParams[i]); }
    }
}

ParamsMgr::ParamsMgr(std::vector<string> appParamNames, std::vector<string> appRenderParamNames)
{
    _appParamNames = appParamNames;
    _appRenderParamNames = appRenderParamNames;
    _dataMgrMap.clear();

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

    map<string, RenParamsContainer *>::iterator itr;
    for (itr = _otherRenParams.begin(); itr != _otherRenParams.end(); ++itr) {
        if (itr->second) delete itr->second;
    }
    _otherRenParams.clear();

    if (_rootSeparator) delete _rootSeparator;
}

ParamsMgr::~ParamsMgr() { _destroy(); }

void ParamsMgr::LoadState()
{
    _destroy();

    _init(_appParamNames, NULL);

    // If data loaded set up data dependent parameters from default state.
    //
    if (_dataMgrMap.size()) { addDataMgrNew(); }
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
    vector<string> dataSetNames = GetDataMgrNames();
    for (auto dataSetName : dataSetNames) {
        map<string, DataMgr *>::const_iterator itr = _dataMgrMap.find(dataSetName);
        if (itr != _dataMgrMap.end()) addDataMgrMerge(itr->first);
    }
}

int ParamsMgr::LoadState(string stateFile)
{
    BeginSaveStateGroup("Load state");

    XmlParser parser;
    XmlNode   node;

    int rc = parser.LoadFromFile(&node, stateFile);
    if (rc < 0) {
        SetErrMsg("Invalid session file : %s", stateFile.c_str());
        EndSaveStateGroup();
        return (-1);
    }

    LoadState(&node);
    EndSaveStateGroup();
    return (0);
}

void ParamsMgr::addDataMgrNew()
{
    // Delete all of the renderer containers
    //
    delete_ren_containers();
}

void ParamsMgr::addDataMgrMerge(string dataSetName)
{
    ParamsSeparator *windowsSep = new ParamsSeparator(_rootSeparator, _windowsTag);

    XmlNode *winSepNode = windowsSep->GetNode();
    for (int i = 0; i < winSepNode->GetNumChildren(); i++) {
        XmlNode *winNode = winSepNode->GetChild(i);
        string   winName = winNode->GetTag();

        if (winNode->HasChild(_renderersTag)) {
            XmlNode *renderersNode = winNode->GetChild(_renderersTag);

            for (int j = 0; j < renderersNode->GetNumChildren(); j++) {
                XmlNode *dataSepNode = renderersNode->GetChild(j);
                string   s = dataSepNode->GetTag();

                if (s != dataSetName) continue;

                for (int k = 0; k < dataSepNode->GetNumChildren(); k++) { (void)make_ren_container(winName, dataSetName, dataSepNode->GetChild(k)->GetTag()); }
            }
        }
    }

    delete windowsSep;

    _createAppRenParams(dataSetName);
}

void ParamsMgr::_createAppRenParams(string dataSetName)
{
    // Deal with any application render Params registered by the application
    //

    ParamsSeparator *appRenParamsSep = new ParamsSeparator(_rootSeparator, _appRenderersTag);

    // Delete any existing container for this data set
    //
    map<string, RenParamsContainer *>::const_iterator itr;
    itr = _otherRenParams.find(dataSetName);
    RenParamsContainer *rpc = itr == _otherRenParams.end() ? NULL : itr->second;
    if (rpc) delete rpc;

    if (appRenParamsSep->HasChild(dataSetName)) {
        XmlNode *node = appRenParamsSep->GetNode()->GetChild(dataSetName);
        VAssert(node);

        rpc = new RenParamsContainer(_dataMgrMap[dataSetName], &_ssave, node);
    } else {
        rpc = new RenParamsContainer(_dataMgrMap[dataSetName], &_ssave, dataSetName);
        rpc->GetSeparator()->SetParent(appRenParamsSep);
    }

    delete appRenParamsSep;

    // Now add any application render params to the container for this
    // data set
    //
    for (int i = 0; i < _appRenderParamNames.size(); i++) {
        if (!rpc->GetParams(_appRenderParamNames[i])) { rpc->Create(_appRenderParamNames[i], _appRenderParamNames[i]); }
    }
    _otherRenParams[dataSetName] = rpc;
}

void ParamsMgr::AddDataMgr(string dataSetName, DataMgr *dataMgr)
{
    _dataMgrMap[dataSetName] = dataMgr;

    // If state already exist use it. Otherwise, create new, default
    // state
    //
    if (_rootSeparator->HasChild(_windowsTag)) {
        addDataMgrMerge(dataSetName);
    } else {
        addDataMgrNew();
    }

    ParamsSeparator windowsSep(_rootSeparator, _windowsTag);
    XmlNode *       winSepNode = windowsSep.GetNode();
    for (int i = 0; i < winSepNode->GetNumChildren(); i++) {
        string winName = winSepNode->GetChild(i)->GetTag();

        // Instantiating ParamsSeparator classes will add child nodes to the
        // XML tree as a side effect
        //
        ParamsSeparator windowSep(&windowsSep, winName);

        ParamsSeparator renderSep(&windowSep, _renderersTag);

        ParamsSeparator dataSep(&renderSep, dataSetName);
    }
}

void ParamsMgr::RemoveVisualizer(string winName)
{
    if (!_rootSeparator->HasChild(_windowsTag)) return;

    ParamsSeparator windowsSep(_rootSeparator, _windowsTag);

    if (!windowsSep.HasChild(winName)) return;

    _ssave.BeginGroup("CreateVisualizer");

    delete_ren_containers(winName);

    RemoveVisualizerParamsInstance(winName);

    ParamsSeparator windowSep(ParamsSeparator(&windowsSep, winName));

    // Set parent to root so  Xml representation will be deleted
    //
    windowSep.SetParent(NULL);

    _ssave.EndGroup();
}

void ParamsMgr::RemoveDataMgr(string dataSetName)
{
    map<string, DataMgr *>::iterator itr;
    itr = _dataMgrMap.find(dataSetName);
    if (itr == _dataMgrMap.end()) return;

    _dataMgrMap.erase(itr);

    delete_datasets(dataSetName);
}

vector<string> ParamsMgr::GetDataMgrNames() const
{
    vector<string> dataMgrNames;

    ParamsSeparator windowsSep(_rootSeparator, _windowsTag);

    XmlNode *winSepNode = windowsSep.GetNode();
    for (int i = 0; i < winSepNode->GetNumChildren(); i++) {
        XmlNode *winNode = winSepNode->GetChild(i);
        string   winName = winNode->GetTag();

        if (winNode->HasChild(_renderersTag)) {
            XmlNode *renderersNode = winNode->GetChild(_renderersTag);

            for (int j = 0; j < renderersNode->GetNumChildren(); j++) {
                XmlNode *dataSepNode = renderersNode->GetChild(j);
                string   s = dataSepNode->GetTag();

                dataMgrNames.push_back(s);
            }
        }
    }

    sort(dataMgrNames.begin(), dataMgrNames.end());
    dataMgrNames.erase(unique(dataMgrNames.begin(), dataMgrNames.end()), dataMgrNames.end());

    return (dataMgrNames);
}

ViewpointParams *ParamsMgr::CreateVisualizerParamsInstance(string winName)
{
    _ssave.BeginGroup("CreateVisualizerParamsInstance");

    ViewpointParams *vpParams = get_vp_params(winName);
    if (!vpParams) { vpParams = make_vp_params(winName); }
    VAssert(vpParams != NULL);

    _ssave.EndGroup();

    return (vpParams);
}

void ParamsMgr::RemoveVisualizerParamsInstance(string winName)
{
    _ssave.BeginGroup("RemoveVisualizerParamsInstance");

    map<string, ViewpointParams *>::const_iterator itr;
    itr = _viewpointParamsMap.find(winName);
    if (itr == _viewpointParamsMap.end()) return;

    ViewpointParams *vParams = itr->second;

    // Set parent to root so  Xml representation will be deleted
    //
    vParams->SetParent(NULL);
    delete vParams;

    _viewpointParamsMap.erase(itr);

    _ssave.EndGroup();
}

RenParamsContainer *ParamsMgr::createRenderParamsHelper(string winName, string dataSetName, string className, string instName)
{
    map<string, DataMgr *>::const_iterator itr;
    itr = _dataMgrMap.find(dataSetName);
    if (itr == _dataMgrMap.end()) {
        SetErrMsg("Invalid state : no data set");
        return (NULL);
    }

    vector<string> instNames;
    GetRenderParamNames(instNames);
    if (find(instNames.begin(), instNames.end(), instName) != instNames.end()) {
        string myWinName, myDataSetName, myClassName;
        (void)RenderParamsLookup(instName, myWinName, myDataSetName, myClassName);
        if (!(myWinName == winName) && (myDataSetName == dataSetName) && (myClassName == className)) {
            SetErrMsg("Non-unique render instance name : %s", instName.c_str());
            return (NULL);
        }
    }

    // Create ViewpointParams if we don't have one
    //
    ViewpointParams *vpParams = get_vp_params(winName);
    if (!vpParams) { vpParams = CreateVisualizerParamsInstance(winName); }
    VAssert(vpParams != NULL);

    RenParamsContainer *container = get_ren_container(winName, dataSetName, className);
    if (!container) { container = make_ren_container(winName, dataSetName, className); }
    VAssert(container != NULL);

    return (container);
}

RenderParams *ParamsMgr::CreateRenderParamsInstance(string winName, string dataSetName, string className, string instName)
{
    _ssave.BeginGroup("CreateRenderParamsInstance");

    RenParamsContainer *container = createRenderParamsHelper(winName, dataSetName, className, instName);
    if (!container) {
        _ssave.EndGroup();
        return (NULL);
    }

    RenderParams *rp = container->GetParams(instName);
    if (!rp) { rp = container->Create(className, instName); }

    _ssave.EndGroup();

    if (!rp) {
        SetErrMsg("Invalid derived RenderParams  class name %s", className.c_str());
        return (NULL);
    }

    return (rp);
}

RenderParams *ParamsMgr::CreateRenderParamsInstance(string winName, string dataSetName, string instName, const RenderParams *rp)
{
    VAssert(rp);

    _ssave.BeginGroup("CreateRenderParamsInstance");

    string              className = rp->GetName();
    RenParamsContainer *container = createRenderParamsHelper(winName, dataSetName, className, instName);
    if (!container) {
        _ssave.EndGroup();
        return (NULL);
    }

    RenderParams *newRP = container->Insert(rp, instName);

    _ssave.EndGroup();

    if (!newRP) {
        SetErrMsg("Invalid derived RenderParams  class name %s", className.c_str());
        return (NULL);
    }

    return (newRP);
}

void ParamsMgr::RemoveRenderParamsInstance(string winName, string dataSetName, string className, string instName)
{
    RenParamsContainer *container = get_ren_container(winName, dataSetName, className);
    if (!container) return;

    container->Remove(instName);
}

RenderParams *ParamsMgr::GetRenderParams(string winName, string dataSetName, string className, string instName) const
{
    RenParamsContainer *container = get_ren_container(winName, dataSetName, className);
    if (!container) { return (NULL); }

    return (container->GetParams(instName));
}

void ParamsMgr::GetRenderParams(string winName, string dataSetName, vector<RenderParams *> &rParams) const
{
    rParams.clear();

    const map<string, RenParamsContainer *> *m1Ptr;
    m1Ptr = getWinMap3(_renderParamsMap, winName, dataSetName);
    if (!m1Ptr) return;

    // m1Ptr[className]
    //
    const map<string, RenParamsContainer *> &         ref = *m1Ptr;
    map<string, RenParamsContainer *>::const_iterator itr;
    for (itr = ref.begin(); itr != ref.end(); ++itr) {
        RenParamsContainer *rpc = itr->second;
        vector<string>      names = rpc->GetNames();
        for (int i = 0; i < names.size(); i++) { rParams.push_back(rpc->GetParams(names[i])); }
    }
}

void ParamsMgr::GetRenderParams(string winName, vector<RenderParams *> &rParams) const
{
    rParams.clear();

    const map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(_renderParamsMap, winName);
    if (!m2Ptr) return;

    // m2Ptr[dataSetName][className]
    //
    const map<string, map<string, RenParamsContainer *>> &         ref = *m2Ptr;
    map<string, map<string, RenParamsContainer *>>::const_iterator itr;
    for (itr = ref.begin(); itr != ref.end(); ++itr) {
        vector<RenderParams *> tmp;
        GetRenderParams(winName, itr->first, tmp);
        rParams.insert(rParams.end(), tmp.begin(), tmp.end());
    }
}

void ParamsMgr::GetRenderParams(vector<RenderParams *> &rParams) const
{
    rParams.clear();

    map<string, map<string, map<string, RenParamsContainer *>>>::const_iterator itr;

    for (itr = _renderParamsMap.begin(); itr != _renderParamsMap.end(); ++itr) {
        vector<RenderParams *> tmp;
        GetRenderParams(itr->first, tmp);
        rParams.insert(rParams.end(), tmp.begin(), tmp.end());
    }
}

void ParamsMgr::GetRenderParamNames(string winName, string dataSetName, string className, vector<string> &instNames) const
{
    instNames.clear();

    RenParamsContainer *container = get_ren_container(winName, dataSetName, className);
    if (!container) return;

    instNames = container->GetNames();

    // Sanity check.  Names should always be unique!
    //
    sort(instNames.begin(), instNames.end());
    instNames.erase(unique(instNames.begin(), instNames.end()), instNames.end());
}

void ParamsMgr::GetRenderParamNames(string winName, string dataSetName, vector<string> &instNames) const
{
    instNames.clear();

    const map<string, RenParamsContainer *> *m1Ptr;
    m1Ptr = getWinMap3(_renderParamsMap, winName, dataSetName);
    if (!m1Ptr) return;

    // m1Ptr[className]
    //
    const map<string, RenParamsContainer *> &         ref = *m1Ptr;
    map<string, RenParamsContainer *>::const_iterator itr;
    for (itr = ref.begin(); itr != ref.end(); ++itr) {
        vector<string> tmp;
        GetRenderParamNames(winName, dataSetName, itr->first, tmp);

        instNames.insert(instNames.end(), tmp.begin(), tmp.end());
    }
    sort(instNames.begin(), instNames.end());
    instNames.erase(unique(instNames.begin(), instNames.end()), instNames.end());
}

void ParamsMgr::GetRenderParamNames(string winName, vector<string> &instNames) const
{
    instNames.clear();

    const map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(_renderParamsMap, winName);
    if (!m2Ptr) return;

    // m2Ptr[dataSetName][className]
    //
    const map<string, map<string, RenParamsContainer *>> &         ref = *m2Ptr;
    map<string, map<string, RenParamsContainer *>>::const_iterator itr;
    for (itr = ref.begin(); itr != ref.end(); ++itr) {
        vector<string> tmp;
        GetRenderParamNames(winName, itr->first, tmp);
        instNames.insert(instNames.end(), tmp.begin(), tmp.end());
    }
    sort(instNames.begin(), instNames.end());
    instNames.erase(unique(instNames.begin(), instNames.end()), instNames.end());
}

void ParamsMgr::GetRenderParamNames(vector<string> &instNames) const
{
    instNames.clear();

    map<string, map<string, map<string, RenParamsContainer *>>>::const_iterator itr;

    for (itr = _renderParamsMap.begin(); itr != _renderParamsMap.end(); ++itr) {
        vector<string> tmp;
        GetRenderParamNames(itr->first, tmp);
        instNames.insert(instNames.end(), tmp.begin(), tmp.end());
    }
    sort(instNames.begin(), instNames.end());
    instNames.erase(unique(instNames.begin(), instNames.end()), instNames.end());
}

bool ParamsMgr::RenderParamsLookup(string instName, string &winName, string &dataSetName, string &className) const
{
    winName.clear();
    dataSetName.clear();
    className.clear();

    // Exhaustively search through _renderParamsMap looking for an
    // occurrence of instName
    //
    map<string, map<string, map<string, RenParamsContainer *>>>::const_iterator itr1;
    for (itr1 = _renderParamsMap.begin(); itr1 != _renderParamsMap.end(); ++itr1) {
        map<string, map<string, RenParamsContainer *>>::const_iterator itr2;
        for (itr2 = itr1->second.begin(); itr2 != itr1->second.end(); ++itr2) {
            map<string, RenParamsContainer *>::const_iterator itr3;
            for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); ++itr3) {
                const RenParamsContainer &ref = *(itr3->second);
                vector<string>            instNames = ref.GetNames();

                if (find(instNames.begin(), instNames.end(), instName) != instNames.end()) {
                    winName = itr1->first;
                    dataSetName = itr2->first;
                    className = itr3->first;
                    return (true);
                }
            }
        }
    }
    return (false);
}

vector<string> ParamsMgr::GetVisualizerNames() const
{
    vector<string> vizNames;

    map<string, ViewpointParams *>::const_iterator itr;
    for (itr = _viewpointParamsMap.begin(); itr != _viewpointParamsMap.end(); ++itr) { vizNames.push_back(itr->first); }

    return (vizNames);
}

// m2[b][c] <- m3[a][b][c]
//
const map<string, map<string, RenParamsContainer *>> *ParamsMgr::getWinMap3(const map<string, map<string, map<string, RenParamsContainer *>>> &m3, string key) const
{
    // map[a][b][c]
    //
    map<string, map<string, map<string, RenParamsContainer *>>>::const_iterator itr;

    itr = m3.find(key);
    if (itr == m3.end()) return (NULL);

    return (&(itr->second));
}

// m1[c] <- m3[a][b][c]
//
const map<string, RenParamsContainer *> *ParamsMgr::getWinMap3(const map<string, map<string, map<string, RenParamsContainer *>>> &m3, string key1, string key2) const
{
    const map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(m3, key1);
    if (!m2Ptr) return (NULL);

    const map<string, RenParamsContainer *> *m1Ptr;
    m1Ptr = getWinMap2(*m2Ptr, key2);

    return (m1Ptr);
}

// m1[c] <- m2[b][c]
//
const map<string, RenParamsContainer *> *ParamsMgr::getWinMap2(const map<string, map<string, RenParamsContainer *>> &m2, string key) const
{
    // map[b][c]
    //
    map<string, map<string, RenParamsContainer *>>::const_iterator itr;

    itr = m2.find(key);
    if (itr == m2.end()) return (NULL);

    return (&(itr->second));
}

// m2[b][c] <- m3[a][b][c]
//
map<string, map<string, RenParamsContainer *>> *ParamsMgr::getWinMap3(map<string, map<string, map<string, RenParamsContainer *>>> &m3, string key) const
{
    // map[a][b][c]
    //
    map<string, map<string, map<string, RenParamsContainer *>>>::iterator itr;

    itr = m3.find(key);
    if (itr == m3.end()) return (NULL);

    return (&(itr->second));
}

// m1[c] <- m3[a][b][c]
//
map<string, RenParamsContainer *> *ParamsMgr::getWinMap3(map<string, map<string, map<string, RenParamsContainer *>>> &m3, string key1, string key2) const
{
    map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(m3, key1);
    if (!m2Ptr) return (NULL);

    map<string, RenParamsContainer *> *m1Ptr;
    m1Ptr = getWinMap2(*m2Ptr, key2);

    return (m1Ptr);
}

// m1[c] <- m2[b][c]
//
map<string, RenParamsContainer *> *ParamsMgr::getWinMap2(map<string, map<string, RenParamsContainer *>> &m2, string key) const
{
    // map[b][c]
    //
    map<string, map<string, RenParamsContainer *>>::iterator itr;

    itr = m2.find(key);
    if (itr == m2.end()) return (NULL);

    return (&(itr->second));
}

vector<string> ParamsMgr::GetRenderParamsClassNames(string winName, string dataSetName) const
{
    vector<string> rClassNames;

    // _renderParamsMap[winName][dataSetName][className]
    //

    const map<string, RenParamsContainer *> *m1Ptr;
    m1Ptr = getWinMap3(_renderParamsMap, winName, dataSetName);
    if (!m1Ptr) return (rClassNames);

    // m1Ptr[className]
    //
    const map<string, RenParamsContainer *> &ref = *m1Ptr;

    map<string, RenParamsContainer *>::const_iterator itr;
    for (itr = ref.begin(); itr != ref.end(); ++itr) { rClassNames.push_back(itr->first); }

    // remove duplicates
    //
    sort(rClassNames.begin(), rClassNames.end());
    rClassNames.erase(unique(rClassNames.begin(), rClassNames.end()), rClassNames.end());
    return (rClassNames);
}

vector<string> ParamsMgr::GetRenderParamsClassNames(string winName) const
{
    vector<string> rClassNames;

    // _renderParamsMap[winName][dataSetName][className]
    //

    const map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(_renderParamsMap, winName);
    if (!m2Ptr) return (rClassNames);

    // m2Ptr[dataSetName][className]
    //

    const map<string, map<string, RenParamsContainer *>> &ref = *m2Ptr;

    map<string, map<string, RenParamsContainer *>>::const_iterator itr;
    for (itr = ref.begin(); itr != ref.end(); ++itr) {
        string dataSetName = itr->first;

        vector<string> tmpV = GetRenderParamsClassNames(winName, dataSetName);
        rClassNames.insert(rClassNames.end(), tmpV.begin(), tmpV.end());
    }

    // remove duplicates
    //
    sort(rClassNames.begin(), rClassNames.end());
    rClassNames.erase(unique(rClassNames.begin(), rClassNames.end()), rClassNames.end());
    return (rClassNames);
}

vector<string> ParamsMgr::GetRenderParamInstances(string winName, string dataSetName, string className) const
{
    vector<string> instances;

    RenParamsContainer *rpc = get_ren_container(winName, dataSetName, className);
    if (rpc) {
        vector<string> names = rpc->GetNames();
        instances.insert(instances.end(), names.begin(), names.end());
    }

    // Sanity check.  Names should always be unique!
    //
    sort(instances.begin(), instances.end());
    instances.erase(unique(instances.begin(), instances.end()), instances.end());
    return (instances);
}

vector<string> ParamsMgr::GetRenderParamInstances(string winName, string className) const
{
    vector<string> instances;

    const map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(_renderParamsMap, winName);
    if (!m2Ptr) return (instances);

    const map<string, map<string, RenParamsContainer *>> &         ref = *m2Ptr;
    map<string, map<string, RenParamsContainer *>>::const_iterator itr;

    for (itr = ref.begin(); itr != ref.end(); ++itr) {
        string dataSetName = itr->first;

        RenParamsContainer *rpc = get_ren_container(winName, dataSetName, className);
        if (rpc) {
            vector<string> names = rpc->GetNames();
            instances.insert(instances.end(), names.begin(), names.end());
        }
    }

    // Sanity check.  Names should always be unique!
    //
    sort(instances.begin(), instances.end());
    instances.erase(unique(instances.begin(), instances.end()), instances.end());
    return (instances);
}

ViewpointParams *ParamsMgr::GetViewpointParams(string winName) const { return (get_vp_params(winName)); }

#ifdef VAPOR3_0_0_ALPHA
void ParamsMgr::InsertRenderParamsInstance(RenderParams *rp, string winName, string instName)
{
    string className = rp->GetName();

    RenParamsContainer *container = get_ren_container(winName, className);
    if (!container) { container = make_ren_container(className, winName); }
    VAssert(container != NULL);

    container->Insert(rp, instName);
}
#endif

RenParamsContainer *ParamsMgr::get_ren_container(string winName, string dataSetName, string renderName) const
{
    // map[winName][dataSetName][renderName]
    //

    const map<string, RenParamsContainer *> *m1Ptr;
    m1Ptr = getWinMap3(_renderParamsMap, winName, dataSetName);
    if (!m1Ptr) return (NULL);

    // m1Ptr[className]
    //
    const map<string, RenParamsContainer *> &ref = *m1Ptr;

    map<string, RenParamsContainer *>::const_iterator itr;
    itr = ref.find(renderName);
    if (itr == ref.end()) return (NULL);

    return (itr->second);
}

void ParamsMgr::delete_ren_container(string winName, string dataSetName, string renderName)
{
    // _renderParamsMap[winName][dataSetName][renderName] -> ref[renderName]
    //
    map<string, RenParamsContainer *> *m1Ptr;
    m1Ptr = getWinMap3(_renderParamsMap, winName, dataSetName);
    if (!m1Ptr) return;

    // *m1Ptr[renderName] == ref[renderName]
    //
    map<string, RenParamsContainer *> &ref = *m1Ptr;

    map<string, RenParamsContainer *>::iterator itr;
    itr = ref.find(renderName);
    if (itr == ref.end()) return;

    RenParamsContainer *rpc = itr->second;
    ref.erase(renderName);

    if (!rpc) return;

    // Set parent to root so  Xml representation will be deleted
    //
    rpc->GetNode()->SetParent(NULL);
    delete rpc;
}

void ParamsMgr::delete_ren_containers(string winName, string dataSetName)
{
    // _renderParamsMap[winName][dataSetName][renderName] ->
    //		ref1[dataSetName][renderName]
    //
    map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(_renderParamsMap, winName);
    if (!m2Ptr) return;

    map<string, map<string, RenParamsContainer *>> &ref1 = *m2Ptr;

    map<string, map<string, RenParamsContainer *>>::iterator itr1;
    itr1 = ref1.find(dataSetName);
    if (itr1 == ref1.end()) return;

    //	ref1[dataSetName][renderName] -> ref2[renderName]
    //
    map<string, RenParamsContainer *> &         ref2 = itr1->second;
    map<string, RenParamsContainer *>::iterator itr2;
    while ((itr2 = ref2.begin()) != ref2.end()) { delete_ren_container(winName, dataSetName, itr2->first); }
    ref1.erase(dataSetName);
}

void ParamsMgr::delete_ren_containers(string winName)
{
    // _renderParamsMap[winName][dataSetName][renderName] ->
    //		ref1[dataSetName][renderName]
    //
    map<string, map<string, RenParamsContainer *>> *m2Ptr;
    m2Ptr = getWinMap3(_renderParamsMap, winName);
    if (!m2Ptr) return;

    map<string, map<string, RenParamsContainer *>> &         ref = *m2Ptr;
    map<string, map<string, RenParamsContainer *>>::iterator itr;
    while ((itr = ref.begin()) != ref.end()) { delete_ren_containers(winName, itr->first); }

    _renderParamsMap.erase(winName);
}

void ParamsMgr::delete_ren_containers()
{
    // For each window name delete all of the windows render containers
    //
    map<string, map<string, map<string, RenParamsContainer *>>>::iterator itr;
    while ((itr = _renderParamsMap.begin()) != _renderParamsMap.end()) { delete_ren_containers(itr->first); }
}

void ParamsMgr::delete_datasets(string dataSetName)
{
    ParamsSeparator windowsSep(_rootSeparator, _windowsTag);

    XmlNode *winSepNode = windowsSep.GetNode();
    for (int i = 0; i < winSepNode->GetNumChildren(); i++) {
        XmlNode *winNode = winSepNode->GetChild(i);
        string   winName = winNode->GetTag();

        delete_ren_containers(winName, dataSetName);

        XmlNode *renderersNode = winNode->GetChild(_renderersTag);
        for (int j = 0; j < renderersNode->GetNumChildren(); j++) {
            XmlNode *dataSepNode = renderersNode->GetChild(j);
            string   s = dataSepNode->GetTag();

            if (s == dataSetName) {
                dataSepNode->SetParent(NULL);
                break;
            }
        }
    }
}

RenParamsContainer *ParamsMgr::make_ren_container(string winName, string dataSetName, string renderName)
{
    ParamsSeparator *windowsSep = new ParamsSeparator(_rootSeparator, _windowsTag);

    ParamsSeparator *windowSep = new ParamsSeparator(windowsSep, winName);

    ParamsSeparator *renderSep = new ParamsSeparator(windowSep, _renderersTag);

    ParamsSeparator *dataSep = new ParamsSeparator(renderSep, dataSetName);

    // Delete any existing occurences with the same name
    //
    RenParamsContainer *rpc = get_ren_container(winName, dataSetName, renderName);
    if (rpc) delete rpc;

    if (dataSep->HasChild(renderName)) {
        XmlNode *node = dataSep->GetNode()->GetChild(renderName);
        VAssert(node);

        rpc = new RenParamsContainer(_dataMgrMap[dataSetName], &_ssave, node);
    } else {
        rpc = new RenParamsContainer(_dataMgrMap[dataSetName], &_ssave, renderName);
        rpc->GetSeparator()->SetParent(dataSep);
    }

    _renderParamsMap[winName][dataSetName][renderName] = rpc;

    delete dataSep;
    delete renderSep;
    delete windowSep;
    delete windowsSep;

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
    ParamsSeparator *windowsSep = new ParamsSeparator(_rootSeparator, _windowsTag);

    ParamsSeparator *windowSep = new ParamsSeparator(windowsSep, winName);

    // Delete any existing occurences with the same name
    //
    ViewpointParams *vpParams = get_vp_params(winName);
    if (vpParams) delete vpParams;

    if (windowSep->HasChild(ViewpointParams::GetClassType())) {
        XmlNode *node = windowSep->GetNode()->GetChild(ViewpointParams::GetClassType());
        VAssert(node);

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

#ifdef VAPOR3_0_0_ALPHA
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

void ParamsMgr::GetAppRenderParams(string dataSetName, vector<RenderParams *> &appRenderParams) const
{
    appRenderParams.clear();
    std::map<string, RenParamsContainer *>::const_iterator itr;
    itr = _otherRenParams.find(dataSetName);
    if (itr == _otherRenParams.cend()) return;
    vector<string> v = itr->second->GetNames();
    for (int i = 0; i < v.size(); i++) {
        if (itr->second->GetParams(v[i])) { appRenderParams.push_back(itr->second->GetParams(v[i])); }
    }
}

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
    const XmlNode *newNode = _ssave.GetTopUndo(description);
    if (!newNode) { newNode = _ssave.GetBase(); }
    if (!newNode) return (false);    // nothing to undo - shouldnt get here

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
    bool status = _ssave.Undo();

    if (!status) return (status);

    return (undoRedoHelper());
}

bool ParamsMgr::Redo()
{
    bool status = _ssave.Redo();

    if (!status) return (status);

    return (undoRedoHelper());
}

void ParamsMgr::UndoRedoClear()
{
    _ssave.Clear();
    RebaseStateSave();
}

string ParamsMgr::GetTopUndoDesc() const
{
    string s;
    _ssave.GetTopUndo(s);
    return (s);
}

string ParamsMgr::GetTopRedoDesc() const
{
    string s;
    _ssave.GetTopRedo(s);
    return (s);
}

ParamsMgr::PMgrStateSave::PMgrStateSave(int stackSize) : StateSave()
{
    _enabled = true;
    _stackSize = stackSize;
    _rootNode = NULL;
    _state0 = NULL;
    _undoStack.clear();
    _redoStack.clear();
}

ParamsMgr::PMgrStateSave::~PMgrStateSave()
{
    cleanStack(0, _undoStack);
    cleanStack(0, _redoStack);
    if (_state0) delete _state0;
}

void ParamsMgr::PMgrStateSave::Save(const XmlNode *node, string description)
{
    VAssert(_rootNode);
    VAssert(node);

    if (!GetEnabled()) { return; }

    // Only save state if the node is a branch of the tree rooted at
    // the node named by _rootTag
    //
    vector<string> pathvec = node->GetPathVec();
    if ((!pathvec.size()) || (pathvec[0] != _rootTag)) { return; }

    if (GetUndoEnabled()) {
        const XmlNode *topNode = NULL;
        string         s;
        topNode = GetTopUndo(s);
        if (topNode && (*topNode == *_rootNode)) {
            // Don't save tree if no changes
            return;
        }
    }

    if (!_groups.empty()) { return; }

    if (!_state0) { _state0 = new XmlNode(*_rootNode); }

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _undoStack);

    // It not inside a group push this element onto the stack
    //
    if (GetUndoEnabled()) _undoStack.push_back(make_pair(description, new XmlNode(*_rootNode)));

//#define DEBUG
#ifdef DEBUG
    cout << "ParamsMgr::PMgrStateSave::Save() : saving node " << node->GetTag() << " : " << description << endl;
#endif

    // Clear redo stack
    //
    cleanStack(0, _redoStack);

    emitStateChange();
}

void ParamsMgr::PMgrStateSave::BeginGroup(string description)
{
    VAssert(_rootNode);
    VAssert(!description.empty());
    if (!GetEnabled()) return;

    _groups.push(description);
}

void ParamsMgr::PMgrStateSave::EndGroup()
{
    VAssert(_rootNode);

    if (!GetEnabled()) return;

    if (!_groups.size()) return;    // BeginGroup() not called

    string desc = _groups.top();
    _groups.pop();

    // Don't do anything until _groups is empty
    //
    if (_groups.size()) return;

    const XmlNode *topNode = NULL;
    string         s;
    topNode = GetTopUndo(s);

    if (topNode && (*topNode == *_rootNode)) {
        // Don't save tree if no changes
        //
        return;
    }

    if (!_state0) { _state0 = new XmlNode(*_rootNode); }

#ifdef DEBUG
    cout << "ParamsMgr::PMgrStateSave::EndGroup() : saving "
         << " : " << desc << endl;
#endif

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _undoStack);

    // Clear redo stack
    //
    cleanStack(0, _redoStack);

    _undoStack.push_back(make_pair(desc, new XmlNode(*_rootNode)));

    emitStateChange();
}

void ParamsMgr::PMgrStateSave::IntermediateChange() { emitIntermediateStateChange(); }

const XmlNode *ParamsMgr::PMgrStateSave::GetTopUndo(string &description) const
{
    VAssert(_rootNode);
    description.clear();

    if (!_undoStack.size()) return (NULL);

    const pair<string, XmlNode *> &p1 = _undoStack.back();

    description = p1.first;
    return (p1.second);
}

const XmlNode *ParamsMgr::PMgrStateSave::GetTopRedo(string &description) const
{
    VAssert(_rootNode);
    description.clear();

    if (!_redoStack.size()) return (NULL);

    const pair<string, XmlNode *> &p1 = _redoStack.back();

    description = p1.first;
    return (p1.second);
}

bool ParamsMgr::PMgrStateSave::Undo()
{
    VAssert(_rootNode);

    if (!_undoStack.size()) return (false);

    pair<string, XmlNode *> &p1 = _undoStack.back();

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _redoStack);

    _redoStack.push_back(p1);

    _undoStack.pop_back();

    emitStateChange();

    return (true);
}

bool ParamsMgr::PMgrStateSave::Redo()
{
    VAssert(_rootNode);

    if (!_redoStack.size()) return (false);

    pair<string, XmlNode *> &p1 = _redoStack.back();

    // Delete oldest elements if needed
    //
    cleanStack(_stackSize, _undoStack);

    _undoStack.push_back(p1);

    _redoStack.pop_back();

    emitStateChange();

    return (true);
}

void ParamsMgr::PMgrStateSave::Clear()
{
    VAssert(_rootNode);

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

void ParamsMgr::PMgrStateSave::emitStateChange()
{
    // Trigger state change flags and CBs
    //
    for (int i = 0; i < _stateChangeFlags.size(); i++) { *(_stateChangeFlags[i]) = true; }
    for (int i = 0; i < _stateChangeCBs.size(); i++) { _stateChangeCBs[i](); }
}

void ParamsMgr::PMgrStateSave::emitIntermediateStateChange()
{
    for (auto func : _intermediateStateChangeCBs) func();
}
