//************************************************************************
//                                                                       *
//         Copyright (C)  2016                                           *
//   University Corporation for Atmospheric Research                     *
//         All Rights Reserved                                           *
//                                                                       *
//************************************************************************
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
#include "Statistics.h"
#include "GUIStateParams.h"
#include "ErrorReporter.h"
#include "Flags.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include "vapor/VAssert.h"
#include <cstdio>
#include <algorithm>
#include <vapor/MyBase.h>
#include <vapor/DataStatus.h>
#include "PWidgets.h"
#include "VPushButton.h"

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
    MyFidelityWidget->Reinit((VariableFlags)AUXILIARY);

    Connect();

    auto rs = new PRegionSelector;
    verticalLayout_2->insertWidget(0, rs);
    _pw.push_back(rs);

    auto cr = new PCopyRegionWidget;
    verticalLayout_2->insertWidget(1, cr);
    _pw.push_back(cr);

    VPushButton *close = new VPushButton("Close Window");
    connect(close, &VPushButton::ButtonClicked, this, &QDialog::accept);
    layout()->addWidget(close);
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
    VAPoR::DataStatus *      dataStatus = _controlExec->GetDataStatus();
    std::vector<std::string> dmNames = dataStatus->GetDataMgrNames();
    if (dmNames.empty()) {
        this->close();
        return false;
    }
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string     currentDatasetName = guiParams->GetStatsDatasetName();
    if (currentDatasetName == "" || currentDatasetName == "NULL") {
        this->close();
        return false;
    }

    int currentIdx = -1;
    for (int i = 0; i < dmNames.size(); i++)
        if (currentDatasetName == dmNames[i]) {
            currentIdx = i;
            break;
        }
    if (currentIdx == -1)    // currentDatasetName is closed!!!
    {
        currentDatasetName = dmNames[0];
        currentIdx = 0;
        guiParams->SetStatsDatasetName(currentDatasetName);

        _validStats.Clear();    // since the old dataset is closed, we clear all stats.
        _validStats.currentDataSourceName = currentDatasetName;
    }
    VAPoR::DataMgr *         currentDmgr = dataStatus->GetDataMgr(currentDatasetName);
    StatisticsParams *       statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(currentDatasetName, StatisticsParams::GetClassType()));
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    if (_validStats.GetVariableCount() == 0)    // likely because the current data set is switched
    {
        for (int i = 0; i < enabledVars.size(); i++) _validStats.AddVariable(enabledVars[i]);
    }

    // detect params changed by undo/redo
    if (_validStats.currentDataSourceName != currentDatasetName || !_validStats.HaveSameParams(statsParams)) {
        _validStats.currentDataSourceName = currentDatasetName;
        _validStats.UpdateMyParams(statsParams);
    }

    // Update DataMgrCombo
    DataMgrCombo->blockSignals(true);
    DataMgrCombo->clear();
    for (int i = 0; i < dmNames.size(); i++) DataMgrCombo->addItem(QString::fromStdString(dmNames[i]));
    DataMgrCombo->setCurrentIndex(currentIdx);
    DataMgrCombo->blockSignals(false);

    // Update auto-update checkbox
    bool autoUpdate = statsParams->GetAutoUpdateEnabled();
    UpdateCheckbox->blockSignals(true);
    if (autoUpdate)
        UpdateCheckbox->setCheckState(Qt::Checked);
    else
        UpdateCheckbox->setCheckState(Qt::Unchecked);
    UpdateCheckbox->blockSignals(false);

    // Update "Add a Variable"
    std::vector<std::string> availVars = currentDmgr->GetDataVarNames();
    for (int i = 0; i < enabledVars.size(); i++)
        for (int rmIdx = 0; rmIdx < availVars.size(); rmIdx++)
            if (availVars[rmIdx] == enabledVars[i]) {
                availVars.erase(availVars.begin() + rmIdx);
                break;
            }
    std::sort(availVars.begin(), availVars.end());
    NewVarCombo->blockSignals(true);
    NewVarCombo->clear();
    NewVarCombo->blockSignals(false);
    NewVarCombo->addItem(QString("Add a Variable"));
    for (std::vector<std::string>::iterator it = availVars.begin(); it != availVars.end(); ++it) { NewVarCombo->addItem(QString::fromStdString(*it)); }
    NewVarCombo->setCurrentIndex(0);

    // Update "Remove a Variable"
    VAssert(enabledVars.size() == _validStats.GetVariableCount());
    std::sort(enabledVars.begin(), enabledVars.end());
    RemoveVarCombo->blockSignals(true);
    RemoveVarCombo->clear();
    RemoveVarCombo->addItem(QString("Remove a Variable"));
    for (int i = 0; i < enabledVars.size(); i++) { RemoveVarCombo->addItem(QString::fromStdString(enabledVars[i])); }
    RemoveVarCombo->setCurrentIndex(0);
    RemoveVarCombo->blockSignals(false);

    // Update Statistics table: header
    this->_updateStatsTable();

    // Update calculations
    NewCalcCombo->blockSignals(true);
    RemoveCalcCombo->blockSignals(true);
    NewCalcCombo->clear();
    RemoveCalcCombo->clear();
    NewCalcCombo->addItem(QString("Add a Calculation"));
    RemoveCalcCombo->addItem(QString("Remove a Calculation"));
    if (statsParams->GetMinEnabled())
        RemoveCalcCombo->addItem(QString("Min"));
    else
        NewCalcCombo->addItem(QString("Min"));
    if (statsParams->GetMaxEnabled())
        RemoveCalcCombo->addItem(QString("Max"));
    else
        NewCalcCombo->addItem(QString("Max"));
    if (statsParams->GetMeanEnabled())
        RemoveCalcCombo->addItem(QString("Mean"));
    else
        NewCalcCombo->addItem(QString("Mean"));
    if (statsParams->GetMedianEnabled())
        RemoveCalcCombo->addItem(QString("Median"));
    else
        NewCalcCombo->addItem(QString("Median"));
    if (statsParams->GetStdDevEnabled())
        RemoveCalcCombo->addItem(QString("StdDev"));
    else
        NewCalcCombo->addItem(QString("StdDev"));
    NewCalcCombo->setCurrentIndex(0);
    RemoveCalcCombo->setCurrentIndex(0);
    NewCalcCombo->blockSignals(false);
    RemoveCalcCombo->blockSignals(false);

    // Update LOD, Refinement
    MyFidelityWidget->Update(currentDmgr, _controlExec->GetParamsMgr(), statsParams);

    // Update timesteps
    MinTimestepSpinbox->blockSignals(true);
    MinTimestepSpinbox->setMinimum(0);
    MinTimestepSpinbox->setMaximum(currentDmgr->GetNumTimeSteps() - 1);
    MinTimestepSpinbox->setValue(statsParams->GetCurrentTimestep());
    MinTimestepSpinbox->blockSignals(false);

    MaxTimestepSpinbox->blockSignals(true);
    MaxTimestepSpinbox->setMinimum(0);
    MaxTimestepSpinbox->setMaximum(currentDmgr->GetNumTimeSteps() - 1);
    MaxTimestepSpinbox->setValue(statsParams->GetCurrentMaxTS());
    MaxTimestepSpinbox->blockSignals(false);

    bool has3DVar = false;
    for (const auto &var : enabledVars) has3DVar |= 3 == currentDmgr->GetNumDimensions(var);

    _controlExec->GetParamsMgr()->BeginSaveStateGroup("Update Box Dims");
    if (has3DVar) {
        statsParams->GetBox()->SetPlanar(false);
        statsParams->GetBox()->SetOrientation(Box::XYZ);
    } else {
        statsParams->GetBox()->SetPlanar(true);
        statsParams->GetBox()->SetOrientation(Box::XY);
    }
    _controlExec->GetParamsMgr()->EndSaveStateGroup();

    for (auto p : _pw) p->Update(statsParams, _controlExec->GetParamsMgr(), currentDmgr);

    return true;
}

