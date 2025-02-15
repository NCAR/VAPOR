#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cfloat>

#include <vapor/STLUtils.h>
#include <vapor/FileUtils.h>
#include <vapor/ParamsMgr.h>
#include <vapor/ControlExecutive.h>
#include <vapor/CalcEngineMgr.h>
#include <vapor/VisualizerGLContextManager.h>
#include <vapor/Visualizer.h>
#include <vapor/DataStatus.h>
#include <vapor/GUIStateParams.h>
#include <vapor/SettingsParams.h>
#include <vapor/NavigationUtils.h>

#include <vapor/VolumeRenderer.h>
#include <vapor/VolumeIsoRenderer.h>
#include <vapor/OpenMPSupport.h>

using namespace VAPoR;
using namespace std;

ControlExec::ControlExec(ParamsMgr *pm, size_t cacheSizeMB, int nThreads) : MyBase()
{
    _paramsMgr = pm;
    _dataStatus = new DataStatus(cacheSizeMB, nThreads);
    _calcEngineMgr = new CalcEngineMgr(_dataStatus, _paramsMgr);
    _visualizers.clear();
    auto sp = pm->GetParams<SettingsParams>();
    SetCacheSize(sp->GetCacheMB());
    SetNumThreads(sp->GetNumThreads());
}

ControlExec::~ControlExec()
{
#ifdef DEBUG
    cout << "Allocated XmlNode count before delete " << XmlNode::GetAllocatedNodes().size() << endl;

    const vector<XmlNode *> &nodes = XmlNode::GetAllocatedNodes();
    for (int i = 0; i < nodes.size(); i++) { cout << "   " << nodes[i]->GetTag() << " " << XmlNode::streamOut(cout, nodes[i]) << endl; }
#endif

    if (_paramsMgr) delete _paramsMgr;
    if (_dataStatus) delete _dataStatus;

#ifdef DEBUG
    cout << "Allocated XmlNode count after delete " << XmlNode::GetAllocatedNodes().size() << endl;

    for (int i = 0; i < nodes.size(); i++) { cout << "   " << nodes[i]->GetTag() << " " << XmlNode::streamOut(cout, nodes[i]) << endl; }
#endif
}

int ControlExec::NewVisualizer(string winName)
{
    // TODO Maybe remove this and RemoveVisualizer
    winName = _paramsMgr->CreateVisualizerParamsInstance(winName);
    if (winName.empty()) {
        SetErrMsg("Failed to create Visualizer parameters");
        return -1;
    }
    return 0;
}

void ControlExec::RemoveVisualizer(string winName, bool hasOpenGLContext)
{
    _paramsMgr->RemoveVisualizer(winName);
}


void ControlExec::SyncWithParams()
{
    syncDatasetsWithParams();
    syncVisualizersWithParams();
    _calcEngineMgr->SyncWithParams();
}


void ControlExec::syncVisualizersWithParams()
{
    vector<string> vizNames = GetParamsMgr()->GetVisualizerNames();

    for (const auto &name : STLUtils::SyncToRemove(vizNames, STLUtils::MapKeys(_visualizers)))
        CleanupVisualizer(name, false);

    for (const auto &name : STLUtils::SyncToAdd(vizNames, STLUtils::MapKeys(_visualizers)))
        _visualizers[name] = new Visualizer(_paramsMgr, _dataStatus, name);
}


void ControlExec::syncDatasetsWithParams()
{
    auto gp = GetParams<GUIStateParams>();
    auto openDatasets = gp->GetOpenDataSetNames();
    auto toRemove = STLUtils::SyncToRemove(openDatasets, _dataStatus->GetDataMgrNames());
    if (_dataCachedProjStr != gp->GetProjectionString()) {
        toRemove = _dataStatus->GetDataMgrNames();
        _dataCachedProjStr = gp->GetProjectionString();
    }

    for (const auto &name : toRemove)
        _dataStatus->Close(name);

    for (const auto &name : STLUtils::SyncToAdd(openDatasets, _dataStatus->GetDataMgrNames()))
        OpenData(gp->GetOpenDataSetPaths(name), name, gp->GetOpenDataSetFormat(name));

    _dataStatus->_wasCacheDirty = _dataStatus->_isDataCacheDirty;
    _dataStatus->_isDataCacheDirty = false;
}


