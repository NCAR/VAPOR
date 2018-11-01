#include "PythonVariables.h"
#include "ui_PythonVariablesGUI.h"

#include <ctime>
#include <QLineEdit>
#include <QThread>
#include <QFileDialog>
#include <QStandardItemModel>

#include <vapor/DC.h>
#include <vapor/GetAppPath.h>
#include "ErrorReporter.h"
#include "FileOperationChecker.h"

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

    setWindowTitle("Derived variables with Python");

    _script = "";
    _scriptName = "";
    _dataMgrName = "";

    _saveFader = nullptr;
    _testFader = nullptr;

    _newItemDialog = new ::NewItemDialog(this);
    _openAndDeleteDialog = new ::OpenAndDeleteDialog(this);

    _justSaved = false;

    QColor   background = palette().color(QWidget::backgroundRole());
    QPalette labelPalette = _scriptSaveLabel->palette();
    labelPalette.setColor(_scriptSaveLabel->foregroundRole(), background);
    _scriptSaveLabel->setPalette(labelPalette);
    labelPalette = _scriptTestLabel->palette();
    labelPalette.setColor(_scriptTestLabel->foregroundRole(), background);
    _scriptTestLabel->setPalette(labelPalette);

    std::vector<string> imagePathVec = {"images"};
    string              imagePath = GetAppPath("VAPOR", "share", imagePathVec);
    string              pythonImagePath = imagePath + "/PythonLogo.png";
    QPixmap             thumbnail(pythonImagePath.c_str());
    _pythonLabel->setPixmap(thumbnail);

    _2DInputVarTable = new VaporTable(_2DVarTable, false, true);
    _2DInputVarTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(0));

    _3DInputVarTable = new VaporTable(_3DVarTable, false, true);
    _3DInputVarTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(0));

    _summaryTable = new VaporTable(_varSummaryTable);
    _summaryTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(0));

    _outputVarTable = new VaporTable(_outputVariablesTable);
    _outputVarTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(VaporTable::ROWS));

    _connectWidgets();
}

PythonVariables::~PythonVariables()
{
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
        _setGUIEnabled(false);
    } else {
        _setGUIEnabled(true);
    }

    _scriptNameLabel->setText(QString::fromStdString(_scriptName));
    _dataMgrNameLabel->setText(QString::fromStdString(_dataMgrName));
    _scriptEdit->setText(QString::fromStdString(_script));

    int                 numRows;
    int                 numCols = 2;
    std::vector<string> tableValues2D, tableValues3D, summaryValues;
    _makeInputTableValues(tableValues2D, tableValues3D, summaryValues);

    _2DInputVarTable->blockSignals(true);
    _3DInputVarTable->blockSignals(true);
    _summaryTable->blockSignals(true);
    _outputVarTable->blockSignals(true);

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

    _2DInputVarTable->blockSignals(false);
    _3DInputVarTable->blockSignals(false);
    _summaryTable->blockSignals(false);
    _outputVarTable->blockSignals(false);

    if (_justSaved && internalUpdate) {
        cout << "fading out " << endl;
        bool fadeIn = false;
        _fadeTest(fadeIn);
        _fadeSaveSession(fadeIn);
        _justSaved = false;
    }
}

void PythonVariables::_fadeTest(bool fadeIn)
{
    if (_testFader != nullptr) return;

    QColor background = palette().color(QWidget::backgroundRole());

    QColor textColor;
    if (fadeIn)
        textColor = QColor(0, 0, 255);
    else
        textColor = _scriptTestLabel->palette().color(QPalette::WindowText);

    _testFader = new Fader(fadeIn, background, textColor);

    connect(_testFader, SIGNAL(cycle(int, int, int)), this, SLOT(_updateTestLabelColor(int, int, int)));
    connect(_testFader, SIGNAL(faderDone()), this, SLOT(_deleteTestFader()));
    _testFader->start(QThread::IdlePriority);
}

