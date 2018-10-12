#include "PythonVariablesParams.h"

const string PythonVariablesParams::_pythonScriptsTag = "PythonScripts";

const string PythonScript::_pythonScriptTag = "PythonScript";
const string PythonScript::_scritpNameTag = "PythonScriptName";
const string PythonScript::_dataMgrNameTag = "DataMgrName";
const string PythonScript::_dataMgrGridsTag = "DataMgrGrids";
const string PythonScript::_inputVarsTag = "InputVars";
const string PythonScript::_outputVarsTag = "OutputVars";
const string PythonScript::_outputVarGridsTag = "OutputVarGrids";

static ParamsRegistrar<PythonVariablesParams> registrar(PythonVariablesParams::GetClassType());

static ParamsRegistrar<PythonScript> registrar_ps(PythonScript::GetClassType());

PythonVariablesParams::PythonVariablesParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : ParamsBase(ssave, GetClassType())
{
    _pythonScripts = new ParamsContainer(ssave, _pythonScriptsTag);
    _pythonScripts->SetParent(this);
}

PythonVariablesParams::PythonVariablesParams(DataMgr *dataMgr, XmlNode *node) : ParamsBase(ssave, node)
{
    if (node->HasChild(_pythonScriptsTag)) {
        _pythonScripts = new ParamsContainer(ssave, node->GetChild(_pythonScriptsTag));
    } else {
        _pythonScripts = new ParamsContainer(ssave, _pythonScriptsTag);
    }
}

PythonScript::PythonScript(VAPoR::ParamsBase::StateSave *ssave) : ParamsBase(ssave, PythonScript::GetClassType()) {}

PythonScript::PythonScript(VAPOR::ParamsBase::StateSave *ssave, VAPOR::XmlNode *node) : ParamsBase(ssave, node){};

string GetScript() const
{
    string defaultScript = "";
    script = GetValueString(_pythonScriptTag, defaultScript);
}
