//                                                                    *
//         Copyright (C)  2016                                      *
//   University Corporation for Atmospheric Research                  *
//         All Rights Reserved                                      *
//                                                                    *
//************************************************************************/
//
//  File:      Statistics.cpp
//
//  Author:  Samuel Li
//        National Center for Atmospheric Research
//        PO 3000, Boulder, Colorado
//
//  Date:      November 2017
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
    MyGeometryWidget->Reinit((GeometryWidget::Flags) (GeometryWidget::THREED) );
    MyGeometryWidget->SetUseAuxVariables( true );

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
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string currentDatasetName = guiParams->GetStatsDatasetName();
    assert( currentDatasetName != "" );
    VAPoR::DataMgr* currentDmgr = dataStatus->GetDataMgr( currentDatasetName );
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>(_controlExec->GetParamsMgr()->
                      GetAppRenderParams(currentDatasetName, StatisticsParams::GetClassType()));

    // Update DataMgrCombo 
    std::vector<std::string> dmNames = dataStatus->GetDataMgrNames();
    assert( dmNames.size() > 0 );
    DataMgrCombo->blockSignals( true );
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
    DataMgrCombo->blockSignals( false );

    // Update auto-update checkbox
    bool autoUpdate = statsParams->GetAutoUpdateEnabled();
    UpdateCheckbox->blockSignals(true);
    if (autoUpdate) 
        UpdateCheckbox->setCheckState(Qt::Checked);
    else 
        UpdateCheckbox->setCheckState(Qt::Unchecked);
    UpdateCheckbox->blockSignals(false);

    // Update "Add a Variable"
    std::vector<std::string> availVars   = currentDmgr->GetDataVarNames(2, true);
    std::vector<std::string> availVars3D = currentDmgr->GetDataVarNames(3, true);
    for( int i = 0; i < availVars3D.size(); i++ )
        availVars.push_back( availVars3D[i] );
    // remove variables already enabled
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    for( int i = 0; i < enabledVars.size(); i++ )
        for( int rmIdx = 0; rmIdx < availVars.size(); rmIdx++ )
            if( availVars[ rmIdx ] == enabledVars[i] )
            {
                availVars.erase( availVars.begin() + rmIdx );
                break;
            }
    std::sort( availVars.begin(), availVars.end() );
    NewVarCombo->blockSignals( true );
    NewVarCombo->clear();
    NewVarCombo->addItem( QString::fromAscii("Add a Variable") );
    for(std::vector<std::string>::iterator it = availVars.begin(); it != availVars.end(); ++it)
    {
        NewVarCombo->addItem( QString::fromStdString(*it));
    }
    NewVarCombo->setCurrentIndex( 0 );
    NewVarCombo->blockSignals( false );

    // Update "Remove a Variable"
    assert( enabledVars.size() == _validStats.GetVariableCount() );
    std::sort( enabledVars.begin(), enabledVars.end() );
    RemoveVarCombo->blockSignals( true );
    RemoveVarCombo->clear();
    RemoveVarCombo->addItem( QString::fromAscii("Remove a Variable") );
    for( int i = 0; i < enabledVars.size(); i++ )
    {
        RemoveVarCombo->addItem( QString::fromStdString(enabledVars[i]));
    }
    RemoveVarCombo->setCurrentIndex( 0 );
    RemoveVarCombo->blockSignals( false );

    // Update Statistics table: header
    this->_updateStatsTable();

    // Update calculations
    NewCalcCombo->blockSignals( true );
    RemoveCalcCombo->blockSignals( true );
    NewCalcCombo->clear();
    RemoveCalcCombo->clear();
    NewCalcCombo->addItem(      QString::fromAscii("Add a Calculation") );
    RemoveCalcCombo->addItem(   QString::fromAscii("Remove a Calculation") );
    if( statsParams->GetMinEnabled() )
        RemoveCalcCombo->addItem( QString::fromAscii("Min") );
    else
        NewCalcCombo->addItem( QString::fromAscii("Min") );
    if( statsParams->GetMaxEnabled() )
        RemoveCalcCombo->addItem( QString::fromAscii("Max") );
    else
        NewCalcCombo->addItem( QString::fromAscii("Max") );
    if( statsParams->GetMeanEnabled() )
        RemoveCalcCombo->addItem( QString::fromAscii("Mean") );
    else
        NewCalcCombo->addItem( QString::fromAscii("Mean") );
    if( statsParams->GetMedianEnabled() )
        RemoveCalcCombo->addItem( QString::fromAscii("Median") );
    else
        NewCalcCombo->addItem( QString::fromAscii("Median") );
    if( statsParams->GetStdDevEnabled() )
        RemoveCalcCombo->addItem( QString::fromAscii("StdDev") );
    else
        NewCalcCombo->addItem( QString::fromAscii("StdDev") );
    NewCalcCombo->setCurrentIndex(0);
    RemoveCalcCombo->setCurrentIndex(0);
    NewCalcCombo->blockSignals( false );
    RemoveCalcCombo->blockSignals( false );

    // Update LOD, Refinement
    RefCombo->blockSignals( true );
    LODCombo->blockSignals( true );
    RefCombo->clear();
    LODCombo->clear();

    int numRefLevels = currentDmgr->GetNumRefLevels( availVars[0] );
    vector<size_t> availLODs = currentDmgr->GetCRatios( availVars[0] );
    // sanity check on enabledVars.
    for( int i = 1; i < enabledVars.size(); i++ )   
    {
        assert( numRefLevels == currentDmgr->GetNumRefLevels( enabledVars[i] ));
        assert( availLODs.size() == currentDmgr->GetCRatios(  enabledVars[i] ).size() );
    }

    std::string referenceVar;
    if( availVars3D.size() > 0 )
        referenceVar = availVars3D[0];
    else
        referenceVar = availVars[0];

    // add refinement levels
    std::vector<size_t> dims, blockSizes;
    for( int level = 0; level < numRefLevels; level++ )
    {
        currentDmgr->GetDimLensAtLevel( referenceVar, level, dims, blockSizes );
        QString line = QString::number( level );
        line += " (";
        for( int i = 0; i < dims.size(); i++ )
        {
            line += QString::number(dims[i]);
            line += "x";
        }
        line.remove( line.size()-1, 1 );
        line += ")";
        RefCombo->addItem( line );
    }
    RefCombo->setCurrentIndex( statsParams->GetRefinementLevel() );

    // add LOD levels
    for( int lod = 0; lod < availLODs.size(); lod++ )
    {
        QString line = QString::number( lod );
        line += " (";
        line += QString::number( availLODs[lod] );
        line += ":1)";
        LODCombo->addItem( line );
    }
    LODCombo->setCurrentIndex( statsParams->GetCompressionLevel() );

    RefCombo->blockSignals( false );
    LODCombo->blockSignals( false );

    // Update timesteps
    MinTimestepSpinbox->blockSignals( true );
    MinTimestepSpinbox->setMinimum(0);
    MinTimestepSpinbox->setMaximum( currentDmgr->GetNumTimeSteps( availVars[0] ) - 1);
    MinTimestepSpinbox->setValue( statsParams->GetCurrentMinTS() );
    MinTimestepSpinbox->blockSignals( false );

    MaxTimestepSpinbox->blockSignals( true );
    MaxTimestepSpinbox->setMinimum(0);
    MaxTimestepSpinbox->setMaximum( currentDmgr->GetNumTimeSteps( availVars[0] ) - 1);
    MaxTimestepSpinbox->setValue( statsParams->GetCurrentMaxTS() );
    MaxTimestepSpinbox->blockSignals( false );

    // Update geometry extents
    MyGeometryWidget->Update( _controlExec->GetParamsMgr(), currentDmgr, statsParams );

    return true;
}

