//************************************************************************
//                                                                      *
//           Copyright (C)  2016                                        *
//     University Corporation for Atmospheric Research                  *
//           All Rights Reserved                                        *
//                                                                      *
//************************************************************************/
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       January 2018
//

#include "GUIStateParams.h"
#include <vapor/GetAppPath.h>
#include <vapor/DataMgrUtils.h>
#include "Plot.h"

#define NPY_NO_DEPRECATED_API NPY_1_8_API_VERSION
#include <numpy/ndarrayobject.h>

// Constructor
Plot::Plot(VAPoR::DataStatus *status, VAPoR::ParamsMgr *manager, QWidget *parent)
{
    _dataStatus = status;
    _paramsMgr = manager;

    // Get the active dataset name
    std::string              currentDatasetName;
    std::vector<std::string> dmNames = _dataStatus->GetDataMgrNames();
    if (dmNames.empty()) {
        std::cerr << "No data set chosen yet. Plot shouldn't run into this condition." << std::endl;
    } else {
        GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
        currentDatasetName = guiParams->GetPlotDatasetName();
        if (currentDatasetName == "" || currentDatasetName == "NULL")    // not initialized yet
        {
            currentDatasetName = dmNames[0];
            guiParams->SetPlotDatasetName(currentDatasetName);
        }
    }

    VAPoR::DataMgr *currentDmgr = _dataStatus->GetDataMgr(currentDatasetName);
    PlotParams *    plotParams = dynamic_cast<PlotParams *>(_paramsMgr->GetAppRenderParams(currentDatasetName, PlotParams::GetClassType()));

    // Do some static QT stuff
    setupUi(this);
    setWindowTitle("Plot Utility");
    myFidelityWidget->Reinit(FidelityWidget::AUXILIARY);
    spaceTimeTab->setCurrentIndex(0);      // default to load space tab
    plotParams->SetSpaceTimeMode(true);    //

    timeTabSinglePoint->SetMainLabel(QString::fromAscii("Select one data point in space:"));
    timeTabTimeRange->SetMainLabel(QString::fromAscii("Select the minimum and maximum time steps:"));
    timeTabTimeRange->SetDecimals(0);

    spaceTabP1->SetMainLabel(QString::fromAscii("Select spatial location of Point 1"));
    spaceTabP2->SetMainLabel(QString::fromAscii("Select spatial location of Point 2"));
    spaceTabTimeSelector->SetText(QString::fromAscii("T"));

    // Connect signals with slots
    connect(newVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_newVarChanged(int)));
    connect(removeVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_removeVarChanged(int)));
    connect(dataMgrCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_dataSourceChanged(int)));
    connect(spaceTimeTab, SIGNAL(currentChanged(int)), this, SLOT(_spaceTimeModeChanged(int)));
    connect(timeTabSinglePoint, SIGNAL(pointUpdated()), this, SLOT(_timeModePointChanged()));
    connect(timeTabTimeRange, SIGNAL(rangeChanged()), this, SLOT(_timeModeT1T2Changed()));

    // Put the current window on top
    show();
    raise();
    activateWindow();
}

// Destructor
Plot::~Plot()
{
    _dataStatus = NULL;
    _paramsMgr = NULL;
}

