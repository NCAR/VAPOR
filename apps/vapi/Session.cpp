#include "Session.h"

#include <vapor/GUIStateParams.h>
#include <vapor/AnimationParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/NavigationUtils.h>
#include <vapor/ParamsMgr.h>
#include <vapor/FileUtils.h>

#include "RenderManager.h"

using namespace VAPoR;

Session::Session()
{
    vector<string> myParams;
    myParams.push_back(GUIStateParams::GetClassType());
    myParams.push_back(AnimationParams::GetClassType());
    vector<string> myRenParams;
    _controlExec = new ControlExec(myParams, myRenParams);
    _renderManager = new RenderManager(_controlExec);

    Reset();
}

Session::~Session()
{
    delete _renderManager;
    delete _controlExec;
}

void Session::CloseDataset(String name)
{
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
    GUIStateParams *p = getGUIStateParams();
    vector<String>  options = {"-project_to_pcs", "-vertical_xform"};
    // TODO: autostretch
    //    if (GetSettingsParams()->GetAutoStretchEnabled()) options.push_back("-auto_stretch_z");

    if (!p->GetProjectionString().empty()) {
        options.push_back("-proj4");
        options.push_back(p->GetProjectionString());
    }

    if (name.empty()) name = ControlExec::MakeStringConformant(FileUtils::Basename(files[0]));

    return OpenDataset(name, format, files, options);
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

    _controlExec->UndoRedoClear();
    return 0;
}

void Session::Reset()
{
    CloseAllDatasets();
    _controlExec->LoadState();
    //    _controlExec->NewVisualizer("viz_1");
}

int Session::Render(String imagePath)
{
    if (!_controlExec->GetParamsMgr()->GetDataMgrNames().size()) {
        LogWarning("Nothing to render");
        return -1;
    }

    return _renderManager->Render(imagePath);
}

void Session::SetTimestep(int ts) { NavigationUtils::SetTimestep(_controlExec, ts); }

void Session::loadAllParamsDatasets()
{
    auto dataSetNames = _controlExec->GetParamsMgr()->GetDataMgrNames();

    for (int i = 0; i < dataSetNames.size(); i++) {
        string         name = dataSetNames[i];
        string         format;
        vector<string> paths;
        getParamsDatasetInfo(name, &format, &paths);
        if (std::all_of(paths.begin(), paths.end(), FileUtils::Exists)) {
            if (OpenDataset(format, paths, name) < 0)
                getGUIStateParams()->RemoveOpenDateSet(name);
        } else {
            getGUIStateParams()->RemoveOpenDateSet(name);

            string err = "This session links to the dataset " + name + " which was not found. Please open this dataset if it is in a different location";

            string details;
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
