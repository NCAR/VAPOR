#ifndef PYTHONVARIABLESPARAMS_H
#define PYTHONVARIABLESPARAMS_H

#include <vapor/ParamsBase.h>

namespace VAPoR {

class PythonScript;

class PythonVariablesParams : public ParamsBase {
public:
    PythonVariablesParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    PythonVariablesParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    ~PythonVariablesParams() {}

    static string GetClassType() { return ("PythonVariablesParams"); }

    std::vector<string> GetPythonScripts() const;

    std::vector<string> GetPythonScriptNames();

    void AddPythonScript(string scriptName);
    void RemovePythonScript(string scriptName);

private:
    VAPoR::ParamsContainer *_pythonScripts;

    static const string _pythonScriptsTag;
};

class PythonScript : public ParamBase {
public:
    PythonScript(VAPoR::ParamsBase::StateSave *ssave);

    PythonScript(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node);

    ~PythonScript() {}

    static string GetClassType() { return ("PythonScript"); }

    string GetScript() const;
    void   SetScript(string script);

    string GetScriptName() const;
    void   SetScriptName(string scriptName);

    std::vector<string> GetOutputVars() const;
    void                SetOutputVars(std::vector<string> outputVars);

    std::vector<string> GetOutputGrids() const;
    void                SetOutputGrids(std::vector<string> grids);

    std::vector<string> GetInputVars() const;
    void                SetInputVars(std::vector<string> inputVars);

    std::vector<string> GetInputGrids() const;
    void                SetInputGrids(std::vector<string> inputGrids);

private:
    string              _pythonScript;
    string              _scriptName;
    DataMgr *           _dataMgr;
    std::vector<string> _grids;
    std::vector<string> _inputVars;
    std::vector<string> _outputVars;
    std::vector<string> _outputVarGrids;

    static const string _pythonScriptTag;
    static const string _scriptNameTag;
    static const string _dataMgrNameTag;
    static const string _dataMgrGridsTag;
    static const string _inputVarsTag;
    static const string _outputVarsTag;
    static const string _outputVarGridsTag;
};

};    // namespace VAPoR

#endif
