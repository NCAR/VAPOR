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
#include "vapor/glutil.h" // Must be included first!!!
#include "Statistics.h"
#include "GUIStateParams.h"

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

// Class Statistics
//
Statistics::Statistics(QWidget* parent) : QDialog(parent), Ui_StatsWindow()
{
    _errMsg = NULL;
    _controlExec = NULL;
    
    setupUi(this);
    setWindowTitle("Statistics");
    //adjustTables();
    //VariablesTable->installEventFilter(this); //for responding to keyboard?

    Connect();
}

Statistics::~Statistics() 
{
    if (_errMsg) 
    {
        delete _errMsg;
        _errMsg = NULL;
    }
}



bool Statistics::Update() 
{
    // Initialize pointers
    VAPoR::DataStatus* dataStatus = _controlExec->getDataStatus();
    std::vector<std::string> dmNames = dataStatus->GetDataMgrNames();
    assert( dmNames.size() > 0 );
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string currentDatasetName = guiParams->GetStatsDatasetName();
    assert( currentDatasetName != "" );
    VAPoR::DataMgr* currentDmgr = dataStatus->GetDataMgr( currentDatasetName );
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>(_controlExec->GetParamsMgr()->
                      GetAppRenderParams(currentDatasetName, StatisticsParams::GetClassType()));

    // Update DataMgrCombo 
    DataMgrCombo->clear();
    int currentIdx = -1;
    for (int i=0; i<dmNames.size(); i++) 
    {
        QString item = QString::fromStdString(dmNames[i]);
        DataMgrCombo->addItem(item);
        if( dmNames[i] == currentDatasetName )
            currentIdx = i;
    }
    assert( currentIdx != -1 );
    DataMgrCombo->setCurrentIndex( currentIdx );

    // Update Timesteps
    int minTS = statsParams->GetMinTS();
    MinTimestepSpinbox->blockSignals(true);
    MinTimestepSpinbox->setValue( minTS );
    MinTimestepSpinbox->blockSignals(false);
    int maxTS = statsParams->GetMaxTS();
    MaxTimestepSpinbox->blockSignals(true);
    MaxTimestepSpinbox->setValue( maxTS );
    MaxTimestepSpinbox->blockSignals(false);

    // Update auto-update checkbox
    bool autoUpdate = statsParams->GetAutoUpdate();
    UpdateCheckbox->blockSignals(true);
    if (autoUpdate) 
        UpdateCheckbox->setCheckState(Qt::Checked);
    else 
        UpdateCheckbox->setCheckState(Qt::Unchecked);
    UpdateCheckbox->blockSignals(false);

    // Update available variables
    std::vector<std::string> availVars   = currentDmgr->GetDataVarNames(3, true);
    std::vector<std::string> availVars2D = currentDmgr->GetDataVarNames(2, true);
    for( int i = 0; i < availVars2D.size(); i++ )
        availVars.push_back( availVars2D[i] );
    sort( availVars.begin(), availVars.end());
    for(std::vector<std::string>::iterator it = availVars.begin(); it != availVars.end(); ++it)
    {
        NewVarCombo->addItem(QString::fromStdString(*it));
    }
    NewVarCombo->setCurrentIndex( 0 );
    RemoveVarCombo->setCurrentIndex( 0 );

    // Update statistics to calculate
    if( statsParams->GetMinEnabled() )
    {
        if( RemoveStatCombo->findText(QString::fromAscii("Min")) == -1 )
            RemoveStatCombo->addItem( QString::fromAscii("Min") );
    }
    else
    {
        if( AddStatCombo->findText(QString::fromAscii("Min")) == -1 )
            AddStatCombo->addItem( QString::fromAscii("Min") );
    }
    if( statsParams->GetMaxEnabled() )
    {
        if( RemoveStatCombo->findText(QString::fromAscii("Max")) == -1 )
            RemoveStatCombo->addItem( QString::fromAscii("Max") );
    }
    else
    {
        if( AddStatCombo->findText(QString::fromAscii("Max")) == -1 )
            AddStatCombo->addItem( QString::fromAscii("Max") );
    }
    if( statsParams->GetMeanEnabled() )
    {
        if( RemoveStatCombo->findText(QString::fromAscii("Mean")) == -1 )
            RemoveStatCombo->addItem( QString::fromAscii("Mean") );
    }
    else
    {
        if( AddStatCombo->findText(QString::fromAscii("Mean")) == -1 )
            AddStatCombo->addItem( QString::fromAscii("Mean") );
    }
    if( statsParams->GetMedianEnabled() )
    {
        if( RemoveStatCombo->findText(QString::fromAscii("Median")) == -1 )
            RemoveStatCombo->addItem( QString::fromAscii("Median") );
    }
    else
    {
        if( AddStatCombo->findText(QString::fromAscii("Median")) == -1 )
            AddStatCombo->addItem( QString::fromAscii("Median") );
    }
    if( statsParams->GetStdDevEnabled() )
    {
        if( RemoveStatCombo->findText(QString::fromAscii("StdDev")) == -1 )
            RemoveStatCombo->addItem( QString::fromAscii("StdDev") );
    }
    else
    {
        if( AddStatCombo->findText(QString::fromAscii("StdDev")) == -1 )
            AddStatCombo->addItem( QString::fromAscii("StdDev") );
    }
    AddStatCombo->setCurrentIndex(0);
    RemoveStatCombo->setCurrentIndex(0);

    // Update Statistics table
    QStringList header;
    header << "Variable";
    if( statsParams->GetMinEnabled() )
        header << "Min";
    if( statsParams->GetMaxEnabled() )
        header << "Max";
    if( statsParams->GetMeanEnabled() )
        header << "Mean";
    if( statsParams->GetMedianEnabled() )
        header << "Median";
    if( statsParams->GetStdDevEnabled() )
        header << "StdDev";
    VariablesTable->setColumnCount( header.size() );
    VariablesTable->setHorizontalHeaderLabels( header );
    VariablesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);


    return true;
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

    _cRatio = _params->GetCRatio();
    CRatioCombo->blockSignals(true);
    CRatioCombo->setCurrentIndex(_cRatio);
    CRatioCombo->blockSignals(false);
    _refLevel = _params->GetRefinement();
    RefCombo->blockSignals(true);
    RefCombo->setCurrentIndex(_refLevel);
    RefCombo->blockSignals(false);

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

