#ifndef PYTHOVARIABLES_H
#define PYTHOVARIABLES_H

#include <vapor/ControlExecutive.h>

#include "ui_PythonVariablesGUI.h"
#include "PythonVariablesParams.h"
#include "VaporTable.h"

#include <QThread>
#include <QDialog>
#include <QMenuBar>
#include <QComboBox>
#include <QMenu>

//
// QObjects do not support nested classes, so use a namespace
//
namespace PythonVariables_ {
class NewItemDialog;
class OpenAndDeleteDialog;

static const string _scriptType = "Python";
}    // namespace PythonVariables_

class PythonVariables : public QDialog, Ui_PythonVariablesGUI {
    Q_OBJECT

public:
    PythonVariables(QWidget *parent);
    ~PythonVariables();
    void Update(bool internal = false);
    void InitControlExec(VAPoR::ControlExec *ce);
    void ShowMe();

private slots:
    void _newScript();
    void _openScript();
    void _deleteScript();
    void _importScript();
    void _exportScript();
    bool _getFilePath(QString &filePath, bool operation = true);
    void _testScript();
    void _saveScript();
    void _closeScript();
    void _updateSaveLabelColor(int r, int g, int b);
    void _updateTestLabelColor(int r, int g, int b);

    void _createNewVariable();
    void _deleteVariable();
    void _scriptChanged();

    void _coordInputVarChanged(int row, int col);
    void _2DInputVarChanged(int row, int col);
    void _3DInputVarChanged(int row, int col);

    void _findEnabledCoordinateVariables(const std::vector<string> variables, const std::vector<bool> variablesEnabled);
    void _coordinatesCheckboxClicked(int state);

private:
    const QColor *_background;

    VAPoR::ControlExec *_controlExec;

    PythonVariables_::NewItemDialog *      _newItemDialog;
    PythonVariables_::OpenAndDeleteDialog *_openAndDeleteDialog;

    VaporTable *_coordInputVarTable;
    VaporTable *_2DInputVarTable;
    VaporTable *_3DInputVarTable;
    VaporTable *_summaryTable;
    VaporTable *_outputVarTable;

    string _script;
    string _scriptName;
    string _dataMgrName;

    bool _justSaved;
    bool _includeCoordVars;

    std::vector<string> _coordVars;
    std::vector<bool>   _coordVarsEnabled;
    std::vector<string> _2DVars;
    std::vector<bool>   _2DVarsEnabled;
    std::vector<string> _3DVars;
    std::vector<bool>   _3DVarsEnabled;
    std::vector<string> _outputVars;
    std::vector<string> _outputGrids;
    std::vector<string> _inputGrids;
    std::vector<string> _otherGrids;

    void                _connectWidgets();
    void                _setGUIEnabled(bool enabled);
    void                _makeInputTableValues(std::vector<string> &tableValuesCoords, std::vector<string> &tableValues2D, std::vector<string> &tableValues3D, std::vector<string> &summaryValues) const;
    void                _makeOutputTableValues(std::vector<string> &outputValues) const;
    std::vector<string> _makeDialogOptions(std::vector<string> grids);
    std::vector<string> _buildInputVars() const;
    int                 _checkForDuplicateNames(std::vector<string> names, string name);
    bool                _isGridSelected(string grid, std::vector<string> selectedVars, std::vector<bool> varEnabled) const;
    void                _saveToSession();

    void _updateNewItemDialog();
    void _updateLabelColor(int r, int g, int b, QLabel *label);
    void _updateInputVarTable(){};
    void _updateOutputVarTable(){};
    void _updatePythonScript(){};

    void _showTestLabel(bool fadeIn);
    void _showSaveLabel(bool fadeIn);

    void _reset();
};

namespace PythonVariables_ {

class NewItemDialog : public QDialog {
    Q_OBJECT

public:
    enum { SCRIPT = 0, OUTVAR = 1 };

    NewItemDialog(QWidget *parent = 0);
    ~NewItemDialog(){};

    void   Update(int type, std::vector<string> optionNames, std::vector<int> categoryItems = std::vector<int>());
    string GetItemName() const;
    string GetOptionName() const;

private:
    void _connectWidgets();
    void _setupGUI();
    void _adjustToType(int type);
    void _disableComboItem(int index);

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
};

class OpenAndDeleteDialog : public QDialog {
    Q_OBJECT

public:
    enum {
        _OPEN = 0,
        _DELETE = 1    // DELETE is a reserved keyword on Windows
    };

    OpenAndDeleteDialog(QWidget *parent = 0);
    ~OpenAndDeleteDialog(){};

    int Update(int type, VAPoR::ControlExec *controlExec);

    string GetDataMgrName() const;
    string GetScriptName() const;

private:
    void _setupGUI();

    string _dataMgrName;
    string _scriptName;

    QLabel *     _dataMgrNameLabel;
    QComboBox *  _dataMgrNameCombo;
    QLabel *     _scriptNameLabel;
    QComboBox *  _scriptNameCombo;
    QPushButton *_okButton;
    QPushButton *_cancelButton;

    VAPoR::ControlExec *_controlExec;

private slots:
    void _okClicked();
    void _updateOptions(int index);
};

}    // namespace PythonVariables_

#endif    // PYTHOVARIABLES_H
