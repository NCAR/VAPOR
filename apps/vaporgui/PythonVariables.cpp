#include "PythonVariables.h"
#include "ui_PythonVariablesGUI.h"

#include <ctime>
#include <QLineEdit>
#include <QThread>

#include "ErrorReporter.h"

PythonVariables::PythonVariables(QWidget *parent) : QDialog(parent), Ui_PythonVariablesGUI()
{
    setupUi(this);

    _scriptName = "";
    _dataMgrName = "";

    _newScriptDialog = new NewItemDialog(this);

    _menuBar = new QMenuBar();
    _fileMenu = new QMenu("File");
    _menuBar->addMenu(_fileMenu);
    _fileMenu->addAction("New");
    _fileMenu->addAction("Save");
    _fileMenu->addAction("Open");
    layout()->setMenuBar(_menuBar);
    // PythonVariablesGUI->layout()->setMenuBar(_menuBar);

    // QColor background = palette().color(QWidget::backgroundRole());
    // QPalette* palette;
    // const QPalette* pal = &palette();
    // palette = &pal;
    //_fader = new Fader(_scriptSaveLabel, background);

    string  pythonImagePath = "/Users/pearse/vapor30/share/images/PythonLogo.png";
    QPixmap thumbnail(pythonImagePath.c_str());
    _pythonLabel->setPixmap(thumbnail);

    _saveCombo->setEditable(true);
    _saveCombo->lineEdit()->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < _saveCombo->count(); ++i) { _saveCombo->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole); }
    _saveCombo->setEditable(false);

    // QColor background = palette().color(QWidget::backgroundRole());
    // QPalette labelPalette = _outputVariablesLabel->palette();
    // labelPalette.setColor(_scriptSaveLabel->foregroundRole(), background);
    //_scriptSaveLabel->setPalette(labelPalette);

    _inputVariableTable = new VaporTable(_inputTable, false, true);

    _connectWidgets();
}

void PythonVariables::Update()
{
    _paramsMgr = _controlExec->GetParamsMgr();

    // PythonVariablesParams* pParams = dynamic_cast<PythonVariablesParams>(
    //    _paramsMgr->GetAppRenderParams(

    //_dataMgr = dataMgr;

    //_updateNewItemDialog();

    if ((_scriptName == "") || (_dataMgrName == "")) {
        _setGUIEnabled(false);
        return;
    } else {
        _setGUIEnabled(true);
    }

    _updateGridComboBox();

    std::vector<string> foo;
    foo.push_back("foo");
    _inputVariableTable->Update(1, 2, foo);
}

void PythonVariables::_connectWidgets()
{
    connect(_newButton, SIGNAL(clicked()), this, SLOT(_newScript()));
    connect(_saveCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_saveScript(int)));
}

void PythonVariables::_setGUIEnabled(bool enabled)
{
    _inputOutputTitleFrame->setEnabled(enabled);
    _variableTableFrame->setEnabled(enabled);
    _scriptEdit->setEnabled(enabled);
    _testFrame->setEnabled(enabled);
}

void PythonVariables::_updateNewItemDialog()
{
    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    _newScriptDialog->Update(NewItemDialog::SCRIPT, dataMgrNames);
}

void PythonVariables::_updateGridComboBox()
{
    _gridComboBox->clear();
    VAPoR::DataStatus *dataStatus = _controlExec->GetDataStatus();
    VAPoR::DataMgr *   dataMgr = dataStatus->GetDataMgr(_dataMgrName);
    if (dataMgr == NULL) MSG_ERR("Invalid DataMgr " + _dataMgrName);

    std::vector<string> grids = dataMgr->GetMeshNames();

    QString qGridName;
    for (int i = 0; i < grids.size(); i++) {
        cout << grids[i] << endl;
        qGridName = QString::fromStdString(grids[i]);
        _gridComboBox->addItem(qGridName);
    }
}

void PythonVariables::_newScript()
{
    VAPoR::DataStatus * dataStatus = _controlExec->GetDataStatus();
    std::vector<string> dataMgrNames = dataStatus->GetDataMgrNames();
    _newScriptDialog->Update(NewItemDialog::SCRIPT, dataMgrNames);

    _newScriptDialog->exec();
    int result = _newScriptDialog->result();

    if (result > 0) {
        _scriptName = _newScriptDialog->GetItemName();
        if (_scriptName == "") return;

        _dataMgrName = _newScriptDialog->GetOptionName();

        _scriptNameLabel->setText(QString::fromStdString(_scriptName));
        _dataMgrNameLabel->setText(QString::fromStdString(_dataMgrName));
        Update();
    }
}

void PythonVariables::_saveScript(int index)
{
    // if (_fader) delete _fader;

    QColor background = palette().color(QWidget::backgroundRole());
    _fader = new Fader(_scriptSaveLabel, background);

    //_newScriptDialog->show();
}

PythonVariables::~PythonVariables()
{
    if (_menuBar) delete _menuBar;
    if (_fileMenu) delete _fileMenu;
    if (_fader) delete _fader;
    if (_newScriptDialog) delete _newScriptDialog;
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

Fader::Fader(QLabel *label, QColor background, QObject *parent) : QObject(0)
{
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
    QColor   textColor = labelPalette.color(QPalette::Text);
    int      startRed = textColor.red();
    int      endRed = _background.red();
    int      redIncrement = (endRed - startRed) / cycles;
    int      startGreen = textColor.green();
    int      endGreen = _background.green();
    int      greenIncrement = (endGreen - startGreen) / cycles;
    int      startBlue = textColor.blue();
    int      endBlue = _background.blue();
    int      blueIncrement = (endBlue - startBlue) / cycles;

    clock_t startTime = clock();
    double  secondsPassed = 0.f;
    double  secondsToDelay = .15;
    bool    flag = true;
    int     counter = 0;
    // bool onOff = true;

    while (flag) {
        secondsPassed = (clock() - startTime) / (float)CLOCKS_PER_SEC;
        cout << secondsPassed << endl;
        if (secondsPassed >= secondsToDelay) {
            cout << "    " << secondsPassed << endl;

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
    _itemName = "";
    _optionName = "";

    _itemNameEdit->clear();
    _optionNameCombo->clear();

    for (int i = 0; i < optionNames.size(); i++) {
        QString qName = QString::fromStdString(optionNames[i]);
        _optionNameCombo->addItem(qName);
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