void Statistics::_updateStatsTable()
{
    // Initialize pointers
    VAPoR::DataStatus* dataStatus = _controlExec->getDataStatus();
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string currentDatasetName = guiParams->GetStatsDatasetName();
    assert( currentDatasetName != "" );
    VAPoR::DataMgr* currentDmgr = dataStatus->GetDataMgr( currentDatasetName );
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>(_controlExec->GetParamsMgr()->
                      GetAppRenderParams(currentDatasetName, StatisticsParams::GetClassType()));

    // Update Statistics Table: header
    VariablesTable->clear();    // this also deletes the items properly.
    QStringList header;
    header << "Variable" << "No. of Samples";
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

    // Update Statistics Table: cells
    QBrush brush(QColor(255,0,0));
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    assert( enabledVars.size() == _validStats.GetVariableCount() );
    VariablesTable->setRowCount( enabledVars.size() );
    for( int row = 0; row < enabledVars.size(); row++ )
    {
        double m3[3], median, stddev;
        long   count;
        _validStats.Get3MStats( enabledVars[row], m3 );
        _validStats.GetMedian(  enabledVars[row], &median );
        _validStats.GetStddev(  enabledVars[row], &stddev );
        _validStats.GetCount(   enabledVars[row], &count );
        
        VariablesTable->setItem( row, 0, new QTableWidgetItem( QString::fromStdString(enabledVars[row]) ));
        if( count == -1 )
        {
            VariablesTable->setItem( row, 1, new QTableWidgetItem(QString::fromAscii("??")));
            VariablesTable->item(    row, 1)->setForeground( brush );
        }
        else 
            VariablesTable->setItem( row, 1, new QTableWidgetItem( QString::number(count) ));

        int column = 2;
        if( statsParams->GetMinEnabled() )
        {
            if( !std::isnan( m3[0] ) )
            {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[0], 'g', 3)));
            }
            else
            {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground( brush );
            }
            column++;
        }
        if( statsParams->GetMaxEnabled() )
        {
            if( !std::isnan( m3[1] ) )
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[1], 'g', 3)));
            else
            {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground( brush );
            }
            column++;
        }
        if( statsParams->GetMeanEnabled() )
        {
            if( !std::isnan( m3[2] ) )
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[2], 'g', 3)));
            else
            {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground( brush );
            }
            column++;
        }
        if( statsParams->GetMedianEnabled() )
        {
            if( !std::isnan( median ) )
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(median, 'g', 3)));
            else
            {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground( brush );
            }
            column++;
        }
        if( statsParams->GetStdDevEnabled() )
        {
            if( !std::isnan( stddev ) )
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(stddev, 'g', 3)));
            else
            {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground( brush );
            }
            column++;
        }
    }
    for( int r = 0; r < VariablesTable->rowCount(); r++ )
        for( int c = 0; c < VariablesTable->columnCount(); c++ )
            VariablesTable->item(r, c)->setFlags(Qt::NoItemFlags);

    VariablesTable->update();
    VariablesTable->repaint();
    VariablesTable->viewport()->update();
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
        _controlExec = ce;
    else 
        return -1;

    // Store the active dataset name 
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    if( dsName == "" )      // not initialized yet
    {
        std::vector<std::string> dmNames = _controlExec->getDataStatus()->GetDataMgrNames();
        assert( dmNames.size() > 0 );
        guiParams->SetStatsDatasetName( dmNames[0] );
    }
    dsName = guiParams->GetStatsDatasetName();
    _validStats.SetDatasetName( dsName );

    return 0;
}

