//                                                                    *
//         Copyright (C)  2016                                      *
//   University Corporation for Atmospheric Research                  *
//         All Rights Reserved                                      *
//                                                                    *
//************************************************************************/
//
//  File:      Statistics.cpp
//
//  Author:  Scott Pearse
//        National Center for Atmospheric Research
//        PO 3000, Boulder, Colorado
//
//  Date:      August 2016
//
//  Description:    Implements the Statistics class.
//
#ifdef WIN32
    #pragma warning(disable : 4100)
#endif
#include "vapor/glutil.h"    // Must be included first!!!
#include "Statistics.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <vapor/MyBase.h>

using namespace Wasp;
using namespace VAPoR;
using namespace std;

// ValidStats class
//
int Statistics::ValidStats::getVarIdx(std::string varName)
{
    int idx = -1;
    for (int i = 0; i < _variables.size(); i++) {
        if (_variables[i] == varName) {
            idx = i;
            break;
        }
    }
    return idx;
}

bool Statistics::ValidStats::addVariable(std::string newVar)
{
    if (getVarIdx(newVar) != -1)    // this variable already exists.
        return false;

    _variables.push_back(newVar);
    for (int i = 0; i < 5; i++) {
        _values[i].push_back(std::nan("1"));
        assert(_values[i].size() == _variables.size());
    }
    return true;
}

bool Statistics::ValidStats::add3MStats(std::string varName, const double *input3M)
{
    int idx = getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    for (int i = 0; i < 3; i++) { _values[i][idx] = input3M[i]; }
    return true;
}

bool Statistics::ValidStats::addMedian(std::string varName, double inputMedian)
{
    int idx = getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    _values[3][idx] = inputMedian;
    return true;
}

bool Statistics::ValidStats::addStddev(std::string varName, double inputStddev)
{
    int idx = getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    _values[4][idx] = inputStddev;
    return true;
}

bool Statistics::ValidStats::get3MStats(std::string varName, double *output3M)
{
    int idx = getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    for (int i = 0; i < 3; i++) { output3M[i] = _values[i][idx]; }
    return true;
}

bool Statistics::ValidStats::getMedian(std::string varName, double *outputMedian)
{
    int idx = getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    *outputMedian = _values[3][idx];
    return true;
}

bool Statistics::ValidStats::getStddev(std::string varName, double *outputStddev)
{
    int idx = getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    *outputStddev = _values[4][idx];
    return true;
}

bool Statistics::ValidStats::invalidAll()
{
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < _values[i].size(); j++) _values[i][j] = std::nan("1");
    return true;
}

// Class Statistics
//
Statistics::Statistics(QWidget *parent) : QDialog(parent), Ui_StatsWindow()
{
    _errMsg = NULL;
    _controlExec = NULL;
    _dataStatus = NULL;
    _rGrid = NULL;

    setupUi(this);
    setWindowTitle("Statistics");
    // adjustTables();
    // VariablesTable->installEventFilter(this); //for responding to keyboard?
}

Statistics::~Statistics()
{
    if (_errMsg) {
        delete _errMsg;
        _errMsg = NULL;
    }
}

void Statistics::Update(VAPoR::StatisticsParams *sParams)
{
#if 0
    _params = sParams;

    vector<double> minExts, maxExts;
    minExts = _params->GetMinExtents();
    maxExts = _params->GetMaxExtents();

    _xRange->blockSignals(true);
    _yRange->blockSignals(true);
    _zRange->blockSignals(true);
    _xRange->setUserMin(minExts[0]);
    _yRange->setUserMin(minExts[1]);
    _zRange->setUserMin(minExts[2]);
    _xRange->setUserMax(maxExts[0]);
    _yRange->setUserMax(maxExts[1]);
    _zRange->setUserMax(maxExts[2]);
    _xRange->blockSignals(false);
    _yRange->blockSignals(false);
    _zRange->blockSignals(false);

    _minTS = _params->GetMinTS();
    MinTimestepSpinbox->blockSignals(true);
    MinTimestepSpinbox->setValue(_minTS);
    MinTimestepSpinbox->blockSignals(false);
    _maxTS = _params->GetMaxTS();
    MaxTimestepSpinbox->blockSignals(true);
    MaxTimestepSpinbox->setValue(_maxTS);
    MaxTimestepSpinbox->blockSignals(false);

    _cRatio = _params->GetCRatio();
    CRatioCombo->blockSignals(true);
    CRatioCombo->setCurrentIndex(_cRatio);
    CRatioCombo->blockSignals(false);
    _refLevel = _params->GetRefinement();
    RefCombo->blockSignals(true);
    RefCombo->setCurrentIndex(_refLevel);
    RefCombo->blockSignals(false);

    _autoUpdate = _params->GetAutoUpdate();
    UpdateCheckbox->blockSignals(true);
    if (!_autoUpdate) 
        UpdateCheckbox->setCheckState(Qt::Unchecked);
    else 
        UpdateCheckbox->setCheckState(Qt::Checked);
    UpdateCheckbox->blockSignals(false);

    updateStatisticSelection();
    updateVariables();
#endif
}

void Statistics::showMe()
{
    show();
    raise();
    activateWindow();
}