void ControlExec::EnforceDefaultAppState()
{
    vector<string> vizNames = _paramsMgr->GetVisualizerNames();
    auto gsp = GetParams<GUIStateParams>();
    if (!vizNames.empty() && !STLUtils::Contains(vizNames, gsp->GetActiveVizName())) {
        _paramsMgr->PushSaveStateEnabled(false);
        gsp->SetActiveVizName(vizNames[0]);
        _paramsMgr->PopSaveStateEnabled();
    }
}


void ControlExec::CleanupVisualizer(string winName, bool hasOpenGLContext)
{
    auto viz = getVisualizer(winName);
    if (!viz) return;

    if (!hasOpenGLContext)
        _vizGLMgr->Activate(winName);

    RemoveAllRenderers(winName, true, false);

    _visualizers.erase(winName);
    delete viz;
}

int ControlExec::InitializeViz(string winName, GLManager *glManager)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    if (v->InitializeGL(glManager) < 0) {
        SetErrMsg("InitializeGL failure");
        return -1;
    }

    _cachedVendor = glManager->GetVendor();

    return 0;
}

vector<string> ControlExec::GetVisualizerNames() const
{
    vector<string> names;

    std::map<string, Visualizer *>::const_iterator itr;
    for (itr = _visualizers.begin(); itr != _visualizers.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

int ControlExec::ResizeViz(string winName, int width, int height)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }
    if (v->resizeGL(width, height) < 0) return -1;

    return 0;
}

void ControlExec::ClearRenderCache(const string &winName, const string &inst)
{
    getVisualizer(winName)->ClearRenderCache(inst);
}

void ControlExec::ClearAllRenderCaches()
{
    std::map<string, Visualizer *>::const_iterator itr;
    for (itr = _visualizers.begin(); itr != _visualizers.end(); ++itr) { itr->second->ClearRenderCache(); }
}

GLManager::Vendor ControlExec::GetGPUVendor() const { return _cachedVendor; }

int ControlExec::Paint(string winName, bool fast)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    // Disable state saving when generating the transfer function
    //
    bool enabled = _paramsMgr->GetSaveStateEnabled();
    _paramsMgr->SetSaveStateEnabled(false);

    int rc = v->paintEvent(fast);

    _paramsMgr->SetSaveStateEnabled(enabled);

    if (rc) SetErrMsg("Error performing paint event");
    return rc;
}

// TODO Replace with params-only
int ControlExec::ActivateRender(string winName, string dataSetName, string renderType, string renderName, bool on)
{
    if (!_dataStatus->GetDataMgrNames().size()) {
        SetErrMsg("Invalid state : no data");
        return -1;
    }

    if (!STLUtils::Contains(_paramsMgr->GetVisualizerNames(), winName)) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    _paramsMgr->BeginSaveStateGroup("Activate Renderer");

    string paramsType = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);
    VAssert(!paramsType.empty());

    RenderParams *rp = _paramsMgr->GetRenderParams(winName, dataSetName, paramsType, renderName);
    if (!rp) {
        rp = _paramsMgr->CreateRenderParamsInstance(winName, dataSetName, paramsType, renderName);
        if (!rp) {
            SetErrMsg("Invalid renderer of type \"%s\"", renderType.c_str());
            _paramsMgr->EndSaveStateGroup();
            return -1;
        }

        rp->SetCurrentTimestep(NavigationUtils::GetCurrentTimeStep(this));
        int rc = rp->Initialize();
        if (rc < 0) {
            SetErrMsg("Failed to initialize of type \"%s\"", renderType.c_str());
            _paramsMgr->EndSaveStateGroup();
            return (-1);
        }
    }

    VAssert(rp);
    rp->SetEnabled(on);

    if (on) {
        Visualizer *v = getVisualizer(winName);
        if (v) { // TODO Replace rendering order using params
            v->MoveRendererToFront(renderType, renderName);
            v->MoveRenderersOfTypeToFront(VolumeRenderer::GetClassType());
        }
    }

    _paramsMgr->EndSaveStateGroup();

    return 0;
}