void PythonVariables::_fadeSaveSession(bool fadeIn)
{
    if (_saveFader != nullptr) return;

    QColor background = palette().color(QWidget::backgroundRole());

    QColor textColor;
    if (fadeIn)
        textColor = QColor(0, 0, 255);
    else
        textColor = _scriptSaveLabel->palette().color(QPalette::WindowText);
    _saveFader = new Fader(fadeIn, background, textColor);

    connect(_saveFader, SIGNAL(cycle(int, int, int)), this, SLOT(_updateSaveLabelColor(int, int, int)));
    connect(_saveFader, SIGNAL(faderDone()), this, SLOT(_deleteSaveFader()));
    _saveFader->start(QThread::IdlePriority);
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
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(_cancelScript()));

    connect(_2DInputVarTable, SIGNAL(valueChanged(int, int)), this, SLOT(_2DInputVarChanged(int, int)));
    connect(_3DInputVarTable, SIGNAL(valueChanged(int, int)), this, SLOT(_3DInputVarChanged(int, int)));

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
        _scriptName = _newItemDialog->GetItemName();
        if (_scriptName == "") return;

        _dataMgrName = _newItemDialog->GetOptionName();

        _scriptNameLabel->setText(QString::fromStdString(_scriptName));
        _dataMgrNameLabel->setText(QString::fromStdString(_dataMgrName));

        _scriptEdit->clear();
        VAPoR::DataMgr *dataMgr = dataStatus->GetDataMgr(_dataMgrName);
        _2DVars = dataMgr->GetDataVarNames(TWOD);
        _2DVarsEnabled.resize(_2DVars.size());
        std::fill(_2DVarsEnabled.begin(), _2DVarsEnabled.end(), false);
        _3DVars = dataMgr->GetDataVarNames(THREED);
        _3DVarsEnabled.resize(_3DVars.size());
        std::fill(_3DVarsEnabled.begin(), _3DVarsEnabled.end(), false);

        _outputVars.clear();
        _outputGrids.clear();
        _inputGrids.clear();
        _otherGrids.clear();

        Update(true);
    }
}

