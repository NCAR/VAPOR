//-- ControlExec.cpp ----------------------------------------------------------------

// Implementation of ControlExec methods
//----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <vapor/GetAppPath.h>
#include <vapor/ParamsMgr.h>
#include <vapor/ShaderMgr.h>
#include <vapor/ControlExecutive.h>

using namespace VAPoR;
using namespace std;

ControlExec::ControlExec(vector<string> appParamsNames, size_t cacheSizeMB, int nThreads) : MyBase()
{
    _paramsMgr = new ParamsMgr(appParamsNames);
    _dataStatus = new DataStatus(cacheSizeMB, nThreads);
    _shaderMgrs.clear();
    _visualizers.clear();
}

ControlExec::~ControlExec()
{
    cout << "Allocated XmlNode count before delete " << XmlNode::GetAllocatedNodes().size() << endl;

//#define DEBUG
#ifdef DEBUG
    const vector<XmlNode *> &nodes = XmlNode::GetAllocatedNodes();
    for (int i = 0; i < nodes.size(); i++) { cout << "   " << nodes[i]->GetTag() << " " << XmlNode::streamOut(cout, nodes[i]) << endl; }
#endif

    if (_paramsMgr) delete _paramsMgr;
    if (_dataStatus) delete _dataStatus;

    cout << "Allocated XmlNode count after delete " << XmlNode::GetAllocatedNodes().size() << endl;

#ifdef DEBUG

    for (int i = 0; i < nodes.size(); i++) { cout << "   " << nodes[i]->GetTag() << " " << XmlNode::streamOut(cout, nodes[i]) << endl; }
#endif
}

int ControlExec::NewVisualizer(string winName)
{
    // Remove if already exists. Else no-op
    //
    RemoveVisualizer(winName);

    // Need to create a params instance for this visualizer
    //
    ViewpointParams *vpParams = _paramsMgr->CreateVisualizerParamsInstance(winName);
    if (!vpParams) {
        SetErrMsg("Failed to create Visualizer parameters");
        return -1;
    }

    Visualizer *viz = new Visualizer(_paramsMgr, _dataStatus, winName);
    _visualizers[winName] = viz;

    return (0);
}

void ControlExec::RemoveVisualizer(string winName)
{
    map<string, ShaderMgr *>::iterator itr = _shaderMgrs.find(winName);
    if (itr != _shaderMgrs.end()) {
        delete itr->second;
        _shaderMgrs.erase(itr);
    }

    std::map<string, Visualizer *>::iterator itr2 = _visualizers.find(winName);
    if (itr2 != _visualizers.end()) {
        delete itr2->second;
        _visualizers.erase(itr2);
    }
}

int ControlExec::InitializeViz(string winName)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    // initialization of ShaderMgr. Do we really need more than
    // one ShaderMgr (one for each window?)
    //
    vector<string> paths;
    paths.push_back("shaders");

    ShaderMgr *shaderMgr = new ShaderMgr();

    string shaderPath = GetAppPath("VAPOR", "share", paths);
    shaderMgr->SetShaderSourceDir(shaderPath);
    int rc = shaderMgr->LoadShaders();
    if (rc < 0) {
        SetErrMsg("Failed to initialize GLSL shaders in dir %s", shaderPath.c_str());
        printf("%s\n", GetErrMsg());
        delete shaderMgr;
        return (-1);
    }
    _shaderMgrs[winName] = shaderMgr;

    if (v->initializeGL(shaderMgr) < 0) {
        SetErrMsg("InitializeGL failure");
        return -1;
    }

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

int ControlExec::Paint(string winName, bool force)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

#ifdef DEAD
    if (!force) {
        if (!VizWinParams::VizIsDirty(viz)) return 0;    // Do nothing
    }
#endif

    // Disable state saving when generating the transfer function
    //
    bool enabled = _paramsMgr->GetSaveStateEnabled();
    _paramsMgr->SetSaveStateEnabled(false);

    int rc = v->paintEvent();

    _paramsMgr->SetSaveStateEnabled(enabled);

    if (rc) SetErrMsg("Error performing paint event");
    return rc;
}

int ControlExec::ActivateRender(string winName, string dataSetName, string renderType, string renderName, bool on)
{
    if (!_dataStatus->GetDataMgrNames().size()) {
        SetErrMsg("Invalid state : no data");
        return -1;
    }

    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    Renderer *ren = v->getRenderer(renderType, renderName);

    _paramsMgr->BeginSaveStateGroup("ActivateRender");

    if (!ren) {
        string paramsType = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);

        assert(!paramsType.empty());

        // Need to create a params instance for this renderer
        //
        RenderParams *rp = _paramsMgr->CreateRenderParamsInstance(winName, dataSetName, paramsType, renderName);
        if (!rp) {
            SetErrMsg("Invalid renderer of type \"%s\"", renderType.c_str());
            _paramsMgr->EndSaveStateGroup();
            return -1;
        }

        ren = RendererFactory::Instance()->CreateInstance(_paramsMgr, winName, dataSetName, renderType, renderName, _dataStatus->GetDataMgr(dataSetName));
        if (!ren) {
            SetErrMsg("Invalid renderer of type \"%s\"", renderType.c_str());
            _paramsMgr->EndSaveStateGroup();
            return (-1);
        }
        v->insertSortedRenderer(ren);
    }

    RenderParams *rp = ren->GetActiveParams();
    assert(rp);

    cout << "ControlExec setting " << renderName << " to " << on << endl;
    rp->SetEnabled(on);

    _paramsMgr->EndSaveStateGroup();

    return 0;
}

