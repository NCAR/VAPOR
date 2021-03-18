
//															*
//			 Copyright (C)  2014										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		RenderHolder.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		October 2014
//
//	Description:	Implements the RenderHolder class

#include "vapor/VAssert.h"
#include <qcombobox.h>
#include <QStringList>
#include <QTableWidget>
#include <QCheckBox>
#include <QList>
#include <qpushbutton.h>
#include <sstream>
#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include "qdialog.h"
#include "ui_NewRendererDialog.h"
#include "VizSelectCombo.h"
#include "ErrorReporter.h"
#include "RenderHolder.h"
#include "QPushButtonWithDoubleClick.h"
#include <SettingsParams.h>
#include <vapor/VolumeRenderer.h>
#include <vapor/VolumeIsoRenderer.h>
#include <vapor/DataStatus.h>

using namespace VAPoR;

namespace {
const string DuplicateInStr = "Duplicate in:";
};

CBWidget::CBWidget(QWidget *parent, QString text) : QWidget(parent), QTableWidgetItem(text){};

NewRendererDialog::NewRendererDialog(QWidget *parent, std::vector<string> rendererNames, std::vector<string> descriptions, std::vector<string> iconPaths, std::vector<string> smallIconPaths,
                                     std::vector<bool> dim2DSupport, std::vector<bool> dim3DSupport)
: QDialog(parent), Ui_NewRendererDialog()
{
    setupUi(this);

    _rendererNames = rendererNames;
    _descriptions = descriptions;
    _iconPaths = iconPaths;
    _smallIconPaths = smallIconPaths;
    _dim2DSupport = dim2DSupport;
    _dim3DSupport = dim3DSupport;

    rendererNameEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9_]{1,64}")));
    dataMgrCombo->clear();

    _createButtons();

    connect(dataMgrCombo, SIGNAL(activated(int)), this, SLOT(_showRelevantRenderers()));
};

void NewRendererDialog::_createButtons()
{
    int size = _rendererNames.size();
    _buttons.clear();
    for (int i = 0; i < size; i++) {
        QString      iconPath = QString::fromStdString(_smallIconPaths[i]);
        QIcon        icon(iconPath);
        QString      name = QString::fromStdString(_rendererNames[i]);
        QPushButton *button = _createButton(icon, name, i);

        buttonHolderGridLayout->addWidget(button, i, 0);
        _buttons.push_back(button);

        QPixmap thumbnail(_smallIconPaths[i].c_str());
        QLabel *label = new QLabel();
        label->setPixmap(thumbnail);
        label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        buttonHolderGridLayout->addWidget(label, i, 1);

        if (i == 0) button->setChecked(true);
    }
}

QPushButton *NewRendererDialog::_createButton(QIcon icon, QString name, int index)
{
    QPushButton *button = new QPushButtonWithDoubleClick(name, this);
    button->setIconSize(QSize(50, 50));
    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    button->setLayoutDirection(Qt::RightToLeft);
    button->setCheckable(true);
    button->setProperty("index", index);

    connect(button, SIGNAL(toggled(bool)), this, SLOT(_buttonChecked()));
    connect(button, SIGNAL(doubleClicked()), this, SLOT(_buttonDoubleClicked()));
    return button;
}

void NewRendererDialog::InitializeDataSources(VAPoR::DataStatus *dataStatus)
{
    _dataStatus = dataStatus;
    const vector<string> datasets = dataStatus->GetDataMgrNames();

    dataMgrCombo->blockSignals(true);
    dataMgrCombo->clear();
    for (int i = 0; i < datasets.size(); i++) { dataMgrCombo->addItem(QString::fromStdString(datasets[i])); }
    dataMgrCombo->blockSignals(false);

    _showRelevantRenderers();
}