void ControlExec::_removeRendererHelper(string winName, string dataSetName, string pClassName, string renderName, bool hasOpenGLContext)
{
    // No-op if tuple of winName, dataSetName, renderType, and
    // renderName is unknown
    //

    // Convert from params render type to render type. Sigh
    //
    string rClassName = RendererFactory::Instance()->GetRenderClassFromParamsClass(pClassName);

    RenderParams *rParams = _paramsMgr->GetRenderParams(winName, dataSetName, pClassName, renderName);
    if (!rParams) return;

    Visualizer *v = getVisualizer(winName);
    if (!v) return;

    v->DestroyRenderer(rClassName, renderName, hasOpenGLContext);

    _paramsMgr->RemoveRenderParamsInstance(winName, dataSetName, pClassName, renderName);
}

void ControlExec::RemoveRenderer(string winName, string dataSetName, string renderType, string renderName, bool hasOpenGLContext)
{
    string pClassName = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);

    RenderParams *rParams = _paramsMgr->GetRenderParams(winName, dataSetName, pClassName, renderName);
    if (!rParams) return;

    _removeRendererHelper(winName, dataSetName, pClassName, renderName, hasOpenGLContext);
}

void ControlExec::RemoveAllRenderers(string winName, bool hasOpenGLContext, bool removeFromParamsFlag)
{
    vector<string> dataSetNames = _paramsMgr->GetDataMgrNames();
    for (int k = 0; k < dataSetNames.size(); k++) {
        vector<string> pClassNames = _paramsMgr->GetRenderParamsClassNames(winName, dataSetNames[k]);

        for (int j = 0; j < pClassNames.size(); j++) {
            vector<string> instNames = _paramsMgr->GetRenderParamInstances(winName, dataSetNames[k], pClassNames[j]);

            for (int i = 0; i < instNames.size(); i++) { _removeRendererHelper(winName, dataSetNames[k], pClassNames[j], instNames[i], hasOpenGLContext); }
        }
    }
}

void ControlExec::LoadState()
{
    _paramsMgr->LoadState();
}

void ControlExec::LoadState(const XmlNode *rootNode)
{
    _paramsMgr->LoadState(rootNode);
}