int ControlExec::ActivateRender(string winName, string dataSetName, const RenderParams *rp, string renderName, bool on)
{
    assert(rp);

    if (!_dataStatus->GetDataMgrNames().size()) {
        SetErrMsg("Invalid state : no data");
        return -1;
    }

    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    string renderType = RendererFactory::Instance()->GetRenderClassFromParamsClass(rp->GetName());

    Renderer *ren = v->getRenderer(renderType, renderName);

    _paramsMgr->BeginSaveStateGroup("ActivateRender");

    if (!ren) {
        // Need to create a params instance for this renderer
        //
        RenderParams *newRP = _paramsMgr->CreateRenderParamsInstance(winName, dataSetName, renderName, rp);
        if (!newRP) {
            SetErrMsg("Invalid renderer of type \"%s\"", renderType.c_str());
            _paramsMgr->EndSaveStateGroup();
            return -1;
        }

        ren = RendererFactory::Instance()->CreateInstance(_paramsMgr, winName, dataSetName, renderType, renderName, _dataStatus->GetDataMgr(dataSetName));
        if (!ren) {
            SetErrMsg("Invalid renderer of type \"%s\"", renderType.c_str());
            _paramsMgr->EndSaveStateGroup();
            return (-1);
        }
        v->insertSortedRenderer(ren);
    }

    RenderParams *newRP = ren->GetActiveParams();
    assert(newRP);

    newRP->SetEnabled(on);

    _paramsMgr->EndSaveStateGroup();

    return 0;
}

void ControlExec::RemoveRenderer(string winName, string dataSetName, string renderType, string renderName)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) return;

    Renderer *ren = v->getRenderer(renderType, renderName);
    if (!ren) return;

    v->RemoveRenderer(ren);

    delete ren;

    string paramsType = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);

    _paramsMgr->RemoveRenderParamsInstance(winName, dataSetName, paramsType, renderName);
}

void ControlExec::LoadState()
{
    vector<string> vizNames = GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) { RemoveVisualizer(vizNames[i]); }

    _paramsMgr->LoadState();
}

void ControlExec::LoadState(const XmlNode *rootNode)
{
    // Destroy current visualizers
    //
    vector<string> vizNames = GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) { RemoveVisualizer(vizNames[i]); }

    // Set up param state
    //
    _paramsMgr->LoadState(rootNode);

    // Create new visualizers based on new parameter state
    //
    vizNames = _paramsMgr->GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) {
        string      winName = vizNames[i];
        Visualizer *viz = new Visualizer(_paramsMgr, _dataStatus, winName);
        _visualizers[winName] = viz;
    }
}

int ControlExec::LoadState(string stateFile)
{
    vector<string> vizNames = GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) { RemoveVisualizer(vizNames[i]); }

    int rc = _paramsMgr->LoadState(stateFile);
    if (rc < 0) return (-1);

    vizNames = _paramsMgr->GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) {
        string      winName = vizNames[i];
        Visualizer *viz = new Visualizer(_paramsMgr, _dataStatus, winName);
        _visualizers[winName] = viz;
    }

    return (0);
}

int ControlExec::activateClassRenderers(string vizName, string dataSetName, string pClassName, vector<string> instNames, bool reportErrs)
{
    bool errEnabled = MyBase::GetEnableErrMsg();

    for (int i = 0; i < instNames.size(); i++) {
        RenderParams *rp = _paramsMgr->GetRenderParams(vizName, dataSetName, pClassName, instNames[i]);
        assert(rp);

        // Convert from params render type to render type. Sigh
        //
        string rClassName = RendererFactory::Instance()->GetRenderClassFromParamsClass(pClassName);

        if (!reportErrs) { EnableErrMsg(false); }

        int rc = ActivateRender(vizName, dataSetName, rClassName, instNames[i], rp->IsEnabled());
        if (rc < 0) {
            SetErrMsg("Failed to activate render: %s", instNames[i].c_str());
            EnableErrMsg(errEnabled);
            return (rc);
        }
    }

    if (!reportErrs) { EnableErrMsg(errEnabled); }

    return (0);
}

