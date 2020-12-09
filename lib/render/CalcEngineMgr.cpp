#include <iostream>
#include <vapor/CalcEngineMgr.h>
#include <vapor/DataStatus.h>
#include <vapor/ParamsMgr.h>
#include <vapor/PyEngine.h>
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

    _sync();

    PyEngine *                                   pyEngine = NULL;
    std::map<string, PyEngine *>::const_iterator itr;
    itr = _pyScripts.find(dataSetName);
    if (itr == _pyScripts.cend()) {
        pyEngine = new PyEngine(dataMgr);

        int rc = pyEngine->Initialize();
        if (rc < 0) {
            delete pyEngine;
            return (rc);
        }

        _pyScripts[dataSetName] = pyEngine;
    } else {
        pyEngine = itr->second;
    }

    int rc = pyEngine->AddFunction(scriptName, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);
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

    _sync();

    // Destroy the PyEngine class
    //
    std::map<string, PyEngine *>::const_iterator itr;
    itr = _pyScripts.find(dataSetName);
    if (itr != _pyScripts.cend()) {
        PyEngine *pyEngine = itr->second;
        if (pyEngine) delete pyEngine;
        _pyScripts.erase(itr);
    }

    // And remove from the params database
    //
    _paramsMgr->GetDatasetsParams()->RemoveScript(dataSetName, scriptName);
}

vector<string> CalcEngineMgr::GetFunctionNames(string scriptType, string dataSetName)
{
    if (scriptType != "Python") return (vector<string>());

    _sync();

    std::map<string, PyEngine *>::const_iterator itr;
    itr = _pyScripts.find(dataSetName);
    if (itr == _pyScripts.cend()) return (vector<string>());

    PyEngine *pyEngine = itr->second;

    return (pyEngine->GetFunctionNames());
}

bool CalcEngineMgr::GetFunctionScript(string scriptType, string dataSetName, string scriptName, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames,
                                      vector<string> &outputVarMeshes, bool &coordFlag)
{
    script.clear();
    inputVarNames.clear();
    outputVarNames.clear();
    outputVarMeshes.clear();

    if (scriptType != "Python") return (false);

    _sync();

    std::map<string, PyEngine *>::const_iterator itr;
    itr = _pyScripts.find(dataSetName);
    if (itr == _pyScripts.cend()) return (false);

    PyEngine *pyEngine = itr->second;

    return (pyEngine->GetFunctionScript(scriptName, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag));
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

void CalcEngineMgr::_sync()
{
    // Remove PyEngine instances
    //
    _clean();

    // Reubild PyEngine instances from database
    //

    vector<string>  dataSetNames = _paramsMgr->GetDataMgrNames();
    DatasetsParams *dParams = _paramsMgr->GetDatasetsParams();
    VAssert(dParams);

    for (int i = 0; i < dataSetNames.size(); i++) {
        PyEngine *pyEngine = new PyEngine(_dataStatus->GetDataMgr(dataSetNames[i]));
        int       rc = pyEngine->Initialize();
        VAssert(rc >= 0);

        _pyScripts[dataSetNames[i]] = pyEngine;

        vector<string> scriptNames = dParams->GetScriptNames(dataSetNames[i]);

        for (int j = 0; j < scriptNames.size(); j++) {
            string         script;
            vector<string> inputVarNames;
            vector<string> outputVarNames;
            vector<string> outputVarMeshes;
            bool           coordFlag;

            dParams->GetScript(dataSetNames[i], scriptNames[j], script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);

            // Disable error reporting
            //
            bool errEnabled = MyBase::GetEnableErrMsg();
            EnableErrMsg(false);

            (void)pyEngine->AddFunction(scriptNames[j], script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);

            EnableErrMsg(errEnabled);
        }
    }
}
