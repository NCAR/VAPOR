//*************************************************************************
//                                                                        *
//           Copyright (C)  2015                                          *
//   University Corporation for Atmospheric Research                      *
//           All Rights Reserved                                          *
//                                                                        *
//************************************************************************/
//
//  File:       FidelityWidget.cpp
//
//      Author: John Clyne
//          Scott Pearse
//          Alan Norton
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:       December 2017
//
//  Description:    Implements the FidelityWidget class.
//
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QButtonGroup>
#include <qradiobutton.h>
#include <qcolordialog.h>
#include "vapor/RenderParams.h"
#include "vapor/ParamsMgr.h"
#include "vapor/DataMgr.h"
#include "FidelityWidget.h"

using namespace VAPoR;

FidelityWidget::FidelityWidget(QWidget *parent) : QWidget(parent), Ui_FidelityWidgetGUI()
{
    setupUi(this);

    _fidelityButtons = new QButtonGroup(fidelityBox);
    _fidelityButtons->setExclusive(true);

    QHBoxLayout *hlay = new QHBoxLayout(fidelityBox);
    hlay->setAlignment(Qt::AlignHCenter);
    fidelityBox->setLayout(hlay);

    int dpi = qApp->desktop()->logicalDpiX();
    if (dpi > 96) fidelityFrame->setMinimumHeight(100);

    connect(refinementCombo, SIGNAL(activated(int)), this, SLOT(setNumRefinements(int)));
    connect(lodCombo, SIGNAL(activated(int)), this, SLOT(setCompRatio(int)));
    connect(fidelityDefaultButton, SIGNAL(clicked()), this, SLOT(SetFidelityDefault()));
    connect(_fidelityButtons, SIGNAL(buttonClicked(int)), this, SLOT(setFidelity(int)));
}

void FidelityWidget::setNumRefinements(int num)
{
    VAssert(_rParams);

    _rParams->SetRefinementLevel(num);

    // Fidelity settings no longer valid
    //
    uncheckFidelity();
}

