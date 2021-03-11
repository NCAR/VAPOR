#include "PythonVariables.h"
#include "ui_PythonVariablesGUI.h"

#include <ctime>
#include <fstream>

#include <QDebug>
#include <QLineEdit>
#include <QThread>
#include <QFileDialog>
#include <QStandardItemModel>

#include <vapor/DC.h>
#include <vapor/DataStatus.h>
#include "vapor/ResourcePath.h"
#include "ErrorReporter.h"
#include "FileOperationChecker.h"

#define READ  true
#define WRITE false

#define TWOD   2
#define THREED 3

using namespace VAPoR;

template<typename T> static void printVector(std::vector<T> v)
{
    for (int i = 0; i < v.size(); i++) cout << v[i] << " ";
    cout << endl;
}

using namespace PythonVariables_;

PythonVariables::PythonVariables(QWidget *parent) : QDialog(parent), Ui_PythonVariablesGUI()
{
    setupUi(this);

    _importScriptButton->hide();
    _exportScriptButton->hide();

    setWindowTitle("Derived variables with Python");

    _script = "";
    _scriptName = "";
    _dataMgrName = "";

    _newItemDialog = new ::NewItemDialog(this);
    _openAndDeleteDialog = new ::OpenAndDeleteDialog(this);

    _justSaved = false;

    string pythonImagePath = Wasp::GetSharePath(string("images") + string("/PythonLogo.png"));

    QPixmap thumbnail(pythonImagePath.c_str());
    _pythonLabel->setPixmap(thumbnail);

    _includeCoordVars = false;
    _variableTabs->removeTab(0);
    ;

    _coordInputVarTable = new VaporTable(_coordVarTable, false, true);
    bool checkboxesEnabled = false;
    _coordInputVarTable->EnableDisableCheckboxes(checkboxesEnabled);
    _coordInputVarTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(0));

    _2DInputVarTable = new VaporTable(_2DVarTable, false, true);
    _2DInputVarTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(0));

    _3DInputVarTable = new VaporTable(_3DVarTable, false, true);
    _3DInputVarTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(0));

    _summaryTable = new VaporTable(_varSummaryTable);
    _summaryTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(0));

    _outputVarTable = new VaporTable(_outputVariablesTable);
    _outputVarTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(VaporTable::ROWS));

    _connectWidgets();

    setModal(true);
}

PythonVariables::~PythonVariables()
{
    if (_coordInputVarTable) {
        delete _coordInputVarTable;
        _coordInputVarTable = nullptr;
    }
    if (_2DInputVarTable) {
        delete _2DInputVarTable;
        _2DInputVarTable = nullptr;
    }
    if (_3DInputVarTable) {
        delete _3DInputVarTable;
        _3DInputVarTable = nullptr;
    }
    if (_varSummaryTable) {
        delete _varSummaryTable;
        _varSummaryTable = nullptr;
    }
    if (_outputVariablesTable) {
        delete _outputVariablesTable;
        _outputVariablesTable = nullptr;
    }
}