int ControlExec::LoadState(string stateFile, LoadStateRelAndAbsPathsExistAction relAndAbsPathsExistAction)
{
    _paramsMgr->BeginSaveStateGroup("Load state");

    vector<string> vizNames = GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) { RemoveVisualizer(vizNames[i]); }

    // Bug prevents using the real PM here
    ParamsMgr tempPM({GUIStateParams::GetClassType()});
    if (tempPM.LoadState(stateFile) < 0) {
        _paramsMgr->EndSaveStateGroup();
        return -1;
    }

    GUIStateParams *tmpGsp = (GUIStateParams *)tempPM.GetParams(GUIStateParams::GetClassType());
    map<string, vector<string>> useRelativePaths;
    auto sesDir = FileUtils::Dirname(stateFile);

    for (auto dataset : tmpGsp->GetOpenDataSetNames()) {
        auto paths = tmpGsp->GetOpenDataSetPaths(dataset);
        auto relPaths = tmpGsp->GetOpenDataSetRelativePaths(dataset);
        for (int i = 0; i < relPaths.size(); i++) relPaths[i] = FileUtils::JoinPaths({sesDir, relPaths[i]});
        if (relPaths.empty())
            continue;

        auto absoluteExist = std::all_of(paths.begin(),    paths.end(),    [](string p){return FileUtils::Exists(p);});
        auto relativeExist = std::all_of(relPaths.begin(), relPaths.end(), [](string p){return FileUtils::Exists(p);});

        if (absoluteExist && !relativeExist) continue;
        if (!absoluteExist && relativeExist) {
//            tmpGsp->InsertOpenDateSet(dataset, tmpGsp->GetOpenDataSetFormat(dataset), relPaths);
            useRelativePaths[dataset] = relPaths;
            continue;
        }
        if (absoluteExist && relativeExist) {
            bool same = paths.size() == relPaths.size();
            for (int i = 0; i < paths.size() && same; i++)
                same &= FileUtils::AreSameFile(paths[i], relPaths[i]);

            if (same) continue;

            switch (relAndAbsPathsExistAction) {
                case LoadStateRelAndAbsPathsExistAction::LoadAbs:
                    continue;
                case LoadStateRelAndAbsPathsExistAction::LoadRel:
//                    tmpGsp->InsertOpenDateSet(dataset, tmpGsp->GetOpenDataSetFormat(dataset), relPaths);
                    useRelativePaths[dataset] = relPaths;
                    continue;
                case LoadStateRelAndAbsPathsExistAction::Ask:
                    _paramsMgr->EndSaveStateGroup();
                    throw RelAndAbsPathsExistException(paths[0], relPaths[0]);
            }
        }
    }

    int rc = _paramsMgr->LoadState(stateFile);
    if (rc < 0) {
        _paramsMgr->EndSaveStateGroup();
        return (-1);
    }

    auto gsp = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());
    for (const auto &it : useRelativePaths)
        gsp->InsertOpenDateSet(it.first, gsp->GetOpenDataSetFormat(it.first), it.second);

    _paramsMgr->EndSaveStateGroup();
    return (0);
}

void ControlExec::SetNumThreads(size_t nthreads)
{
    // Set the number of PThreads to use
    _dataStatus->SetNumThreads(nthreads);

    if (nthreads > 0) { omp_set_num_threads(nthreads); }
}

size_t ControlExec::GetNumThreads() const { return (_dataStatus->GetNumThreads()); }

void ControlExec::SetCacheSize(size_t sizeMB) { _dataStatus->SetCacheSize(sizeMB); }

int ControlExec::OpenData(const std::vector<string> &files, string dataSetName, string typ)
{
    _paramsMgr->BeginSaveStateGroup("Open Dataset");

    vector<string> options = {"-project_to_pcs", "-vertical_xform"};
    if (GetParams<SettingsParams>()->GetAutoStretchEnabled()) options.push_back("-auto_stretch_z");
    // This is a minor bug if the first dataset has an empty proj string however this has been a bug for as long as I can tell and it is not worth fixing at the moment
    if (!GetParams<GUIStateParams>()->GetProjectionString().empty()) STLUtils::AppendTo(options, {"-proj4", GetParams<GUIStateParams>()->GetProjectionString()});

    SetDataCacheDirty(dataSetName);

    int rc = _dataStatus->Open(files, options, dataSetName, typ);
    if (rc < 0) {
        SetErrMsg("Failure to open data set of type \"%s\"", typ.c_str());
        _paramsMgr->EndSaveStateGroup();
        return -1;
    }

    _paramsMgr->AddDataMgr(dataSetName, _dataStatus->GetDataMgr(dataSetName));

    // Need to call initializers for any registered application renderers
    //
    vector<RenderParams *> appRenderParams;
    _paramsMgr->GetAppRenderParams(dataSetName, appRenderParams);
    for (int i = 0; i < appRenderParams.size(); i++) {
        int rc = appRenderParams[i]->Initialize();
        if (rc < 0) {
            _calcEngineMgr->Clean();
            _dataStatus->Close(dataSetName);
            _paramsMgr->RemoveDataMgr(dataSetName);
            SetErrMsg("Failure to initialize application renderer \"%s\"", appRenderParams[i]->GetName().c_str());
            _paramsMgr->EndSaveStateGroup();
            return -1;
        }
    }

    _setDefaultOrigin(dataSetName);

    if (STLUtils::Contains(options, string("-auto_stretch_z"))) _autoStretchExtents(dataSetName);

    // vvvv NOPs if existing used
    _dataCachedProjStr = _dataStatus->GetMapProjection();
    GetParams<GUIStateParams>()->SetProjectionString(_dataCachedProjStr);
    // ^^^^

    _paramsMgr->EndSaveStateGroup();
    return rc;
}