void Statistics::_updateStatsTable()
{
    // Initialize pointers
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string     currentDatasetName = guiParams->GetStatsDatasetName();
    VAssert(currentDatasetName != "");
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(currentDatasetName, StatisticsParams::GetClassType()));

    // Update Statistics Table: header
    VariablesTable->clear();    // this also deletes the items properly.
    QStringList header;
    header << "Variable"
           << "No. of Samples";
    if (statsParams->GetMinEnabled()) header << "Min";
    if (statsParams->GetMaxEnabled()) header << "Max";
    if (statsParams->GetMeanEnabled()) header << "Mean";
    if (statsParams->GetMedianEnabled()) header << "Median";
    if (statsParams->GetStdDevEnabled()) header << "StdDev";
    VariablesTable->setColumnCount(header.size());
    VariablesTable->setHorizontalHeaderLabels(header);
    VariablesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Update Statistics Table: cells
    QBrush                   brush(QColor(255, 0, 0));
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    VAssert(enabledVars.size() == _validStats.GetVariableCount());
    VariablesTable->setRowCount(enabledVars.size());
    int numberOfDigits = 3;
    for (int row = 0; row < enabledVars.size(); row++) {
        float m3[3]{0.0f, 0.0f, 0.0f}, median = 0.0f, stddev = 0.0f;
        long  count = 0;
        _validStats.GetCount(enabledVars[row], &count);
        _validStats.Get3MStats(enabledVars[row], m3);
        _validStats.GetMedian(enabledVars[row], &median);
        _validStats.GetStddev(enabledVars[row], &stddev);

        VariablesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(enabledVars[row])));
        if (count == -1) {
            VariablesTable->setItem(row, 1, new QTableWidgetItem(QString("??")));
            VariablesTable->item(row, 1)->setForeground(brush);
        } else
            VariablesTable->setItem(row, 1, new QTableWidgetItem(QString::number(count)));

        int column = 2;
        if (statsParams->GetMinEnabled()) {
            if (!std::isnan(m3[0])) {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[0], 'g', numberOfDigits)));
            } else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetMaxEnabled()) {
            if (!std::isnan(m3[1]))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[1], 'g', numberOfDigits)));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetMeanEnabled()) {
            if (!std::isnan(m3[2]))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(m3[2], 'g', numberOfDigits)));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetMedianEnabled()) {
            if (!std::isnan(median))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(median, 'g', numberOfDigits)));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
        if (statsParams->GetStdDevEnabled()) {
            if (!std::isnan(stddev))
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString::number(stddev, 'g', numberOfDigits)));
            else {
                VariablesTable->setItem(row, column, new QTableWidgetItem(QString("??")));
                VariablesTable->item(row, column)->setForeground(brush);
            }
            column++;
        }
    }
    for (int r = 0; r < VariablesTable->rowCount(); r++)
        for (int c = 0; c < VariablesTable->columnCount(); c++) {
            QTableWidgetItem *item = VariablesTable->item(r, c);
            item->setFlags(Qt::NoItemFlags);
        }

    VariablesTable->update();
    VariablesTable->repaint();
    VariablesTable->viewport()->update();
}