bool Statistics::Connect()
{
    connect( NewVarCombo,       SIGNAL(currentIndexChanged(int)), this, SLOT(_newVarChanged(int)) );
    connect( RemoveVarCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(_removeVarChanged(int)) );
    connect( NewCalcCombo,      SIGNAL(currentIndexChanged(int)), this, SLOT(_newCalcChanged(int)) );
    connect( RemoveCalcCombo,   SIGNAL(currentIndexChanged(int)), this, SLOT(_removeCalcChanged(int)) );
    connect( RefCombo,          SIGNAL(currentIndexChanged(int)), this, SLOT(_refinementChanged(int)) );
    connect( LODCombo,          SIGNAL(currentIndexChanged(int)), this, SLOT(_lodChanged(int)) );
    connect( MinTimestepSpinbox,       SIGNAL(valueChanged(int)), this, SLOT(_minTSChanged(int)) );
    connect( MaxTimestepSpinbox,       SIGNAL(valueChanged(int)), this, SLOT(_maxTSChanged(int)) );
    connect( UpdateButton,      SIGNAL(clicked()),                this, SLOT(_updateButtonClicked()));
    connect( MyGeometryWidget,  SIGNAL(valueChanged()),           this, SLOT(_geometryValueChanged()));
    connect( DataMgrCombo,      SIGNAL(currentIndexChanged(int)), this, SLOT(_dataSourceChanged(int)) );
    connect( UpdateCheckbox,    SIGNAL(stateChanged(int)),        this, SLOT(_autoUpdateClicked(int)));
    connect( ExportButton,      SIGNAL(clicked()),                this, SLOT(_exportTextClicked()));
    return true;
}