void PythonVariables::Update(bool internalUpdate)
{
    if ((_scriptName == "") || (_dataMgrName == "")) {
        _reset();
        _setGUIEnabled(false);
    } else {
        _setGUIEnabled(true);
        if (_includeCoordVars) {
            _coordVarsEnabled.clear();
            _coordVarsEnabled.resize(_coordVars.size(), false);
            _findEnabledCoordinateVariables(_2DVars, _2DVarsEnabled);
            _findEnabledCoordinateVariables(_3DVars, _3DVarsEnabled);
        }
    }

    _scriptNameLabel->setText(QString::fromStdString(_scriptName));
    _dataMgrNameLabel->setText(QString::fromStdString(_dataMgrName));
    _scriptEdit->setText(QString::fromStdString(_script));

    int                 numRows;
    int                 numCols = 2;
    std::vector<string> tableValuesCoords, tableValues2D;
    std::vector<string> tableValues3D, summaryValues;
    _makeInputTableValues(tableValuesCoords, tableValues2D, tableValues3D, summaryValues);

    _coordInputVarTable->blockSignals(true);
    _2DInputVarTable->blockSignals(true);
    _3DInputVarTable->blockSignals(true);
    _summaryTable->blockSignals(true);
    _outputVarTable->blockSignals(true);

    numRows = _coordVars.size();
    _coordInputVarTable->Update(numRows, numCols, tableValuesCoords);

    numRows = _2DVars.size();
    _2DInputVarTable->Update(numRows, numCols, tableValues2D);

    numRows = _3DVars.size();
    _3DInputVarTable->Update(numRows, numCols, tableValues3D);

    numRows = summaryValues.size() / 2;
    _summaryTable->Update(numRows, numCols, summaryValues);

    std::vector<string> outputValues;
    _makeOutputTableValues(outputValues);
    numRows = outputValues.size() / 2;
    _outputVarTable->Update(numRows, numCols, outputValues);
    _outputVarTable->StretchToColumn(1);

    _coordInputVarTable->blockSignals(false);
    _2DInputVarTable->blockSignals(false);
    _3DInputVarTable->blockSignals(false);
    _summaryTable->blockSignals(false);
    _outputVarTable->blockSignals(false);
}

void PythonVariables::_connectWidgets()
{
    connect(_newScriptButton, SIGNAL(clicked()), this, SLOT(_newScript()));
    connect(_openScriptButton, SIGNAL(clicked()), this, SLOT(_openScript()));
    connect(_deleteScriptButton, SIGNAL(clicked()), this, SLOT(_deleteScript()));
    connect(_importScriptButton, SIGNAL(clicked()), this, SLOT(_importScript()));
    connect(_exportScriptButton, SIGNAL(clicked()), this, SLOT(_exportScript()));
    connect(_testScriptButton, SIGNAL(clicked()), this, SLOT(_testScript()));
    connect(_saveScriptButton, SIGNAL(clicked()), this, SLOT(_saveScript()));
    connect(_closeButton, SIGNAL(clicked()), this, SLOT(_closeScript()));

    connect(_coordInputVarTable, SIGNAL(valueChanged(int, int)), this, SLOT(_coordInputVarChanged(int, int)));
    connect(_2DInputVarTable, SIGNAL(valueChanged(int, int)), this, SLOT(_2DInputVarChanged(int, int)));
    connect(_3DInputVarTable, SIGNAL(valueChanged(int, int)), this, SLOT(_3DInputVarChanged(int, int)));

    connect(_coordinatesCheckbox, SIGNAL(stateChanged(int)), this, SLOT(_coordinatesCheckboxClicked(int)));

    connect(_scriptEdit, SIGNAL(textChanged()), this, SLOT(_scriptChanged()));

    connect(_newOutVarButton, SIGNAL(clicked()), this, SLOT(_createNewVariable()));
    connect(_deleteOutVarButton, SIGNAL(clicked()), this, SLOT(_deleteVariable()));
}

void PythonVariables::_setGUIEnabled(bool enabled)
{
    _variableSelectionFrame->setEnabled(enabled);
    _scriptEdit->setEnabled(enabled);
    _testScriptButton->setEnabled(enabled);
    _saveScriptButton->setEnabled(enabled);
    _exportScriptButton->setEnabled(enabled);
    _importScriptButton->setEnabled(enabled);
    _exportScriptButton->setEnabled(enabled);
}

void PythonVariables::_updateNewItemDialog()
{
    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    _newItemDialog->Update(::NewItemDialog::SCRIPT, dataMgrNames);
}

