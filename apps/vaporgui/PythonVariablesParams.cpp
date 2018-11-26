#include "PythonVariablesParams.h"

const string PythonVariablesParams::_pythonScriptsTag = "PythonScripts";

const string PythonScript::_pythonScriptTag = "PythonScript";
const string PythonScript::_scritpNameTag = "PythonScriptName";
const string PythonScript::_dataMgrNameTag = "DataMgrName";
const string PythonScript::_dataMgrGridsTag = "DataMgrGrids";
const string PythonScript::_inputVarsTag = "InputVars";
const string PythonScript::_outputVarsTag = "OutputVars";
const string PythonScript::_outputVarGridsTag = "OutputVarGrids";
const string PythonScript::_dataMgrNameTag = "DataMgrName";

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

string PythonScript::GetScript() const
{
    string defaultScript = "";
    script = GetValueString(_pythonScriptTag, defaultScript);
}

void PythonScript::SetScript(string script) { SetValueString(_pythonScriptTag, "Set Python Script", script); }

string PythonScript::GetScriptName() const
{
    string defaultName = "default.py";
    GetValueString(_pythonScriptName, defaultName);
}

void PythonScript::SetScriptName(string scriptName) { SetValueString(_pythonScriptName, "Set Python Script Name", scriptName); }

std::vector<string> PythonScript::GetOutputVars() const { return GetValueStringVec(_outputVarsTag); }

void PythonScript::SetOutputVars(std::vector<string> vars) { SetValueStringVec(_outputVarsTag, "Set Python output variables", vars); }

std::vector<string> PythonScript::GetOutputVars() const { return GetValueStringVec(_outputVarsTag); }

void PythonScript::SetOutputGrids(std::vector<string> grids) { SetValueStringVec(_outputGridsTag, "Set Python output grids", grids); }

std::vector<string> PythonScript::GetOutputGrids() const { return GetValueStringVec(_outputGridsTag); }

void PythonScript::SetInputVars(std::vector<string> vars) { SetValueStringVec(_inputVarsTag, "Set Python input variables", vars); }

std::vector<string> PythonScript::GetInputVars() const { return GetValueStringVec(_inputVarsTag); }

void PythonScript::SetInputGrids(std::vector<string> grids) { SetValueStringVec(_inputGridsTag, "Set Python input grids", grids); }

std::vector<string> PythonScript::GetInputGrids() const { return GetValueStringVec(_inputGridsTag); }

string GetDataMgr() const { return GetValueString(_dataMgrNameTag, ""); }

void PythonScript::SetDataMgr(string dataMgrName) { SetValueString(_dataMgrNameTag, "Set DataMgr name for Python script", dataMgrName); }
