//************************************************************************
//                                                                      *
//           Copyright (C)  2016                                        *
//     University Corporation for Atmospheric Research                  *
//           All Rights Reserved                                        *
//                                                                      *
//************************************************************************/
//
//  File:       plot.h
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       January 2018
//

#ifdef WIN32
    #pragma warning(disable : 4100)
#endif

#include "GUIStateParams.h"
#include <vapor/MyPython.h>
#include <vapor/GetAppPath.h>
#include "Plot.h"

#define NPY_NO_DEPRECATED_API NPY_1_8_API_VERSION
#include <numpy/ndarrayobject.h>

// Constructor
Plot::Plot(QWidget *parent)
{
    _controlExec = NULL;

    setupUi(this);
    setWindowTitle("Plot Utility");
    P1P2Widget->setTabText(0, QString::fromAscii("Point 1 Position"));
    P1P2Widget->setTabText(1, QString::fromAscii("Point 2 Position"));
    P1Widget->Reinit(GeometryWidget::THREED, GeometryWidget::SINGLEPOINT, GeometryWidget::AUXILIARY);
    P2Widget->Reinit(GeometryWidget::THREED, GeometryWidget::SINGLEPOINT, GeometryWidget::AUXILIARY);
    P1Widget->hideSinglePointTabHeader();
    P2Widget->hideSinglePointTabHeader();

    MyFidelityWidget->Reinit(FidelityWidget::AUXILIARY);

    Connect();
}

// Destructor
Plot::~Plot() {}

int Plot::initControlExec(VAPoR::ControlExec *ce)
{
    if (ce != NULL)
        _controlExec = ce;
    else
        return -1;

    // Store the active dataset name
    std::vector<std::string> dmNames = _controlExec->getDataStatus()->GetDataMgrNames();
    if (dmNames.empty())
        return -1;
    else {
        GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
        std::string     dsName = guiParams->GetStatsDatasetName();
        if (dsName == "" || dsName == "NULL")    // not initialized yet
            guiParams->SetPlotDatasetName(dmNames[0]);
    }

    return 0;
}

void Plot::showMe()
{
    show();
    raise();
    activateWindow();
}

void Plot::Update()
{
    // Initialize pointers
    VAPoR::DataStatus *      dataStatus = _controlExec->getDataStatus();
    std::vector<std::string> dmNames = dataStatus->GetDataMgrNames();
    if (dmNames.empty()) { this->close(); }
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
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
    VAPoR::DataMgr *         currentDmgr = dataStatus->GetDataMgr(currentDatasetName);
    PlotParams *             plotParams = dynamic_cast<PlotParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(currentDatasetName, PlotParams::GetClassType()));
    std::vector<std::string> enabledVars = plotParams->GetAuxVariableNames();

    // Update DataMgrCombo
    DataMgrCombo->blockSignals(true);
    DataMgrCombo->clear();
    for (int i = 0; i < dmNames.size(); i++) DataMgrCombo->addItem(QString::fromStdString(dmNames[i]));
    DataMgrCombo->setCurrentIndex(currentIdx);
    DataMgrCombo->blockSignals(false);

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
    NewVarCombo->blockSignals(true);
    NewVarCombo->clear();
    NewVarCombo->addItem(QString::fromAscii("Add a Variable"));
    for (std::vector<std::string>::iterator it = availVars.begin(); it != availVars.end(); ++it) NewVarCombo->addItem(QString::fromStdString(*it));
    NewVarCombo->setCurrentIndex(0);
    NewVarCombo->blockSignals(false);

    // Update "Remove a Variable"
    std::sort(enabledVars.begin(), enabledVars.end());
    RemoveVarCombo->blockSignals(true);
    RemoveVarCombo->clear();
    RemoveVarCombo->addItem(QString::fromAscii("Remove a Variable"));
    for (int i = 0; i < enabledVars.size(); i++) RemoveVarCombo->addItem(QString::fromStdString(enabledVars[i]));
    RemoveVarCombo->setCurrentIndex(0);
    RemoveVarCombo->blockSignals(false);

    // Update "Variable Table"
    VariablesTable->clear();    // This also deletes the items properly.
    QStringList header;         // Start from the header
    header << "Enabled Variables";
    VariablesTable->setColumnCount(header.size());
    VariablesTable->setHorizontalHeaderLabels(header);
    VariablesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    VariablesTable->horizontalHeader()->setFixedHeight(30);
    VariablesTable->verticalHeader()->setFixedWidth(30);

    VariablesTable->setRowCount(enabledVars.size());    // Then work on the cells
    for (int row = 0; row < enabledVars.size(); row++) {
        QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(enabledVars[row]));
        item->setFlags(Qt::NoItemFlags);
        item->setTextAlignment(Qt::AlignCenter);
        VariablesTable->setItem(row, 0, item);
    }
    VariablesTable->update();
    VariablesTable->repaint();
    VariablesTable->viewport()->update();

    // Update LOD, Refinement
    MyFidelityWidget->Update(currentDmgr, _controlExec->GetParamsMgr(), plotParams);

    // Update geometry extents
    P1Widget->Update(_controlExec->GetParamsMgr(), currentDmgr, plotParams);
    P2Widget->Update(_controlExec->GetParamsMgr(), currentDmgr, plotParams);
}

void Plot::Connect()
{
    connect(NewVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_newVarChanged(int)));
    connect(RemoveVarCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_removeVarChanged(int)));
}

void Plot::_newVarChanged(int index)
{
    assert(index > 0);

    // Initialize pointers
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string     dsName = guiParams->GetPlotDatasetName();
    PlotParams *    plotParams = dynamic_cast<PlotParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, PlotParams::GetClassType()));
    std::string     varName = NewVarCombo->itemText(index).toStdString();

    // Add this variable to parameter
    std::vector<std::string> vars = plotParams->GetAuxVariableNames();
    vars.push_back(varName);
    plotParams->SetAuxVariableNames(vars);
}

void Plot::_removeVarChanged(int index)
{
    assert(index > 0);

    // Initialize pointers
    GUIStateParams *guiParams = dynamic_cast<GUIStateParams *>(_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string     dsName = guiParams->GetPlotDatasetName();
    PlotParams *    plotParams = dynamic_cast<PlotParams *>(_controlExec->GetParamsMgr()->GetAppRenderParams(dsName, PlotParams::GetClassType()));
    std::string     varName = RemoveVarCombo->itemText(index).toStdString();

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
}

void Plot::_dataSourceChanged(int) {}

void Plot::_plotClicked() {}

void Plot::_spaceTimeModeChanged(bool) {}

void Plot::_spaceModeP1P2Changed() {}

void Plot::_spaceModeTimeChanged() {}

void Plot::_timeModePointChanged() {}

void Plot::_timeModeT1T2Changed() {}

void Plot::_fidelityChanged() {}
