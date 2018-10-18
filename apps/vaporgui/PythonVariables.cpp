#include "PythonVariables.h"
#include "ui_PythonVariablesGUI.h"

#include <ctime>
#include <QLineEdit>
#include <QThread>
#include <QFileDialog>

#include "ErrorReporter.h"
#include "FileOperationChecker.h"

#define TWOD   2
#define THREED 3

PythonVariables::PythonVariables(QWidget *parent) : QDialog(parent), Ui_PythonVariablesGUI()
{
    setupUi(this);

    _scriptName = "";
    _dataMgrName = "";

    _newItemDialog = new NewItemDialog(this);

    _justSaved = false;

    QColor   background = palette().color(QWidget::backgroundRole());
    QPalette labelPalette = _scriptSaveLabel->palette();
    labelPalette.setColor(_scriptSaveLabel->foregroundRole(), background);
    _scriptSaveLabel->setPalette(labelPalette);

    /*_menuBar = new QMenuBar();
    _fileMenu = new QMenu("File");
    _menuBar->addMenu(_fileMenu);
    _fileMenu->addAction("New");
    _fileMenu->addAction("Save");
    _fileMenu->addAction("Open");
    layout()->setMenuBar(_menuBar);
    PythonVariablesGUI->layout()->setMenuBar(_menuBar);*/

    cout << "Need to use GetAppPath, not hard coded path" << endl;
    string  pythonImagePath = "/Users/pearse/vapor30/share/images/PythonLogo.png";
    QPixmap thumbnail(pythonImagePath.c_str());
    _pythonLabel->setPixmap(thumbnail);

    _saveScriptCombo->setEditable(true);
    _saveScriptCombo->lineEdit()->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < _saveScriptCombo->count(); ++i) { _saveScriptCombo->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole); }
    _saveScriptCombo->setEditable(false);

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
    if (_menuBar) delete _menuBar;
    if (_fileMenu) delete _fileMenu;
    if (_fader) delete _fader;
    if (_newItemDialog) delete _newItemDialog;
}

void PythonVariables::Update()
{
    _paramsMgr = _controlExec->GetParamsMgr();

    // PythonVariablesParams* pParams = dynamic_cast<PythonVariablesParams>(
    //    _paramsMgr->GetAppRenderParams(

    if ((_scriptName == "") || (_dataMgrName == "")) {
        _setGUIEnabled(false);
        return;
    } else {
        _setGUIEnabled(true);
    }

    int                 numRows;
    int                 numCols = 2;
    std::vector<string> tableValues2D, tableValues3D, summaryValues;
    _makeInputTableValues(tableValues2D, tableValues3D, summaryValues);

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

    if (_justSaved) {
        bool fadeIn = false;
        _fade(fadeIn);
        _justSaved = false;
    }
}

void PythonVariables::_fade(bool fadeIn)
{
    QColor background = palette().color(QWidget::backgroundRole());
    _fader = new Fader(fadeIn, _scriptSaveLabel, background);
    connect(_fader, SIGNAL(faderDone()), this, SLOT(_deleteFader()));
}

void PythonVariables::_connectWidgets()
{
    connect(_newScriptButton, SIGNAL(clicked()), this, SLOT(_newScript()));
    connect(_openScriptButton, SIGNAL(clicked()), this, SLOT(_openScript()));
    connect(_deleteScriptButton, SIGNAL(clicked()), this, SLOT(_deleteScript()));
    connect(_saveScriptCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_saveScript(int)));
    connect(_testScriptButton, SIGNAL(clicked()), this, SLOT(_testScript()));

    connect(_2DInputVarTable, SIGNAL(valueChanged(int, int)), this, SLOT(_2DInputVarChanged(int, int)));
    connect(_3DInputVarTable, SIGNAL(valueChanged(int, int)), this, SLOT(_3DInputVarChanged(int, int)));

    connect(_newOutVarButton, SIGNAL(clicked()), this, SLOT(_createNewVariable()));
    connect(_deleteOutVarButton, SIGNAL(clicked()), this, SLOT(_deleteVariable()));
}

void PythonVariables::_setGUIEnabled(bool enabled)
{
    _variableSelectionFrame->setEnabled(enabled);
    _scriptEdit->setEnabled(enabled);
    _testFrame->setEnabled(enabled);
}

