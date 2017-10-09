//************************************************************************
//														*
//		     Copyright (C)  2015									*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
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
#include <qradiobutton.h>
#include "vapor/RenderParams.h"
#include "vapor/ParamsMgr.h"
#include "vapor/DataMgr.h"
#include "VariablesWidget.h"

using namespace VAPoR;

string VariablesWidget::_nDimsTag = "ActiveDimension";

VariablesWidget::VariablesWidget(QWidget *parent) : QWidget(parent), Ui_VariablesWidgetGUI()
{
    setupUi(this);

    _fidelityButtons = new QButtonGroup(fidelityBox);
    _fidelityButtons->setExclusive(true);

    QHBoxLayout *hlay = new QHBoxLayout(fidelityBox);
    hlay->setAlignment(Qt::AlignHCenter);
    fidelityBox->setLayout(hlay);

    connect(refinementCombo, SIGNAL(activated(int)), this, SLOT(setNumRefinements(int)));
    connect(lodCombo, SIGNAL(activated(int)), this, SLOT(setCompRatio(int)));
    connect(fidelityDefaultButton, SIGNAL(clicked()), this, SLOT(SetFidelityDefault()));
    connect(varnameCombo, SIGNAL(activated(const QString &)), this, SLOT(setVarName(const QString &)));
    connect(varCombo1, SIGNAL(activated(const QString &)), this, SLOT(setXVarName(const QString &)));
    connect(varCombo2, SIGNAL(activated(const QString &)), this, SLOT(setYVarName(const QString &)));
    connect(varCombo3, SIGNAL(activated(const QString &)), this, SLOT(setZVarName(const QString &)));
    connect(distvarCombo1, SIGNAL(activated(const QString &)), this, SLOT(setXDistVarName(const QString &)));
    connect(distvarCombo2, SIGNAL(activated(const QString &)), this, SLOT(setYDistVarName(const QString &)));
    connect(distvarCombo3, SIGNAL(activated(const QString &)), this, SLOT(setZDistVarName(const QString &)));
    connect(dimensionCombo, SIGNAL(activated(int)), this, SLOT(setVariableDims(int)));
    connect(heightCombo, SIGNAL(activated(const QString &)), this, SLOT(setHeightVarName(const QString &)));

    connect(_fidelityButtons, SIGNAL(buttonClicked(int)), this, SLOT(setFidelity(int)));

    // Legacy crap. Should remove
    //
    distribVariableFrame->hide();

#ifdef DEAD
    if (!(dspFlags & COLOR)) { colorVarCombo->hide(); }
#endif
}

void VariablesWidget::Reinit(DisplayFlags dspFlags, DimFlags dimFlags)
{
    _dspFlags = dspFlags;
    _dimFlags = dimFlags;

    showHideVar(true);

    if (!((_dimFlags & TWOD) && (_dimFlags & THREED))) { dimensionFrame->hide(); }
}

void VariablesWidget::setNumRefinements(int num)
{
    assert(_rParams);

    _rParams->SetRefinementLevel(num);

    // Fidelity settings no longer valid
    //
    uncheckFidelity();
}

void VariablesWidget::setVarName(const QString &qname)
{
    assert(_rParams);

    if (!(_dspFlags & SCALAR)) return;

    string name = qname.toStdString();
    name = name == "0" ? "" : name;
    _rParams->SetVariableName(name);
}