void PythonVariables::_newScript()
{
    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    _newItemDialog->Update(::NewItemDialog::SCRIPT, dataMgrNames);

    _newItemDialog->exec();
    int rc = _newItemDialog->result();

    if (rc > 0) {
        string scriptName = _newItemDialog->GetItemName();
        if (scriptName == "") return;

        scriptName = _controlExec->MakeStringConformant(scriptName);

        _reset();
        _scriptName = scriptName;
        _dataMgrName = _newItemDialog->GetOptionName();

        _scriptNameLabel->setText(QString::fromStdString(_scriptName));
        _dataMgrNameLabel->setText(QString::fromStdString(_dataMgrName));

        VAPoR::DataMgr *dataMgr = dataStatus->GetDataMgr(_dataMgrName);
        _coordVars = dataMgr->GetCoordVarNames();
        _coordVarsEnabled.resize(_coordVars.size());
        std::fill(_coordVarsEnabled.begin(), _coordVarsEnabled.end(), false);

        _2DVars = dataMgr->GetDataVarNames(TWOD);
        _2DVarsEnabled.resize(_2DVars.size());
        std::fill(_2DVarsEnabled.begin(), _2DVarsEnabled.end(), false);

        _3DVars = dataMgr->GetDataVarNames(THREED);
        _3DVarsEnabled.resize(_3DVars.size());
        std::fill(_3DVarsEnabled.begin(), _3DVarsEnabled.end(), false);

        Update(true);
    }
}

void PythonVariables::_reset()
{
    _script = "";
    _scriptName = "";
    _dataMgrName = "";
    _justSaved = false;
    _coordVars.clear();
    _coordVarsEnabled.clear();
    _2DVars.clear();
    _2DVarsEnabled.clear();
    _3DVars.clear();
    _3DVarsEnabled.clear();
    _outputVars.clear();
    _outputGrids.clear();
    _inputGrids.clear();
    _otherGrids.clear();
}

void PythonVariables::_openScript()
{
    int rc = _openAndDeleteDialog->Update(OpenAndDeleteDialog::_OPEN, _controlExec);

    if (rc < 0) return;

    _openAndDeleteDialog->exec();

    rc = _openAndDeleteDialog->result();

    if (rc > 0) {
        _reset();

        string scriptName = _openAndDeleteDialog->GetScriptName();
        string dataMgrName = _openAndDeleteDialog->GetDataMgrName();

        std::vector<string> inputVars;

        bool coordFlag;    // TODO: add support for coordinate flag
        bool rc2 = _controlExec->GetFunction(_scriptType, dataMgrName, scriptName, _script, inputVars, _outputVars, _outputGrids, coordFlag);
        if (rc2 == false) {
            MSG_ERR("Invalid script: " + scriptName);
            return;
        }

        _dataMgrName = dataMgrName;
        _scriptName = scriptName;

        VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
        VAPoR::DataMgr *   dataMgr = dataStatus->GetDataMgr(_dataMgrName);

        _coordVars = dataMgr->GetCoordVarNames();
        _coordVarsEnabled.resize(_coordVars.size(), false);
        _2DVars = dataMgr->GetDataVarNames(TWOD);
        _2DVarsEnabled.resize(_2DVars.size(), false);
        _3DVars = dataMgr->GetDataVarNames(THREED);
        _3DVarsEnabled.resize(_3DVars.size(), false);

        std::vector<string>::iterator it;

        for (int i = 0; i < inputVars.size(); i++) {
            string inVar = inputVars[i];

            it = std::find(_coordVars.begin(), _coordVars.end(), inVar);
            if (it != _coordVars.end()) {
                int index = it - _coordVars.begin();
                _coordVarsEnabled[index] = true;
            }

            it = std::find(_2DVars.begin(), _2DVars.end(), inVar);
            if (it != _2DVars.end()) {
                int index = it - _2DVars.begin();
                _2DVarsEnabled[index] = true;
            }

            it = std::find(_3DVars.begin(), _3DVars.end(), inVar);
            if (it != _3DVars.end()) {
                int index = it - _3DVars.begin();
                _3DVarsEnabled[index] = true;
            }
        }

        _inputGrids.clear();
        _otherGrids.clear();

        Update(true);
    }
}