void PythonVariables::_updateNewItemDialog()
{
    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    _newItemDialog->Update(NewItemDialog::SCRIPT, dataMgrNames);
}

/*void PythonVariables::_updateGridComboBox() {
    VAPoR::DataStatus* dataStatus = _controlExec->GetDataStatus();
    VAPoR::DataMgr* dataMgr = dataStatus->GetDataMgr(_dataMgrName);
    if (dataMgr == NULL)
        MSG_ERR("Invalid DataMgr " + _dataMgrName);

    std::vector<string> grids = dataMgr->GetMeshNames();

    QString qGridName;
    for (int i=0; i<grids.size(); i++) {
        cout << grids[i] << endl;
        qGridName = QString::fromStdString(grids[i]);
        _gridComboBox->addItem(qGridName);
    }
}*/

void PythonVariables::_newScript()
{
    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    _newItemDialog->Update(NewItemDialog::SCRIPT, dataMgrNames);

    _newItemDialog->exec();
    int rc = _newItemDialog->result();

    if (rc > 0) {
        _scriptName = _newItemDialog->GetItemName();
        if (_scriptName == "") return;

        _dataMgrName = _newItemDialog->GetOptionName();

        _scriptNameLabel->setText(QString::fromStdString(_scriptName));
        _dataMgrNameLabel->setText(QString::fromStdString(_dataMgrName));

        VAPoR::DataMgr *dataMgr = dataStatus->GetDataMgr(_dataMgrName);
        _2DVars = dataMgr->GetDataVarNames(TWOD);
        _2DVarsEnabled.resize(_2DVars.size(), false);
        _3DVars = dataMgr->GetDataVarNames(THREED);
        _3DVarsEnabled.resize(_3DVars.size(), false);

        Update();
    }
}

void PythonVariables::_saveScript(int index)
{
    _saveScriptCombo->blockSignals(true);
    _saveScriptCombo->setCurrentIndex(0);
    _saveScriptCombo->blockSignals(false);

    if (index == 0) return;
    if (index == 1) { _saveToSession(); }
    if (index == 2) { _saveToFile(); }
}

void PythonVariables::_saveToSession()
{
    cout << "Save to session" << endl;

    /*  bool fadeIn = true;
    QColor background = palette().color(QWidget::backgroundRole());
    _fader = new Fader(fadeIn, _scriptSaveLabel, background);
    connect(_fader, SIGNAL(faderDone()),
        this, SLOT(_deleteFader()));
*/
    bool fadeIn = true;
    _fade(fadeIn);
    string script = _scriptEdit->toPlainText().toStdString();
    _justSaved = true;
    cout << script << endl;
}

