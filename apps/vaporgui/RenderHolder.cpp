
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
#include <vapor/SettingsParams.h>
#include <vapor/VolumeRenderer.h>
#include <vapor/VolumeIsoRenderer.h>
#include <vapor/DataStatus.h>

using namespace VAPoR;

namespace {
const string DuplicateInStr = "Duplicate in:";
};

CBWidget::CBWidget(QWidget *parent, QString text) : QWidget(parent), QTableWidgetItem(text){};

NewRendererDialog::NewRendererDialog(QWidget *parent, std::vector<string> rendererNames, std::vector<string> descriptions, std::vector<string> iconPaths, std::vector<string> smallIconPaths,
                                     std::vector<bool> dim2DSupport, std::vector<bool> dim3DSupport, vector<bool> particleSupport)
: QDialog(parent), Ui_NewRendererDialog()
{
    setupUi(this);

    _rendererNames = rendererNames;
    _descriptions = descriptions;
    _iconPaths = iconPaths;
    _smallIconPaths = smallIconPaths;
    _dim2DSupport = dim2DSupport;
    _dim3DSupport = dim3DSupport;
    _particleSupport = particleSupport;

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

    bool has2D = dm->GetDataVarNames(2, DataMgr::VarType::Scalar).size();
    bool has3D = dm->GetDataVarNames(3, DataMgr::VarType::Scalar).size();
    bool hasParticle = dm->GetDataVarNames(3, DataMgr::VarType::Particle).size();

    for (int i = 0; i < _buttons.size(); i++) {
        if ((has2D && _dim2DSupport[i]) || (has3D && _dim3DSupport[i]) || (hasParticle && _particleSupport[i])) {
            _buttons[i]->setEnabled(true);
            _buttons[i]->setToolTip("");
        } else {
            _buttons[i]->setEnabled(false);
            _buttons[i]->setToolTip(QString::fromStdString("Dataset \"" + datasetName + "\" does not have data supported by this renderer"));
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

void RenderHolder::_showIntelDriverWarning(ControlExec *_controlExec, const string &rendererType)
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

void RenderHolder::_newRendererDialogAccepted(ControlExec *_controlExec, NewRendererDialog *_newRendererDialog)
{
    ParamsMgr *    paramsMgr = _controlExec->GetParamsMgr();
    vector<string> dataSetNames = _controlExec->GetDataStatus()->GetDataMgrNames();

    string rendererType = _newRendererDialog->GetSelectedRenderer();
    _showIntelDriverWarning(_controlExec, rendererType);

    int    selection = _newRendererDialog->dataMgrCombo->currentIndex();
    string dataSetName = dataSetNames[selection];

    GUIStateParams *p = (GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    string          activeViz = p->GetActiveVizName();

    // figure out the name
    //
    QString qname = _newRendererDialog->rendererNameEdit->text();
    string  rendererName = qname.toStdString();

    // Check that it's not all blanks:
    //
    if (rendererName.find_first_not_of(' ') == string::npos) { rendererName = rendererType; }

    rendererName = _controlExec->MakeRendererNameUnique(rendererName);
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

//    Update();

//    emit newRendererSignal(activeViz, rendererType, rendererName);
    paramsMgr->EndSaveStateGroup();
}