void PythonVariables::_deleteScript()
{
    int rc = _openAndDeleteDialog->Update(OpenAndDeleteDialog::_DELETE, _controlExec);

    if (rc < 0) return;

    _openAndDeleteDialog->exec();

    rc = _openAndDeleteDialog->result();
    if (rc < 1) return;

    string scriptName = _openAndDeleteDialog->GetScriptName();
    string dataMgrName = _openAndDeleteDialog->GetDataMgrName();

    _controlExec->RemoveFunction(_scriptType, dataMgrName, scriptName);

    if (scriptName == _scriptName) _reset();

    Update(true);
}

bool PythonVariables::_getFilePath(QString &filePath, bool operation)
{
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    QFileDialog::FileMode   fileMode = QFileDialog::ExistingFile;
    QString                 title = "Import Python script from file";

    if (operation == WRITE) {
        acceptMode = QFileDialog::AcceptSave;
        fileMode = QFileDialog::AnyFile;
        title = "Export your Python script to a file";
    }

    string      pythonPath = Wasp::GetSharePath("python");
    QFileDialog fileDialog(this, "Import Python script from file", QString::fromStdString(pythonPath), QString("Python file (*.py)"));

    fileDialog.setAcceptMode(acceptMode);
    fileDialog.setDefaultSuffix(QString("py"));
    fileDialog.setFileMode(fileMode);
    if (fileDialog.exec() != QDialog::Accepted) return false;

    QStringList files = fileDialog.selectedFiles();
    if (files.size() != 1) return false;

    filePath = files[0];

    bool operable;
    if (operation == READ)
        operable = FileOperationChecker::FileGoodToRead(filePath);
    else
        operable = FileOperationChecker::FileGoodToWrite(filePath);
    if (!operable) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        return false;
    }

    return true;
}

void PythonVariables::_importScript()
{
    QString filePath;
    if (!_getFilePath(filePath, READ)) return;

    _script.clear();
    std::ifstream file;
    file.open(filePath.toStdString());
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            _script += line;
            _script += "\n";
        }
    }

    Update(true);
}

void PythonVariables::_exportScript()
{
    QString filePath;
    if (!_getFilePath(filePath, WRITE)) return;

    qDebug() << filePath;

    std::ofstream file;
    file.open(filePath.toStdString());
    file << _script;
    file.close();
}

void PythonVariables::_testScript()
{
    string script = _scriptEdit->toPlainText().toStdString();

    std::vector<string> inputVars = _buildInputVars();

    if (inputVars.empty() || _outputVars.empty()) {
        MSG_ERR("At least one Input Variable and one "
                "Output Variable must be defined");
        return;
    }

    int rc = _controlExec->AddFunction(_scriptType, _dataMgrName, _scriptName, script, inputVars, _outputVars, _outputGrids, _includeCoordVars);

    if (rc < 0) {
        MSG_ERR("Failed to add script");
        return;
    }

    DataMgr *dataMgr = _controlExec->GetDataStatus()->GetDataMgr(_dataMgrName);

    string varname = _outputVars[0];
    Grid * g = dataMgr->GetVariable(0, varname, 0, 0);
    if (!g) {
        MSG_ERR("Failed to calculate variable " + varname);
        return;
    }

    //
    // Get any output from script
    //
    string      s = _controlExec->GetFunctionStdout(_scriptType, _dataMgrName, _scriptName);
    QMessageBox msgBox;
    if (!s.empty()) {
        msgBox.setText("Script output:");
        msgBox.setInformativeText(s.c_str());
        msgBox.exec();
    }

    msgBox.setText("Test passed.");
    msgBox.exec();
}

void PythonVariables::_saveScript()
{
    string script = _scriptEdit->toPlainText().toStdString();

    std::vector<string> inputVars = _buildInputVars();

    int rc = _controlExec->AddFunction(_scriptType, _dataMgrName, _scriptName, script, inputVars, _outputVars, _outputGrids, _includeCoordVars);

    if (rc < 0) {
        MSG_ERR("Invalid syntax");
        return;
    }

    QMessageBox msgBox;
    msgBox.setText("Script saved to session.");
    msgBox.exec();
}