void PythonVariables::_saveToFile()
{
    QString defaultDir = QDir::homePath() + "/" + QString::fromStdString(_scriptName) + ".py";
    cout << defaultDir.toStdString() << endl;
    QFileDialog fileDialog(this, "Save Python Script", defaultDir);
    fileDialog.setDefaultSuffix(QString::fromAscii("py"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (fileDialog.exec() != QDialog::Accepted) return;

    QStringList qsl = fileDialog.selectedFiles();

    for (int i = 0; i < qsl.size(); ++i) cout << qsl.at(i).toLocal8Bit().constData() << endl;

    string script = _scriptEdit->toPlainText().toStdString();
    cout << script << endl;
}

void PythonVariables::_deleteFader()
{
    if (_fader) delete _fader;
}

void PythonVariables::_2DInputVarChanged(int row, int col)
{
    string value = _2DInputVarTable->GetValue(row, col);
    if (value == "1")
        _2DVarsEnabled[row] = true;
    else
        _2DVarsEnabled[row] = false;

    Update();
}

void PythonVariables::_3DInputVarChanged(int row, int col)
{
    string value = _3DInputVarTable->GetValue(row, col);
    if (value == "1")
        _3DVarsEnabled[row] = true;
    else
        _3DVarsEnabled[row] = false;

    Update();
}

void PythonVariables::_createNewVariable()
{
    VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
    VAPoR::DataMgr *   dataMgr = dataStatus->GetDataMgr(_dataMgrName);
    if (dataMgr == NULL) MSG_ERR("Invalid DataMgr " + _dataMgrName);
    std::vector<string> grids = dataMgr->GetMeshNames();
    _newItemDialog->Update(NewItemDialog::OUTVAR, grids);
    _newItemDialog->exec();
    int rc = _newItemDialog->result();

    if (rc < 1) return;

    string outputVar = _newItemDialog->GetItemName();
    rc = _checkForDuplicateNames(_outputVars, outputVar);

    if (rc < 1) return;

    _outputVars.push_back(outputVar);
    string outputGrid = _newItemDialog->GetOptionName();
    _outputGrids.push_back(outputGrid);

    Update();
}

void PythonVariables::_deleteVariable()
{
    int activeRow = _outputVarTable->GetActiveRow();
    if (activeRow < 0) return;

    string varName = _outputVarTable->GetValue(activeRow, 0);
    auto   it = std::find(_outputVars.begin(), _outputVars.end(), varName);
    int    index = std::distance(_outputVars.begin(), it);

    _outputVars.erase(_outputVars.begin() + index);
    Update();
}

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
    Update();
    show();
    raise();
    activateWindow();
}

Fader::Fader(bool fadeIn, QLabel *label, QColor background, QObject *parent) : QObject(0)
{
    _fadeIn = fadeIn;
    _myLabel = label;
    _background = background;

    QThread *thread = new QThread(parent);
    connect(thread, SIGNAL(started()), this, SLOT(_fade()));
    this->moveToThread(thread);
    thread->start();
}

void Fader::_fade()
{
    int cycles = 10;

    QPalette labelPalette = _myLabel->palette();
    QColor   textColor(0, 0, 255);    //= labelPalette.color(QPalette::Text);

    QColor startColor = _background;
    QColor endColor = textColor;
    if (!_fadeIn) {
        startColor = textColor;
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
    double  secondsToDelay = .15;
    bool    flag = true;
    int     counter = 0;
    // bool onOff = true;

    while (flag) {
        secondsPassed = (clock() - startTime) / (float)CLOCKS_PER_SEC;
        if (secondsPassed >= secondsToDelay) {
            int    newRed = startRed + redIncrement * counter;
            int    newGreen = startGreen + greenIncrement * counter;
            int    newBlue = startBlue + blueIncrement * counter;
            QColor newColor = QColor(newRed, newGreen, newBlue);
            labelPalette.setColor(_myLabel->foregroundRole(), newColor);
            _myLabel->setPalette(labelPalette);
            // secondsPassed = 0.f;
            startTime = clock();
            counter++;
            // if (onOff==true) {
            /*if (i%2) {

                labelPalette.setColor(_myLabel->foregroundRole(), Qt::yellow);
                _myLabel->setPalette(labelPalette);
                onOff = false;
            }
            else {
                labelPalette.setColor(_myLabel->foregroundRole(), _background);
                _myLabel->setPalette(labelPalette);
                onOff = true;
            }*/
        }
        // if(secondsPassed >= cycles) flag = false;
        if (counter > cycles) flag = false;
    }

    if (!_fadeIn) {
        labelPalette.setColor(_myLabel->foregroundRole(), _background);
        _myLabel->setPalette(labelPalette);
    }

    emit faderDone();
    // labelPalette.setColor(_scriptSaveLabel->foregroundRole(), background);
    //_scriptSaveLabel->setPalette(labelPalette);
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

    _okButton->setAutoDefault(false);
    _cancelButton->setAutoDefault(false);

    _setupGUI();

    _connectWidgets();
}

void NewItemDialog::_connectWidgets()
{
    connect(_okButton, SIGNAL(clicked()), this, SLOT(_okClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_optionNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_dataMgrChanged(int)));
    connect(_itemNameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(_itemNameChanged(const QString &)));
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

void NewItemDialog::Update(int type, std::vector<string> optionNames)
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
}

void NewItemDialog::_adjustToType(int type)
{
    if (type == SCRIPT) {
        _itemNameLabel->setText("Script name:");
        _optionNameLabel->setText("Data Set:");
    } else if (type == OUTVAR) {
        _itemNameLabel->setText("Variable name:");
        _optionNameLabel->setText("Output Grid:");
    }
}

void NewItemDialog::_okClicked()
{
    _itemName = _itemNameEdit->text().toStdString();
    _optionName = _optionNameCombo->currentText().toStdString();

    _itemNameEdit->clear();

    accept();
}

string NewItemDialog::GetItemName() const { return _itemName; }

string NewItemDialog::GetOptionName() const { return _optionName; }
