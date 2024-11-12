#include <iostream>
#include <vapor/CalcEngineMgr.h>
#include <vapor/DataStatus.h>
#include <vapor/ParamsMgr.h>
#include <vapor/PyEngine.h>
#include <vapor/STLUtils.h>
using namespace Wasp;
using namespace VAPoR;

using namespace VAPoR;
using namespace Wasp;

int CalcEngineMgr::AddFunction(string scriptType, string dataSetName, string scriptName, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames,
                               const vector<string> &outputVarMeshes, bool coordFlag)
{
    if (scriptType != "Python") {
        SetErrMsg("Unsupported script type %s", scriptType.c_str());
        return (-1);
    }

    DataMgr *dataMgr = _dataStatus->GetDataMgr(dataSetName);
    if (!dataMgr) {
        SetErrMsg("Invalid data set name %s", dataSetName.c_str());
        return (-1);
    }

    int rc = _pyScripts[dataSetName]->AddFunction(scriptName, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);
    if (rc < 0) return (-1);

    // If the output variable had a previous definition we need to purge
    // the variable from the RenderParams mapper functions :-(
    //
    vector<RenderParams *> rParams;
    _paramsMgr->GetRenderParams(rParams);
    for (int j = 0; j < rParams.size(); j++) {
        for (int i = 0; i < outputVarNames.size(); i++) { rParams[j]->RemoveMapperFunc(outputVarNames[i]); }
    }

    _paramsMgr->GetDatasetsParams()->SetScript(dataSetName, scriptName, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);

    return (0);
}

void CalcEngineMgr::RemoveFunction(string scriptType, string dataSetName, string scriptName)
{
    if (scriptType != "Python") return;

    string script;
    vector<string> inputVarNames, outputVarNames, inputVarMeshes;
    bool coordFlag;
    _paramsMgr->GetDatasetsParams()->GetScript(dataSetName, scriptName, script, inputVarNames, outputVarNames, inputVarMeshes, coordFlag);
    _paramsMgr->GetDatasetsParams()->RemoveScript(dataSetName, scriptName);
    SetCacheDirty();
}

vector<string> CalcEngineMgr::GetFunctionNames(string scriptType, string dataSetName)
{
    if (scriptType != "Python") return (vector<string>());
    return _paramsMgr->GetDatasetsParams()->GetScriptNames(dataSetName);
}

bool CalcEngineMgr::GetFunctionScript(string scriptType, string dataSetName, string scriptName, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames,
                                      vector<string> &outputVarMeshes, bool &coordFlag)
{
    script.clear();
    inputVarNames.clear();
    outputVarNames.clear();
    outputVarMeshes.clear();

    if (scriptType != "Python") return (false);

    return _paramsMgr->GetDatasetsParams()->GetScript(dataSetName, scriptName, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);
}

string CalcEngineMgr::GetFunctionStdout(string scriptType, string dataSetName, string scriptName)
{
    if (scriptType != "Python") return ("");

    std::map<string, PyEngine *>::const_iterator itr;
    itr = _pyScripts.find(dataSetName);
    if (itr == _pyScripts.cend()) return ("");

    PyEngine *pyEngine = itr->second;

    return (pyEngine->GetFunctionStdout(scriptName));
}

CalcEngineMgr::~CalcEngineMgr() { _clean(); }

void CalcEngineMgr::_clean()
{
    std::map<string, PyEngine *>::iterator itr;
    while ((itr = _pyScripts.begin()) != _pyScripts.end()) {
        PyEngine *pyEngine = itr->second;
        if (pyEngine) delete pyEngine;

        _pyScripts.erase(itr);
    }
}

void CalcEngineMgr::ReinitFromState()
{
    _clean();
    SyncWithParams();
    SetCacheDirty();
}

void CalcEngineMgr::SyncWithParams()
{
    const vector<string> datasetNames = _dataStatus->GetDataMgrNames();
    DatasetsParams *datasetsParams = _paramsMgr->GetDatasetsParams();

    for (const auto &name : STLUtils::SyncToRemove(datasetNames, STLUtils::MapKeys(_pyScripts))) {
        delete _pyScripts[name];
        _pyScripts.erase(name);
    }

    for (const auto &name : STLUtils::SyncToAdd(datasetNames, STLUtils::MapKeys(_pyScripts))) {
        _pyScripts[name] = new PyEngine(_dataStatus->GetDataMgr(name));
        _pyScripts[name]->Initialize();
    }

    for (const auto &dataset : datasetNames) {
        const auto funcNames = datasetsParams->GetScriptNames(dataset);
        auto engine = _pyScripts[dataset];

        for (const auto &name : STLUtils::SyncToRemove(funcNames, engine->GetFunctionNames())) {
            engine->RemoveFunction(name);
            SetCacheDirty();
        }

        for (const auto &name : engine->GetFunctionNames()) {
            string script, p_script;
            vector<string> inputVarNames, outputVarNames, outputMeshNames, p_inputVarNames, p_outputVarNames, p_outputMeshNames;
            bool coordFlag, p_coordFlag;
            engine->GetFunctionScript(name, script, inputVarNames, outputVarNames, outputMeshNames, coordFlag);
            datasetsParams->GetScript(dataset, name, p_script, p_inputVarNames, p_outputVarNames, p_outputMeshNames, p_coordFlag);
#define T(v) (v != p_##v)
            if (T(script) || T(inputVarNames) || T(outputVarNames) || T(outputMeshNames) || T(coordFlag)) {
                engine->RemoveFunction(name);
                engine->AddFunction(name, p_script, p_inputVarNames, p_outputVarNames, p_outputMeshNames, p_coordFlag);
                SetCacheDirty();
            }
        }

        for (const auto &name : STLUtils::SyncToAdd(funcNames, engine->GetFunctionNames())) {
            string p_script;
            vector<string> p_inputVarNames, p_outputVarNames, p_outputMeshNames;
            bool p_coordFlag;
            datasetsParams->GetScript(dataset, name, p_script, p_inputVarNames, p_outputVarNames, p_outputMeshNames, p_coordFlag);
            engine->AddFunction(name, p_script, p_inputVarNames, p_outputVarNames, p_outputMeshNames, p_coordFlag);
        }
    }

    _wasCacheDirty = _isDataCacheDirty;
    _isDataCacheDirty = false;
}