void NewRendererDialog::_showRelevantRenderers()
{
    const string datasetName = dataMgrCombo->currentText().toStdString();
    DataMgr *    dm = _dataStatus->GetDataMgr(datasetName);
    VAssert(dm);

    bool has2D = dm->GetDataVarNames(2).size();
    bool has3D = dm->GetDataVarNames(3).size();

    for (int i = 0; i < _buttons.size(); i++) {
        if ((has2D && _dim2DSupport[i]) || (has3D && _dim3DSupport[i])) {
            _buttons[i]->setEnabled(true);
            _buttons[i]->setToolTip("");
        } else {
            _buttons[i]->setEnabled(false);
            _buttons[i]->setToolTip(QString::fromStdString("Dataset \"" + datasetName + "\" has no " + (has2D ? "3D" : "2D") + " data"));
        }
    }
    _selectFirstValidRenderer();
}

void NewRendererDialog::_selectFirstValidRenderer()
{
    for (int i = 0; i < _buttons.size(); i++) {
        if (_buttons[i]->isEnabled()) {
            _buttons[i]->click();
            return;
        }
    }
}

void NewRendererDialog::_setUpImage(std::string imageName, QLabel *label)
{
    QPixmap thumbnail(imageName.c_str());
    label->setPixmap(thumbnail);
}

void NewRendererDialog::_buttonChecked()
{
    QPushButton *button = (QPushButton *)sender();
    int          index = button->property("index").toInt();

    _uncheckAllButtons();
    button->blockSignals(true);
    button->setChecked(true);
    button->blockSignals(false);

    string icon = _iconPaths[index];
    _setUpImage(icon, bigDisplay);

    QString title = "\n" + QString::fromStdString(_rendererNames[index]) + " Renderer";
    titleLabel->setText(title);

    QString description = QString::fromStdString(_descriptions[index]);
    descriptionLabel->setText(description);

    _selectedRenderer = _rendererNames[index];
}

void NewRendererDialog::_buttonDoubleClicked()
{
    _buttonChecked();
    this->accept();
}

void NewRendererDialog::_uncheckAllButtons()
{
    int count = buttonHolderGridLayout->count() / 2;
    for (int i = 0; i < count; i++) {
        QPushButton *button;
        button = (QPushButton *)buttonHolderGridLayout->itemAt(i * 2)->widget();
        button->blockSignals(true);
        button->setChecked(false);
        button->blockSignals(false);
    }
}

RenderHolder::RenderHolder(QWidget *parent, ControlExec *ce, const vector<QWidget *> &widgets, const vector<string> &widgetNames, const vector<string> &descriptions, const vector<string> &iconPaths,
                           const vector<string> &smallIconPaths, const vector<bool> &dim2DSupport, const vector<bool> &dim3DSupport)
: QWidget(parent), Ui_LeftPanel()
{
    VAssert(widgets.size() == widgetNames.size());
    VAssert(widgets.size() == iconPaths.size());
    VAssert(widgets.size() == smallIconPaths.size());

    setupUi(this);

    _controlExec = ce;
    //_newRendererDialog = new NewRendererDialog(this, ce);
    _newRendererDialog = new NewRendererDialog(this, widgetNames, descriptions, iconPaths, smallIconPaths, dim2DSupport, dim3DSupport);
    // Remove [X] button from title bar to fix bug #2184
    _newRendererDialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    _vaporTable = new VaporTable(tableWidget, false, true);
    _vaporTable->Reinit((VaporTable::ValidatorFlags)(0), (VaporTable::MutabilityFlags)(VaporTable::IMMUTABLE), (VaporTable::HighlightFlags)(VaporTable::ROWS));
    _vaporTable->ShowToolTips(true);
    _vaporTable->StretchToColumn(2);
    _vaporTable->HideColumn(1);
    _currentRow = 0;
    tableWidget->setWordWrap(false);

    _widgetNames = widgetNames;
    for (int i = 0; i < widgets.size(); i++) { stackedWidget->addWidget(widgets[i]); }
    stackedWidget->setCurrentIndex(0);

    _makeConnections();
    _initializeSplitter();
}