int ControlExec::openDataHelper(bool reportErrs)
{
    // Activate/Create renderers as needed. This is a no-op if renderers
    // already exist
    //
    vector<string> dataSetNames = _paramsMgr->GetDataMgrNames();
    vector<string> vizNames = _paramsMgr->GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) {
        for (int j = 0; j < dataSetNames.size(); j++) {
            vector<string> pClassNames = _paramsMgr->GetRenderParamsClassNames(vizNames[i], dataSetNames[j]);

            for (int k = 0; k < pClassNames.size(); k++) {
                vector<string> instNames = _paramsMgr->GetRenderParamInstances(vizNames[i], dataSetNames[j], pClassNames[k]);

                int rc = activateClassRenderers(vizNames[i], dataSetNames[j], pClassNames[k], instNames, reportErrs);
                if (rc < 0) return (rc);
            }
        }
    }
    return (0);
}

int ControlExec::OpenData(vector<string> files, string dataSetName, string typ)
{
    int rc = _dataStatus->Open(files, dataSetName, typ);
    if (rc < 0) {
        SetErrMsg("Failure to open data set of type \"%s\"", typ.c_str());
        return -1;
    }

    _paramsMgr->AddDataMgr(dataSetName, _dataStatus->GetDataMgr(dataSetName));

    // Re-initialize the ControlExec to match the new state
    //
    rc = openDataHelper(true);

    return (rc);
}

void ControlExec::CloseData(string dataSetName)
{
    _dataStatus->Close(dataSetName);
    _paramsMgr->RemoveDataMgr(dataSetName);
}

int ControlExec::EnableImageCapture(string filename, string winName)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }
    if (v->setImageCaptureEnabled(true, filename)) return -1;
    return 0;
}

int ControlExec::EnableAnimationCapture(string winName, bool onOff, string filename)
{
    Visualizer *v = getVisualizer(winName);
    if (!v) {
        SetErrMsg("Invalid Visualizer \"%s\"", winName.c_str());
        return -1;
    }

    if (v->setAnimationCaptureEnabled(onOff, filename)) return -1;
    return 0;
}

void ControlExec::undoRedoHelper()
{
    bool enabled = GetSaveStateEnabled();
    SetSaveStateEnabled(false);

    // Destroy current visualizers
    //
    vector<string> vizNames = GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) { RemoveVisualizer(vizNames[i]); }

    // Create new visualizers based on new parameter state
    //
    vizNames = _paramsMgr->GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) {
        string      winName = vizNames[i];
        Visualizer *viz = new Visualizer(_paramsMgr, _dataStatus, winName);
        _visualizers[winName] = viz;
    }

    // Data-dependent re-initialization
    //
    int rc = openDataHelper(false);
    assert(rc >= 0);

    SetSaveStateEnabled(enabled);
}

bool ControlExec::Undo()
{
    // Attempt to undo parameter state
    //
    bool status = _paramsMgr->Undo();
    if (!status) return (status);

    undoRedoHelper();

    return (true);
}

bool ControlExec::Redo()
{
    // Attempt to redo parameter state
    //
    bool status = _paramsMgr->Redo();
    if (!status) return (status);

    undoRedoHelper();

    return (true);
}

void ControlExec::UndoRedoClear() { _paramsMgr->UndoRedoClear(); }

int ControlExec::SaveSession(string filename)
{
    ofstream fileout;
    string   s;

    fileout.open(filename.c_str());
    if (!fileout) {
        SetErrMsg("Unable to open output session file : %M");
        return (-1);
    }

    const XmlNode *node = _paramsMgr->GetXMLRoot();
    XmlNode::streamOut(fileout, *node);
    if (fileout.bad()) {
        SetErrMsg("Unable to write output session file : %M");
        return (-1);
    }

    fileout.close();
    return (0);
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

bool ControlExec::RenderLookup(string instName, string &winName, string &dataSetName, string &renderType) const
{
    string paramsType = RendererFactory::Instance()->GetParamsClassFromRenderClass(renderType);

    return (_paramsMgr->RenderParamsLookup(instName, winName, dataSetName, paramsType));
}

int ControlExec::DrawText(string winName, string text, int x, int y, int size, float color[3], int type)
{
    Visualizer *v = getVisualizer(winName);
    if (v == NULL) {
        string msg = "Could not get Visualizer " + winName;
        SetErrMsg(msg.c_str());
        return -1;
    }

    v->DrawText(text, x, y, size, color, type);

    return 0;
}

int ControlExec::DrawText(string text, int x, int y, int size, float color[3], int type)
{
    vector<string> visNames = GetVisualizerNames();
    for (int i = 0; i < visNames.size(); i++) {
        cout << "Calling DrawText on " << visNames[i] << endl;
        DrawText(visNames[i], text, x, y, size, color, type);
    }

    return 0;
}

int ControlExec::ClearText(string winName)
{
    Visualizer *v = getVisualizer(winName);
    if (v == NULL) {
        string msg = "Could not get Visualizer " + winName;
        SetErrMsg(msg.c_str());
        return -1;
    }

    v->ClearText();

    return 0;
}

int ControlExec::ClearText()
{
    vector<string> visNames = GetVisualizerNames();
    for (int i = 0; i < visNames.size(); i++) { ClearText(visNames[i]); }

    return 0;
}