std::vector<string> PythonVariables::_buildInputVars() const
{
    std::vector<string> inputVars;
    for (int i = 0; i < _2DVars.size(); i++) {
        if (_2DVarsEnabled[i] == true) inputVars.push_back(_2DVars[i]);
    }
    for (int i = 0; i < _3DVars.size(); i++) {
        if (_3DVarsEnabled[i] == true) inputVars.push_back(_3DVars[i]);
    }

    return inputVars;
}

void PythonVariables::_closeScript()
{
    close();
}

void PythonVariables::_coordInputVarChanged(int row, int col)
{
    if (col == 0) return;

    string value = _coordInputVarTable->GetValue(row, col);
    if (value == "1")
        _coordVarsEnabled[row] = true;
    else
        _coordVarsEnabled[row] = false;

    Update(true);
}

void PythonVariables::_2DInputVarChanged(int row, int col)
{
    if (col == 0) return;

    string value = _2DInputVarTable->GetValue(row, col);
    if (value == "1")
        _2DVarsEnabled[row] = true;
    else
        _2DVarsEnabled[row] = false;

    Update(true);
}

void PythonVariables::_3DInputVarChanged(int row, int col)
{
    if (col == 0) return;

    string value = _3DInputVarTable->GetValue(row, col);
    if (value == "1")
        _3DVarsEnabled[row] = true;
    else
        _3DVarsEnabled[row] = false;

    Update(true);
}

void PythonVariables::_coordinatesCheckboxClicked(int state)
{
    if (state == Qt::Unchecked) {
        _includeCoordVars = false;
        _variableTabs->removeTab(0);
        _coordVarsEnabled.resize(_coordVars.size());
        std::fill(_coordVarsEnabled.begin(), _coordVarsEnabled.end(), false);
    } else {
        _includeCoordVars = true;
        _variableTabs->insertTab(0, _coordTab, "Coordinates");
    }
    Update(true);
}

void PythonVariables::_findEnabledCoordinateVariables(const std::vector<string> vars, const std::vector<bool> varsEnabled)
{
    VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
    VAPoR::DataMgr *   dataMgr = dataStatus->GetDataMgr(_dataMgrName);
    if (dataMgr == NULL) { MSG_ERR("Invalid DataMgr " + _dataMgrName); }

    std::vector<string> enabledCoordVarNames;
    std::vector<string> tmpCoordVarNames;
    VAPoR::DC::DataVar  dataVar;
    VAPoR::DC::Mesh     mesh;

    // Gather the currently used coord variables
    for (int i = 0; i < vars.size(); i++) {
        if (varsEnabled[i] == false) continue;

        string varName = vars[i];
        dataMgr->GetDataVarInfo(varName, dataVar);
        string meshName = dataVar.GetMeshName();
        dataMgr->GetMesh(meshName, mesh);
        tmpCoordVarNames = mesh.GetCoordVars();

        std::move(tmpCoordVarNames.begin(), tmpCoordVarNames.end(), std::back_inserter(enabledCoordVarNames));
    }

    // Set current coord variables to be enabled
    for (int i = 0; i < _coordVars.size(); i++) {
        for (int j = 0; j < enabledCoordVarNames.size(); j++) {
            if (_coordVars[i] == enabledCoordVarNames[j]) { _coordVarsEnabled[i] = true; }
        }
    }
}

void PythonVariables::_createNewVariable()
{
    VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
    VAPoR::DataMgr *   dataMgr = dataStatus->GetDataMgr(_dataMgrName);
    if (dataMgr == NULL) MSG_ERR("Invalid DataMgr " + _dataMgrName);

    std::vector<string> grids = dataMgr->GetMeshNames();

    std::vector<string> options = _makeDialogOptions(grids);

    std::vector<int> cateogryIndices;
    cateogryIndices.push_back(0);
    cateogryIndices.push_back(_inputGrids.size() + 1);

    _newItemDialog->Update(::NewItemDialog::OUTVAR, options, cateogryIndices);
    _newItemDialog->exec();

    int rc = _newItemDialog->result();
    if (rc < 1) return;

    string outputVar = _newItemDialog->GetItemName();

    if (outputVar == "") return;

    rc = _checkForDuplicateNames(_outputVars, outputVar);
    if (rc < 1) return;

    _outputVars.push_back(outputVar);
    string outputGrid = _newItemDialog->GetOptionName();
    _outputGrids.push_back(outputGrid);

    Update(true);
}