void Statistics::_autoUpdateClicked( int state )
{
    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    if( state == 0 )        // unchecked
        statsParams->SetAutoUpdateEnabled( false );
    else if( state == 2 )   // checked
        statsParams->SetAutoUpdateEnabled( true );
    else
    {
        std::cerr << "Dont know what this state is!!!" << std::endl;
        // REPORT ERROR!!!
    }
}

void Statistics::_dataSourceChanged( int index )
{
    std::string newDataSourceName = DataMgrCombo->itemText(index).toStdString();

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>(_controlExec->GetParamsMgr()->
                    GetAppRenderParams( newDataSourceName, StatisticsParams::GetClassType()));

    guiParams->SetStatsDatasetName( newDataSourceName );
    _validStats.SetDatasetName( newDataSourceName );

    // add variables to _validStats if there are any
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    for( int i = 0; i < enabledVars.size(); i++ )
        _validStats.AddVariable( enabledVars[i] );
}

void Statistics::_geometryValueChanged()
{
    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    _validStats.InvalidAll();

    // Auto-update if enabled
    if( statsParams->GetAutoUpdateEnabled() )
        _updateButtonClicked();
}

void Statistics::_updateButtonClicked()
{
    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    for( int i = 0; i < _validStats.GetVariableCount(); i++ )
    {
        std::string varname = _validStats.GetVariableName(i);
        double m3[3], median, stddev;
        _validStats.Get3MStats( varname, m3 );
        _validStats.GetMedian ( varname, &median );
        _validStats.GetStddev ( varname, &stddev );
        if( ( statsParams->GetMinEnabled() || 
              statsParams->GetMaxEnabled() ||
              statsParams->GetMinEnabled()    )  && std::isnan(m3[2]) )
        {
            _calc3M( varname );
            _updateStatsTable();
        }
        if( statsParams->GetMedianEnabled() && std::isnan( median ) )
        {
            _calcMedian( varname );
            _updateStatsTable();
        }
        if( statsParams->GetStdDevEnabled() && std::isnan( stddev ) )
        {
            _calcStddev( varname );
            _updateStatsTable();
        }
    }
}