int Statistics::initControlExec(ControlExec* ce) 
{
    if (ce!=NULL) 
    {
        _controlExec = ce;
    }
    else 
    { 
        return -1;
    }

    // Store the actuve dataset name 
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    if( dsName == "" )      // not initialized yet
    {
        VAPoR::DataStatus* dataStatus = _controlExec->getDataStatus();
        std::vector<std::string> dmNames = dataStatus->GetDataMgrNames();
        assert( dmNames.size() > 0 );
        guiParams->SetStatsDatasetName( dmNames[0] );
    }
    dsName = guiParams->GetStatsDatasetName();

    //this->Update( params );

    return 0;
}

bool Statistics::Connect()
{
    connect( NewVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_newVarChanged(int)) );
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
    connect(AddStatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(addStatistic(int)));
    connect(RemoveStatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(removeStatistic(int)));
    
    _initialized = 1;
    
#endif
    return true;
}

void Statistics::_newVarChanged( int index )
{
    if (index == 0) 
        return;

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    // Add this variable to "Remove a Variable" list
    RemoveVarCombo->addItem( NewVarCombo->itemText(index) );
    std::string varName = NewVarCombo->itemText(index).toStdString();

    // Remove this variable from "Add a Variable" list
    NewVarCombo->blockSignals( true );
    NewVarCombo->setCurrentIndex( 0 );
    NewVarCombo->blockSignals( false );
    NewVarCombo->removeItem( index );

    // Add a variable to parameter 
    std::vector<std::string> vars = statsParams->GetAuxVariableNames();
    vars.push_back( varName );
    statsParams->SetAuxVariableNames( vars );
    std::vector<std::string> rt = statsParams->GetAuxVariableNames();


#if 0
    vector<string> varNames = _params->GetVarNames();
    varNames.push_back(varName);
    _params->SetVarNames(varNames);

    _stats[varName] = _statistics();

    int rowCount = VariablesTable->rowCount();
    VariablesTable->insertRow(rowCount);
    VariablesTable->setVerticalHeaderItem(rowCount, new QTableWidgetItem(QString::fromStdString(varName)));

    QHeaderView *verticalHeader = VariablesTable->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(20);

    int colCount = VariablesTable->columnCount();
    for (int j=0; j<colCount; j++){
        QTableWidgetItem* twi = new QTableWidgetItem("");
        twi->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        VariablesTable->setItem(rowCount,j,twi);
    }

    NewVarCombo->setCurrentIndex(0);

    RemoveVarCombo->addItem(QString::fromStdString(varName));
    VariablesTable->resizeRowsToContents();

    if (_autoUpdate) {
            updateStats();
    }
    else (makeItRed());
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


// ValidStats class
//
int Statistics::ValidStats::_getVarIdx( std::string varName )
{
    int idx = -1;
    for( int i = 0; i < _variables.size(); i++ )
    {
        if( _variables[i] == varName )
        {
            idx = i;
            break;
        }
    }
    return idx;
}

bool Statistics::ValidStats::AddVariable( std::string newVar )
{
    if( _getVarIdx( newVar ) != -1 )     // this variable already exists.
        return false;

    _variables.push_back( newVar );
    for( int i = 0; i < 5; i++ )
    {
        _values[i].push_back( std::nan("1") );
        assert( _values[i].size() == _variables.size() );
    }
    return true;
}

bool Statistics::ValidStats::Add3MStats( std::string varName, const double* input3M )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    for( int i = 0; i < 3; i++ )
    {
        _values[i][idx] = input3M[i];
    }
    return true;
}

bool Statistics::ValidStats::AddMedian( std::string varName, double inputMedian )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    _values[3][idx] = inputMedian;
    return true;
}

bool Statistics::ValidStats::AddStddev( std::string varName, double inputStddev )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    _values[4][idx] = inputStddev;
    return true;
}

bool Statistics::ValidStats::Get3MStats( std::string varName, double* output3M )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    for( int i = 0; i < 3; i++ )
    {
        output3M[i] = _values[i][idx];
    }
    return true;
}

bool Statistics::ValidStats::GetMedian( std::string varName, double* outputMedian )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    *outputMedian = _values[3][idx];
    return true;
}

bool Statistics::ValidStats::GetStddev( std::string varName, double* outputStddev )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    *outputStddev = _values[4][idx];
    return true;
}

bool Statistics::ValidStats::InvalidAll()
{
    for( int i = 0; i < 5; i++ )
        for( int j = 0; j < _values[i].size(); j++ )
            _values[i][j] = std::nan("1");
    return true;
}

