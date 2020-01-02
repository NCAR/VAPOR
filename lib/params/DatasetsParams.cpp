
#include <string>
#include <vapor/ParamsBase.h>
#include <vapor/DatasetsParams.h>

namespace VAPoR {

//
// Register class with object factory!!!
//
static ParamsRegistrar<DatasetsParams>              registrar(DatasetsParams::GetClassType());
static ParamsRegistrar<DatasetParams>               registrar1(DatasetParams::GetClassType());
static ParamsRegistrar<DatasetParams::ScriptParams> registrar2(DatasetParams::ScriptParams::GetClassType());

const string DatasetsParams::_datasetsTag = "Datasets";

const string DatasetParams::_datasetTag = "Dataset";
const string DatasetParams::_scriptsTag = "Scripts";

const string DatasetParams::ScriptParams::_scriptTag = "Script";
const string DatasetParams::ScriptParams::_inputVarNamesTag = "InputVarNames";
const string DatasetParams::ScriptParams::_outputVarNamesTag = "OutputVarNames";
const string DatasetParams::ScriptParams::_outputVarMeshesTag = "OutputVarMeshes";
const string DatasetParams::ScriptParams::_coordFlagTag = "CoordFlag";

// DatasetsParams class: A collection of data sets
//

DatasetsParams::DatasetsParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, DatasetsParams::GetClassType())
{
    SetDiagMsg("DatasetsParams::DatasetsParams() this=%p", this);

    _datasets = new ParamsContainer(ssave, _datasetsTag);
    _datasets->SetParent(this);
}

DatasetsParams::DatasetsParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    SetDiagMsg("DatasetsParams::DatasetsParams() this=%p", this);

    if (node->HasChild(_datasetsTag)) {
        _datasets = new ParamsContainer(ssave, node->GetChild(_datasetsTag));
    } else {
        // Node doesn't contain a contours container
        _datasets = new ParamsContainer(ssave, _datasetsTag);
        _datasets->SetParent(this);
    }
}

DatasetsParams::DatasetsParams(const DatasetsParams &rhs) : ParamsBase(rhs) { _datasets = new ParamsContainer(*(rhs._datasets)); }

DatasetsParams &DatasetsParams::operator=(const DatasetsParams &rhs)
{
    if (_datasets) delete _datasets;

    ParamsBase::operator=(rhs);
    _datasets = new ParamsContainer(*(rhs._datasets));

    return (*this);
}

DatasetsParams::~DatasetsParams()
{
    SetDiagMsg("DatasetsParams::~DatasetsParams() this=%p", this);

    if (_datasets != NULL) {
        delete _datasets;
        _datasets = NULL;
    }
}

void DatasetsParams::SetScript(string datasetName, string name, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes,
                               bool coordFlag)
{
    DatasetParams *s = (DatasetParams *)_datasets->GetParams(datasetName);
    if (s == NULL) {
        DatasetParams sParams(_ssave);

        _datasets->Insert(&sParams, datasetName);
        s = (DatasetParams *)_datasets->GetParams(datasetName);
        VAssert(s);
    }

    s->SetScript(name, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);
}

bool DatasetsParams::GetScript(string datasetName, string name, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes, bool &coordFlag) const
{
    script.clear();
    inputVarNames.clear();
    outputVarNames.clear();
    outputVarMeshes.clear();

    DatasetParams *s = (DatasetParams *)_datasets->GetParams(datasetName);
    if (s == NULL) return (false);

    s->GetScript(name, script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);

    return (true);
}

void DatasetsParams::RemoveScript(string datasetName, string scriptName)
{
    DatasetParams *s = (DatasetParams *)_datasets->GetParams(datasetName);
    if (s == NULL) return;
    s->RemoveScript(scriptName);
}

vector<string> DatasetsParams::GetScriptNames(string datasetName) const
{
    DatasetParams *s = (DatasetParams *)_datasets->GetParams(datasetName);
    if (s == NULL) return (vector<string>());
    return (s->GetScriptNames());
}

// DatasetParams class: A single data set
//
DatasetParams::DatasetParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, DatasetParams::GetClassType())
{
    SetDiagMsg("DatasetParams::DatasetParams() this=%p", this);

    _scripts = new ParamsContainer(ssave, _scriptsTag);
    _scripts->SetParent(this);
}

DatasetParams::DatasetParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    SetDiagMsg("DatasetParams::DatasetParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != DatasetParams::GetClassType()) { node->SetTag(DatasetParams::GetClassType()); }

    if (node->HasChild(_scriptsTag)) {
        _scripts = new ParamsContainer(ssave, node->GetChild(_scriptsTag));
    } else {
        // Node doesn't contain a contours container
        _scripts = new ParamsContainer(ssave, _scriptsTag);
        _scripts->SetParent(this);
    }
}

DatasetParams::DatasetParams(const DatasetParams &rhs) : ParamsBase(rhs) { _scripts = new ParamsContainer(*(rhs._scripts)); }

DatasetParams &DatasetParams::operator=(const DatasetParams &rhs)
{
    if (_scripts) delete _scripts;

    ParamsBase::operator=(rhs);
    _scripts = new ParamsContainer(*(rhs._scripts));

    return (*this);
}

DatasetParams::~DatasetParams()
{
    SetDiagMsg("DatasetParams::~DatasetParams() this=%p", this);

    if (_scripts != NULL) {
        delete _scripts;
        _scripts = NULL;
    }
}

void DatasetParams::SetScript(string name, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames, const vector<string> &outputVarMeshes, bool coordFlag)
{
    ScriptParams *s = (ScriptParams *)_scripts->GetParams(name);
    if (s == NULL) {
        ScriptParams sParams(_ssave);

        _scripts->Insert(&sParams, name);
        s = (ScriptParams *)_scripts->GetParams(name);
        VAssert(s);
    }

    s->SetScript(script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);
}

bool DatasetParams::GetScript(string name, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes, bool &coordFlag) const
{
    script.clear();
    inputVarNames.clear();
    outputVarNames.clear();
    outputVarMeshes.clear();

    ScriptParams *s = (ScriptParams *)_scripts->GetParams(name);
    if (s == NULL) return (false);

    s->GetScript(script, inputVarNames, outputVarNames, outputVarMeshes, coordFlag);

    return (true);
}

};    // end namespace VAPoR