void PythonVariables::_openScript()
{
    int rc = _openAndDeleteDialog->Update(OpenAndDeleteDialog::OPEN, _controlExec);

    if (rc < 0) return;

    _openAndDeleteDialog->exec();

    rc = _openAndDeleteDialog->result();
    if (rc > 0) {
        string scriptName = _openAndDeleteDialog->GetScriptName();
        string dataMgrName = _openAndDeleteDialog->GetDataMgrName();

        std::vector<string> inputVars;
        bool                rc2 = _controlExec->GetFunction(_scriptType, dataMgrName, scriptName, _script, inputVars, _outputVars, _outputGrids);
        if (rc2 == false) {
            MSG_ERR("Invalid script: " + scriptName);
            return;
        }

        VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
        VAPoR::DataMgr *   dataMgr = dataStatus->GetDataMgr(_dataMgrName);

        _2DVars = dataMgr->GetDataVarNames(TWOD);
        _2DVarsEnabled.resize(_2DVars.size(), false);
        _3DVars = dataMgr->GetDataVarNames(THREED);
        _3DVarsEnabled.resize(_3DVars.size(), false);

        std::vector<string>::iterator it;

        for (int i = 0; i < inputVars.size(); i++) {
            string inVar = inputVars[i];
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

        cout << endl << "Opening script with the following parameters:" << endl;
        cout << "   _scriptType  " << _scriptType << endl;
        cout << "   _dataMgrName " << _dataMgrName << endl;
        cout << "   scriptName   " << _scriptName << endl;
        cout << "   inputVars    ";
        printVector(inputVars);
        cout << "   _outputVars  ";
        printVector(_outputVars);
        cout << "   _outputGrids ";
        printVector(_outputGrids);
        cout << "   RC " << rc << endl << endl;

        _inputGrids.clear();
        _otherGrids.clear();

        Update(true);
    }
}

void PythonVariables::_deleteScript()
{
    int rc = _openAndDeleteDialog->Update(OpenAndDeleteDialog::DELETE, _controlExec);

    if (rc < 0) return;

    _openAndDeleteDialog->exec();

    rc = _openAndDeleteDialog->result();
    if (rc < 1) return;

    string scriptName = _openAndDeleteDialog->GetScriptName();
    string dataMgrName = _openAndDeleteDialog->GetDataMgrName();

    _controlExec->RemoveFunction(_scriptType, dataMgrName, scriptName);

    if (_scriptName == _scriptName) {
        _script = "";
        _scriptName = "";
        _dataMgrName = "";

        _2DVars.clear();
        _2DVarsEnabled.clear();
        _3DVars.clear();
        _3DVarsEnabled.clear();
        _outputVars.clear();
        _outputGrids.clear();
        _inputGrids.clear();
        _otherGrids.clear();
    }

    Update(true);
}

void PythonVariables::_testScript()
{
    string script = _scriptEdit->toPlainText().toStdString();

    std::vector<string> inputVars;
    for (int i = 0; i < _2DVars.size(); i++) {
        if (_2DVarsEnabled[i] == true) inputVars.push_back(_2DVars[i]);
    }
    for (int i = 0; i < _3DVars.size(); i++) {
        if (_3DVarsEnabled[i] == true) inputVars.push_back(_3DVars[i]);
    }

    if (inputVars.empty() || _outputVars.empty()) return;

    int rc = _controlExec->AddFunction(_scriptType, _dataMgrName, _scriptName, script, inputVars, _outputVars, _outputGrids);

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

    // TODO: Post success message popup
    //
    /*QMessageBox message(
        QMessageBox::Information,
        "Success",
        "Test passed.\n\nClick Apply to save the script to your session.",
        QMessageBox::Ok,
        this
    );
    message.exec();*/

    bool fadeIn = true;
    _fadeTest(fadeIn);

    _justSaved = true;
}

void PythonVariables::_saveScript()
{
    string script = _scriptEdit->toPlainText().toStdString();

    std::vector<string> inputVars;
    for (int i = 0; i < _2DVars.size(); i++) {
        if (_2DVarsEnabled[i] == true) inputVars.push_back(_2DVars[i]);
    }
    for (int i = 0; i < _3DVars.size(); i++) {
        if (_3DVarsEnabled[i] == true) inputVars.push_back(_3DVars[i]);
    }

    int rc = _controlExec->AddFunction(_scriptType, _dataMgrName, _scriptName, script, inputVars, _outputVars, _outputGrids);

    if (rc < 0) {
        MSG_ERR("Invalid syntax");
        return;
    }

    cout << endl << "Saving script with the following parameters:" << endl;
    cout << "   _scriptType  " << _scriptType << endl;
    cout << "   _dataMgrName " << _dataMgrName << endl;
    cout << "   scriptName   " << _scriptName << endl;
    cout << "   inputVars    ";
    printVector(inputVars);
    cout << "   _outputVars  ";
    printVector(_outputVars);
    cout << "   _outputGrids ";
    printVector(_outputGrids);
    cout << "   RC " << rc << endl << endl;

    bool fadeIn = true;
    _fadeSaveSession(fadeIn);
    _justSaved = true;
}

void PythonVariables::_cancelScript()
{
    _scriptEdit->clear();
    _dataMgrName = "";
    _scriptName = "";
    _outputVars.clear();
    _outputGrids.clear();
    _2DVars.clear();
    _2DVarsEnabled.clear();
    _3DVars.clear();
    _3DVarsEnabled.clear();

    Update(true);
    close();
}

void PythonVariables::_saveToSession()
{
    /*  bool fadeIn = true;
    QColor background = palette().color(QWidget::backgroundRole());
    _fader = new Fader(fadeIn, _scriptSaveLabel, background);
    connect(_fader, SIGNAL(faderDone()),
        this, SLOT(_deleteFader()));
*/
    bool fadeIn = true;
    _fadeSaveSession(fadeIn);
    _justSaved = true;
}

void PythonVariables::_saveToFile()
{
    QString     defaultDir = QDir::homePath() + "/" + QString::fromStdString(_scriptName) + ".py";
    QFileDialog fileDialog(this, "Save Python Script", defaultDir);
    fileDialog.setDefaultSuffix(QString::fromAscii("py"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (fileDialog.exec() != QDialog::Accepted) return;

    QStringList qsl = fileDialog.selectedFiles();

    for (int i = 0; i < qsl.size(); ++i) cout << qsl.at(i).toLocal8Bit().constData() << endl;

    string script = _scriptEdit->toPlainText().toStdString();
}

void PythonVariables::_updateSaveLabelColor(int r, int g, int b)
{
    //_updateLabelColor(r, g, b, _scriptSaveLabel);
    QColor   newColor = QColor(r, g, b);
    QPalette labelPalette = _scriptSaveLabel->palette();
    labelPalette.setColor(_scriptSaveLabel->foregroundRole(), newColor);
    _scriptSaveLabel->setPalette(labelPalette);
}

void PythonVariables::_updateTestLabelColor(int r, int g, int b)
{
    //_updateLabelColor(r, g, b, _scriptTestLabel);
    QColor   newColor = QColor(r, g, b);
    QPalette labelPalette = _scriptTestLabel->palette();
    labelPalette.setColor(_scriptTestLabel->foregroundRole(), newColor);
    _scriptTestLabel->setPalette(labelPalette);
}

void PythonVariables::_updateLabelColor(int r, int g, int b, QLabel *label)
{
    cout << "Updating label color to " << r << " " << g << " " << b << endl;
    QColor newColor = QColor(r, g, b);
    // QPalette labelPalette = _scriptSaveLabel->palette();
    // labelPalette.setColor(_scriptSaveLabel->foregroundRole(), newColor);
    //_scriptSaveLabel->setPalette(labelPalette);
    QPalette labelPalette = label->palette();
    labelPalette.setColor(label->foregroundRole(), newColor);
    label->setPalette(labelPalette);
}

void PythonVariables::_deleteTestFader()
{
    if (_testFader != nullptr) {
        _testFader->wait();
        delete _testFader;
        _testFader = nullptr;
    }
}

void PythonVariables::_deleteSaveFader()
{
    if (_saveFader != nullptr) {
        _saveFader->wait();
        delete _saveFader;
        _saveFader = nullptr;
    }
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
    Update(true);
}

void PythonVariables::_scriptChanged() { _script = _scriptEdit->toPlainText().toStdString(); }

void PythonVariables::_makeInputTableValues(std::vector<string> &tableValues2D, std::vector<string> &tableValues3D, std::vector<string> &summaryValues) const
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
    assert(ce);
    _controlExec = ce;
}

void PythonVariables::ShowMe()
{
    Update(true);
    show();
    raise();
    activateWindow();
}

Fader::Fader(bool fadeIn, QColor background, QColor textColor, QObject *parent) : QThread(parent)
{
    _fadeIn = fadeIn;
    _textColor = textColor;
    _background = background;
}

void Fader::run() { _fade(); }

void Fader::_fade()
{
    int cycles = 20;

    //    QColor textColor(0, 0, 255);

    QColor startColor = _background;
    QColor endColor = _textColor;
    if (!_fadeIn) {
        startColor = _textColor;
        endColor = _background;
    }

    int startRed = startColor.red();
    int endRed = endColor.red();
    int redIncrement = (endRed - startRed) / cycles;

    int startGreen = startColor.green();
    int endGreen = endColor.green();
    int greenIncrement = (endGreen - startGreen) / cycles;

    int startBlue = startColor.blue();
    int endBlue = endColor.blue();
    int blueIncrement = (endBlue - startBlue) / cycles;

    clock_t startTime = clock();
    double  secondsPassed = 0.f;
    double  secondsToDelay = .05;
    bool    flag = true;
    int     counter = 0;

    while (flag) {
        secondsPassed = (clock() - startTime) / (float)CLOCKS_PER_SEC;
        if (secondsPassed >= secondsToDelay) {
            int  newRed = startRed + redIncrement * counter;
            int  newGreen = startGreen + greenIncrement * counter;
            int  newBlue = startBlue + blueIncrement * counter;
            emit cycle(newRed, newGreen, newBlue);
            startTime = clock();
            counter++;
        }
        if (counter > cycles) flag = false;
    }

    emit cycle(endRed, endGreen, endBlue);

    emit faderDone();
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

    connect(_okButton, SIGNAL(clicked()), this, SLOT(_okClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
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
    if (type == OPEN)
        setWindowTitle("Open saved script");
    else if (type == DELETE)
        setWindowTitle("Delete saved script");

    _dataMgrNameCombo->clear();
    _scriptNameCombo->clear();

    VAPoR::DataStatus * dataStatus = controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    std::vector<string> scriptNames;
    for (int i = 0; i < dataMgrNames.size(); i++) {
        QString qName = QString::fromStdString(dataMgrNames[i]);
        _dataMgrNameCombo->addItem(qName);

        cout << "# of scripts for this DataMgr ";
        cout << controlExec->GetFunctionNames(_scriptType, dataMgrNames[i]).size() << endl;
        if (scriptNames.empty()) {
            scriptNames = controlExec->GetFunctionNames(_scriptType, dataMgrNames[i]);
            _dataMgrNameCombo->setCurrentIndex(i);
            _dataMgrName = dataMgrNames[i];
        }
    }

    if (scriptNames.empty()) {
        MSG_ERR("There are no scripts to open.");
        return -1;
    }

    for (int i = 0; i < scriptNames.size(); i++) {
        QString qName = QString::fromStdString(scriptNames[i]);
        _scriptNameCombo->addItem(qName);
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
