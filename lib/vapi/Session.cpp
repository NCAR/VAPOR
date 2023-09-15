#include "Session.h"

#include <vapor/GUIStateParams.h>
#include <vapor/AnimationParams.h>
#include <vapor/SettingsParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/NavigationUtils.h>
#include <vapor/ParamsMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/STLUtils.h>
#include <vapor/PythonDataMgr.h>

#include <vapor/RenderManager.h>

using namespace VAPoR;

Session::Session()
{
    vector<string> myParams;
    myParams.push_back(GUIStateParams::GetClassType());
    myParams.push_back(AnimationParams::GetClassType());
    myParams.push_back(SettingsParams::GetClassType());
    vector<string> myRenParams;
    _controlExec = new ControlExec(myParams, myRenParams);

    Reset();
}

Session::~Session()
{
    delete _renderManager;
    delete _controlExec;
}

void Session::CloseDataset(String name)
{
    getGUIStateParams()->RemoveOpenDateSet(name);
    _controlExec->CloseData(name);
    _controlExec->UndoRedoClear();
}

void Session::CloseAllDatasets()
{
    auto datasetNames = _controlExec->GetDataNames();
    for (auto &name : datasetNames) { CloseDataset(name); }
}

int Session::OpenDataset(String name, String format, const vector<String> &files, const vector<String> &options)
{
    int rc = _controlExec->OpenData(files, options, name, format);
    if (rc < 0) {
        LogWarning("Failed to load data '%s'", files.empty() ? name.c_str() : files[0].c_str());
        return rc;
    }

    GUIStateParams *p = getGUIStateParams();
    DataStatus *    ds = _controlExec->GetDataStatus();
    bool            isFirstDataset = p->GetOpenDataSetNames().empty();
    p->SetProjectionString(ds->GetMapProjection());
    p->InsertOpenDateSet(name, format, files);

    if (isFirstDataset) { NavigationUtils::ViewAll(_controlExec); }

    _controlExec->UndoRedoClear();
    return 0;
}

int Session::OpenDataset(String format, const vector<String> &files, String name)
{
    VAssert(!name.empty());
    
    GUIStateParams *p = getGUIStateParams();
    vector<String>  options = {"-project_to_pcs", "-vertical_xform"};
    if (getSettingsParams()->GetAutoStretchEnabled()) options.push_back("-auto_stretch_z");

    if (!p->GetProjectionString().empty()) {
        options.push_back("-proj4");
        options.push_back(p->GetProjectionString());
    }

    return OpenDataset(name, format, files, options);
}

String Session::OpenDataset(String format, vector<String> files)
{
    if (files.empty())
        return "";

    String name = ControlExec::MakeStringConformant(FileUtils::Basename(files[0]));
    
    if (OpenDataset(format, files, name) < 0)
        return "";
    return name;
}

int Session::Load(String path)
{
    Reset();

    int rc = _controlExec->LoadState(path);
    if (rc < 0) {
        LogWarning("Session '%s' failed to load", path.c_str());
        return rc;
    }

    auto dataSetNames = _controlExec->GetParamsMgr()->GetDataMgrNames();
    for (auto d : dataSetNames) { printf("Dataset: '%s'\n", d.c_str()); }

    loadAllParamsDatasets();
    
    getGUIStateParams()->SetActiveVizName(getWinName());

    _controlExec->UndoRedoClear();
    return 0;
}

int Session::Save(String path)
{
    for (auto name : _controlExec->GetDataNames()) {
        if (dynamic_cast<PythonDataMgr*>(_controlExec->GetDataStatus()->GetDataMgr(name))) {
            LogWarning("Cannot save session that contains data dynamically loaded from python (dataset.PYTHON)");
            return -1;
        }
    }
            
    
    if (_controlExec->SaveSession(path) < 0) {
        LogWarning("Failed to save session \"%s\"", path.c_str());
        return -1;
    }
    
    getGUIStateParams()->SetCurrentSessionFile(path);
    return 0;
}

void Session::Reset()
{
    CloseAllDatasets();
    _controlExec->LoadState();
    _controlExec->SetCacheSize(getSettingsParams()->GetCacheMB());

    _controlExec->NewVisualizer("viz_1");
    getGUIStateParams()->SetActiveVizName("viz_1");

    
    if (_renderManager) delete _renderManager;
    _renderManager = new RenderManager(_controlExec);
    
    _controlExec->UndoRedoClear();
}

