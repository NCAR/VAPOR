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
    void _openScript() { cout << "Open" << endl; }
    void _saveScript(int index);
    void _deleteScript() { cout << "Delete" << endl; }

    void _createNewVariable();
    void _deleteVariable();

    void _testScript() { cout << "Test" << endl; }

    void _2DInputVarChanged(int row, int col);
    void _3DInputVarChanged(int row, int col);

private:
    QMenuBar *    _menuBar;
    QMenu *       _fileMenu;
    const QColor *_background;

    VAPoR::ControlExec *_controlExec;
    //    VAPoR::DataMgr* _dataMgr;
    VAPoR::ParamsMgr *            _paramsMgr;
    VAPoR::PythonVariablesParams *_pythonParams;

    Fader *        _fader;
    NewItemDialog *_newItemDialog;
    VaporTable *   _2DInputVarTable;
    VaporTable *   _3DInputVarTable;
    VaporTable *   _summaryTable;
    VaporTable *   _outputVarTable;

    string _scriptName;
    string _dataMgrName;

    bool _justSaved;

    std::vector<string> _2DVars;
    std::vector<bool>   _2DVarsEnabled;
    std::vector<string> _3DVars;
    std::vector<bool>   _3DVarsEnabled;
    std::vector<string> _outputVars;
    std::vector<string> _outputGrids;

    void _connectWidgets();
    void _setGUIEnabled(bool enabled);
    void _makeInputTableValues(std::vector<string> &tableValues2D, std::vector<string> &tableValues3D, std::vector<string> &summaryValues) const;
    void _makeOutputTableValues(std::vector<string> &outputValues) const;
    int  _checkForDuplicateNames(std::vector<string> names, string name);
    void _deleteFader();
    void _saveToSession();
    void _saveToFile();

    void _updateNewItemDialog();
    void _updateInputVarTable(){};
    void _updateOutputVarTable(){};
    void _updatePythonScript(){};

    void _fade(bool fadeIn);
};

class Fader : public QObject {
    Q_OBJECT

public:
    Fader(bool fadeIn, QLabel *label, QColor background, QObject *parent = 0);

signals:
    void faderDone();

private:
    bool     _fadeIn;
    QThread *_thread;
    QLabel * _myLabel;
    QColor   _background;

private slots:
    void _fade();
};

class NewItemDialog : public QDialog {
    Q_OBJECT

public:
    enum { SCRIPT = 0, OUTVAR = 1 };

    NewItemDialog(QWidget *parent = 0);
    ~NewItemDialog(){};

    void   Update(int type, std::vector<string>);
    string GetItemName() const;
    string GetOptionName() const;

private:
    void _connectWidgets();
    void _setupGUI();
    void _adjustToType(int type);

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
