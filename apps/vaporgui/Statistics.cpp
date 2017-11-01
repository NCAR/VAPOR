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
Statistics::Statistics(QWidget *parent) : QDialog(parent), Ui_StatsWindow()
{
    _errMsg = NULL;
    _controlExec = NULL;

    setupUi(this);
    setWindowTitle("Statistics");
    // adjustTables();
    // VariablesTable->installEventFilter(this); //for responding to keyboard?

    Connect();
}

Statistics::~Statistics()
{
    if (_errMsg) {
        delete _errMsg;
        _errMsg = NULL;
    }
}

bool Statistics::Update()
{
    // Initialize pointers
    VAPoR::DataStatus *dataStatus = _controlExec->getDataStatus();
    GUIStateParams *   guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string        currentDatasetName = guiParams->GetStatsDatasetName();
    assert(currentDatasetName != "");
    VAPoR::DataMgr *  currentDmgr = dataStatus->GetDataMgr(currentDatasetName);
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(currentDatasetName, StatisticsParams::GetClassType()));

    // Update DataMgrCombo
    std::vector<std::string> dmNames = dataStatus->GetDataMgrNames();
    assert(dmNames.size() > 0);
    DataMgrCombo->blockSignals(true);
    int currentIdx = -1;
    for (int i = 0; i < dmNames.size(); i++) {
        QString item = QString::fromStdString(dmNames[i]);
        if (DataMgrCombo->findText(item) == -1) DataMgrCombo->addItem(item);
        if (dmNames[i] == currentDatasetName) currentIdx = i;
    }
    assert(currentIdx != -1);
    DataMgrCombo->setCurrentIndex(currentIdx);
    DataMgrCombo->blockSignals(false);

    // Update Timesteps
    int minTS = statsParams->GetMinTS();
    MinTimestepSpinbox->blockSignals(true);
    MinTimestepSpinbox->setValue(minTS);
    MinTimestepSpinbox->blockSignals(false);
    int maxTS = statsParams->GetMaxTS();
    MaxTimestepSpinbox->blockSignals(true);
    MaxTimestepSpinbox->setValue(maxTS);
    MaxTimestepSpinbox->blockSignals(false);

    // Update auto-update checkbox
    bool autoUpdate = statsParams->GetAutoUpdate();
    UpdateCheckbox->blockSignals(true);
    if (autoUpdate)
        UpdateCheckbox->setCheckState(Qt::Checked);
    else
        UpdateCheckbox->setCheckState(Qt::Unchecked);
    UpdateCheckbox->blockSignals(false);

    // Update "Add a Variable"
    std::vector<std::string> availVars = currentDmgr->GetDataVarNames(3, true);
    std::vector<std::string> availVars2D = currentDmgr->GetDataVarNames(2, true);
    for (int i = 0; i < availVars2D.size(); i++) availVars.push_back(availVars2D[i]);
    // remove variables already enabled
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    for (int i = 0; i < enabledVars.size(); i++)
        for (int rmIdx = 0; rmIdx < availVars.size(); rmIdx++)
            if (availVars[rmIdx] == enabledVars[i]) {
                availVars.erase(availVars.begin() + rmIdx);
                break;
            }
    std::sort(availVars.begin(), availVars.end());
    NewVarCombo->blockSignals(true);
    NewVarCombo->clear();
    NewVarCombo->addItem(QString::fromAscii("Add a Variable"));
    for (std::vector<std::string>::iterator it = availVars.begin(); it != availVars.end(); ++it) { NewVarCombo->addItem(QString::fromStdString(*it)); }
    NewVarCombo->setCurrentIndex(0);
    NewVarCombo->blockSignals(false);

    // Update "Remove a Variable"
    assert(enabledVars.size() == _validStats.GetVariableCount());
    std::sort(enabledVars.begin(), enabledVars.end());
    RemoveVarCombo->blockSignals(true);
    RemoveVarCombo->clear();
    RemoveVarCombo->addItem(QString::fromAscii("Remove a Variable"));
    for (int i = 0; i < enabledVars.size(); i++) { RemoveVarCombo->addItem(QString::fromStdString(enabledVars[i])); }
    RemoveVarCombo->setCurrentIndex(0);
    RemoveVarCombo->blockSignals(false);

    // Update statistics to calculate
    AddStatCombo->blockSignals(true);
    RemoveStatCombo->blockSignals(true);
    AddStatCombo->clear();
    RemoveStatCombo->clear();
    AddStatCombo->addItem(QString::fromAscii("Add a Calculation"));
    RemoveStatCombo->addItem(QString::fromAscii("Remove a Calculation"));
    if (statsParams->GetMinEnabled())
        RemoveStatCombo->addItem(QString::fromAscii("Min"));
    else
        AddStatCombo->addItem(QString::fromAscii("Min"));
    if (statsParams->GetMaxEnabled())
        RemoveStatCombo->addItem(QString::fromAscii("Max"));
    else
        AddStatCombo->addItem(QString::fromAscii("Max"));
    if (statsParams->GetMeanEnabled())
        RemoveStatCombo->addItem(QString::fromAscii("Mean"));
    else
        AddStatCombo->addItem(QString::fromAscii("Mean"));
    if (statsParams->GetMedianEnabled())
        RemoveStatCombo->addItem(QString::fromAscii("Median"));
    else
        AddStatCombo->addItem(QString::fromAscii("Median"));
    if (statsParams->GetStdDevEnabled())
        RemoveStatCombo->addItem(QString::fromAscii("StdDev"));
    else
        AddStatCombo->addItem(QString::fromAscii("StdDev"));
    AddStatCombo->setCurrentIndex(0);
    RemoveStatCombo->setCurrentIndex(0);
    AddStatCombo->blockSignals(false);
    RemoveStatCombo->blockSignals(false);

    this->_updateVarTable();

    return true;
}