int Session::Render(String imagePath, bool fast)
{
    if (!_controlExec->GetParamsMgr()->GetDataMgrNames().size()) {
        LogWarning("Nothing to render");
        return -1;
    }
    if (_controlExec->GetParamsMgr()->GetDataMgrNames() != getGUIStateParams()->GetOpenDataSetNames()) {
        LogWarning("Cannot render: There are missing datasets");
        return -1;
    }

    return _renderManager->Render(imagePath, fast);
}

void Session::SetTimestep(int ts) { NavigationUtils::SetTimestep(_controlExec, ts); }
int Session::GetTimesteps() const { return _controlExec->GetDataStatus()->getMaxTimestep() + 1; }


void Session::SetWaspMyBaseErrMsgFilePtrToSTDERR()
{
    Wasp::MyBase::SetErrMsgFilePtr(stderr);
}

vector<String> Session::GetDatasetNames() const
{
    return _controlExec->GetDataNames();
}

vector<String> Session::GetRendererNames() const
{
    vector<String> names;
    for (auto cls : _controlExec->GetAllRenderClasses())
        for (auto inst : _controlExec->GetRenderInstances(getWinName(), cls))
            names.push_back(inst);
    return names;
}

String Session::NewRenderer(String type, String dataset)
{
    auto all = _controlExec->GetAllRenderClasses();
    VAssert(STLUtils::Contains(all, type));
    VAssert(!_controlExec->GetDataNames().empty());
    
    if (dataset.empty())
        dataset = _controlExec->GetDataNames()[0];
    
    String rendererName = _controlExec->MakeRendererNameUnique(type);

    _controlExec->GetParamsMgr()->BeginSaveStateGroup(_controlExec->GetActivateRendererUndoTag());

    int rc = _controlExec->ActivateRender(getWinName(), dataset, type, rendererName, false);
    if (rc < 0) {
        _controlExec->GetParamsMgr()->EndSaveStateGroup();
        return "";
    }

    getGUIStateParams()->SetActiveRenderer(getWinName(), type, rendererName);
    _controlExec->GetParamsMgr()->EndSaveStateGroup();
    
    // ControlExec::RenderLookup
    string _win, _data, _type;
    _controlExec->RenderLookup(rendererName, _win, _data, _type);
    return rendererName;
}


void Session::DeleteRenderer(String name)
{
    string win, data, type;
    _controlExec->RenderLookup(name, win, data, type);
    _controlExec->RemoveRenderer(win, data, type, name, false);
}


String Session::GetPythonWinName() const
{
    return getWinName();
}


void Session::loadAllParamsDatasets()
{
    auto dataSetNames = _controlExec->GetParamsMgr()->GetDataMgrNames();

    for (int i = 0; i < dataSetNames.size(); i++) {
        string         name = dataSetNames[i];
        string         format;
        vector<string> paths;
        getParamsDatasetInfo(name, &format, &paths);
        if (std::all_of(paths.begin(), paths.end(), FileUtils::Exists)) {
            if (OpenDataset(format, paths, name) < 0) getGUIStateParams()->RemoveOpenDateSet(name);
        } else {
            getGUIStateParams()->RemoveOpenDateSet(name);

            string details = "This session links to the dataset " + name + " which was not found. Please open this dataset if it is in a different location\n";

            for (const auto &path : paths)
                if (!FileUtils::Exists(path)) details += "\"" + path + "\" not found.\n";

            LogWarning("%s", details.c_str());
        }
    }
}

void Session::getParamsDatasetInfo(String name, String *type, vector<String> *files)
{
    auto gsp = getGUIStateParams();
    *type = gsp->GetOpenDataSetFormat(name);
    *files = gsp->GetOpenDataSetPaths(name);
}

GUIStateParams *Session::getGUIStateParams() const { return ((GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType())); }

AnimationParams *Session::getAnimationParams() const { return ((AnimationParams *)_controlExec->GetParamsMgr()->GetParams(AnimationParams::GetClassType())); }

SettingsParams *Session::getSettingsParams() const { return ((SettingsParams *)_controlExec->GetParamsMgr()->GetParams(SettingsParams::GetClassType())); }

String Session::getWinName() const
{
//    assert(not _controlExec->GetVisualizerNames().empty());
//    return _controlExec->GetVisualizerNames()[0];
    return _renderManager->GetWinName();
}