void ControlExec::CloseData(string dataSetName)
{
    if (!_dataStatus->GetDataMgr(dataSetName)) return;
    SetDataCacheDirty(dataSetName);

    _paramsMgr->BeginSaveStateGroup("Close Dataset");
    GetParams<GUIStateParams>()->RemoveOpenDateSet(dataSetName);

    for (const auto &renName : _paramsMgr->GetRenderParamNamesForDataset(dataSetName)) {
        string vizName, className, _;
        _paramsMgr->RenderParamsLookup(renName, vizName, _, className);
        _paramsMgr->RemoveRenderParamsInstance(vizName, dataSetName, className, renName);
    }

    _paramsMgr->RemoveDataMgr(dataSetName);
    _dataStatus->Close(dataSetName);

    _paramsMgr->EndSaveStateGroup();
}

int ControlExec::EnableImageCapture(string filename, string winName, bool fast)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }
    if (v->SetImageCaptureEnabled(true, filename)) {
        SetErrMsg("Visualizer (%s) failed to enable capturing  image.", winName.c_str());
        return -1;
    }

    // Disable state saving when capturing an image
    //
    bool enabled = _paramsMgr->GetSaveStateEnabled();
    _paramsMgr->SetSaveStateEnabled(false);
    int rc = v->paintEvent(fast);    // paint with image capture enabled
    _paramsMgr->SetSaveStateEnabled(enabled);
    if (rc != 0) {
        SetErrMsg("Visualizer (%s) failed to paint and thus not capturing image.", winName.c_str());
        return -1;
    }
    return 0;
}

int ControlExec::EnableAnimationCapture(string winName, bool onOff, string filename)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    if (v->SetAnimationCaptureEnabled(onOff, filename)) return -1;
    return 0;
}

string ControlExec::MakeStringConformant(string s)
{
    if (s.empty()) s += "_";

    if (!(isalpha(s[0]) || s[0] == '_')) { s = "_" + s; }

    for (string::iterator itr = s.begin(); itr != s.end(); ++itr) {
        if (!(isalnum(*itr) || isdigit(*itr) || *itr == '-' || *itr == '_' || *itr == '.')) { *itr = '_'; }
        if (isspace(*itr)) { *itr = '_'; }
    }
    return (s);
}


int ControlExec::SaveSession(string filename)
{
    ofstream fileout;
    const XmlNode *node = nullptr;

    GUIStateParams *gsp = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());
    bool saveStateEnabled = _paramsMgr->GetSaveStateEnabled();
    _paramsMgr->SetSaveStateEnabled(false);
    auto datasets = gsp->GetOpenDataSetNames();
    for (auto dataset : datasets) {
        auto paths = gsp->GetOpenDataSetPaths(dataset);
        auto format = gsp->GetOpenDataSetFormat(dataset);
        vector<string> relpaths(paths.size());
        for (int i = 0; i < paths.size(); i++) {
            paths[i] = FileUtils::Realpath(paths[i]);
            relpaths[i] = FileUtils::Relpath(paths[i], filename);
        }
        gsp->InsertOpenDateSet(dataset, format, paths, relpaths);
    }
    _paramsMgr->SetSaveStateEnabled(saveStateEnabled);

    fileout.open(filename.c_str());
    if (!fileout) {
        SetErrMsg("Unable to open output session file : %M");
        return -1;
    }

    node = _paramsMgr->GetXMLRoot();
    XmlNode::streamOut(fileout, *node);
    if (fileout.bad()) {
        SetErrMsg("Unable to write output session file : %M");
        return -1;
    }

    return 0;
}