void Plot::Update()
{
    // Initialize pointers
    std::vector<std::string> dmNames = _dataStatus->GetDataMgrNames();
    if (dmNames.empty()) { this->close(); }
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    std::string     currentDatasetName = guiParams->GetPlotDatasetName();
    assert(currentDatasetName != "" && currentDatasetName != "NULL");
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
        guiParams->SetPlotDatasetName(currentDatasetName);
    }
    VAPoR::DataMgr *         currentDmgr = _dataStatus->GetDataMgr(currentDatasetName);
    PlotParams *             plotParams = dynamic_cast<PlotParams *>(_paramsMgr->GetAppRenderParams(currentDatasetName, PlotParams::GetClassType()));
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();

    // Update DataMgrCombo
    dataMgrCombo->blockSignals(true);
    dataMgrCombo->clear();
    for (int i = 0; i < dmNames.size(); i++) dataMgrCombo->addItem(QString::fromStdString(dmNames[i]));
    dataMgrCombo->setCurrentIndex(currentIdx);
    dataMgrCombo->blockSignals(false);

    // Update "Add a Variable"
    std::vector<std::string> availVars = currentDmgr->GetDataVarNames(2, true);
    std::vector<std::string> availVars3D = currentDmgr->GetDataVarNames(3, true);
    for (int i = 0; i < availVars3D.size(); i++) availVars.push_back(availVars3D[i]);
    for (int i = 0; i < enabledVars.size(); i++)
        for (int rmIdx = 0; rmIdx < availVars.size(); rmIdx++)
            if (availVars[rmIdx] == enabledVars[i]) {
                availVars.erase(availVars.begin() + rmIdx);
                break;
            }
    std::sort(availVars.begin(), availVars.end());
    newVarCombo->blockSignals(true);
    newVarCombo->clear();
    newVarCombo->addItem(QString::fromAscii("Add a Variable"));
    for (std::vector<std::string>::iterator it = availVars.begin(); it != availVars.end(); ++it) newVarCombo->addItem(QString::fromStdString(*it));
    newVarCombo->setCurrentIndex(0);
    newVarCombo->blockSignals(false);

    // Update "Remove a Variable"
    std::sort(enabledVars.begin(), enabledVars.end());
    removeVarCombo->blockSignals(true);
    removeVarCombo->clear();
    removeVarCombo->addItem(QString::fromAscii("Remove a Variable"));
    for (int i = 0; i < enabledVars.size(); i++) removeVarCombo->addItem(QString::fromStdString(enabledVars[i]));
    removeVarCombo->setCurrentIndex(0);
    removeVarCombo->blockSignals(false);

    // Update "Variable Table"
    variablesTable->clear();    // This also deletes the items properly.
    QStringList header;         // Start from the header
    header << "Enabled Variables";
    variablesTable->setColumnCount(header.size());
    variablesTable->setHorizontalHeaderLabels(header);
    variablesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    variablesTable->horizontalHeader()->setFixedHeight(30);
    variablesTable->verticalHeader()->setFixedWidth(30);

    variablesTable->setRowCount(enabledVars.size());    // Then work on the cells
    for (int row = 0; row < enabledVars.size(); row++) {
        QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(enabledVars[row]));
        item->setFlags(Qt::NoItemFlags);
        item->setTextAlignment(Qt::AlignCenter);
        variablesTable->setItem(row, 0, item);
    }
    variablesTable->update();
    variablesTable->repaint();
    variablesTable->viewport()->update();

    // Update LOD, Refinement
    myFidelityWidget->Update(currentDmgr, _paramsMgr, plotParams);

    // Update widgets
    _setWidgetExtents();
}

void Plot::_newVarChanged(int index)
{
    if (index == 0) return;

    std::string varName = newVarCombo->itemText(index).toStdString();

    // Add this variable to parameter
    PlotParams *             plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *         dataMgr = this->_getCurrentDataMgr();
    std::vector<std::string> vars = plotParams->GetAuxVariableNames();
    vars.push_back(varName);
    plotParams->SetAuxVariableNames(vars);

    // Find out if there are 3D variables.
    std::vector<double> min, max;
    std::vector<int>    axes;
    VAPoR::DataMgrUtils::GetExtents(dataMgr, 0, vars, min, max, axes);
    assert(axes.size() == 2 || axes.size() == 3);
    timeTabSinglePoint->SetDimensionality(axes.size());
    spaceTabP1->SetDimensionality(axes.size());
    spaceTabP2->SetDimensionality(axes.size());
}