void Statistics::_updateVarTable()
{
    // Initialize pointers
    VAPoR::DataStatus *dataStatus = _controlExec->getDataStatus();
    GUIStateParams *   guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string        currentDatasetName = guiParams->GetStatsDatasetName();
    assert(currentDatasetName != "");
    VAPoR::DataMgr *  currentDmgr = dataStatus->GetDataMgr(currentDatasetName);
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(currentDatasetName, StatisticsParams::GetClassType()));

    // Update Statistics table header
    VariablesTable->clear();
    QStringList header;
    header << "Variable";
    if (statsParams->GetMinEnabled()) header << "Min";
    if (statsParams->GetMaxEnabled()) header << "Max";
    if (statsParams->GetMeanEnabled()) header << "Mean";
    if (statsParams->GetMedianEnabled()) header << "Median";
    if (statsParams->GetStdDevEnabled()) header << "StdDev";
    VariablesTable->setColumnCount(header.size());
    VariablesTable->setHorizontalHeaderLabels(header);
    VariablesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    // Update cells
    QBrush                   brush(QColor(255, 0, 0));
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    assert(enabledVars.size() == _validStats.GetVariableCount());
    VariablesTable->setRowCount(enabledVars.size());
    for (int row = 0; row < enabledVars.size(); row++) {
        VariablesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(enabledVars[row])));

        double m3[3], median, stddev;
        _validStats.Get3MStats(enabledVars[row], m3);
        _validStats.GetMedian(enabledVars[row], &median);
        _validStats.GetStddev(enabledVars[row], &stddev);

        int column = 1;
        if (statsParams->GetMinEnabled()) {
            if (!std::isnan(m3[0]))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[0])));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetMaxEnabled()) {
            if (!std::isnan(m3[1]))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[1])));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetMeanEnabled()) {
            if (!std::isnan(m3[2]))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[2])));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetMedianEnabled()) {
            if (!std::isnan(median))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(median)));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetStdDevEnabled()) {
            if (!std::isnan(stddev))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(stddev)));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::fromAscii("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
    }
}

void Statistics::showMe()
{
    show();
    raise();
    activateWindow();
}

int Statistics::initControlExec(ControlExec *ce)
{
    if (ce != NULL) {
        _controlExec = ce;
    } else {
        return -1;
    }

    // Store the active dataset name
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string     dsName = guiParams->GetStatsDatasetName();
    if (dsName == "")    // not initialized yet
    {
        VAPoR::DataStatus *      dataStatus = _controlExec->getDataStatus();
        std::vector<std::string> dmNames = dataStatus->GetDataMgrNames();
        assert(dmNames.size() > 0);
        guiParams->SetStatsDatasetName(dmNames[0]);
    }
    dsName = guiParams->GetStatsDatasetName();
    _validStats.SetDatasetName(dsName);

    // this->Update( params );

    return 0;
}

bool Statistics::Connect()
{
    connect(NewVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_newVarChanged(int)));
    connect(RemoveVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_removeVarChanged(int)));
    return true;
}