RenderParams *ControlExec::GetRenderParams(string winName, string dataSetName, string renderType, string instName) const
{
    string paramsType = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);

    RenderParams *rParams = _paramsMgr->GetRenderParams(winName, dataSetName, paramsType, instName);

    if (!rParams) {
        SetErrMsg("Invalid window name, render type, or instance name");
        return (NULL);
    }

    return (rParams);
}

vector<string> ControlExec::GetRenderClassNames(string winName) const
{
    vector<string> v = _paramsMgr->GetRenderParamsClassNames(winName);

    for (int i = 0; i < v.size(); i++) { v[i] = RendererFactory::Instance()->GetRenderClassFromParamsClass(v[i]); }

    return (v);
}

vector<string> ControlExec::GetRenderInstances(string winName, string renderType) const
{
    string paramsType = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);

    return (_paramsMgr->GetRenderParamInstances(winName, paramsType));
}

vector<string> ControlExec::GetAllRenderClasses() { return (RendererFactory::Instance()->GetFactoryNames()); }

bool ControlExec::RenderLookup(string instName, string &winName, string &dataSetName, string &renderType) const
{
    string paramsType;
    bool   ok = _paramsMgr->RenderParamsLookup(instName, winName, dataSetName, paramsType);
    if (!ok) return (ok);

    renderType = RendererFactory::Instance()->GetRenderClassFromParamsClass(paramsType);
    return (ok);
}

string ControlExec::MakeRendererNameUnique(string name) const
{
    string newname = name;

    ParamsMgr *pm = GetParamsMgr();

    vector<string> allInstNames;

    // Get ALL of the renderer instance names defined
    //
    vector<string> vizNames = pm->GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) {
        vector<string> classNames = GetRenderClassNames(vizNames[i]);

        for (int j = 0; j < classNames.size(); j++) {
            vector<string> rendererNames = GetRenderInstances(vizNames[i], classNames[j]);

            allInstNames.insert(allInstNames.begin(), rendererNames.begin(), rendererNames.end());
        }
    }

    while (1) {
        bool match = false;
        for (int i = 0; i < allInstNames.size(); i++) {
            string usedName = allInstNames[i];

            if (newname != usedName) continue;

            match = true;

            size_t lastnonint = newname.find_last_not_of("0123456789");
            if (lastnonint < newname.length() - 1) {
                string endchars = newname.substr(lastnonint + 1);
                int termInt = atoi(endchars.c_str());
                termInt++;
                std::stringstream ss;
                ss << termInt;
                endchars = ss.str();
                newname.replace(lastnonint + 1, string::npos, endchars);
            } else {
                newname = newname + "_1";
            }
        }
        if (!match) break;
    }
    return newname;
}

int ControlExec::AddFunction(string scriptType, string dataSetName, string scriptName, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames,
                             const vector<string> &outputVarMeshes, bool coordFlag)
{
    // Ugh. Need to force each renderer to clear any cached data because
    // if we redefine a variable the variable's data will change but
    // the variable's name will not. Hence, if we don't clear the data
    // the renderer may continue using old data
    //
//    ClearAllRenderCaches();

    return (_calcEngineMgr->AddFunction(scriptType, dataSetName, scriptName, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag));
}

void ControlExec::RemoveFunction(string scriptType, string dataSetName, string scriptName) { return (_calcEngineMgr->RemoveFunction(scriptType, dataSetName, scriptName)); }

