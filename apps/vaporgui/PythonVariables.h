#ifndef PYTHOVARIABLES_H
#define PYTHOVARIABLES_H

#include <QDialog>
#include <vapor/ControlExecutive.h>

namespace Ui {
class PythonVariablesGUI;
}

class PythonVariablesGUI : public QDialog {
    Q_OBJECT

public:
    explicit PythonVariablesGUI(QWidget *parent = 0);
    ~PythonVariablesGUI();
    int  initControlExec(VAPoR::ControlExec *ce);
    void showMe();
    bool Update();

private slots:
    void _newScript();
    void _openScript();
    void _deleteScript();

    void _renameScript();
    void _showGridVariables();
    void _createNewVariable();
    void _deleteVariable();

    void _testScript();
    void _saveScript();

private:
    Ui::PythonVariablesGUI *ui;
};

#endif    // PYTHOVARIABLES_H
