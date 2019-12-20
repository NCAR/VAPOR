//************************************************************************
//														*
//			 Copyright (C)  2015									*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//														*
//************************************************************************/
//
//	File:		variablesWidget.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Implements the VariablesWidget class.  This provides
//		a widget that is inserted in the "Variables" tab of various Renderer GUIs
//
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <qradiobutton.h>
#include <qcolordialog.h>
#include "vapor/RenderParams.h"
#include "vapor/ParamsMgr.h"
#include "vapor/DataMgr.h"
#include "VariablesWidget.h"

#define TWODIMS   2
#define THREEDIMS 3
#define X         0
#define Y         1
#define Z         2

using namespace VAPoR;

VariablesWidget::VariablesWidget(QWidget *parent) : QWidget(parent), Ui_VariablesWidgetGUI()
{
    _activeDim = THREEDIMS;

    setupUi(this);

    connect(varnameCombo, SIGNAL(activated(const QString &)), this, SLOT(setVarName(const QString &)));
    connect(varCombo1, SIGNAL(activated(const QString &)), this, SLOT(setXVarName(const QString &)));
    connect(varCombo2, SIGNAL(activated(const QString &)), this, SLOT(setYVarName(const QString &)));
    connect(varCombo3, SIGNAL(activated(const QString &)), this, SLOT(setZVarName(const QString &)));
    connect(distvarCombo1, SIGNAL(activated(const QString &)), this, SLOT(setXDistVarName(const QString &)));
    connect(distvarCombo2, SIGNAL(activated(const QString &)), this, SLOT(setYDistVarName(const QString &)));
    connect(distvarCombo3, SIGNAL(activated(const QString &)), this, SLOT(setZDistVarName(const QString &)));
    connect(dimensionCombo, SIGNAL(activated(int)), this, SLOT(setVariableDims(int)));
    connect(heightCombo, SIGNAL(activated(const QString &)), this, SLOT(setHeightVarName(const QString &)));
    connect(colormapVarCombo, SIGNAL(activated(const QString &)), this, SLOT(setColorMappedVariable(const QString &)));

    connect(orientationCombo, SIGNAL(activated(const QString &)), this, SLOT(set2DOrientation(const QString &)));

    // Legacy crap. Should remove
    //
    distribVariableFrame->hide();

    orientationFrame->hide();
}

void VariablesWidget::Reinit(VariableFlags variableFlags, DimFlags dimFlags)
{
    _variableFlags = variableFlags;
    _dimFlags = dimFlags;

    showHideVarCombos(true);

    // If the renderer is not both 2D and 3D, hide
    // the dimension selector and set the _activeDim
    if (!((_dimFlags & TWOD) && (_dimFlags & THREED))) {
        dimensionFrame->hide();
        if (dimFlags & THREED) {
            _activeDim = THREEDIMS;
            orientationFrame->hide();
        } else
            _activeDim = TWODIMS;
        // orientationFrame->show();
    }

    // If the renderer is only 3D, hide the 2D orientation selector
    orientationFrame->hide();

    VariableFlags fdf = (VariableFlags)0;
    if (_variableFlags & SCALAR) fdf = (VariableFlags)(fdf | SCALAR);

    if (_variableFlags & VECTOR) fdf = (VariableFlags)(fdf | VECTOR);

    if (_variableFlags & HEIGHT) fdf = (VariableFlags)(fdf | HEIGHT);

    _fidelityWidget->Reinit(fdf);

    variableSelectionWidget->adjustSize();
    adjustSize();
}

void VariablesWidget::collapseColorVarSettings() { colorVariableFrame->hide(); }

void VariablesWidget::setVarName(const QString &qname)
{
    VAssert(_rParams);

    _paramsMgr->BeginSaveStateGroup("Set variable and possible color "
                                    "variable name");

    if (!(_variableFlags & SCALAR)) return;

    string name = qname.toStdString();
    name = name == "<no-variable>" ? "" : name;
    _rParams->SetVariableName(name);

    if (!(_variableFlags & COLOR)) _rParams->SetColorMapVariableName(name);

    _paramsMgr->EndSaveStateGroup();
}

void VariablesWidget::setVectorVarName(const QString &qname, int component)
{
    VAssert(_rParams);
    VAssert(component >= 0 && component <= 2);

    if (!(_variableFlags & VECTOR)) return;

    string name = qname.toStdString();
    name = name == "<no-variable>" ? "" : name;

    vector<string> varnames = _rParams->GetFieldVariableNames();
    varnames[component] = name;
    _rParams->SetFieldVariableNames(varnames);
}

void VariablesWidget::setXVarName(const QString &name)
{
    VAssert(_rParams);
    setVectorVarName(name, X);
}

void VariablesWidget::setYVarName(const QString &name)
{
    VAssert(_rParams);
    setVectorVarName(name, Y);
}

void VariablesWidget::setZVarName(const QString &name)
{
    VAssert(_rParams);
    setVectorVarName(name, Z);
}

void VariablesWidget::setXDistVarName(const QString &name) { VAssert(_rParams); }

void VariablesWidget::setYDistVarName(const QString &name) { VAssert(_rParams); }