void RenderHolder::_makeConnections()
{
    connect(_vaporTable, SIGNAL(cellClicked(int, int)), this, SLOT(_activeRendererChanged(int, int)));
    connect(_vaporTable, SIGNAL(valueChanged(int, int)), this, SLOT(_tableValueChanged(int, int)));
    connect(newButton, SIGNAL(clicked()), this, SLOT(_showNewRendererDialog()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(_deleteRenderer()));
    connect(dupCombo, SIGNAL(activated(int)), this, SLOT(_copyInstanceTo(int)));
    connect(_newRendererDialog, &NewRendererDialog::accepted, this, &RenderHolder::_newRendererDialogAccepted);
}

void RenderHolder::_initializeSplitter()
{
    QList<int> proportions;
    int        topHeight = deleteButton->height() + newButton->height() + newButton->height();
    int        bottomHeight = height() - topHeight;
    proportions.append(topHeight);
    proportions.append(bottomHeight);
    mainSplitter->setSizes(proportions);
}

void RenderHolder::_initializeNewRendererDialog(vector<string> datasetNames) { _newRendererDialog->InitializeDataSources(_controlExec->GetDataStatus()); }

void RenderHolder::_showIntelDriverWarning(const string &rendererType)
{
    if (_controlExec->GetGPUVendor() != GLManager::Vendor::Intel) return;
    if (rendererType != VolumeRenderer::GetClassType() && rendererType != VolumeIsoRenderer::GetClassType()) return;

    ParamsMgr *     paramsMgr = _controlExec->GetParamsMgr();
    SettingsParams *sp = (SettingsParams *)paramsMgr->GetParams(SettingsParams::GetClassType());
    if (sp->GetDontShowIntelDriverWarning()) return;

    // Qt will automatically delete this for us apparently
    QCheckBox *dontShowAgain = new QCheckBox("Don't show again");
    dontShowAgain->blockSignals(true);

    QMessageBox warning;
    warning.setIcon(QMessageBox::Warning);
    warning.setText("Warning");
    warning.setInformativeText("Regular grid renderer is used by default. "
                               "If your data is non-regular, it can result in an image that misrepresents your data. "
                               "\n\n"
                               "To get correct results for non-regular data, select the curvilinear grid rendering algorithm. "
                               "This can be changed under the renderer's Apperance tab under the transfer function editor. "
                               "\n\n"
                               "Your computer uses an Intel GPU which has poor support for the curvilinear renderer. "
                               "It could potentially result in Vapor hanging or crashing. "
                               "In this case, we recommend you use a computer with an AMD or Nvidia GPU. ");
    warning.addButton(dontShowAgain, QMessageBox::ActionRole);
    warning.addButton(QMessageBox::Ok);
    warning.exec();

    if (dontShowAgain->checkState() == Qt::Checked) sp->SetDontShowIntelDriverWarning(true);
}

void RenderHolder::_showNewRendererDialog()
{
    ParamsMgr *    paramsMgr = _controlExec->GetParamsMgr();
    vector<string> dataSetNames = paramsMgr->GetDataMgrNames();

    _initializeNewRendererDialog(dataSetNames);
    _newRendererDialog->open();
}

void RenderHolder::_newRendererDialogAccepted()
{
    ParamsMgr *    paramsMgr = _controlExec->GetParamsMgr();
    vector<string> dataSetNames = paramsMgr->GetDataMgrNames();

    string rendererType = _newRendererDialog->GetSelectedRenderer();
    _showIntelDriverWarning(rendererType);

    int    selection = _newRendererDialog->dataMgrCombo->currentIndex();
    string dataSetName = dataSetNames[selection];

    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();

    // figure out the name
    //
    QString qname = _newRendererDialog->rendererNameEdit->text();
    string  rendererName = qname.toStdString();

    // Check that it's not all blanks:
    //
    if (rendererName.find_first_not_of(' ') == string::npos) { rendererName = rendererType; }

    rendererName = uniqueName(rendererName);
    qname = QString(rendererName.c_str());

    paramsMgr->BeginSaveStateGroup(_controlExec->GetActivateRendererUndoTag());

    int rc = _controlExec->ActivateRender(activeViz, dataSetName, rendererType, rendererName, false);
    if (rc < 0) {
        MSG_ERR("Can't create renderer");
        paramsMgr->EndSaveStateGroup();
        return;
    }

    // Save current instance to state
    //
    p->SetActiveRenderer(activeViz, rendererType, rendererName);

    Update();

    emit newRendererSignal(activeViz, rendererType, rendererName);
    paramsMgr->EndSaveStateGroup();
}

void RenderHolder::_deleteRenderer()
{
    // Check if there is anything to delete:
    //
    if (tableWidget->rowCount() == 0) { return; }

    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();

    // Get the currently selected renderer.
    //
    string rendererName, rendererType, dataSetName;

    rendererName = _getActiveRendererInst();
    int row = _getRow(rendererName);
    _getRowInfo(row, rendererName, rendererType, dataSetName);

    _controlExec->RemoveRenderer(activeViz, dataSetName, rendererType, rendererName, false);

    // Update will rebuild the TableWidget with the updated state
    //
    p->SetActiveRenderer(activeViz, "", "");
    Update();

    // Make the renderer in the first row the active renderer
    //
    _getRowInfo(0, rendererName, rendererType, dataSetName);
    if (rendererName != "" || rendererType != "" || dataSetName != "") {
        _activeRendererChanged(0, 0);
        emit activeChanged(activeViz, rendererType, rendererName);
    }

    _vaporTable->SetActiveRow(0);
}

void RenderHolder::_activeRendererChanged(int row, int col)
{
    _currentRow = row;
    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();
    string          rendererName = _vaporTable->GetValue(row, 0);
    string          rendererType = _vaporTable->GetValue(row, 1);
    p->SetActiveRenderer(activeViz, rendererType, rendererName);
    if (col != 3) _vaporTable->SetActiveRow(row);
    emit activeChanged(activeViz, rendererType, rendererName);
}

VAPoR::RenderParams *RenderHolder::_getRenderParamsFromCell(int row, int col)
{
    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();
    string          activeRenderClass, activeRenderInst;
    p->GetActiveRenderer(activeViz, activeRenderClass, activeRenderInst);

    string rendererName = _vaporTable->GetValue(row, 0);
    string rendererType = _vaporTable->GetValue(row, 1);
    string dataSetName = _vaporTable->GetValue(row, 2);

    RenderParams *rParams = _controlExec->GetRenderParams(activeViz, dataSetName, activeRenderClass, activeRenderInst);
    return rParams;
}

void RenderHolder::_changeRendererName(int row, int col)
{
    string text = _vaporTable->GetValue(row, col);
    string uniqueText = uniqueName(text);
}

void RenderHolder::_tableValueChanged(int row, int col)
{
    if (col == 0) {
        _changeRendererName(row, col);
        return;
    }

    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();
    string          rendererName = _vaporTable->GetValue(row, 0);
    string          rendererType = _vaporTable->GetValue(row, 1);
    string          dataSetName = _vaporTable->GetValue(row, 2);
    int             state = _vaporTable->GetValue(row, 3);

    int rc = _controlExec->ActivateRender(activeViz, dataSetName, rendererType, rendererName, state);
    if (rc < 0) {
        MSG_ERR("Can't activate renderer");
        return;
    }

    _activeRendererChanged(row, col);
}

void RenderHolder::_itemTextChange(QTableWidgetItem *item)
{
#ifdef VAPOR3_0_0_ALPHA
    int row = item->row();
    int col = item->column();
    if (col != 0) return;
    int viznum = _controlExec->GetActiveVizIndex();

    // avoid responding to item creation:
    if (InstanceParams::GetNumInstances(viznum) <= row) return;

    string        text = item->text().toStdString();
    RenderParams *rP = InstanceParams::GetRenderParamsInstance(viznum, row);
    if (stdtext == rP->GetRendererName()) return;
    string uniqueText = uniqueName(text);

    if (uniqueText != text) item->setText(QString(uniqueText.c_str()));

    rP->SetRendererName(uniqueText);
#endif
}

void RenderHolder::_copyInstanceTo(int item)
{
    if (item == 0) return;    // User has selected descriptor item "Duplicate in:"

    GUIStateParams *guiStateParams = _getStateParams();

    string vizName = dupCombo->itemText(item).toStdString();
    string activeViz = guiStateParams->GetActiveVizName();

    string activeRenderClass, activeRenderInst;
    guiStateParams->GetActiveRenderer(activeViz, activeRenderClass, activeRenderInst);

    string dataSetName, dummy1, dummy2;
    bool   status = _controlExec->RenderLookup(activeRenderInst, dummy1, dataSetName, dummy2);
    VAssert(status);

    RenderParams *rParams = _controlExec->GetRenderParams(activeViz, dataSetName, activeRenderClass, activeRenderInst);
    VAssert(rParams);

    string rendererName = uniqueName(activeRenderInst);

    int rc = _controlExec->ActivateRender(vizName, dataSetName, rParams, rendererName, false);
    if (rc < 0) {
        MSG_ERR("Can't create renderer");
        return;
    }

    // Save current instance to state
    //
    guiStateParams->SetActiveRenderer(activeViz, activeRenderClass, rendererName);

    Update();

    emit newRendererSignal(activeViz, activeRenderClass, rendererName);
}

std::string RenderHolder::uniqueName(std::string name)
{
    string newname = name;

    ParamsMgr *pm = _controlExec->GetParamsMgr();

    vector<string> allInstNames;

    // Get ALL of the renderer instance names defined
    //
    vector<string> vizNames = pm->GetVisualizerNames();
    for (int i = 0; i < vizNames.size(); i++) {
        vector<string> classNames = _controlExec->GetRenderClassNames(vizNames[i]);

        for (int j = 0; j < classNames.size(); j++) {
            vector<string> rendererNames = _controlExec->GetRenderInstances(vizNames[i], classNames[j]);

            allInstNames.insert(allInstNames.begin(), rendererNames.begin(), rendererNames.end());
        }
    }

    while (1) {
        bool match = false;
        for (int i = 0; i < allInstNames.size(); i++) {
            string usedName = allInstNames[i];

            if (newname != usedName) continue;

            match = true;

            // found a match.  Modify newname
            // If newname ends with a number, increase the number.
            // Otherwise just append _1
            //
            size_t lastnonint = newname.find_last_not_of("0123456789");
            if (lastnonint < newname.length() - 1) {
                // remove terminating int
                string endchars = newname.substr(lastnonint + 1);
                // Convert to int:
                int termInt = atoi(endchars.c_str());
                // int termInt = std::stoi(endchars);
                termInt++;
                // convert termInt to a string
                std::stringstream ss;
                ss << termInt;
                endchars = ss.str();
                newname.replace(lastnonint + 1, string::npos, endchars);
            } else {
                newname = newname + "_1";
            }
        }
        if (!match) break;
    }
    return newname;
}

string RenderHolder::_getActiveRendererClass()
{
    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();

    string activeRenderClass, activeRenderInst;
    p->GetActiveRenderer(activeViz, activeRenderClass, activeRenderInst);

    return activeRenderClass;
}

string RenderHolder::_getActiveRendererInst()
{
    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();

    string activeRenderClass, activeRenderInst;
    p->GetActiveRenderer(activeViz, activeRenderClass, activeRenderInst);

    return activeRenderInst;
}

void RenderHolder::_updateDupCombo()
{
    ParamsMgr *    pm = _controlExec->GetParamsMgr();
    vector<string> vizNames = pm->GetVisualizerNames();

    dupCombo->clear();
    dupCombo->addItem(QString::fromStdString(DuplicateInStr));

    string activeRenderClass = _getActiveRendererClass();
    string activeRenderInst = _getActiveRendererInst();
    if (activeRenderClass.empty() || activeRenderInst.empty()) return;

    for (int i = 0; i < vizNames.size(); i++) { dupCombo->addItem(QString::fromStdString(vizNames[i])); }

    dupCombo->setCurrentIndex(0);
}

void RenderHolder::_makeRendererTableHeaders(vector<string> &tableValues)
{
    tableValues.push_back("Name");
    tableValues.push_back("Type");
    tableValues.push_back("Data Set");
    tableValues.push_back("Enabled");
}

void RenderHolder::Update()
{
    std::vector<std::string> values;

    // Get active params from GUI state
    //
    GUIStateParams *p = _getStateParams();
    string          activeViz = p->GetActiveVizName();
    if (activeViz.empty()) return;

    string activeRenderClass, activeRenderInst;
    p->GetActiveRenderer(activeViz, activeRenderClass, activeRenderInst);

    // Get ALL of the renderer instance names defined for this visualizer
    //
    map<string, vector<string>> rendererNamesMap;
    vector<string>              classNames = _controlExec->GetRenderClassNames(activeViz);
    int                         numRows = 0;
    for (int i = 0; i < classNames.size(); i++) {
        vector<string> rendererNames = _controlExec->GetRenderInstances(activeViz, classNames[i]);
        rendererNamesMap[classNames[i]] = rendererNames;
        numRows += rendererNames.size();
    }

    vector<string> tableValues, rowHeader, colHeader;
    _makeRendererTableHeaders(colHeader);

    map<string, vector<string>>::iterator itr;
    for (itr = rendererNamesMap.begin(); itr != rendererNamesMap.end(); ++itr) {
        vector<string> rendererNames = itr->second;

        string className = itr->first;

        for (int i = 0; i < rendererNames.size(); i++) {
            string rendererName = rendererNames[i];

            string dataSetName, dummy1, dummy2;
            bool   status = _controlExec->RenderLookup(rendererName, dummy1, dataSetName, dummy2);
            VAssert(status);

            RenderParams *rParams = _controlExec->GetRenderParams(activeViz, dataSetName, className, rendererName);
            VAssert(rParams);

            string enabled = rParams->IsEnabled() ? "1" : "0";
            tableValues.push_back(rendererName);
            tableValues.push_back(className);
            tableValues.push_back(dataSetName);
            tableValues.push_back(enabled);
        }
    }

    _vaporTable->Update(numRows, 4, tableValues, rowHeader, colHeader);
    int row = _getRow(activeRenderInst);
    if (row >= 0)
        _vaporTable->SetActiveRow(row);
    else
        p->SetActiveRenderer(activeViz, "", "");

    _updateDupCombo();

    // If there are no rows, there are no renderers, so we now set
    // the current active renderer to be "empty"
    //
    if (numRows == 0) {
        p->SetActiveRenderer(activeViz, "", "");
        SetCurrentWidget("");
        stackedWidget->hide();
        deleteButton->setEnabled(false);
        dupCombo->setEnabled(false);
    } else {
        deleteButton->setEnabled(true);
        dupCombo->setEnabled(true);
    }
}

void RenderHolder::SetCurrentWidget(string name)
{
    int indx = -1;
    for (int i = 0; i < _widgetNames.size(); i++) {
        if (name == _widgetNames[i]) {
            indx = i;
            break;
        }
    }
    if (indx < 0) return;

    stackedWidget->setCurrentIndex(indx);
    stackedWidget->show();
}

int RenderHolder::_getRow(string renderInst) const
{
    int row = -1;
    int rowCount = _vaporTable->RowCount();
    for (int i = 0; i < rowCount; i++) {
        string tableValue = _vaporTable->GetStringValue(i, 0);
        if (renderInst != tableValue) continue;

        row = i;
    }

    return row;
}

void RenderHolder::_getRowInfo(int row, string &rendererName, string &rendererType, string &dataSetName) const
{
    rendererName.clear();
    rendererType.clear();

    if (tableWidget->rowCount() == 0) {
        rendererName = "";
        rendererType = "";
        dataSetName = "";
        return;
    }

    if (row == -1) row = _vaporTable->RowCount() - 1;

    rendererName = _vaporTable->GetStringValue(row, 0);
    rendererType = _vaporTable->GetStringValue(row, 1);
    dataSetName = _vaporTable->GetStringValue(row, 2);
}