// Occurs when user clicks a fidelity radio button
//
void FidelityWidget::setFidelity(int buttonID)
{
    VAssert(_rParams);

    VAssert(buttonID >= 0 && buttonID < _fidelityLodIdx.size());

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

QButtonGroup *FidelityWidget::GetFidelityButtons() { return _fidelityButtons; }

std::vector<int> FidelityWidget::GetFidelityLodIdx() const { return _fidelityLodIdx; }

// User clicks on SetDefault button, need to make current
// fidelity settings the default.

void FidelityWidget::SetFidelityDefault()
{
#ifdef VAPOR3_0_0_ALPHA
    // Check current values of LOD and refinement and their combos.
    _renderEV->confirmText();
    _dataStatus->setFidelityDefault(rParams);
    StartupParams *sParams = (StartupParams *)_paramsMgr->GetDefaultParams(StartupParams::_startupParamsTag);
    _controlExec->SavePreferences(sParams->GetCurrentPrefsPath());
    updateTab(rParams);
#endif
}

void FidelityWidget::getCmpFactors(string varname, vector<float> &lodCF, vector<string> &lodStr, vector<float> &multiresCF, vector<string> &multiresStr) const
{
    VAssert(!varname.empty());

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
        vector<size_t> dims_at_level;
        int            rc = _dataMgr->GetDimLensAtLevel(varname, l, dims_at_level, -1);
        VAssert(rc >= 0);

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

void FidelityWidget::uncheckFidelity()
{
    // Unset all fidelity buttons
    //
    if (!_fidelityButtons) return;

    QList<QAbstractButton *> btns = _fidelityButtons->buttons();
    for (int i = 0; i < btns.size(); i++) {
        if (btns[i]->isChecked()) { btns[i]->setChecked(false); }
    }
}

void FidelityWidget::setCompRatio(int num)
{
    VAssert(_rParams);

    _rParams->SetCompressionLevel(num);

    lodCombo->setCurrentIndex(num);

    // Fidelity no longer valid
    //
    uncheckFidelity();
}

void FidelityWidget::Update(const DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams)
{
    VAssert(dataMgr);
    VAssert(paramsMgr);
    VAssert(rParams);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    string varname;
    if (_variableFlags & SCALAR) {
        varname = _rParams->GetVariableName();
    } else if (_variableFlags & VECTOR) {
        vector<string> varnames = _rParams->GetFieldVariableNames();
        if (varnames.size() > 0) {
            varname = varnames[0];
            size_t vardim;
            for (int i = 0; i < varnames.size(); i++) {
                vardim = _dataMgr->GetNumDimensions(varnames[i]);
                if (vardim == 3) {
                    varname = varnames[i];
                    break;
                }
            }
        }
    } else if (_variableFlags & HEIGHT) {
        varname = _rParams->GetHeightVariableName();
    } else if (_variableFlags & AUXILIARY) {
        vector<string> varnames = _rParams->GetAuxVariableNames();
        if (varnames.size() > 0) {
            varname = varnames[0];
            size_t vardim;
            for (int i = 0; i < varnames.size(); i++) {
                vardim = _dataMgr->GetNumDimensions(varnames[i]);
                if (vardim == 3) {
                    varname = varnames[i];
                    break;
                }
            }
        }
    } else if (_variableFlags & COLOR) {
        varname = _rParams->GetColorMapVariableName();
    }

    if (varname.empty()) {
        fidelityTab->setEnabled(false);
        return;
    }

    fidelityTab->setEnabled(true);
    fidelityTab->show();

    vector<size_t> cratios = _dataMgr->GetCRatios(varname);

    // Get the effective compression rates as a floating point value,
    // and as a string that can be displayed, for the LOD and refinement
    // control
    //
    vector<float>  lodCFs, multiresCFs;
    vector<string> lodStrs, multiresStrs;
    getCmpFactors(varname, lodCFs, lodStrs, multiresCFs, multiresStrs);

    int lodReq = _rParams->GetCompressionLevel();
    int refLevelReq = _rParams->GetRefinementLevel();

    int lod = lodReq < 0 ? 0 : lodReq;
    lod = lodReq >= lodCFs.size() ? lodCFs.size() - 1 : lodReq;

    int refLevel = refLevelReq < 0 ? 0 : refLevelReq;
    refLevel = refLevelReq >= multiresCFs.size() ? multiresCFs.size() - 1 : refLevelReq;

    // set up the refinement and LOD combos
    //
    lodCombo->blockSignals(true);
    lodCombo->clear();
    for (int i = 0; i < lodStrs.size(); i++) {
        QString s = QString::fromStdString(lodStrs[i]);
        lodCombo->addItem(s);
    }
    lodCombo->setCurrentIndex(lod);
    _currentLodStr = lodStrs.at(lod);
    lodCombo->blockSignals(false);

    refinementCombo->blockSignals(true);
    refinementCombo->clear();
    for (int i = 0; i < multiresStrs.size(); i++) { refinementCombo->addItem(QString(multiresStrs[i].c_str())); }
    refinementCombo->setCurrentIndex(refLevel);
    _currentMultiresStr = multiresStrs.at(refLevel);
    refinementCombo->blockSignals(false);

    if (lodReq != lod) { _rParams->SetCompressionLevel(lod); }
    if (refLevelReq != refLevel) { _rParams->SetRefinementLevel(refLevel); }

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

    _fidelityButtons->blockSignals(true);
    // Remove buttons from the group
    //
    QList<QAbstractButton *> btns = _fidelityButtons->buttons();
    for (int i = 0; i < btns.size(); i++) { _fidelityButtons->removeButton(btns[i]); }

    // Remove and delete buttons from the layout
    //
    QHBoxLayout *hlay = (QHBoxLayout *)fidelityBox->layout();
    QLayoutItem *child;
    while ((child = hlay->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }

    int numButtons = _fidelityLodStrs.size();
    for (int i = 0; i < numButtons; i++) {
        QRadioButton *rd = new QRadioButton();
        hlay->addWidget(rd);

        _fidelityButtons->addButton(rd, i);
        QString qs = "Refinement " + QString::fromStdString(_fidelityMultiresStrs[i]) + "\nLOD " + QString::fromStdString(_fidelityLodStrs[i]);

        rd->setToolTip(qs);

        if (lod == _fidelityLodIdx[i] && refLevel == _fidelityMultiresIdx[i]) { rd->setChecked(true); }
    }
    _fidelityButtons->blockSignals(false);
}

std::string FidelityWidget::GetCurrentLodString() const { return _currentLodStr; }

std::string FidelityWidget::GetCurrentMultiresString() const { return _currentMultiresStr; }