void Statistics::showMe()
{
    open();
    Update();
}

int Statistics::initControlExec(ControlExec *ce)
{
    if (ce != NULL)
        _controlExec = ce;
    else
        return -1;

    // Store the active dataset name
    std::vector<std::string> dmNames = _controlExec->GetDataStatus()->GetDataMgrNames();
    if (dmNames.empty())
        return -1;
    else {
        GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
        std::string     dsName = guiParams->GetStatsDatasetName();
        if (dsName == "" || dsName == "NULL")    // not initialized yet
            guiParams->SetStatsDatasetName(dmNames[0]);
    }

    return 0;
}

bool Statistics::Connect()
{
    connect(NewVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_newVarChanged(int)));
    connect(RemoveVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_removeVarChanged(int)));
    connect(NewCalcCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_newCalcChanged(int)));
    connect(RemoveCalcCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_removeCalcChanged(int)));
    connect(MinTimestepSpinbox, SIGNAL(valueChanged(int)), this, SLOT(_minTSChanged(int)));
    connect(MaxTimestepSpinbox, SIGNAL(valueChanged(int)), this, SLOT(_maxTSChanged(int)));
    connect(UpdateButton, SIGNAL(clicked()), this, SLOT(_updateButtonClicked()));
    connect(DataMgrCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_dataSourceChanged(int)));
    connect(UpdateCheckbox, SIGNAL(stateChanged(int)), this, SLOT(_autoUpdateClicked(int)));
    connect(ExportButton, SIGNAL(clicked()), this, SLOT(_exportTextClicked()));
    return true;
}