std::vector<string> PythonVariables::_makeDialogOptions(std::vector<string> grids)
{
    std::vector<string> options;
    string              inputVarGrid;
    bool                gridSelected;

    _inputGrids.clear();
    _otherGrids.clear();

    for (int i = 0; i < grids.size(); i++) {
        string grid = grids[i];
        gridSelected = _isGridSelected(grid, _2DVars, _2DVarsEnabled);
        if (gridSelected) {
            _inputGrids.push_back(grid);
            continue;
        }

        gridSelected = _isGridSelected(grid, _3DVars, _3DVarsEnabled);
        if (gridSelected) {
            _inputGrids.push_back(grid);
            continue;
        }

        _otherGrids.push_back(grids[i]);
    }

    options.push_back("Input variable grids:");
    for (int i = 0; i < _inputGrids.size(); i++) { options.push_back(_inputGrids[i]); }
    options.push_back("Other grids:");
    for (int i = 0; i < _otherGrids.size(); i++) { options.push_back(_otherGrids[i]); }

    return options;
}

bool PythonVariables::_isGridSelected(string grid, std::vector<string> variables, std::vector<bool> varEnabled) const
{
    VAPoR::DC::DataVar dataVar;
    string             inputVarGrid;
    bool               isInputGrid = false;

    for (int j = 0; j < variables.size(); j++) {
        if (varEnabled[j] == false) { continue; }

        VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
        VAPoR::DataMgr *   dataMgr = dataStatus->GetDataMgr(_dataMgrName);
        int                rc = dataMgr->GetDataVarInfo(variables[j], dataVar);
        if (!rc) { continue; }

        inputVarGrid = dataVar.GetMeshName();
        if (grid == inputVarGrid) {
            isInputGrid = true;
            break;
        }
    }
    return isInputGrid;
}

void PythonVariables::_deleteVariable()
{
    int activeRow = _outputVarTable->GetActiveRow();
    if (activeRow < 0) return;

    string varName = _outputVarTable->GetValue(activeRow, 0);
    auto   it = std::find(_outputVars.begin(), _outputVars.end(), varName);
    int    index = std::distance(_outputVars.begin(), it);

    _outputVars.erase(_outputVars.begin() + index);
    _outputGrids.erase(_outputGrids.begin() + index);
    Update(true);
}

void PythonVariables::_scriptChanged() { _script = _scriptEdit->toPlainText().toStdString(); }

void PythonVariables::_makeInputTableValues(std::vector<string> &tableValuesCoords, std::vector<string> &tableValues2D, std::vector<string> &tableValues3D, std::vector<string> &summaryValues) const
{
    string onOff;
    for (int i = 0; i < _2DVars.size(); i++) {
        tableValues2D.push_back(_2DVars[i]);

        onOff = "0";
        if (_2DVarsEnabled[i]) {
            onOff = "1";
            summaryValues.push_back(_2DVars[i]);
            summaryValues.push_back("2D");
        }
        tableValues2D.push_back(onOff);
    }

    for (int i = 0; i < _3DVars.size(); i++) {
        tableValues3D.push_back(_3DVars[i]);

        onOff = "0";
        if (_3DVarsEnabled[i]) {
            onOff = "1";
            summaryValues.push_back(_3DVars[i]);
            summaryValues.push_back("3D");
        }
        tableValues3D.push_back(onOff);
    }

    for (int i = 0; i < _coordVars.size(); i++) {
        tableValuesCoords.push_back(_coordVars[i]);

        onOff = "0";
        if (_coordVarsEnabled[i]) {
            onOff = "1";
            summaryValues.push_back(_coordVars[i]);
            summaryValues.push_back("Coordinate");
        }
        tableValuesCoords.push_back(onOff);
    }
}