void Plot::_removeVarChanged(int index)
{
    if (index == 0) return;

    std::string     varName = removeVarCombo->itemText(index).toStdString();
    PlotParams *    plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *dataMgr = this->_getCurrentDataMgr();

    // Remove this variable from parameter
    std::vector<std::string> vars = plotParams->GetAuxVariableNames();
    int                      rmIdx = -1;
    for (int i = 0; i < vars.size(); i++)
        if (vars[i] == varName) {
            rmIdx = i;
            break;
        }
    assert(rmIdx != -1);
    vars.erase(vars.begin() + rmIdx);
    plotParams->SetAuxVariableNames(vars);

    // Find out if there are 3D variables.
    std::vector<double> min, max;
    std::vector<int>    axes;
    VAPoR::DataMgrUtils::GetExtents(dataMgr, 0, vars, min, max, axes);
    assert(axes.size() == 2 || axes.size() == 3);
    timeTabSinglePoint->SetDimensionality(axes.size());
    spaceTabP1->SetDimensionality(axes.size());
    spaceTabP2->SetDimensionality(axes.size());
}

void Plot::_plotClicked() {}

void Plot::_spaceTimeModeChanged(int mode)
{
    PlotParams *plotParams = _getCurrentPlotParams();
    if (mode == 0)
        plotParams->SetSpaceTimeMode(true);
    else if (mode == 1)
        plotParams->SetSpaceTimeMode(false);
    else
        std::cerr << "Plot: spaceTimeTab value not known!" << std::endl;
}

void Plot::_spaceModeP1P2Changed() {}

void Plot::_spaceModeTimeChanged() {}

void Plot::_timeModePointChanged()
{
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    assert(!plotParams->GetSpaceTimeMode());

    std::vector<double> currentPoint;
    timeTabSinglePoint->GetCurrentPoint(currentPoint);

    plotParams->SetSinglePoint(currentPoint);
}

void Plot::_timeModeT1T2Changed()
{
    VAPoR::PlotParams *plotParams = this->_getCurrentPlotParams();
    assert(!plotParams->GetSpaceTimeMode());

    std::vector<double> range;
    timeTabTimeRange->GetRange(range);
    assert(range.size() == 2);
    std::vector<long int> rangeInt;
    rangeInt.push_back((long int)range[0]);
    rangeInt.push_back((long int)range[1]);

    plotParams->SetMinMaxTS(rangeInt);
}

void Plot::_fidelityChanged() {}

void Plot::_dataSourceChanged(int index)
{
    std::string newDataSourceName = dataMgrCombo->itemText(index).toStdString();

    // Inform GUIStateParams the change of data source.
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));

    guiParams->SetPlotDatasetName(newDataSourceName);
}

VAPoR::PlotParams *Plot::_getCurrentPlotParams() const
{
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    std::string     dsName = guiParams->GetPlotDatasetName();
    return (dynamic_cast<PlotParams *>(_paramsMgr->GetAppRenderParams(dsName, PlotParams::GetClassType())));
}

VAPoR::DataMgr *Plot::_getCurrentDataMgr() const
{
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    std::string     currentDatasetName = guiParams->GetPlotDatasetName();
    assert(currentDatasetName != "" && currentDatasetName != "NULL");

    return (_dataStatus->GetDataMgr(currentDatasetName));
}

void Plot::_setWidgetExtents()
{
    VAPoR::PlotParams *      plotParams = this->_getCurrentPlotParams();
    VAPoR::DataMgr *         dataMgr = this->_getCurrentDataMgr();
    size_t                   ts = plotParams->GetCurrentTimestep();
    std::vector<std::string> allVars = plotParams->GetAuxVariableNames();
    std::vector<double>      minFullExtents, maxFullExtents;
    if (!allVars.empty()) {
        std::vector<int> axes;
        VAPoR::DataMgrUtils::GetExtents(dataMgr, ts, allVars, minFullExtents, maxFullExtents, axes);
        timeTabSinglePoint->SetExtents(minFullExtents, maxFullExtents);
        spaceTabP1->SetExtents(minFullExtents, maxFullExtents);
        spaceTabP2->SetExtents(minFullExtents, maxFullExtents);
    }

    int numOfTimeSteps = dataMgr->GetNumTimeSteps();
    timeTabTimeRange->SetExtents(0.0, (double)(numOfTimeSteps - 1));
    timeTabTimeRange->SetDecimals(0);
    spaceTabTimeSelector->SetExtents(0.0, (double)(numOfTimeSteps - 1));
    spaceTabTimeSelector->SetDecimals(0);
}