void Statistics::_autoUpdateClicked(int state)
{
    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    if (state == 0)    // unchecked
        statsParams->SetAutoUpdateEnabled(false);
    else if (state == 2)    // checked
    {
        statsParams->SetAutoUpdateEnabled(true);
        _updateButtonClicked();
    } else {
        std::cerr << "Dont know what this state is!!!" << std::endl;
        // REPORT ERROR!!!
    }
}

void Statistics::_dataSourceChanged(int index)
{
    std::string newDataSourceName = DataMgrCombo->itemText(index).toStdString();

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(newDataSourceName, StatisticsParams::GetClassType()));

    guiParams->SetStatsDatasetName(newDataSourceName);

    _validStats.currentDataSourceName = newDataSourceName;

    // add variables to _validStats if there are any
    _validStats.Clear();
    std::vector<std::string> enabledVars = statsParams->GetAuxVariableNames();
    for (int i = 0; i < enabledVars.size(); i++) _validStats.AddVariable(enabledVars[i]);
}

void Statistics::_geometryValueChanged()
{
    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    _validStats.InvalidAll();

    std::vector<double> myMin, myMax;
    statsParams->GetBox()->GetExtents(myMin, myMax);
    _validStats.SetCurrentExtents(myMin, myMax);

    // Auto-update if enabled
    if (statsParams->GetAutoUpdateEnabled()) _updateButtonClicked();
}

void Statistics::_updateButtonClicked()
{
    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    for (int i = 0; i < _validStats.GetVariableCount(); i++) {
        std::string varname = _validStats.GetVariableName(i);
        long        count = 0;
        _validStats.GetCount(varname, &count);
        if (count == -1) {
            _calc3M(varname);
            _updateStatsTable();
        }
        float m3[3]{0.0f, 0.0f, 0.0f}, median = 0.0f, stddev = 0.0f;
        _validStats.Get3MStats(varname, m3);
        _validStats.GetMedian(varname, &median);
        _validStats.GetStddev(varname, &stddev);
        if ((statsParams->GetMinEnabled() || statsParams->GetMaxEnabled() || statsParams->GetMeanEnabled()) && std::isnan(m3[2])) {
            _calc3M(varname);
            _updateStatsTable();
        }
        if (statsParams->GetMedianEnabled() && std::isnan(median)) {
            _calcMedian(varname);
            _updateStatsTable();
        }
        if (statsParams->GetStdDevEnabled() && std::isnan(stddev)) {
            _calcStddev(varname);
            _updateStatsTable();
        }
    }
}

void Statistics::_minTSChanged(int val)
{
    VAssert(val >= 0);

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    _validStats.currentTimeStep[0] = val;

    // Add this minTS to parameter if different
    if (val != statsParams->GetCurrentTimestep()) {
        statsParams->SetCurrentTimestep(val);
        _validStats.InvalidAll();

        if (val > statsParams->GetCurrentMaxTS()) {
            _validStats.currentTimeStep[1] = val;
            statsParams->SetCurrentMaxTS(val);
            MaxTimestepSpinbox->setValue(val);
        }
    }

    // Auto-update if enabled
    if (statsParams->GetAutoUpdateEnabled()) _updateButtonClicked();
}

void Statistics::_maxTSChanged(int val)
{
    VAssert(val >= 0);

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));

    _validStats.currentTimeStep[1] = val;

    // Add this maxTS to parameter if different
    if (val != statsParams->GetCurrentMaxTS()) {
        statsParams->SetCurrentMaxTS(val);
        _validStats.InvalidAll();

        if (val < statsParams->GetCurrentTimestep()) {
            _validStats.currentTimeStep[0] = val;
            statsParams->SetCurrentTimestep(val);
            MinTimestepSpinbox->setValue(val);
        }
    }

    // Auto-update if enabled
    if (statsParams->GetAutoUpdateEnabled()) _updateButtonClicked();
}

void Statistics::_newCalcChanged(int index)
{
    VAssert(index > 0);

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string       calcName = NewCalcCombo->itemText(index).toStdString();

    // Add this calculation to parameter
    if (calcName == "Min")
        statsParams->SetMinEnabled(true);
    else if (calcName == "Max")
        statsParams->SetMaxEnabled(true);
    else if (calcName == "Mean")
        statsParams->SetMeanEnabled(true);
    else if (calcName == "Median")
        statsParams->SetMedianEnabled(true);
    else if (calcName == "StdDev")
        statsParams->SetStdDevEnabled(true);
    else {
        // REPORT ERROR!!
    }

    // Auto-update if enabled
    if (statsParams->GetAutoUpdateEnabled()) _updateButtonClicked();
}