void VariablesWidget::setZDistVarName(const QString &name) { VAssert(_rParams); }

void VariablesWidget::setHeightVarName(const QString &qname)
{
    VAssert(_rParams);

    if (!(_variableFlags & HEIGHT)) return;

    string name = qname.toStdString();
    name = name == "<no-variable>" ? "" : name;
    _rParams->SetHeightVariableName(name);
}

void VariablesWidget::setColorMappedVariable(const QString &qname)
{
    VAssert(_rParams);

    if (!(_variableFlags & COLOR)) return;

    string name = qname.toStdString();
    name = name == "<no-variable>" ? "" : name;
    _rParams->SetColorMapVariableName(name);
}

void VariablesWidget::set2DOrientation(const QString &orientation) { cout << "2D orientation is currently a no-op" << endl; }

// This takes the dropdown menu index, not the dimension
void VariablesWidget::setVariableDims(int index)
{
    VAssert(_rParams);
    if (!((_dimFlags & TWOD) && (_dimFlags & THREED))) return;
    VAssert(index >= 0 && index <= 1);

    //_activeDim = index == 0 ? TWODIMS : THREEDIMS;
    if (index == 0) {
        _activeDim = TWODIMS;
        _rParams->GetBox()->SetOrientation(VAPoR::Box::XY);
        // orientationFrame->show();
    } else {
        _activeDim = THREEDIMS;
        orientationFrame->hide();
        _rParams->GetBox()->SetOrientation(VAPoR::Box::XYZ);
    }

    _paramsMgr->BeginSaveStateGroup("Set variable dimensions");
    setDefaultVariables();
    _rParams->InitBox(_activeDim);
    _paramsMgr->EndSaveStateGroup();

    // Need to referesh variable list if dimension changes
    //
    updateCombos();
}

void VariablesWidget::setDefaultVariables()
{
    bool secondaryColormapVariable = false;
    if (_variableFlags & COLOR) secondaryColormapVariable = true;
    _rParams->SetDefaultVariables(_activeDim, secondaryColormapVariable);
}

// Default scalar variable will just be the first variable
// of the active dimension (2D or 3D)
void VariablesWidget::setDefaultScalarVar(std::vector<string> vars)
{
    if (_variableFlags & SCALAR) {
        string defaultVar = vars[0];
        _rParams->SetVariableName(defaultVar);
        if (_variableFlags ^ COLOR) _rParams->SetColorMapVariableName(defaultVar);
    }
}

// Default vector variables only apply to X and Y components.
// We try to find variables that correspond to U and V
void VariablesWidget::setDefaultVectorVar(std::vector<string> vars)
{
    if (_variableFlags & VECTOR) {
        std::vector<string> defaultVars;

        string defaultVar = findVarStartingWithLetter(vars, 'u');
        defaultVars.push_back(defaultVar);
        defaultVar = findVarStartingWithLetter(vars, 'v');
        defaultVars.push_back(defaultVar);
        if (_dimFlags & THREED) {
            defaultVar = findVarStartingWithLetter(vars, 'w');
            defaultVars.push_back(defaultVar);
        } else
            defaultVars.push_back("");
        _rParams->SetFieldVariableNames(defaultVars);
    }
}

// A common color-mapped-variable is temperature, so we will try
// finding a variable starting with 't'
void VariablesWidget::setDefaultColorVar(std::vector<string> vars)
{
    string defaultVar;

    if (_variableFlags & COLOR) {
        string defaultVar = findVarStartingWithLetter(vars, 't');
        _rParams->SetColorMapVariableName(defaultVar);
    }
}

void VariablesWidget::showHideVarCombos(bool on)
{
    if ((_variableFlags & SCALAR) && on) {
        singleVariableFrame->show();
    } else {
        singleVariableFrame->hide();
    }

    if ((_variableFlags & VECTOR) && on) {
        fieldVariableFrame->show();
    } else {
        fieldVariableFrame->hide();
    }

    if ((_variableFlags & HEIGHT) && on) {
        heightVariableFrame->show();
    } else {
        heightVariableFrame->hide();
    }
}

// Populate the specified combo box with a list of variables and set
// the current variable to 'currentVar'. If currentVar does not exist
// in 'varnames', pick another variable name. The name of the variable
// set as the current on in the combo box is returned
//
string VariablesWidget::updateVarCombo(QComboBox *varCombo, const vector<string> &varnames, bool doZero, string currentVar)
{
    vector<string> my_varnames = varnames;

    if (doZero) my_varnames.insert(my_varnames.begin(), "<no-variable>");

    if (currentVar == "") { currentVar = "<no-variable>"; }

    varCombo->clear();
    varCombo->setMaxCount(my_varnames.size());

    if (my_varnames.size() == 0) return "";

    int currentIndex = -1;
    for (int i = 0; i < my_varnames.size(); i++) {
        const string s = my_varnames[i];
        varCombo->addItem(QString::fromStdString(s));
        if (s == currentVar) { currentIndex = i; }
    }
    if (currentIndex == -1) {
        varCombo->setCurrentIndex(0);
        return (my_varnames[0]);
    } else {
        varCombo->setCurrentIndex(currentIndex);
        return (my_varnames[currentIndex]);
    }
}