void PythonVariables::_makeOutputTableValues(std::vector<string> &outputValues) const
{
    for (int i = 0; i < _outputVars.size(); i++) {
        outputValues.push_back(_outputVars[i]);
        outputValues.push_back(_outputGrids[i]);
    }
}

int PythonVariables::_checkForDuplicateNames(std::vector<string> names, string name)
{
    std::vector<string>::iterator it;
    it = std::find(names.begin(), names.end(), name);
    if (it == names.end())
        return 1;
    else {
        MSG_ERR("Names must be unique");
        return 0;
    }
}

void PythonVariables::InitControlExec(VAPoR::ControlExec *ce)
{
    VAssert(ce);
    _controlExec = ce;
}

void PythonVariables::ShowMe()
{
    Update(true);
    open();
}

NewItemDialog::NewItemDialog(QWidget *parent) : QDialog(parent)
{
    setModal(true);

    _itemNameLabel = new QLabel(tr("Script name:"));
    _itemNameEdit = new QLineEdit();
    _optionNameLabel = new QLabel(tr("Data Set:"));
    _optionNameCombo = new QComboBox();
    _okButton = new QPushButton(tr("Ok"));
    _cancelButton = new QPushButton(tr("Cancel"));

    _okButton->setAutoDefault(true);
    _cancelButton->setAutoDefault(false);

    _setupGUI();

    _connectWidgets();

    _itemNameEdit->setFocus();
}