bool ControlExec::GetFunction(string scriptType, string dataSetName, string scriptName, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes,
                              bool &coordFlag) const
{
    script.clear();
    inputVarNames.clear();
    outputVarNames.clear();
    outputVarMeshes.clear();

    return (_calcEngineMgr->GetFunctionScript(scriptType, dataSetName, scriptName, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag));
}

string ControlExec::GetFunctionStdout(string scriptType, string dataSetName, string scriptName) const { return (_calcEngineMgr->GetFunctionStdout(scriptType, dataSetName, scriptName)); }

std::vector<string> ControlExec::GetFunctionNames(string scriptType, string dataSetName) const { return (_calcEngineMgr->GetFunctionNames(scriptType, dataSetName)); }


// Function moved from old NavigationEventRouter.cpp
void ControlExec::_autoStretchExtents(string dataSetName)
{
    DataStatus *ds = GetDataStatus();

    ParamsMgr *    paramsMgr = GetParamsMgr();
    vector<string> winNames = paramsMgr->GetVisualizerNames();

    CoordType minExt, maxExt;

    for (int i = 0; i < winNames.size(); i++) {
        ViewpointParams *   vpParams = paramsMgr->GetViewpointParams(winNames[i]);
        Transform *         transform = vpParams->GetTransform(dataSetName);
        std::vector<double> scales = transform->GetScales();
        int                 xDimension = 0;
        int                 yDimension = 1;
        int                 zDimension = 2;

        // If a dimension's scale is not 1.f, the user has saved a session with
        // a non-default value.  Don't modify it.
        if (scales[xDimension] != 1.f) continue;
        if (scales[yDimension] != 1.f) continue;
        if (scales[zDimension] != 1.f) continue;

        //        size_t ts = GetCurrentTimeStep();
        size_t ts = 0;
        ds->GetActiveExtents(paramsMgr, winNames[i], dataSetName, ts, minExt, maxExt);

        vector<float> range;
        float         maxRange = 0.0;
        for (int i = 0; i < minExt.size(); i++) {
            float r = fabs(maxExt[i] - minExt[i]);
            if (maxRange < r) { maxRange = r; }
            range.push_back(r);
        }

        if (fabs(maxRange) <= FLT_EPSILON) maxRange = 1.0;

        vector<double> scale(range.size(), 1.0);
        for (int i = 0; i < range.size(); i++) {
            if (range[i] < (maxRange / 10.0) && fabs(range[i]) > FLT_EPSILON) { scale[i] = maxRange / (10.0 * range[i]); }
        }

        transform->SetScales(scale);
    }
}


void ControlExec::_setDefaultOrigin(string datasetName)
{
    DataStatus *ds = GetDataStatus();

    ParamsMgr *    paramsMgr = GetParamsMgr();
    vector<string> winNames = paramsMgr->GetVisualizerNames();

    CoordType minExt, maxExt;

    for (int i = 0; i < winNames.size(); i++) {
        ViewpointParams *   vpParams = paramsMgr->GetViewpointParams(winNames[i]);
        Transform *         transform = vpParams->GetTransform(datasetName);
        std::vector<double> origin = transform->GetOrigin();

        // Skip if not default
        bool skip = false;
        for (int i = 0; i < 3; i++)
            if (origin[i] != 0.f) skip = true;
        if (skip) continue;

        size_t ts = 0;
        ds->GetActiveExtents(paramsMgr, winNames[i], datasetName, ts, minExt, maxExt);

        for (int i = 0; i < minExt.size(); i++) origin[i] = (maxExt[i] + minExt[i]) / 2;

        transform->SetOrigin(origin);
    }
}

bool ControlExec::WasDataCacheDirty() const {
    return _dataStatus->WasCacheDirty() || _calcEngineMgr->_wasCacheDirty;
}