void Statistics::_removeCalcChanged(int index)
{
    VAssert(index > 0);

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string       calcName = RemoveCalcCombo->itemText(index).toStdString();

    // Remove this calculation from parameter
    if (calcName == "Min")
        statsParams->SetMinEnabled(false);
    else if (calcName == "Max")
        statsParams->SetMaxEnabled(false);
    else if (calcName == "Mean")
        statsParams->SetMeanEnabled(false);
    else if (calcName == "Median")
        statsParams->SetMedianEnabled(false);
    else if (calcName == "StdDev")
        statsParams->SetStdDevEnabled(false);
    else {
        // REPORT ERROR!!
    }
}

void Statistics::_newVarChanged(int index)
{
    if (index <= 0) return;

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    VAPoR::DataMgr *  currentDmgr = _controlExec->GetDataStatus()->GetDataMgr(dsName);
    std::string       varName = NewVarCombo->itemText(index).toStdString();

    // Test if the selected variable available at the specific time step,
    //   compression level, etc.
    if (!currentDmgr->VariableExists(statsParams->GetCurrentTimestep(), varName, statsParams->GetRefinementLevel(), statsParams->GetCompressionLevel())) {
        MSG_WARN("Selected variable not available at this settings!");
        NewVarCombo->setCurrentIndex(0);
        return;
    } else {
        // Add this variable to parameter
        std::vector<std::string> vars = statsParams->GetAuxVariableNames();
        vars.push_back(varName);
        statsParams->SetAuxVariableNames(vars);

        // Add this variable to _validStats
        _validStats.AddVariable(varName);

        // Auto-update if enabled
        if (statsParams->GetAutoUpdateEnabled()) _updateButtonClicked();
    }
}

void Statistics::_removeVarChanged(int index)
{
    VAssert(index > 0);

    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    std::string       varName = RemoveVarCombo->itemText(index).toStdString();

    // Remove this variable from parameter
    std::vector<std::string> vars = statsParams->GetAuxVariableNames();
    int                      rmIdx = -1;
    for (int i = 0; i < vars.size(); i++)
        if (vars[i] == varName) {
            rmIdx = i;
            break;
        }
    VAssert(rmIdx != -1);
    vars.erase(vars.begin() + rmIdx);
    statsParams->SetAuxVariableNames(vars);

    // Remove this variable from _validStats
    _validStats.RemoveVariable(varName);
}

bool Statistics::_calc3M(std::string varname)
{
    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    VAPoR::DataMgr *  currentDmgr = _controlExec->GetDataStatus()->GetDataMgr(dsName);

    int minTS = statsParams->GetCurrentTimestep();
    int maxTS = statsParams->GetCurrentMaxTS();
    if (!currentDmgr->IsTimeVarying(varname)) maxTS = minTS;
    std::vector<double> minExtent, maxExtent;
    statsParams->GetBox()->GetExtents(minExtent, maxExtent);

    float c = 0.0;
    float sum = 0.0;
    float min = std::numeric_limits<float>::max();
    float max = -min;
    long  count = 0;

    for (int ts = minTS; ts <= maxTS; ts++) {
        VAPoR::Grid *grid = currentDmgr->GetVariable(ts, varname, statsParams->GetRefinementLevel(), statsParams->GetCompressionLevel(), minExtent, maxExtent);
        if (grid) {
            Grid::ConstIterator endItr = grid->cend();
            float               missingVal = grid->GetMissingValue();

            for (Grid::ConstIterator it = grid->cbegin(minExtent, maxExtent); it != endItr; ++it) {
                if (*it != missingVal) {
                    float val = *it;
                    min = min < val ? min : val;
                    max = max > val ? max : val;
                    float y = val - c;
                    float t = sum + y;
                    c = t - sum - y;
                    sum = t;
                    count++;
                }
            }

            delete grid;    // delete the grid after using it!
        }
    }

    if (count > 0) {
        float m3[3] = {min, max, sum / (float)count};
        _validStats.Add3MStats(varname, m3);
    } else    // count == 0
    {
        // std::cerr << "Error: Zero value got selected!!" << std::endl;
    }

    _validStats.AddCount(varname, count);

    return true;
}