void NewItemDialog::_connectWidgets()
{
    connect(_okButton, SIGNAL(clicked()), this, SLOT(_okClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void NewItemDialog::_setupGUI()
{
    QVBoxLayout *topLeftLayout = new QVBoxLayout;
    QVBoxLayout *topRightLayout = new QVBoxLayout;
    QHBoxLayout *topLayout = new QHBoxLayout;
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    QVBoxLayout *mainLayout = new QVBoxLayout;

    topLeftLayout->addWidget(_itemNameLabel);
    topLeftLayout->addWidget(_optionNameLabel);

    topRightLayout->addWidget(_itemNameEdit);
    topRightLayout->addWidget(_optionNameCombo);

    topLayout->addLayout(topLeftLayout);
    topLayout->addLayout(topRightLayout);

    bottomLayout->addWidget(_cancelButton);
    bottomLayout->addWidget(_okButton);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
}

void NewItemDialog::Update(int type, std::vector<string> optionNames, std::vector<int> categoryIndices)
{
    _adjustToType(type);

    _itemName = "";
    _optionName = "";

    _itemNameEdit->clear();
    _optionNameCombo->clear();
    for (int i = 0; i < optionNames.size(); i++) {
        QString qName = QString::fromStdString(optionNames[i]);
        _optionNameCombo->addItem(qName);
    }

    bool nextIndexIsInvalid = true;
    int  size = categoryIndices.size();
    for (int i = 0; i < size; i++) {
        _disableComboItem(categoryIndices[i]);

        if (nextIndexIsInvalid && categoryIndices[i] + 1 != categoryIndices[i + 1] && i != size) {
            nextIndexIsInvalid = false;
            _optionNameCombo->setCurrentIndex(categoryIndices[i] + 1);
        }
    }
}

void NewItemDialog::_disableComboItem(int index)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(_optionNameCombo->model());
    bool                disabled = true;
    QStandardItem *     item = model->item(index);
    item->setFlags(disabled ? item->flags() & ~Qt::ItemIsEnabled : item->flags() | Qt::ItemIsEnabled);
}

void NewItemDialog::_adjustToType(int type)
{
    if (type == SCRIPT) {
        setWindowTitle("Create new script");
        _itemNameLabel->setText("Script name:");
        _optionNameLabel->setText("Data Set:");
    } else if (type == OUTVAR) {
        setWindowTitle("Create new variable");
        _itemNameLabel->setText("Variable name:");
        _optionNameLabel->setText("Output Grid:");
    }
}

void ::NewItemDialog::_okClicked()
{
    _itemName = _itemNameEdit->text().toStdString();
    _optionName = _optionNameCombo->currentText().toStdString();

    _itemNameEdit->clear();

    accept();
}

string NewItemDialog::GetItemName() const { return _itemName; }

string NewItemDialog::GetOptionName() const { return _optionName; }

OpenAndDeleteDialog::OpenAndDeleteDialog(QWidget *parent)
{
    setModal(true);

    _dataMgrNameLabel = new QLabel(tr("Data Set name:"));
    _dataMgrNameCombo = new QComboBox();
    _scriptNameLabel = new QLabel(tr("Script:"));
    _scriptNameCombo = new QComboBox();
    _okButton = new QPushButton(tr("Ok"));
    _cancelButton = new QPushButton(tr("Cancel"));

    _okButton->setAutoDefault(true);
    _cancelButton->setAutoDefault(false);

    _setupGUI();

    _dataMgrNameCombo->setFocus();

    connect(_okButton, SIGNAL(clicked()), this, SLOT(_okClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_dataMgrNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_updateOptions(int)));
}

void OpenAndDeleteDialog::_updateOptions(int index)
{
    string              dataSetName = _dataMgrNameCombo->itemText(index).toStdString();
    std::vector<string> functionNames;
    functionNames = _controlExec->GetFunctionNames(_scriptType, dataSetName);

    _scriptNameCombo->clear();
    for (int i = 0; i < functionNames.size(); i++) {
        QString qName = QString::fromStdString(functionNames[i]);
        _scriptNameCombo->addItem(qName);
    }
}

void OpenAndDeleteDialog::_setupGUI()
{
    QVBoxLayout *topLeftLayout = new QVBoxLayout;
    QVBoxLayout *topRightLayout = new QVBoxLayout;
    QHBoxLayout *topLayout = new QHBoxLayout;
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    QVBoxLayout *mainLayout = new QVBoxLayout;

    topLeftLayout->addWidget(_dataMgrNameLabel);
    topLeftLayout->addWidget(_scriptNameLabel);

    topRightLayout->addWidget(_dataMgrNameCombo);
    topRightLayout->addWidget(_scriptNameCombo);

    topLayout->addLayout(topLeftLayout);
    topLayout->addLayout(topRightLayout);

    bottomLayout->addWidget(_cancelButton);
    bottomLayout->addWidget(_okButton);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
}

int OpenAndDeleteDialog::Update(int type, VAPoR::ControlExec *controlExec)
{
    _controlExec = controlExec;

    string errMsg;
    if (type == _OPEN) {
        setWindowTitle("Open saved script");
        errMsg = "There are no scripts to open in this session";
    } else if (type == _DELETE) {
        setWindowTitle("Delete saved script");
        errMsg = "There are no scripts to delete in this session";
    }

    _dataMgrNameCombo->clear();
    _scriptNameCombo->clear();

    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    std::vector<string> scriptNames;
    for (int i = 0; i < dataMgrNames.size(); i++) {
        QString qName = QString::fromStdString(dataMgrNames[i]);
        _dataMgrNameCombo->addItem(qName);

        if (scriptNames.empty()) {
            scriptNames = controlExec->GetFunctionNames(_scriptType, dataMgrNames[i]);
            _dataMgrNameCombo->setCurrentIndex(i);
            _dataMgrName = dataMgrNames[i];
        }
    }

    if (scriptNames.empty()) {
        MSG_ERR(errMsg);
        return -1;
    }

    _scriptName = _scriptNameCombo->currentText().toStdString();

    return 0;
}

void OpenAndDeleteDialog::_okClicked()
{
    _dataMgrName = _dataMgrNameCombo->currentText().toStdString();
    _scriptName = _scriptNameCombo->currentText().toStdString();

    accept();
}

string OpenAndDeleteDialog::GetDataMgrName() const { return _dataMgrName; }

string OpenAndDeleteDialog::GetScriptName() const { return _scriptName; }
