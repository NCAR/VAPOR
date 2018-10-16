#ifndef PYTHOVARIABLES_H
#define PYTHOVARIABLES_H

#include <vapor/ControlExecutive.h>

#include "ui_PythonVariablesGUI.h"
#include "PythonVariablesParams.h"
#include "VaporTable.h"

#include <QDialog>
#include <QMenuBar>
#include <QMenu>

class Fader;
class NewItemDialog;

class PythonVariables : public QDialog, Ui_PythonVariablesGUI {
    Q_OBJECT

public:
    PythonVariables(QWidget *parent);
    ~PythonVariables();
    void Update();
    void InitControlExec(VAPoR::ControlExec *ce);
    void ShowMe();

private slots:
    void _newScript();
    void _openScript(){};
    void _saveScript(int index);
    void _deleteScript(){};

    void _renameScript(){};
    void _showGridVariables(){};
    void _createNewVariable(){};
    void _deleteVariable(){};

    void _testScript(){};

private:
    QMenuBar *_menuBar;
    QMenu *   _fileMenu;

    VAPoR::ControlExec *_controlExec;
    //    VAPoR::DataMgr* _dataMgr;
    VAPoR::ParamsMgr *            _paramsMgr;
    VAPoR::PythonVariablesParams *_pythonParams;

    Fader *        _fader;
    NewItemDialog *_newScriptDialog;
    VaporTable *   _inputVariableTable;

    string _scriptName;
    string _dataMgrName;

    void _connectWidgets();
    void _setGUIEnabled(bool enabled);

    void _updateGridComboBox();
    void _updateNewItemDialog();
    void _updateInputVarTable(){};
    void _updateOutputVarTable(){};
    void _updatePythonScript(){};
};

class Fader : public QObject {
    Q_OBJECT

public:
    Fader(QLabel *label, QColor background, QObject *parent = 0);

private:
    QLabel *_myLabel;
    QColor  _background;

private slots:
    void _fade();
};

class NewItemDialog : public QDialog {
    Q_OBJECT

public:
    enum { SCRIPT = 0, INVAR = 1, OUTVAR = 2 };

    NewItemDialog(QWidget *parent = 0);
    ~NewItemDialog(){};

    void   Update(int type, std::vector<string>);
    string GetItemName() const;
    string GetOptionName() const;

private:
    void _connectWidgets();
    void _setupGUI();

    string _itemName;
    string _optionName;

    QLabel *     _itemNameLabel;
    QLineEdit *  _itemNameEdit;
    QLabel *     _optionNameLabel;
    QComboBox *  _optionNameCombo;
    QPushButton *_okButton;
    QPushButton *_cancelButton;

private slots:
    void _okClicked();
    void _cancelClicked(){};
    void _optionChanged(){};
    void _itemNameChanged(const QString &){};
};

#endif    // PYTHOVARIABLES_H