bool Statistics::_calcMedian(std::string varname)
{
    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    VAPoR::DataMgr *  currentDmgr = _controlExec->GetDataStatus()->GetDataMgr(dsName);

    int minTS = statsParams->GetCurrentTimestep();
    int maxTS = statsParams->GetCurrentMaxTS();
    if (!currentDmgr->IsTimeVarying(varname)) maxTS = minTS;
    std::vector<double> minExtent, maxExtent;
    statsParams->GetBox()->GetExtents(minExtent, maxExtent);

    std::vector<float> buffer;
    for (int ts = minTS; ts <= maxTS; ts++) {
        VAPoR::Grid *grid = currentDmgr->GetVariable(ts, varname, statsParams->GetRefinementLevel(), statsParams->GetCompressionLevel(), minExtent, maxExtent);
        if (grid) {
            Grid::ConstIterator endItr = grid->cend();
            float               missingVal = grid->GetMissingValue();

            for (Grid::ConstIterator it = grid->cbegin(minExtent, maxExtent); it != endItr; ++it) {
                if (*it != missingVal) buffer.push_back(*it);
            }
        }
    }

    if (buffer.size() > 0) {
        std::sort(buffer.begin(), buffer.end());
        float median = buffer.at(buffer.size() / 2);
        _validStats.AddMedian(varname, median);
    } else {
        // std::cerr << "Error: Zero value got selected!!" << std::endl;
    }

    _validStats.AddCount(varname, buffer.size());

    return true;
}

bool Statistics::_calcStddev(std::string varname)
{
    // Initialize pointers
    GUIStateParams *  guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string       dsName = guiParams->GetStatsDatasetName();
    StatisticsParams *statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
    VAPoR::DataMgr *  currentDmgr = _controlExec->GetDataStatus()->GetDataMgr(dsName);

    int minTS = statsParams->GetCurrentTimestep();
    int maxTS = statsParams->GetCurrentMaxTS();
    if (!currentDmgr->IsTimeVarying(varname)) maxTS = minTS;
    std::vector<double> minExtent, maxExtent;
    statsParams->GetBox()->GetExtents(minExtent, maxExtent);

    float c = 0.0;
    float sum = 0.0;
    long  count = 0;
    float m3[3];
    _validStats.Get3MStats(varname, m3);
    if (std::isnan(m3[2])) {
        this->_calc3M(varname);
        _validStats.Get3MStats(varname, m3);
    }

    for (int ts = minTS; ts <= maxTS; ts++) {
        VAPoR::Grid *grid = currentDmgr->GetVariable(ts, varname, statsParams->GetRefinementLevel(), statsParams->GetCompressionLevel(), minExtent, maxExtent);
        if (grid) {
            Grid::ConstIterator endItr = grid->cend();
            float               missingVal = grid->GetMissingValue();
            for (Grid::ConstIterator it = grid->cbegin(minExtent, maxExtent); it != endItr; ++it) {
                if (*it != missingVal) {
                    float val = *it;
                    float y = (val - m3[2]) * (val - m3[2]) - c;
                    float t = sum + y;
                    c = t - sum - y;
                    sum = t;
                    count++;
                }
            }
        }
    }

    if (count > 0) {
        _validStats.AddStddev(varname, std::sqrt(sum / (float)count));
    } else {
        // std::cerr << "Error: Zero value got selected!!" << std::endl;
    }

    _validStats.AddCount(varname, count);

    return true;
}

// ValidStats class
//
int Statistics::ValidStats::_getVarIdx(std::string &varName)
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

bool Statistics::ValidStats::AddVariable(std::string &newVar)
{
    if (newVar == "") return false;
    if (_getVarIdx(newVar) != -1)    // this variable already exists.
        return false;

    _variables.push_back(newVar);
    for (int i = 0; i < 5; i++) {
        _values[i].push_back(std::nan("1"));
        VAssert(_values[i].size() == _variables.size());
    }
    _count.push_back(-1);
    if (_count.size() != _variables.size()) std::cerr << "_count.size() = " << _count.size() << ",  _variables.size() = " << _variables.size() << std::endl;
    VAssert(_count.size() == _variables.size());
    return true;
}

