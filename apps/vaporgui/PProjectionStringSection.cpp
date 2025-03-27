#include "PProjectionStringSection.h"

#include <QLabel>
#include <QPlainTextEdit>
#include "VSection.h"
#include "VLineItem.h"
#include "VComboBox.h"
#include "VPushButton.h"
#include "PDisplay.h"

#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include <vapor/ViewpointParams.h>
#include <vapor/GUIStateParams.h>
#include "VLabel.h"

using namespace VAPoR;

PProjectionStringSection::PProjectionStringSection(ControlExec *ce)
: PWidget("", new VSectionGroup("Data Projection",
                                {
                                    _currentProjDisp = (new PStringDisplay(GUIStateParams::m_proj4StringTag, "Current Projection String"))->Selectable(),
                                    new VLabel(""),
                                    new VLabel("Change Projection String"),
                                    new VLineItem("Change projection to", _datasetDropdown = new VComboBox),
                                    new VLineItem("Selected Projection String", _selectedProjDisp = new VLabel),
                                    _customStrEdit = new QPlainTextEdit,
                                    _applyButton = new VPushButton("Apply"),
                                })),
  _ce(ce)
{
    _selectedProjDisp->MakeSelectable();
    connect(_datasetDropdown, &VComboBox::ValueChanged, this, &PProjectionStringSection::datasetDropdownChanged);
    connect(_applyButton, &VPushButton::ButtonClicked, this, &PProjectionStringSection::applyClicked);
}


void PProjectionStringSection::updateGUI() const
{
    ParamsMgr *pm = _ce->GetParamsMgr();
    auto       stateParams = ((GUIStateParams *)pm->GetParams(GUIStateParams::GetClassType()));
    auto       activeViz = stateParams->GetActiveVizName();

    DataStatus *   dataStatus = _ce->GetDataStatus();
    vector<string> datasets = dataStatus->GetDataMgrNames();
    vector<string> datasetProjStrs;
    string         currentProjStr = dataStatus->GetMapProjection();

    for (auto &dataset : datasets) datasetProjStrs.push_back(dataStatus->GetMapProjectionDefault(dataset));

    datasets.push_back("Custom");
    _datasetDropdown->SetOptions(datasets);

    _datasetDropdown->SetValue("Custom");
    if (_changeToDataset.empty()) {
        for (int i = 0; i < datasetProjStrs.size(); i++)
            if (datasetProjStrs[i] == currentProjStr) _datasetDropdown->SetIndex(i);
    } else {
        _datasetDropdown->SetValue(_changeToDataset);
    }

    _currentProjDisp->Update(stateParams);

    auto selected = _datasetDropdown->GetValue();
    if (selected == "Custom") {
        _customStrEdit->setVisible(true);
        _selectedProjDisp->setVisible(false);
    } else {
        auto proj = dataStatus->GetMapProjectionDefault(selected);
        _selectedProjDisp->SetText(proj.size() ? proj : "<empty>");
        _customStrEdit->setVisible(false);
        _selectedProjDisp->setVisible(true);
    }
}


void PProjectionStringSection::datasetDropdownChanged(std::string value)
{
    _changeToDataset = value;
    updateGUI();
}


void PProjectionStringSection::applyClicked()
{
    DataStatus * dataStatus = _ce->GetDataStatus();
    const string selected = _datasetDropdown->GetValue();
    string       proj;
    if (selected == "Custom") {
        proj = _customStrEdit->toPlainText().toStdString();
    } else {
        proj = dataStatus->GetMapProjectionDefault(selected);
    }

    ParamsMgr *pm = _ce->GetParamsMgr();
    auto       p = ((GUIStateParams *)pm->GetParams(GUIStateParams::GetClassType()));
    if (proj == p->GetProjectionString()) return;

    p->SetProjectionString(proj);
}

void PProjectionStringSection::closeEvent(QCloseEvent *event) {
    emit closed();
    QWidget::closeEvent(event);
}
