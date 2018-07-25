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

using namespace VAPoR;

string VariablesWidget::_nDimsTag = "ActiveDimension";

VariablesWidget::VariablesWidget(QWidget *parent) : QWidget(parent), Ui_VariablesWidgetGUI()
{
    _activeDim = 3;

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

    // Legacy crap. Should remove
    //
    distribVariableFrame->hide();
}

void VariablesWidget::Reinit(DisplayFlags dspFlags, DimFlags dimFlags)
{
    _dspFlags = dspFlags;
    _dimFlags = dimFlags;

    showHideVar(true);

    if (!((_dimFlags & TWOD) && (_dimFlags & THREED))) {
        dimensionFrame->hide();
        if (dimFlags & TWOD)
            _activeDim = TWODIMS;
        else
            _activeDim = THREEDIMS;
    }

    if (_dspFlags ^ COLOR) { collapseColorVarSettings(); }

    variableSelectionWidget->adjustSize();

    FidelityWidget::DisplayFlags fdf = (FidelityWidget::DisplayFlags)0;
    if (_dimFlags & VariablesWidget::SCALAR) fdf = (FidelityWidget::DisplayFlags)(fdf | FidelityWidget::SCALAR);
    if (_dimFlags & VariablesWidget::VECTOR) fdf = (FidelityWidget::DisplayFlags)(fdf | FidelityWidget::VECTOR);
    _fidelityWidget->Reinit(fdf);
}

void VariablesWidget::collapseColorVarSettings()
{
    colormapVarCombo->hide();
    colormapVarCombo->resize(0, 0);
    colorVarLabel->hide();
    colorVarLabel->resize(0, 0);
}

void VariablesWidget::setVarName(const QString &qname)
{
    assert(_rParams);

    _paramsMgr->BeginSaveStateGroup("Set variable and possible color "
                                    "variable name");

    if (!(_dspFlags & SCALAR)) return;

    string name = qname.toStdString();
    name = name == "0" ? "" : name;
    _rParams->SetVariableName(name);

    if (!(_dspFlags & COLOR)) _rParams->SetColorMapVariableName(name);

    _paramsMgr->EndSaveStateGroup();
}

void VariablesWidget::setVectorVarName(const QString &qname, int component)
{
    assert(_rParams);
    assert(component >= 0 && component <= 2);

    if (!(_dspFlags & VECTOR)) return;
    // if ((! (_dimFlags & THREED)) && component == 2) return;

    string name = qname.toStdString();
    name = name == "0" ? "" : name;

    vector<string> varnames = _rParams->GetFieldVariableNames();
    varnames[component] = name;
    _rParams->SetFieldVariableNames(varnames);
}

void VariablesWidget::setXVarName(const QString &name)
{
    assert(_rParams);
    setVectorVarName(name, 0);
}

void VariablesWidget::setYVarName(const QString &name)
{
    assert(_rParams);
    setVectorVarName(name, 1);
}

void VariablesWidget::setZVarName(const QString &name)
{
    assert(_rParams);
    setVectorVarName(name, 2);
}

void VariablesWidget::setXDistVarName(const QString &name) { assert(_rParams); }

void VariablesWidget::setYDistVarName(const QString &name) { assert(_rParams); }

void VariablesWidget::setZDistVarName(const QString &name) { assert(_rParams); }

void VariablesWidget::setHeightVarName(const QString &qname)
{
    assert(_rParams);

    if (!(_dspFlags & HGT)) return;

    string name = qname.toStdString();
    name = name == "0" ? "" : name;
    _rParams->SetHeightVariableName(name);
}

void VariablesWidget::setColorMappedVariable(const QString &qname)
{
    assert(_rParams);

    if (!(_dspFlags & COLOR)) return;

    string name = qname.toStdString();
    name = name == "0" ? "" : name;
    _rParams->SetColorMapVariableName(name);
}

