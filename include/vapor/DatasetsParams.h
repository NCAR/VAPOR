
#pragma once

#include <string>
#include <vapor/ParamsBase.h>

namespace VAPoR {

//
// A collection of Dataset params
//
class PARAMS_API DatasetsParams : public ParamsBase {
public:
    DatasetsParams(ParamsBase::StateSave *ssave);

    DatasetsParams(ParamsBase::StateSave *ssave, XmlNode *node);

    DatasetsParams(const DatasetsParams &rhs);

    DatasetsParams &operator=(const DatasetsParams &rhs);

    virtual ~DatasetsParams();

    void SetScript(string datasetName, string name, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes);

    bool GetScript(string datasetName, string name, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes) const;

    void RemoveDataset(string datasetName) { _datasets->Remove(datasetName); }

    void RemoveScript(string datasetName, string scriptName);

    vector<string> GetScriptNames(string datasetName) const;

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("DatasetsParams"); }

private:
    static const string _datasetsTag;

    ParamsContainer *_datasets;
};

//
// Dataset params
//
class PARAMS_API DatasetParams : public ParamsBase {
public:
    DatasetParams(ParamsBase::StateSave *ssave);

    DatasetParams(ParamsBase::StateSave *ssave, XmlNode *node);

    DatasetParams(const DatasetParams &rhs);

    DatasetParams &operator=(const DatasetParams &rhs);

    virtual ~DatasetParams();

    void SetScript(string name, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes);

    bool GetScript(string name, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes) const;

    void RemoveScript(string name) { _scripts->Remove(name); }

    vector<string> GetScriptNames() const { return (_scripts->GetNames()); }

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("DatasetParams"); }

    class PARAMS_API ScriptParams : public ParamsBase {
    public:
        ScriptParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, ScriptParams::GetClassType()) {}

        ScriptParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {}

        virtual ~ScriptParams() {}

        void SetScript(string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes)
        {
            _ssave->BeginGroup("Set derived variable script");

            SetValueString(_scriptTag, "", script);
            SetValueStringVec(_inputVarNamesTag, "", inputVarNames);
            SetValueStringVec(_outputVarNamesTag, "", outputVarNames);
            SetValueStringVec(_outputVarMeshes, "", outputVarMeshes);

            _ssave->EndGroup();
        }

        void GetScript(string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes)
        {
            script = GetValueString(_scriptTag, "");
            inputVarNames = GetValueStringVec(_inputVarNamesTag);
            outputVarNames = GetValueStringVec(_outputVarNamesTag);
            outputVarMeshes = GetValueStringVec(_outputVarMeshes);
        }

        static string GetClassType() { return ("ScriptParams"); }

    private:
        static const string _scriptTag;
        static const string _inputVarNamesTag;
        static const string _outputVarNamesTag;
        static const string _outputVarMeshes;
    };

private:
    static const string _datasetTag;
    static const string _scriptsTag;

    ParamsContainer *_scripts;

};    // End of Class DatasetParams

};    // namespace VAPoR