bool Statistics::ValidStats::RemoveVariable(std::string &varname)
{
    int rmIdx = _getVarIdx(varname);
    if (rmIdx == -1)    // this variable doesn't exist.
        return false;

    _variables.erase(_variables.begin() + rmIdx);
    for (int i = 0; i < 5; i++) {
        _values[i].erase(_values[i].begin() + rmIdx);
        VAssert(_values[i].size() == _variables.size());
    }
    _count.erase(_count.begin() + rmIdx);
    VAssert(_count.size() == _variables.size());
    return true;
}

bool Statistics::ValidStats::Add3MStats(std::string &varName, const float *input3M)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    for (int i = 0; i < 3; i++) { _values[i][idx] = input3M[i]; }
    return true;
}

bool Statistics::ValidStats::AddMedian(std::string &varName, float inputMedian)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    _values[3][idx] = inputMedian;
    return true;
}

bool Statistics::ValidStats::AddStddev(std::string &varName, float inputStddev)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    _values[4][idx] = inputStddev;
    return true;
}

bool Statistics::ValidStats::AddCount(std::string &varName, long inputCount)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    _count[idx] = inputCount;
    return true;
}

bool Statistics::ValidStats::Get3MStats(std::string &varName, float *output3M)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    for (int i = 0; i < 3; i++) { output3M[i] = _values[i][idx]; }
    return true;
}

bool Statistics::ValidStats::GetMedian(std::string &varName, float *outputMedian)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    *outputMedian = _values[3][idx];
    return true;
}

bool Statistics::ValidStats::GetStddev(std::string &varName, float *outputStddev)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    *outputStddev = _values[4][idx];
    return true;
}

bool Statistics::ValidStats::GetCount(std::string &varName, long *outputCount)
{
    int idx = _getVarIdx(varName);
    if (idx == -1)    // This variable doesn't exist
        return false;

    *outputCount = _count[idx];
    return true;
}

bool Statistics::ValidStats::InvalidAll()
{
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < _values[i].size(); j++) _values[i][j] = std::nan("1");
    for (int i = 0; i < _count.size(); i++) _count[i] = -1;
    return true;
}

bool Statistics::ValidStats::Clear()
{
    _variables.clear();
    for (int i = 0; i < 5; i++) _values[i].clear();
    _count.clear();
    return true;
}

size_t Statistics::ValidStats::GetVariableCount() { return _variables.size(); }

std::string Statistics::ValidStats::GetVariableName(int i)
{
    if (i < _variables.size())
        return _variables.at(i);
    else
        return std::string("");
}

bool Statistics::ValidStats::HaveSameParams(const VAPoR::StatisticsParams *rhs) const
{
    std::vector<double> myMin, myMax;
    rhs->GetBox()->GetExtents(myMin, myMax);
    std::vector<float> paramsMin, paramsMax;
    for (int i = 0; i < myMin.size(); i++) {
        paramsMin.push_back((float)myMin[i]);
        paramsMax.push_back((float)myMax[i]);
    }
    return (_variables == rhs->GetAuxVariableNames() && currentExtentMin == paramsMin && currentExtentMax == paramsMax && currentTimeStep[0] == rhs->GetCurrentTimestep()
            && currentTimeStep[1] == rhs->GetCurrentMaxTS() && currentLOD == rhs->GetCompressionLevel() && currentRefLev == rhs->GetRefinementLevel());
}

bool Statistics::ValidStats::UpdateMyParams(const VAPoR::StatisticsParams *rhs)
{
    this->Clear();
    std::vector<std::string> enabledVars = rhs->GetAuxVariableNames();
    for (int i = 0; i < enabledVars.size(); i++) this->AddVariable(enabledVars[i]);
    std::vector<double> myMin, myMax;
    rhs->GetBox()->GetExtents(myMin, myMax);
    this->SetCurrentExtents(myMin, myMax);
    currentTimeStep[0] = rhs->GetCurrentTimestep();
    currentTimeStep[1] = rhs->GetCurrentMaxTS();
    currentLOD = rhs->GetCompressionLevel();
    currentRefLev = rhs->GetRefinementLevel();
    return true;
}