void VariablesWidget::updateScalarCombo()
{
    if (_variableFlags & SCALAR) {
        string setVarReq = _rParams->GetVariableName();

        vector<string> vars = _dataMgr->GetDataVarNames(_activeDim);
        string         setVar = updateVarCombo(varnameCombo, vars, false, setVarReq);
        if (setVar != setVarReq) {
            bool enabled = _paramsMgr->GetSaveStateEnabled();
            _paramsMgr->SetSaveStateEnabled(false);
            _rParams->SetVariableName(setVar);
            _paramsMgr->SetSaveStateEnabled(enabled);
        }
    }
}

void VariablesWidget::updateVectorCombo()
{
    if (_variableFlags & VECTOR) {
        vector<string> setVarsReq = _rParams->GetFieldVariableNames();
        VAssert(setVarsReq.size() == 3);

        vector<string> setVars;
        vector<string> vars = _dataMgr->GetDataVarNames(_activeDim);

        // If our vector variables are empty, choose some defaults
        if (setVarsReq[0] == "" && setVarsReq[1] == "" && setVarsReq[2] == "") {
            setDefaultVectorVar(vars);
            setVarsReq = _rParams->GetFieldVariableNames();
        }

        setVars.push_back(updateVarCombo(varCombo1, vars, true, setVarsReq[0]));
        setVars.push_back(updateVarCombo(varCombo2, vars, true, setVarsReq[1]));
        setVars.push_back(updateVarCombo(varCombo3, vars, true, setVarsReq[2]));

        bool enabled = _paramsMgr->GetSaveStateEnabled();
        _paramsMgr->SetSaveStateEnabled(false);

        for (int i = 0; i < setVars.size(); i++) {
            if (setVars[i] != setVarsReq[i]) { _rParams->SetFieldVariableNames(setVars); }
        }
        _paramsMgr->SetSaveStateEnabled(enabled);
    }
}

void VariablesWidget::updateColorCombo()
{
    if (_variableFlags & COLOR) {
        vector<string> vars = _dataMgr->GetDataVarNames(_activeDim);
        string         setVarReq = _rParams->GetColorMapVariableName();

        string setVar = updateVarCombo(colormapVarCombo, vars, false, setVarReq);

        if (setVar != setVarReq) {
            bool enabled = _paramsMgr->GetSaveStateEnabled();
            _paramsMgr->SetSaveStateEnabled(false);
            _rParams->SetColorMapVariableName(setVar);
            _paramsMgr->SetSaveStateEnabled(enabled);
        }
    } else {
        string colorVar = _rParams->GetVariableName();
        _rParams->SetColorMapVariableName(colorVar);
        collapseColorVarSettings();
    }
}

void VariablesWidget::updateHeightCombo()
{
    if (_variableFlags & HEIGHT) {
        vector<string> vars = _dataMgr->GetDataVarNames(TWODIMS);
        string         setVarReq = _rParams->GetHeightVariableName();

        string setVar = updateVarCombo(heightCombo, vars, true, setVarReq);

        if (setVar != setVarReq) {
            bool enabled = _paramsMgr->GetSaveStateEnabled();
            _paramsMgr->SetSaveStateEnabled(false);
            _rParams->SetHeightVariableName(setVar);
            _paramsMgr->SetSaveStateEnabled(enabled);
        }
    }
}

void VariablesWidget::updateCombos()
{
    VAssert(_activeDim == TWODIMS || _activeDim == THREEDIMS);

    vector<string> vars = _dataMgr->GetDataVarNames(_activeDim);

    updateDimCombo();
    updateScalarCombo();
    updateVectorCombo();
    updateColorCombo();
    updateHeightCombo();
}

void VariablesWidget::updateDimCombo()
{
    string varName = _rParams->GetVariableName();
    if (_dataMgr->VariableExists(_rParams->GetCurrentTimestep(), varName)) _activeDim = _dataMgr->GetVarTopologyDim(varName);

    // Only update if we support multiple dimensions
    if (((_dimFlags & TWOD) && (_dimFlags & THREED))) {
        int index = _activeDim - 2;
        dimensionCombo->setCurrentIndex(index);
    }
}

void VariablesWidget::Update(const DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams)
{
    VAssert(dataMgr);
    VAssert(paramsMgr);
    VAssert(rParams);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    vector<string> setVarsReq = _rParams->GetFieldVariableNames();

    updateCombos();

    _fidelityWidget->Update(_dataMgr, _paramsMgr, _rParams);

    if (_activeDim == THREEDIMS) {
        orientationFrame->hide();
        heightVariableFrame->hide();
    } else {
        if (_variableFlags & HEIGHT) heightVariableFrame->show();
    }
}

DimFlags VariablesWidget::GetDimFlags() const { return _dimFlags; }

string VariablesWidget::findVarStartingWithLetter(vector<string> searchVars, char letter)
{
    for (auto &element : searchVars) {
        if (element[0] == letter || element[0] == toupper(letter)) { return element; }
    }
    return "";
}