void Statistics::_minTSChanged( int val )
{
    assert( val >= 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    // Add this minTS to parameter if different
    if( val != statsParams->GetCurrentMinTS() )
    {
        statsParams->SetCurrentMinTS( val );
        _validStats.InvalidAll();

        if( val > statsParams->GetCurrentMaxTS() )
        {
            statsParams->SetCurrentMaxTS( val );
            MaxTimestepSpinbox->blockSignals( true );
            MaxTimestepSpinbox->setValue( val );
            MaxTimestepSpinbox->blockSignals( false );
        }
    }

    // Auto-update if enabled
    if( statsParams->GetAutoUpdateEnabled() )
        _updateButtonClicked();
}

void Statistics::_maxTSChanged( int val )
{
    assert( val >= 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    // Add this maxTS to parameter if different
    if( val != statsParams->GetCurrentMaxTS() )
    {
        statsParams->SetCurrentMaxTS( val );
        _validStats.InvalidAll();

        if( val < statsParams->GetCurrentMinTS() )
        {
            statsParams->SetCurrentMinTS( val );
            MinTimestepSpinbox->blockSignals( true );
            MinTimestepSpinbox->setValue( val );
            MinTimestepSpinbox->blockSignals( false );
        }
    }

    // Auto-update if enabled
    if( statsParams->GetAutoUpdateEnabled() )
        _updateButtonClicked();
}

void Statistics::_lodChanged( int index )
{
    assert( index >= 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    int lod = index;

    // Add this lod level to parameter if different
    if( lod != statsParams->GetCompressionLevel() )
    {
        statsParams->SetCompressionLevel( lod );
        _validStats.InvalidAll();
    }

    // Auto-update if enabled
    if( statsParams->GetAutoUpdateEnabled() )
        _updateButtonClicked();
}

void Statistics::_refinementChanged( int index )
{
    assert( index >= 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    int refLevel = index; 

    // Add this refinement level to parameter if different
    if( refLevel != statsParams->GetRefinementLevel() )
    {
        statsParams->SetRefinementLevel( refLevel );
        _validStats.InvalidAll();
    }

    // Auto-update if enabled
    if( statsParams->GetAutoUpdateEnabled() )
        _updateButtonClicked();
}

void Statistics::_newCalcChanged( int index )
{
    assert( index > 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string calcName = NewCalcCombo->itemText(index).toStdString();

    // Add this calculation to parameter 
    if( calcName == "Min" )
        statsParams->SetMinEnabled( true );
    else if( calcName == "Max" )
        statsParams->SetMaxEnabled( true );
    else if( calcName == "Mean" )
        statsParams->SetMeanEnabled( true );
    else if( calcName == "Median" )
        statsParams->SetMedianEnabled( true );
    else if( calcName == "StdDev" )
        statsParams->SetStdDevEnabled( true );
    else
    {
        // REPORT ERROR!!
    }

    // Auto-update if enabled
    if( statsParams->GetAutoUpdateEnabled() )
        _updateButtonClicked();
}

void Statistics::_removeCalcChanged( int index )
{
    assert( index > 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string calcName = RemoveCalcCombo->itemText(index).toStdString();

    // Remove this calculation from parameter 
    if( calcName == "Min" )
        statsParams->SetMinEnabled( false );
    else if( calcName == "Max" )
        statsParams->SetMaxEnabled( false );
    else if( calcName == "Mean" )
        statsParams->SetMeanEnabled( false );
    else if( calcName == "Median" )
        statsParams->SetMedianEnabled( false );
    else if( calcName == "StdDev" )
        statsParams->SetStdDevEnabled( false );
    else
    {
        // REPORT ERROR!!
    }
}

void Statistics::_newVarChanged( int index )
{
    assert( index > 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string varName = NewVarCombo->itemText(index).toStdString();

    // Add this variable to parameter 
    std::vector<std::string> vars = statsParams->GetAuxVariableNames();
    vars.push_back( varName );
    statsParams->SetAuxVariableNames( vars );

    // Add this variable to _validStats
    _validStats.AddVariable( varName );

    // Auto-update if enabled
    if( statsParams->GetAutoUpdateEnabled() )
        _updateButtonClicked();
}

void Statistics::_removeVarChanged( int index )
{
    assert( index > 0 );

    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string varName = RemoveVarCombo->itemText(index).toStdString();

    // Remove this variable from parameter 
    std::vector<std::string> vars = statsParams->GetAuxVariableNames();
    int rmIdx = -1;
    for( int i = 0; i < vars.size(); i++ )
        if( vars[i] == varName )
        {
            rmIdx = i;
            break;
        }
    assert( rmIdx != -1 );
    vars.erase( vars.begin() + rmIdx );
    statsParams->SetAuxVariableNames( vars );

    // Remove this variable from _validStats
    _validStats.RemoveVariable( varName );
}

bool Statistics::_calc3M( std::string varname )
{
    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    VAPoR::DataMgr* currentDmgr = _controlExec->getDataStatus()->GetDataMgr( dsName );

    int minTS = statsParams->GetCurrentMinTS();
    int maxTS = statsParams->GetCurrentMaxTS();
    if( !currentDmgr->IsTimeVarying( varname ) )
        maxTS = minTS;
    std::vector<double> minExtent, maxExtent;
    statsParams->GetBox()->GetExtents( minExtent, maxExtent );

    double c = 0.0;
    double sum = 0.0;
    double min = std::numeric_limits<double>::max();
    double max = -min;
    long   count = 0;

    for( int ts = minTS; ts <= maxTS; ts++ )
    {
        VAPoR::Grid* grid = currentDmgr->GetVariable( ts, varname, 
                statsParams->GetRefinementLevel(), statsParams->GetCompressionLevel(),
                minExtent, maxExtent );
        Grid::ConstIterator endItr  = grid->cend(); 
        float missingVal            = grid->GetMissingValue();

        for( Grid::ConstIterator it = grid->cbegin(minExtent, maxExtent); it != endItr; ++it )
        {
            if( *it != missingVal )
            {
                double val = std::abs((double)(*it)) < 1e-38 ? 0.0 : *it; 
                min = min < val ? min : val;
                max = max > val ? max : val;
                double y = val - c;
                double t = sum + y;
                       c = t - sum - y;
                     sum = t; 
                count++;
            }
        }
    }
    
    if( count > 0 )
    {
        double m3[3] = {min, max, sum/(double)count};
        _validStats.Add3MStats( varname, m3 );
    }
    else    // count == 0
    {
        //std::cerr << "Error: Zero value got selected!!" << std::endl;
    }

    _validStats.AddCount( varname, count );

    return true;
}

bool Statistics::_calcMedian( std::string varname )
{
    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    VAPoR::DataMgr* currentDmgr = _controlExec->getDataStatus()->GetDataMgr( dsName );

    int minTS = statsParams->GetCurrentMinTS();
    int maxTS = statsParams->GetCurrentMaxTS();
    if( !currentDmgr->IsTimeVarying( varname ) )
        maxTS = minTS;
    std::vector<double> minExtent, maxExtent;
    statsParams->GetBox()->GetExtents( minExtent, maxExtent );

    std::vector<float> buffer;
    for( int ts = minTS; ts <= maxTS; ts++ )
    {
        VAPoR::Grid* grid = currentDmgr->GetVariable( ts, varname, 
                statsParams->GetRefinementLevel(), statsParams->GetCompressionLevel(),
                minExtent, maxExtent );
        Grid::ConstIterator endItr  = grid->cend(); 
        float missingVal            = grid->GetMissingValue();

        for( Grid::ConstIterator it = grid->cbegin(minExtent, maxExtent); it != endItr; ++it )
        {
            if( *it != missingVal )
                buffer.push_back( std::abs((double)(*it) < 1e-38 ? 0.0 : *it) );
        }
    }
    
    if( buffer.size() > 0 )
    {
        std::sort( buffer.begin(), buffer.end() );
        double median = buffer.at( buffer.size() / 2 );
        _validStats.AddMedian( varname, median );
    }
    else
    {
        //std::cerr << "Error: Zero value got selected!!" << std::endl;
    }
        
    _validStats.AddCount(  varname, buffer.size() );

    return true;
}

bool Statistics::_calcStddev( std::string varname )
{
    // Initialize pointers
    GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                    (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
    std::string dsName = guiParams->GetStatsDatasetName();
    StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
            (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    VAPoR::DataMgr* currentDmgr = _controlExec->getDataStatus()->GetDataMgr( dsName );

    int minTS = statsParams->GetCurrentMinTS();
    int maxTS = statsParams->GetCurrentMaxTS();
    if( !currentDmgr->IsTimeVarying( varname ) )
        maxTS = minTS;
    std::vector<double> minExtent, maxExtent;
    statsParams->GetBox()->GetExtents( minExtent, maxExtent );

    double c = 0.0;
    double sum = 0.0;
    long   count = 0;
    double m3[3];
    _validStats.Get3MStats( varname, m3 );
    if( std::isnan( m3[2] ) )
        this->_calc3M( varname );

    for( int ts = minTS; ts <= maxTS; ts++ )
    {
        VAPoR::Grid* grid = currentDmgr->GetVariable( ts, varname, 
                statsParams->GetRefinementLevel(), statsParams->GetCompressionLevel(),
                minExtent, maxExtent );
        Grid::ConstIterator endItr  = grid->cend(); 
        float missingVal            = grid->GetMissingValue();

        for( Grid::ConstIterator it = grid->cbegin(minExtent, maxExtent); it != endItr; ++it )
        {
            if( *it != missingVal )
            {
                double val = std::abs((double)(*it)) < 1e-38 ? 0.0 : *it; 
                double y = (val - m3[2]) * (val - m3[2]) - c;
                double t = sum + y;
                       c = t - sum - y;
                     sum = t; 
                count++;
            }
        }
    }
    
    if( count > 0 )
    {
        _validStats.AddStddev( varname, std::sqrt( sum / (double)count ));
    }
    else
    {
        //std::cerr << "Error: Zero value got selected!!" << std::endl;
    }
       
    _validStats.AddCount(  varname, count );

    return true;
}


// ValidStats class
//
int Statistics::ValidStats::_getVarIdx( std::string& varName )
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

bool Statistics::ValidStats::AddVariable( std::string& newVar )
{
    if( newVar == "" )
        return false;
    if( _getVarIdx( newVar ) != -1 )     // this variable already exists.
        return false;

    _variables.push_back( newVar );
    for( int i = 0; i < 5; i++ )
    {
        _values[i].push_back( std::nan("1") );
        assert( _values[i].size() == _variables.size() );
    }
    _count.push_back( -1 );
    if( _count.size() != _variables.size() )
        std::cerr << "_count.size() = " << _count.size() << ",  _variables.size() = " << _variables.size() << std::endl;
    assert( _count.size() == _variables.size() );
    return true;
}

bool Statistics::ValidStats::RemoveVariable( std::string& varname )
{
    int rmIdx = _getVarIdx( varname );
    if( rmIdx == -1 )                   // this variable doesn't exist.
        return false;

    _variables.erase( _variables.begin() + rmIdx );
    for( int i = 0; i < 5; i++ )
    {
        _values[i].erase( _values[i].begin() + rmIdx );
        assert( _values[i].size() == _variables.size() );
    }
    _count.erase( _count.begin() + rmIdx );
    assert( _count.size() == _variables.size() );
    return true;
}

bool Statistics::ValidStats::Add3MStats( std::string& varName, const double* input3M )
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

bool Statistics::ValidStats::AddMedian( std::string& varName, double inputMedian )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    _values[3][idx] = inputMedian;
    return true;
}

bool Statistics::ValidStats::AddStddev( std::string& varName, double inputStddev )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    _values[4][idx] = inputStddev;
    return true;
}

bool Statistics::ValidStats::AddCount( std::string& varName, long inputCount )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    _count[idx] = inputCount;
    return true;
}

bool Statistics::ValidStats::Get3MStats( std::string& varName, double* output3M )
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

bool Statistics::ValidStats::GetMedian( std::string& varName, double* outputMedian )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    *outputMedian = _values[3][idx];
    return true;
}

bool Statistics::ValidStats::GetStddev( std::string& varName, double* outputStddev )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    *outputStddev = _values[4][idx];
    return true;
}

bool Statistics::ValidStats::GetCount( std::string& varName, long* outputCount )
{
    int idx = _getVarIdx( varName );
    if( idx == -1 )     // This variable doesn't exist
        return false;

    *outputCount = _count[idx];
    return true;
}

bool Statistics::ValidStats::InvalidAll()
{
    for( int i = 0; i < 5; i++ )
        for( int j = 0; j < _values[i].size(); j++ )
            _values[i][j] = std::nan("1");
    for( int i = 0; i < _count.size(); i++ )
        _count[i] = -1;
    return true;
}

std::string Statistics::ValidStats::GetDatasetName()
{
    return _datasetName;
}

bool Statistics::ValidStats::SetDatasetName( std::string& dsName )
{
    if( dsName != _datasetName )
    {
        _datasetName = dsName;
        _variables.clear();
        for( int i = 0; i < 5; i++ )
            _values[i].clear();
    }
    _count.clear();
    return true;
}

size_t Statistics::ValidStats::GetVariableCount()
{
    return _variables.size();
}

std::string Statistics::ValidStats::GetVariableName( int i )
{
    if( i < _variables.size() )
        return _variables.at(i);
    else
        return std::string("");
}

void Statistics::_exportTextClicked() 
{
    QString homePath = QDir::homePath();
    homePath.append("/Variable_Statistics.txt");
    QString path = QDir::toNativeSeparators(homePath);
    QString fName = QFileDialog::getSaveFileName(this, "Select file to save statistics:", path, 
                                                "Comma-separated values (*.csv)");
    if( !fName.isEmpty() )
    {
        ofstream file;
        file.open(fName.toStdString().c_str());
        if (file.fail()) {
          std::ostringstream ss;
          ss << "Failed to open file ";
          ss << fName.toStdString();
          ss << " for writing.";
          return;
        }
          
        // Initialize pointers
        GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                        (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
        std::string dsName = guiParams->GetStatsDatasetName();
        StatisticsParams* statsParams = dynamic_cast<StatisticsParams*>
                (_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

        file << "Variable, No_of_Samples";
        if( statsParams->GetMinEnabled() )
            file << ", Min";
        if( statsParams->GetMaxEnabled() )
            file << ", Max";
        if( statsParams->GetMeanEnabled() )
            file << ", Mean";
        if( statsParams->GetMedianEnabled() )
            file << ", Median";
        if( statsParams->GetStdDevEnabled() )
            file << ", Stddev";
        file << endl;
    
        for( int i = 0; i < _validStats.GetVariableCount(); i++ )
        {
            std::string varname = _validStats.GetVariableName( i );            
            double m3[3], median, stddev;
            long   count;
            _validStats.Get3MStats( varname, m3 );
            _validStats.GetMedian(  varname, &median );
            _validStats.GetStddev(  varname, &stddev );
            _validStats.GetCount(   varname, &count );
            file << varname << ", " << count ;
            if( statsParams->GetMinEnabled() )
                file << ", " << m3[0];
            if( statsParams->GetMaxEnabled() )
                file << ", " << m3[1];
            if( statsParams->GetMeanEnabled() )
                file << ", " << m3[2];
            if( statsParams->GetMedianEnabled() )
                file << ", " << median;
            if( statsParams->GetStdDevEnabled() )
                file << ", " << stddev;
            file << endl;
        }

        file << endl;

        /*
        std::vector<double> myMin, myMax;
        statsParams->GetBox()->GetExtents( myMin, myMax );

        file << "Spatial Extents:" << endl;
        file << "X min = " << myMin[0] << ",    X max = " << myMax[0] << endl;
        file << "Y min = " << myMin[1] << ",    Y max = " << myMax[1] << endl;
        if( myMin.size() == 3 && myMax.size() == 3 )
            file << "Z min = " << myMin[2] << ",    Z max = " << myMax[2] << endl;
        */

        file << endl;

        file << "Temporal Extents:" << endl;
        file << "Minimum Timestep = " << statsParams->GetCurrentMinTS() 
             << ",    Maximum Timestep = " << statsParams->GetCurrentMaxTS() << endl;

        file.close();
    }
}