bool Statistics::ValidStats::SetCurrentExtents(const std::vector<float> &min, const std::vector<float> &max)
{
    currentExtentMin.clear();
    currentExtentMax.clear();
    if (min.size() != max.size()) {
        // REPORT ERROR!!
        return false;
    }
    for (int i = 0; i < min.size(); i++) {
        currentExtentMin.push_back(min[i]);
        currentExtentMax.push_back(max[i]);
    }

    return true;
}

bool Statistics::ValidStats::SetCurrentExtents(const std::vector<double> &min, const std::vector<double> &max)
{
    std::vector<float> minf, maxf;
    for (int i = 0; i < min.size(); i++) {
        minf.push_back((float)min[i]);
        maxf.push_back((float)max[i]);
    }
    return (this->SetCurrentExtents(minf, maxf));
}

void Statistics::_exportTextClicked()
{
    QString homePath = QDir::homePath();
    homePath.append("/Variable_Statistics.txt");
    QString path = QDir::toNativeSeparators(homePath);
    QString fName = QFileDialog::getSaveFileName(this, "Select file to save statistics:", path, "Comma-separated values (*.txt)");
    if (!fName.isEmpty()) {
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
        GUIStateParams *         guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
        std::string              dsName = guiParams->GetStatsDatasetName();
        StatisticsParams *       statsParams = dynamic_cast<StatisticsParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, StatisticsParams::GetClassType()));
        VAPoR::DataMgr *         currentDmgr = _controlExec->GetDataStatus()->GetDataMgr(dsName);
        std::vector<std::string> availVars3D = currentDmgr->GetDataVarNames(3);
        availVars3D.clear();    // This is a temporary change to hide all 3D variables.
                                //   Removing this line could expose 3D variables again.

        file << "Data Source = " << guiParams->GetStatsDatasetName() << endl << endl;
        file << "#Variable Statistics:" << endl;
        file << "Variable_Name, No_of_Samples";
        if (statsParams->GetMinEnabled()) file << ", Min";
        if (statsParams->GetMaxEnabled()) file << ", Max";
        if (statsParams->GetMeanEnabled()) file << ", Mean";
        if (statsParams->GetMedianEnabled()) file << ", Median";
        if (statsParams->GetStdDevEnabled()) file << ", Stddev";
        file << endl;

        bool has3DVar = false;
        for (int i = 0; i < _validStats.GetVariableCount(); i++) {
            std::string varname = _validStats.GetVariableName(i);
            float       m3[3], median, stddev;
            long        count;
            _validStats.Get3MStats(varname, m3);
            _validStats.GetMedian(varname, &median);
            _validStats.GetStddev(varname, &stddev);
            _validStats.GetCount(varname, &count);
            file << varname << ", " << count;
            if (statsParams->GetMinEnabled()) file << ", " << m3[0];
            if (statsParams->GetMaxEnabled()) file << ", " << m3[1];
            if (statsParams->GetMeanEnabled()) file << ", " << m3[2];
            if (statsParams->GetMedianEnabled()) file << ", " << median;
            if (statsParams->GetStdDevEnabled()) file << ", " << stddev;
            file << endl;

            for (int j = 0; j < availVars3D.size(); j++)
                if (availVars3D[j] == varname) {
                    has3DVar = true;
                    break;
                }
        }

        file << endl;

        std::vector<double> myMin, myMax;
        statsParams->GetBox()->GetExtents(myMin, myMax);

        file << "#Spatial Extents:" << endl;
        file << "X min = " << myMin[0] << endl;
        file << "X max = " << myMax[0] << endl;
        file << "Y min = " << myMin[1] << endl;
        file << "Y max = " << myMax[1] << endl;
        if (has3DVar) {
            file << "Z min = " << myMin[2] << endl;
            file << "Z max = " << myMax[2] << endl;
        }
        file << endl;

        file << "#Temporal Extents:" << endl;
        file << "Minimum Timestep = " << statsParams->GetCurrentTimestep() << endl;
        file << "Maximum Timestep = " << statsParams->GetCurrentMaxTS() << endl;
        file << endl;

        file << "#Compression Parameters:" << endl;
        file << "Level of Detail  =  " << MyFidelityWidget->GetCurrentLodString() << endl;
        file << "Refinement Level = " << MyFidelityWidget->GetCurrentMultiresString() << endl;

        file.close();
    }
}