int Statistics::initControlExec(ControlExec *ce)
{
#if 0
    if (ce!=NULL) 
    {
        _controlExec = ce;
    }
    else 
    { 
        return -1;
    }

    _dataStatus = _controlExec->getDataStatus();
    vector<string> dmNames = _dataStatus->GetDataMgrNames();
    assert( dmNames.size() > 0 )

    dataMgrCombo->clear();
    for (int i=0; i<dmNames.size(); i++) 
    {
        QString item = QString::fromStdString(dmNames[i]);
        dataMgrCombo->addItem(item);
    }
    dataMgrCombo->setCurrentIndex( 0 );

    _dmgr = _dataStatus->GetDataMgr(dmNames[0]);
    assert( _dmgr );

    initialize();

    return 0;
#endif
}

int Statistics::initialize()
{
#if 0
    // This is a bitmask to define which statistics to calculate/display.
    // If a statistic variable is set to 0x00 or undefined, it will not
    // be applied.  The _calculations variable is used as a filter, and
    // is all-inclusive by default.
    //
    StatisticsParams* params = ParamsMgr::GetAppRenderParams(
                    dataMgrCombo->currentText.toStdString(), StatisticParams::GetClassType());
    /*
    _MIN = 0x01;
    _MAX = 0x02;
    _MEAN = 0x04;
    _MEDIAN = 0x00;
    if (_params->GetMedianStat()) {
        _MEDIAN = 0x10;
    }
    _SIGMA = 0x00;
    if (_params->GetStdDevStat()) {
        _SIGMA = 0x08;
    }
    _calculations = 0xFF;
    */

    //if (_dm==NULL) return -1;

    _errMsg = new sErrMsg;
    //_autoUpdate = _params->GetAutoUpdate();
    UpdateCheckbox->setChecked( params->GetAutoUpdate() );

    // for _regionSelection,
    // 0 = center/size, 1 = min/max, 2 = center/size
    //
    //_regionSelection = 0;
    #if 0
    _regionSelection = _params->GetRegionSelection();
    #endif
    //stackedSliderWidget->setCurrentIndex(_regionSelection);

    generateTableColumns();


    initVariables();
    _defaultVar = _vars3d[0];
    if (_defaultVar=="") {
        return -1;
    }

    initTimes();

    initCRatios();
    
    initRefinement();

    initRegion();

    initRanges();

    retrieveRangeParams();
    _regionInitialized = 1;

    ExportButton->setEnabled(true);
    
    if (_initialized==1) return 0;

    connect(MinTimestepSpinbox, SIGNAL(valueChanged(int)), this, SLOT(minTSChanged()));
    connect(MaxTimestepSpinbox, SIGNAL(valueChanged(int)), this, SLOT(maxTSChanged()));
    connect(UpdateCheckbox, SIGNAL(stateChanged(int)), this, SLOT(autoUpdateClicked()));
    connect(UpdateButton, SIGNAL(pressed()), this, SLOT(updateButtonPressed()));
    connect(RefCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(refinementChanged(int)));
    connect(CRatioCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(cRatioChanged(int)));
    connect(NewVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(addVariable(int)));
    connect(RestoreExtentsButton, SIGNAL(pressed()), this, SLOT(restoreExtents()));
    connect(RemoveVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(removeVariable(int)));
    connect(ExportButton, SIGNAL(clicked()), this, SLOT(exportText()));
    //connect(regionSelectorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(rangeComboChanged()));
    //connect(copyActiveRegionButton, SIGNAL(pressed()), this, SLOT(copyActiveRegion()));
    connect(addStatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(addStatistic(int)));
    connect(removeStatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(removeStatistic(int)));
    
    _initialized = 1;
    
    return 0;
#endif
}

#if 0
void Statistics::errReport(string msg) const {
    _errMsg->errorList->setText(QString::fromStdString(msg));
    _errMsg->show();
    _errMsg->raise();
    _errMsg->activateWindow();
}
#endif

#if 0
void Statistics::initTimes() 
{
    MinTimestepSpinbox->setMinimum(0);
    MinTimestepSpinbox->setMaximum(_dm->GetNumTimeSteps(_defaultVar)-1);
    
    ParamsMgr* pMgr = _controlExec->GetParamsMgr();
    pMgr->BeginSaveStateGroup("Initializing statistics time spin boxes");
    _minTS = _params->GetMinTS();
    MinTimestepSpinbox->blockSignals(true);
    MaxTimestepSpinbox->blockSignals(true);
    MinTimestepSpinbox->setValue(_minTS);

    MaxTimestepSpinbox->setMinimum(0);
    MaxTimestepSpinbox->setMaximum(_dm->GetNumTimeSteps(_defaultVar)-1);
    _maxTS = _params->GetMaxTS();   
    MaxTimestepSpinbox->setValue(_maxTS);
    MinTimestepSpinbox->blockSignals(false);
    MaxTimestepSpinbox->blockSignals(false);
    pMgr->EndSaveStateGroup();
}
#endif

#if 0
void Statistics::initRanges() 
{
}
#endif

#if 0
void Statistics::initCRatios() 
{
    _cRatios = _dm->GetCRatios(_defaultVar);

    _cRatio = _params->GetCRatio();
    if (_cRatio == -1) {
        _cRatio = _cRatios.size()-1;
    }

    for (std::vector<size_t>::iterator it = _cRatios.begin(); it != _cRatios.end(); ++it){
        CRatioCombo->addItem("1:"+QString::number(*it));
    }

    CRatioCombo->setCurrentIndex(_cRatio);
}
#endif

#if 0
void Statistics::initRefinement() 
{
    _refLevel = _params->GetRefinement();
    _refLevels = _dm->GetNumRefLevels(_defaultVar);

    for (int i=0; i<=_refLevels; i++){
        RefCombo->addItem(QString::number(i));
    }
    RefCombo->setCurrentIndex(_refLevel);
}
#endif