void VariablesWidget::setVectorVarName(const QString &qname, int component)
{
    assert(_rParams);
    assert(component >= 0 && component <= 2);

    if (!(_dspFlags & VECTOR)) return;
    if ((!(_dimFlags & THREED)) && component == 2) return;

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

void VariablesWidget::setXDistVarName(const QString &name)
{
    assert(_rParams);
#ifdef DEAD
#endif
}

void VariablesWidget::setYDistVarName(const QString &name)
{
    assert(_rParams);
#ifdef DEAD
#endif
}

void VariablesWidget::setZDistVarName(const QString &name)
{
    assert(_rParams);
#ifdef DEAD
#endif
}

void VariablesWidget::setHeightVarName(const QString &qname)
{
    assert(_rParams);

    if (!(_dspFlags & HGT)) return;

    string name = qname.toStdString();
    name = name == "0" ? "" : name;
    _rParams->SetHeightVariableName(name);
}

// Occurs when user clicks a fidelity radio button
void VariablesWidget::setFidelity(int buttonID)
{
    assert(_rParams);

    assert(buttonID >= 0 && buttonID < _fidelityLodIdx.size());

    int lod = _fidelityLodIdx[buttonID];
    int ref = _fidelityMultiresIdx[buttonID];

    _paramsMgr->BeginSaveStateGroup("Set variable fidelity");
    _rParams->SetCompressionLevel(lod);
    _rParams->SetRefinementLevel(ref);

    _paramsMgr->EndSaveStateGroup();

    // Need to update the GUI
    //
    lodCombo->setCurrentIndex(lod);
    refinementCombo->setCurrentIndex(ref);
}

// User clicks on SetDefault button, need to make current
// fidelity settings the default.

void VariablesWidget::SetFidelityDefault()
{
#ifdef DEAD

    // Check current values of LOD and refinement and their combos.

    _renderEV->confirmText();
    _dataStatus->setFidelityDefault(rParams);
    StartupParams *sParams = (StartupParams *)_paramsMgr->GetDefaultParams(StartupParams::_startupParamsTag);
    _controlExec->SavePreferences(sParams->GetCurrentPrefsPath());
    updateTab(rParams);
#endif
}

void VariablesWidget::getCmpFactors(string varname, vector<float> &lodCF, vector<string> &lodStr, vector<float> &multiresCF, vector<string> &multiresStr) const
{
    assert(!varname.empty());

    lodCF.clear();
    lodStr.clear();
    multiresCF.clear();
    multiresStr.clear();

    int numLevels = _dataMgr->GetNumRefLevels(varname);

    // First get compression factors that are based on grid multiresolution
    //

    // Compute sorted list of number of grids points
    // at each level in multiresolution hierarchy
    //
    vector<size_t> nGridPts;
    for (int l = 0; l < numLevels; l++) {
        vector<size_t> dims_at_level, bs_at_level;
        int            rc = _dataMgr->GetDimLensAtLevel(varname, l, dims_at_level, bs_at_level);
        assert(rc >= 0);

        size_t        n = 1;
        ostringstream oss;
        oss << l << " (";
        for (int j = 0; j < dims_at_level.size(); j++) {
            n *= dims_at_level[j];

            oss << dims_at_level[j];
            if (j < dims_at_level.size() - 1) oss << "x";
        }
        nGridPts.push_back(n);

        oss << ")";
        multiresStr.push_back(oss.str());
    }

    for (int i = 0; i < nGridPts.size() - 1; i++) {
        float cf = 1.0 / (nGridPts[nGridPts.size() - 1] / nGridPts[i]);
        multiresCF.push_back(cf);
    }
    multiresCF.push_back(1.0);

    // Now get the "levels of detail" compression factors
    //
    vector<size_t> cratios = _dataMgr->GetCRatios(varname);

    for (int i = 0; i < cratios.size(); i++) {
        ostringstream oss;
        lodCF.push_back((float)1.0 / cratios[i]);

        oss << i << " (" << cratios[i] << ":1)";
        lodStr.push_back(oss.str());
    }
}

void VariablesWidget::updateFidelity(RenderParams *rParams)
{
    string varname;
    if (_dspFlags & SCALAR) {
        varname = rParams->GetVariableName();
    } else if (_dspFlags & VECTOR) {
        vector<string> varnames = rParams->GetFieldVariableNames();
        assert(varnames.size());
        varname = varnames[0];
    }

    vector<size_t> cratios = _dataMgr->GetCRatios(varname);

    // Get the effective compression rates as a floating point value,
    // and as a string that can be displayed, for the LOD and refinement
    // control
    //
    vector<float>  lodCFs, multiresCFs;
    vector<string> lodStrs, multiresStrs;
    getCmpFactors(varname, lodCFs, lodStrs, multiresCFs, multiresStrs);

    int lodReq = rParams->GetCompressionLevel();
    int refLevelReq = rParams->GetRefinementLevel();

    int lod = lodReq < 0 ? 0 : lodReq;
    lod = lodReq >= lodCFs.size() ? lodCFs.size() - 1 : lodReq;

    int refLevel = refLevelReq < 0 ? 0 : refLevelReq;
    refLevel = refLevelReq >= multiresCFs.size() ? multiresCFs.size() - 1 : refLevelReq;

    // set up the refinement and LOD combos
    //
    lodCombo->clear();
    for (int i = 0; i < lodStrs.size(); i++) {
        QString s = QString(lodStrs[i].c_str());
        lodCombo->addItem(s);
    }
    lodCombo->setCurrentIndex(lod);

    refinementCombo->clear();
    for (int i = 0; i < multiresStrs.size(); i++) { refinementCombo->addItem(QString(multiresStrs[i].c_str())); }
    refinementCombo->setCurrentIndex(refLevel);

    if (lodReq != lod) { rParams->SetCompressionLevel(lod); }
    if (refLevelReq != refLevel) { rParams->SetRefinementLevel(refLevel); }

    fidelityBox->adjustSize();

    // Linearize the LOD and refinement compression ratios so that
    // when combined they increase (decrease) monotonically
    //

    _fidelityLodIdx.clear();
    _fidelityMultiresIdx.clear();
    _fidelityLodStrs.clear();
    _fidelityMultiresStrs.clear();

    int l = 0;
    int m = 0;
    do {
        _fidelityLodIdx.push_back(l);
        _fidelityMultiresIdx.push_back(m);

        _fidelityLodStrs.push_back(lodStrs[l]);
        _fidelityMultiresStrs.push_back(multiresStrs[m]);

        if (lodCFs[l] < multiresCFs[m]) {
            l++;
        } else {
            m++;
        }
    } while (l < lodCFs.size() && m < multiresCFs.size());

    // Remove buttons from the group
    //
    QList<QAbstractButton *> btns = _fidelityButtons->buttons();
    for (int i = 0; i < btns.size(); i++) { _fidelityButtons->removeButton(btns[i]); }

    // Remove and delete buttons from the layout
    //
    QHBoxLayout *hlay = (QHBoxLayout *)fidelityBox->layout();
    QLayoutItem *child;
    while ((child = hlay->takeAt(0)) != 0) { delete child; }

    int numButtons = _fidelityLodStrs.size();
    for (int i = 0; i < numButtons; i++) {
        QRadioButton *rd = new QRadioButton();
        hlay->addWidget(rd);

        _fidelityButtons->addButton(rd, i);
        QString qs = "Refinement " + QString(_fidelityMultiresStrs[i].c_str()) + "\nLOD " + QString(_fidelityLodStrs[i].c_str());

        rd->setToolTip(qs);

        if (lod == _fidelityLodIdx[i] && refLevel == _fidelityMultiresIdx[i]) { rd->setChecked(true); }
    }
    fidelityBox->setLayout(hlay);
}

void VariablesWidget::uncheckFidelity()
{
    // Unset all fidelity buttons
    //
    if (!_fidelityButtons) return;

    QList<QAbstractButton *> btns = _fidelityButtons->buttons();
    for (int i = 0; i < btns.size(); i++) {
        if (btns[i]->isChecked()) { btns[i]->setChecked(false); }
    }
}

void VariablesWidget::setCompRatio(int num)
{
    assert(_rParams);

    _rParams->SetCompressionLevel(num);

    lodCombo->setCurrentIndex(num);

    // Fidelity no longer valid
    //
    uncheckFidelity();
}

void VariablesWidget::setVariableDims(int index)
{
    assert(_rParams);
    if (!((_dimFlags & TWOD) && (_dimFlags & THREED))) return;
    assert(index >= 0 && index <= 1);

    int ndim = index == 0 ? 2 : 3;

    _paramsMgr->BeginSaveStateGroup("Set variable dimensions");

    _rParams->SetValueLong(_nDimsTag, "Set variable dimensions", ndim);

    // Need to refresh variable list if dimension changes
    //
    updateVariableCombos(_rParams);

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
        if (!(_dimFlags & THREED)) { varCombo3->hide(); }
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
    varCombo->clear();
    if (doZero) {
        varCombo->setMaxCount(varnames.size() + 1);
        varCombo->addItem(QString("0"));
    } else {
        varCombo->setMaxCount(varnames.size());
    }

    int currentIndex = -1;
    for (int i = 0; i < varnames.size(); i++) {
        const string s = varnames[i];
        varCombo->addItem(QString::fromStdString(s));
        if (s == currentVar) {
            currentIndex = i;
            if (doZero) { currentIndex++; }
        }
    }
    if (currentIndex == -1) {
        varCombo->setCurrentIndex(0);
        return (varCombo->currentText().toStdString());
    } else {
        varCombo->setCurrentIndex(currentIndex);
        return (currentVar);
    }
}

void VariablesWidget::updateVariableCombos(RenderParams *rParams)
{
    int ndim = rParams->GetValueLong(_nDimsTag, 3);
    assert(ndim == 2 || ndim == 3);

    vector<string> vars = _dataMgr->GetDataVarNames(ndim, true);

    if (!vars.size()) {
        showHideVar(false);
        return;
    }
    showHideVar(true);

    if (_dspFlags & SCALAR) {
        string setVarReq = rParams->GetVariableName();
        string setVar = updateVarCombo(varnameCombo, vars, false, setVarReq);
        if (setVar != setVarReq) { rParams->SetVariableName(setVar); }
    }

    if (_dspFlags & VECTOR) {
        vector<string> setVarsReq = rParams->GetFieldVariableNames();
        assert(setVarsReq.size() == 3);

        vector<string> setVars;

        setVars.push_back(updateVarCombo(varCombo1, vars, true, setVarsReq[0]));
        setVars.push_back(updateVarCombo(varCombo2, vars, true, setVarsReq[1]));
        setVars.push_back(updateVarCombo(varCombo3, vars, true, setVarsReq[2]));

        for (int i = 0; i < setVars.size(); i++) {
            if (setVars[i] != setVarsReq[i]) {
                rParams->SetFieldVariableNames(setVars);
                break;
            }
        }
    }

#ifdef DEAD
    if (_dspFlags & COLOR) {
        updateVarCombo(colorVarCombo, vars, true);

        vector<string> varnames;
        varnames.push_back(colorVarCombo->currentText().toStdString());
        rParams->SetColorMapVariableNames(varnames);
    }
#endif

    if (_dspFlags & HGT) {
        vector<string> vars = _dataMgr->GetDataVarNames(2, true);
        string         setVarReq = rParams->GetHeightVariableName();

        string setVar = updateVarCombo(heightCombo, vars, true, setVarReq);

        if (setVar != setVarReq) { rParams->SetHeightVariableName(setVar); }
    }
}

void VariablesWidget::updateDims(RenderParams *rParams)
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

        rParams->SetValueLong(_nDimsTag, "", defaultDim);

        _paramsMgr->SetSaveStateEnabled(enabled);

        dimensionFrame->hide();
        return;
    }

    dimensionFrame->show();

    int ndim = rParams->GetValueLong(_nDimsTag, 3);

    if (ndim < 2 || ndim > 3) {
        ndim = 2;
        rParams->SetValueLong(_nDimsTag, "Set variable dimensions", ndim);
    }

    int index = ndim == 2 ? 0 : 1;
    dimensionCombo->setCurrentIndex(index);
}

void VariablesWidget::Update(const DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams)
{
    assert(dataMgr);
    assert(paramsMgr);
    assert(rParams);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    updateDims(rParams);

    updateVariableCombos(rParams);

    updateFidelity(rParams);
}