void Statistics::_newVarChanged(int index)
{
    if (index == 0) return;

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string       varName = NewVarCombo->itemText(index).toStdString();

    // Add this variable to "Remove a Variable" list
    RemoveVarCombo->addItem(NewVarCombo->itemText(index));

    // Remove this variable from "Add a Variable" list
    NewVarCombo->blockSignals(true);
    NewVarCombo->setCurrentIndex(0);
    NewVarCombo->blockSignals(false);
    NewVarCombo->removeItem(index);

    // Add this variable to parameter
    std::vector<std::string> vars = statsParams->GetAuxVariableNames();
    vars.push_back(varName);
    statsParams->SetAuxVariableNames(vars);

    // Add this variable to _validStats
    _validStats.AddVariable(varName);

    // Update VariableTable
    this->_updateVarTable();
}

void Statistics::_removeVarChanged(int index)
{
    if (index == 0) return;

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string       varName = RemoveVarCombo->itemText(index).toStdString();

    // Add this variable to "Add a Variable" list
    NewVarCombo->addItem(RemoveVarCombo->itemText(index));

    // Remove this variable from "Remove a Variable" list
    RemoveVarCombo->blockSignals(true);
    RemoveVarCombo->setCurrentIndex(0);
    RemoveVarCombo->blockSignals(false);
    RemoveVarCombo->removeItem(index);

    // Remove this variable from parameter
    std::vector<std::string> vars = statsParams->GetAuxVariableNames();
    int                      rmIdx = -1;
    for (int i = 0; i < vars.size(); i++)
        if (vars[i] == varName) {
            rmIdx = i;
            break;
        }
    assert(rmIdx != -1);
    vars.erase(vars.begin() + rmIdx);
    statsParams->SetAuxVariableNames(vars);

    // Remove this variable from _validStats
    _validStats.RemoveVariable(varName);

    // Update VariableTable
    this->_updateVarTable();
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
int Statistics::ValidStats::_getVarIdx(std::string varName)
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

bool Statistics::ValidStats::AddVariable(std::string newVar)
{
    if (newVar == "") return false;
    if (_getVarIdx(newVar) != -1)    // this variable already exists.
        return false;

    _variables.push_back(newVar);
    for (int i = 0; i < 5; i++) {
        _values[i].push_back(std::nan("1"));
        assert(_values[i].size() == _variables.size());
    }
    return true;
}

bool Statistics::ValidStats::RemoveVariable(std::string varname)
{
    int rmIdx = _getVarIdx(varname);
    if (rmIdx == -1)    // this variable doesn't exist.
        return false;

    _variables.erase(_variables.begin() + rmIdx);
    for (int i = 0; i < 5; i++) {
        _values[i].erase(_values[i].begin() + rmIdx);
        assert(_values[i].size() == _variables.size());
    }
    return true;
}

bool Statistics::ValidStats::Add3MStats(std::string varName, const double *input3M)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    for (int i = 0; i < 3; i++) { _values[i][idx] = input3M[i]; }
    return true;
}

bool Statistics::ValidStats::AddMedian(std::string varName, double inputMedian)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    _values[3][idx] = inputMedian;
    return true;
}

bool Statistics::ValidStats::AddStddev(std::string varName, double inputStddev)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    _values[4][idx] = inputStddev;
    return true;
}

bool Statistics::ValidStats::Get3MStats(std::string varName, double *output3M)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    for (int i = 0; i < 3; i++) { output3M[i] = _values[i][idx]; }
    return true;
}

bool Statistics::ValidStats::GetMedian(std::string varName, double *outputMedian)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    *outputMedian = _values[3][idx];
    return true;
}

bool Statistics::ValidStats::GetStddev(std::string varName, double *outputStddev)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    *outputStddev = _values[4][idx];
    return true;
}

bool Statistics::ValidStats::InvalidAll()
{
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < _values[i].size(); j++) _values[i][j] = std::nan("1");
    return true;
}

std::string Statistics::ValidStats::GetDatasetName() { return _datasetName; }

bool Statistics::ValidStats::SetDatasetName(std::string &dsName)
{
    if (dsName != _datasetName) {
        _datasetName = dsName;
        _variables.clear();
        for (int i = 0; i < 5; i++) _values[i].clear();
    }
    return true;
}

size_t Statistics::ValidStats::GetVariableCount() { return _variables.size(); }
