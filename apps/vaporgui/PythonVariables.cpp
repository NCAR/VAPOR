#include "PythonVariables.h"
#include "ui_PythonVariablesgui.h"

PythonVariablesGUI::PythonVariablesGUI(QWidget *parent) : QDialog(parent), ui(new Ui::PythonVariablesGUI) { ui->setupUi(this); }

PythonVariablesGUI::~PythonVariablesGUI() { delete ui; }