void VariablesWidget::setVariableDims(int index)
{
    assert(_rParams);
    if (!((_dimFlags & TWOD) && (_dimFlags & THREED))) return;
    assert(index >= 0 && index <= 1);

    _activeDim = index == 0 ? TWODIMS : THREEDIMS;

    _paramsMgr->BeginSaveStateGroup("Set variable dimensions");

    _rParams->SetValueLong(_nDimsTag, "Set variable dimensions", _activeDim);

    // Need to refresh variable list if dimension changes
    //
    updateCombos();

    _paramsMgr->EndSaveStateGroup();
}

void VariablesWidget::showHideVar(bool on)
{
    if ((_dspFlags & SCALAR) && on) {
        singleVariableFrame->show();
    } else {
        singleVariableFrame->hide();
    }

    if ((_dspFlags & VECTOR) && on) {
        fieldVariableFrame->show();
    } else {
        fieldVariableFrame->hide();
    }

    if ((_dspFlags & HGT) && on) {
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
    my_varnames.insert(my_varnames.begin(), "0");
    if (currentVar == "") { currentVar = "0"; }

    varCombo->clear();
    varCombo->setMaxCount(my_varnames.size());

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
    if (_dspFlags & SCALAR) {
        string         setVarReq = _rParams->GetVariableName();
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
    if (_dspFlags & VECTOR) {
        vector<string> setVarsReq = _rParams->GetFieldVariableNames();
        assert(setVarsReq.size() == 3);

        vector<string> setVars;
        vector<string> vars = _dataMgr->GetDataVarNames(_activeDim);
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
    if (_dspFlags & COLOR) {
        vector<string> vars = _dataMgr->GetDataVarNames(_activeDim);
        string         setVarReq = _rParams->GetColorMapVariableName();

        string setVar = updateVarCombo(colormapVarCombo, vars, true, setVarReq);

        if (setVar != setVarReq) {
            bool enabled = _paramsMgr->GetSaveStateEnabled();
            _paramsMgr->SetSaveStateEnabled(false);
            _rParams->SetColorMapVariableName(setVar);
            _paramsMgr->SetSaveStateEnabled(enabled);
        }
    }
}

void VariablesWidget::updateHeightCombo()
{
    if (_dspFlags & HGT) {
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
    int ndim = _rParams->GetValueLong(_nDimsTag, THREEDIMS);
    assert(ndim == TWODIMS || ndim == THREEDIMS);

    vector<string> vars = _dataMgr->GetDataVarNames(ndim);

    if (!vars.size()) {
        showHideVar(false);
        return;
    }
    showHideVar(true);

    updateScalarCombo();
    updateVectorCombo();
    updateColorCombo();
    updateHeightCombo();
}

void VariablesWidget::updateDims()
{
    if (!((_dimFlags & TWOD) && (_dimFlags & THREED))) {
        // Need to set default variable dimension even if only support
        // single dimension option.
        //
        int defaultDim = 2;
        if (_dimFlags & TWOD) { defaultDim = 2; }
        if (_dimFlags & THREED) { defaultDim = 3; }

        bool enabled = _paramsMgr->GetSaveStateEnabled();

        _paramsMgr->SetSaveStateEnabled(false);
        _rParams->SetValueLong(_nDimsTag, "", defaultDim);
        _paramsMgr->SetSaveStateEnabled(enabled);

        dimensionFrame->hide();
        return;
    }

    dimensionFrame->show();

    int ndim = _rParams->GetValueLong(_nDimsTag, 3);

    if (ndim < 2 || ndim > 3) {
        ndim = 2;
        _rParams->SetValueLong(_nDimsTag, "Set variable dimensions", ndim);
    }

    int index = ndim == 2 ? 0 : 1;
    dimensionCombo->setCurrentIndex(index);

    // Nono!  Do not do this!  We want to
    // keep our old Box after var dimension change!
    //_rParams->_initBox();
}

void VariablesWidget::Update(const DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams)
{
    assert(dataMgr);
    assert(paramsMgr);
    assert(rParams);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    updateDims();
    updateCombos();

    _fidelityWidget->Update(_dataMgr, _paramsMgr, _rParams);
}
