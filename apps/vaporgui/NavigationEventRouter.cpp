//************************************************************************
//															*
//			 Copyright (C)  2006										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		NavigationEventRouter.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the ViewpointsEventRouter class.
//		This class supports routing messages from the gui to the params
//		associated with the viewpoint tab
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif
#include <vapor/glutil.h>    // Must be included first!!!
#include <cstdio>
#include <qdesktopwidget.h>
#include <qrect.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <QTextEdit>

#include <qcombobox.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "NavigationEventRouter.h"
#include "vapor/ViewpointParams.h"
#include "vapor/ControlExecutive.h"
#include "ui_NavigationTab.h"
#include "VizWinMgr.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace VAPoR;

NavigationEventRouter::NavigationEventRouter(QWidget *parent, VizWinMgr *vizMgr, ControlExec *ce) : QWidget(parent), Ui_NavigationTab(), EventRouter(ce, ViewpointParams::GetClassType())
{
    setupUi(this);

    _vizMgr = vizMgr;

    // Not implemented
    //
    camPosLat->setEnabled(false);
    camPosLon->setEnabled(false);
    rotCenterLat->setEnabled(false);
    rotCenterLon->setEnabled(false);
    stereoCombo->setEnabled(false);
    latLonCheckbox->setEnabled(false);
    stereoSeparationEdit->setEnabled(false);
}

NavigationEventRouter::~NavigationEventRouter() {}

/**********************************************************
 * Whenever a new viztab is created it must be hooked up here
 ************************************************************/
void NavigationEventRouter::hookUpTab()
{
    // connect (stereoCombo, SIGNAL (activated(int)), this, SLOT (SetStereoMode(int)));
    // connect (latLonCheckbox, SIGNAL (toggled(bool)), this, SLOT(ToggleLatLon(bool)));

    connect(numLights, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos00, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos01, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos02, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos10, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos11, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos12, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos20, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos21, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightPos22, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));

    // Camera stuff
    //
    connect(viewDir0, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(viewDir1, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(viewDir2, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(upVec0, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(upVec1, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(upVec2, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(camPos0, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(camPos1, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(camPos2, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(rotCenter0, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(rotCenter1, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));
    connect(rotCenter2, SIGNAL(returnPressed()), this, SLOT(setCameraChanged()));

    connect(camPosLat, SIGNAL(returnPressed()), this, SLOT(setCameraLatLonChanged()));
    connect(camPosLon, SIGNAL(returnPressed()), this, SLOT(setCameraLatLonChanged()));
    connect(rotCenterLat, SIGNAL(returnPressed()), this, SLOT(setCameraLatLonChanged()));
    connect(rotCenterLon, SIGNAL(returnPressed()), this, SLOT(setCameraLatLonChanged()));

    connect(lightDiff0, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightDiff1, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightDiff2, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightSpec0, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightSpec1, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(lightSpec2, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(shininessEdit, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(ambientEdit, SIGNAL(returnPressed()), this, SLOT(setLightChanged()));
    connect(stereoSeparationEdit, SIGNAL(returnPressed()), this, SLOT(notImplemented()));
}

void NavigationEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("View Tab Overview", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting"));

    help.push_back(make_pair("Controlling the Viewpoint in VAPOR GUI", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting#ControlView"));

    help.push_back(make_pair("Viewpoint Settings", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting#ViewpointSettings"));

    help.push_back(make_pair("Lighting Settings", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting#LightingControl"));
}

/*********************************************************************************
 * Slots associated with ViewpointTab:
 *********************************************************************************/

void NavigationEventRouter::setCameraChanged()
{
    double posvec[3], dirvec[3], upvec[3], center[3];

    center[0] = rotCenter0->text().toFloat();
    center[1] = rotCenter1->text().toFloat();
    center[2] = rotCenter2->text().toFloat();

    posvec[0] = camPos0->text().toFloat();
    posvec[1] = camPos1->text().toFloat();
    posvec[2] = camPos2->text().toFloat();

    dirvec[0] = viewDir0->text().toFloat();
    dirvec[1] = viewDir1->text().toFloat();
    dirvec[2] = viewDir2->text().toFloat();

    upvec[0] = upVec0->text().toFloat();
    upvec[1] = upVec1->text().toFloat();
    upvec[2] = upVec2->text().toFloat();

    _vizMgr->SetTrackBall(posvec, dirvec, upvec, center, true);
}

void NavigationEventRouter::setCameraLatLonChanged() { cout << "Not implemented" << endl; }

void NavigationEventRouter::notImplemented() { cout << "Not implemented" << endl; }

void NavigationEventRouter::updateCameraChanged()
{
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    double posvec[3], dirvec[3], upvec[3], center[3];
    vpParams->GetCameraPos(posvec);
    vpParams->GetCameraViewDir(dirvec);
    vpParams->GetCameraUpVec(upvec);
    vpParams->GetRotationCenter(center);

    camPos0->setText(QString::number(posvec[0]));
    camPos1->setText(QString::number(posvec[1]));
    camPos2->setText(QString::number(posvec[2]));

    viewDir0->setText(QString::number(dirvec[0]));
    viewDir1->setText(QString::number(dirvec[1]));
    viewDir2->setText(QString::number(dirvec[2]));

    upVec0->setText(QString::number(upvec[0]));
    upVec1->setText(QString::number(upvec[1]));
    upVec2->setText(QString::number(upvec[2]));

    rotCenter0->setText(QString::number(center[0]));
    rotCenter1->setText(QString::number(center[1]));
    rotCenter2->setText(QString::number(center[2]));
}

void NavigationEventRouter::setLightChanged()
{
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    paramsMgr->BeginSaveStateGroup("Light changed");

    // Get the light directions from the gui:
    vpParams->setNumLights(numLights->text().toInt());
    vpParams->setLightDirection(0, 0, lightPos00->text().toFloat());
    vpParams->setLightDirection(0, 1, lightPos01->text().toFloat());
    vpParams->setLightDirection(0, 2, lightPos02->text().toFloat());
    vpParams->setLightDirection(1, 0, lightPos10->text().toFloat());
    vpParams->setLightDirection(1, 1, lightPos11->text().toFloat());
    vpParams->setLightDirection(1, 2, lightPos12->text().toFloat());
    vpParams->setLightDirection(2, 0, lightPos20->text().toFloat());
    vpParams->setLightDirection(2, 1, lightPos21->text().toFloat());
    vpParams->setLightDirection(2, 2, lightPos22->text().toFloat());
    // final component is 0 (for gl directional light)
    vpParams->setLightDirection(0, 3, 0.f);
    vpParams->setLightDirection(1, 3, 0.f);
    vpParams->setLightDirection(2, 3, 0.f);

    // get the lighting coefficients from the gui:
    vpParams->setAmbientCoeff(ambientEdit->text().toFloat());
    vpParams->setDiffuseCoeff(0, lightDiff0->text().toFloat());
    vpParams->setDiffuseCoeff(1, lightDiff1->text().toFloat());
    vpParams->setDiffuseCoeff(2, lightDiff2->text().toFloat());
    vpParams->setSpecularCoeff(0, lightSpec0->text().toFloat());
    vpParams->setSpecularCoeff(1, lightSpec1->text().toFloat());
    vpParams->setSpecularCoeff(2, lightSpec2->text().toFloat());
    vpParams->setExponent(shininessEdit->text().toInt());

    paramsMgr->EndSaveStateGroup();
}

void NavigationEventRouter::updateLightChanged()
{
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    lightPos00->setText(QString::number(vpParams->getLightDirection(0, 0)));
    lightPos01->setText(QString::number(vpParams->getLightDirection(0, 1)));
    lightPos02->setText(QString::number(vpParams->getLightDirection(0, 2)));

    lightPos10->setText(QString::number(vpParams->getLightDirection(1, 0)));
    lightPos11->setText(QString::number(vpParams->getLightDirection(1, 1)));
    lightPos12->setText(QString::number(vpParams->getLightDirection(1, 2)));

    lightPos20->setText(QString::number(vpParams->getLightDirection(2, 0)));
    lightPos21->setText(QString::number(vpParams->getLightDirection(2, 1)));
    lightPos22->setText(QString::number(vpParams->getLightDirection(2, 2)));

    ambientEdit->setText(QString::number(vpParams->getAmbientCoeff()));
    lightDiff0->setText(QString::number(vpParams->getDiffuseCoeff(0)));
    lightDiff1->setText(QString::number(vpParams->getDiffuseCoeff(1)));
    lightDiff2->setText(QString::number(vpParams->getDiffuseCoeff(2)));
    lightSpec0->setText(QString::number(vpParams->getSpecularCoeff(0)));
    lightSpec1->setText(QString::number(vpParams->getSpecularCoeff(1)));
    lightSpec2->setText(QString::number(vpParams->getSpecularCoeff(2)));
    shininessEdit->setText(QString::number(vpParams->getExponent()));

    int nLights = vpParams->getNumLights();
    numLights->setText(QString::number(nLights));

    // Enable light direction text boxes as needed:
    bool lightOn;
    lightOn = (nLights > 0);
    lightPos00->setEnabled(lightOn);
    lightPos01->setEnabled(lightOn);
    lightPos02->setEnabled(lightOn);
    lightSpec0->setEnabled(lightOn);
    lightDiff0->setEnabled(lightOn);
    shininessEdit->setEnabled(lightOn);
    ambientEdit->setEnabled(lightOn);
    lightOn = (nLights > 1);
    lightPos10->setEnabled(lightOn);
    lightPos11->setEnabled(lightOn);
    lightPos12->setEnabled(lightOn);
    lightSpec1->setEnabled(lightOn);
    lightDiff1->setEnabled(lightOn);
    lightOn = (nLights > 2);
    lightPos20->setEnabled(lightOn);
    lightPos21->setEnabled(lightOn);
    lightPos22->setEnabled(lightOn);
    lightSpec2->setEnabled(lightOn);
    lightDiff2->setEnabled(lightOn);
}

void NavigationEventRouter::updateTab() { _updateTab(); }

void NavigationEventRouter::updateTransforms()
{
    map<string, Transform *> transformMap;

    // build a list of transforms for each data set
    //
    ParamsMgr *    paramsMgr = _controlExec->GetParamsMgr();
    vector<string> winNames = paramsMgr->GetVisualizerNames();
    for (int i = 0; i < winNames.size(); i++) {
        ViewpointParams *vParams = paramsMgr->GetViewpointParams(winNames[i]);
        if (!vParams) continue;

        vector<string> names = paramsMgr->GetDataMgrNames();

        for (int j = 0; j < names.size(); j++) {
            Transform *t = vParams->GetTransform(names[j]);

            if (!t->IsOriginInitialized()) {
                size_t         ts = GetCurrentTimeStep();
                vector<double> minExts;
                vector<double> maxExts;
                vector<double> origin;
                DataStatus *   dataStatus = _controlExec->GetDataStatus();

                dataStatus->GetActiveExtents(paramsMgr, winNames[i], names[j], ts, minExts, maxExts);
                origin.resize(minExts.size());
                for (int k = 0; k < minExts.size(); k++) origin[k] = minExts[k] + (maxExts[k] - minExts[k]) * 0.5;

                bool enabled = paramsMgr->GetSaveStateEnabled();
                paramsMgr->SetSaveStateEnabled(false);
                t->SetOrigin(origin);
                paramsMgr->SetSaveStateEnabled(enabled);
            }
            transformMap[names[j]] = t;
        }
    }
    transformTable->Update(transformMap);
}

void NavigationEventRouter::updateProjections()
{
    DataStatus *   dataStatus = _controlExec->GetDataStatus();
    vector<string> dataMgrs = dataStatus->GetDataMgrNames();

    GUIStateParams *params = GetStateParams();
    string          currentProj = params->GetProjectionString();

    int    numDataMgrs = dataMgrs.size();
    string customProj = getCustomProjString();

    datasetProjectionTable->clear();
    datasetProjectionTable->setRowCount(numDataMgrs + 1);

    // Set up table with dataset projection strings
    //
    bool   usingCurrentProj;
    string dataSetName, projString;
    for (int i = 0; i < numDataMgrs; i++) {
        dataSetName = dataMgrs[i];
        projString = dataStatus->GetMapProjectionDefault(dataSetName);
        usingCurrentProj = projString == currentProj;
        createProjCell(i, projString);
        createProjCheckBox(i, usingCurrentProj);
    }

    // Apply the user's custom proj string if needed
    //
    usingCurrentProj = customProj == currentProj;
    createCustomCell(numDataMgrs, customProj);
    createProjCheckBox(numDataMgrs, usingCurrentProj);

    resizeProjTable();
}

string NavigationEventRouter::getCustomProjString()
{
    int               row = datasetProjectionTable->rowCount();
    QTableWidgetItem *item = datasetProjectionTable->item(row - 1, 0);

    string customProj;
    if (item == NULL)
        customProj = "";
    else
        customProj = item->text().toStdString();

    if (customProj == "") customProj = "Custom";

    return customProj;
}

void NavigationEventRouter::resizeProjTable()
{
    datasetProjectionTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    datasetProjectionTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
    datasetProjectionTable->verticalHeader()->hide();
    datasetProjectionTable->resizeRowsToContents();
}

void NavigationEventRouter::createProjCheckBox(int row, bool usingCurrentProj)
{
    QWidget *    checkBoxWidget = new QWidget();
    QCheckBox *  checkBox = new QCheckBox();
    QHBoxLayout *checkBoxLayout = new QHBoxLayout(checkBoxWidget);

    checkBoxLayout->addWidget(checkBox);
    checkBoxLayout->setAlignment(Qt::AlignCenter);
    checkBoxLayout->setContentsMargins(0, 0, 0, 0);
    checkBoxLayout->setSizeConstraint(QLayout::SetNoConstraint);

    checkBoxWidget->setProperty("row", row);
    checkBoxWidget->setLayout(checkBoxLayout);
    datasetProjectionTable->setCellWidget(row, 1, checkBoxWidget);

    Qt::CheckState cs = usingCurrentProj ? Qt::Checked : Qt::Unchecked;
    checkBox->blockSignals(true);
    checkBox->setCheckState(cs);
    checkBox->blockSignals(false);

    if (row == datasetProjectionTable->rowCount() - 1) {
        connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(customCheckboxChanged()));
    } else {
        connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(projCheckboxChanged()));
    }
}

void NavigationEventRouter::createProjCell(int row, string projString)
{
    QLabel *label = new QLabel(datasetProjectionTable);
    label->setText(QString::fromStdString(projString));
    label->setAlignment(Qt::AlignCenter);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);

    // If this is the last row, the item should be editable for custom
    // proj strings from the user
    //
    datasetProjectionTable->setCellWidget(row, 0, label);
}

void NavigationEventRouter::createCustomCell(int row, string projString)
{
    QTextEdit *textEdit = new QTextEdit(datasetProjectionTable);
    textEdit->setText(QString::fromStdString(projString));
    textEdit->setAlignment(Qt::AlignCenter);

    // If this is the last row, the item should be editable for custom
    // proj strings from the user
    //
    // if (row != datasetProjectionTable->rowCount()) textEdit->setReadOnly(true);
    datasetProjectionTable->setCellWidget(row, 0, textEdit);

    connect(textEdit, SIGNAL(textChanged()), this, SLOT(customProjStringChanged()));
}

void NavigationEventRouter::projCheckboxChanged()
{
    QCheckBox *checkBox = (QCheckBox *)sender();
    int        row = checkBox->parentWidget()->property("row").toInt();

    QLabel *label;
    label = qobject_cast<QLabel *>(datasetProjectionTable->cellWidget(row, 0));
    string proj = label->text().toStdString();

    GUIStateParams *params = GetStateParams();
    if (checkBox->checkState() > 0) {
        params->SetProjectionString(proj);
    } else {
        params->SetProjectionString("");
    }
    emit Proj4StringChanged();
}

void NavigationEventRouter::customCheckboxChanged()
{
    QCheckBox *checkBox = (QCheckBox *)sender();
    int        row = checkBox->parentWidget()->property("row").toInt();

    QTextEdit *textEdit;
    textEdit = qobject_cast<QTextEdit *>(datasetProjectionTable->cellWidget(row, 0));
    string proj = textEdit->toPlainText().toStdString();

    GUIStateParams *params = GetStateParams();
    if (checkBox->checkState() > 0) {
        params->SetProjectionString(proj);
    } else {
        params->SetProjectionString("");
    }
    emit Proj4StringChanged();
}

// If the custom proj string gets changed, we do not want to keep updating
// it as the user types in their new string.  Just disable the global
// projection, and let the user re-enable their selection when they're
// done entering text
void NavigationEventRouter::customProjStringChanged()
{
    GUIStateParams *params = GetStateParams();
    string          currentProj = params->GetProjectionString();
    if (currentProj != "") {
        params->SetProjectionString("");
        emit Proj4StringChanged();
    }
}

// Insert values from params into tab panel
//
void NavigationEventRouter::_updateTab()
{
    updateCameraChanged();
    updateLightChanged();
    updateTransforms();
    updateProjections();

    return;

    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    QString strng;

    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    size_t     ts = GetCurrentTimeStep();

    latLonFrame->hide();
    // Always display the current values of the campos and rotcenter
}

void NavigationEventRouter::CenterSubRegion()
{
    cout << "NavigationEventRouter::CenterSubRegion not implemented" << endl;

#ifdef DEAD

    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    // Find the largest of the dimensions of the current region, projected orthogonal to view
    // direction:
    // Make sure the dirvec is normalized:
    double dirvec[3];
    vpParams->GetCameraViewDir(dirvec);

    double upvec[3];
    vpParams->GetCameraUpVec(upvec);

    vnormal(dirvec);
    float          regionSideVector[3], compVec[3], projvec[3];
    float          maxProj = -1.f;
    vector<double> stretch = vpParams->GetStretchFactors();

    vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);
    for (int i = 0; i < 3; i++) {
        // Make a vector that points along side(i) of subregion,

        for (int j = 0; j < 3; j++) {
            regionSideVector[j] = 0.f;
            if (j == i) { regionSideVector[j] = maxExts[j] - minExts[j]; }
        }
        // Now find its component orthogonal to view direction:
        double dotprod = 0.;

        for (int j = 0; j < 3; j++) dotprod += (dirvec[j] * regionSideVector[j]);
        for (int j = 0; j < 3; j++) compVec[j] = dotprod * dirvec[j];
        // projvec is projection orthogonal to view dir:
        vsub(regionSideVector, compVec, projvec);
        float proj = vlength(projvec);
        if (proj > maxProj) maxProj = proj;
    }

    // calculate the camera position: center - 1.5*dirvec*maxSide;
    // Position the camera 1.5*maxSide units away from the center, aimed
    // at the center

    double         posvec[3], center[3];
    vector<double> rotCtr;
    for (int i = 0; i < 3; i++) {
        center[i] = 0.5 * (minExts[i] + maxExts[i]);
        posvec[i] = center[i] - (1.5 * maxProj * dirvec[i] / stretch[i]);
    }

    _vizMgr->SetTrackBall(posvec, dirvec, upvec, center, true);

#endif
}

// Align the view direction to one of the axes.
// axis is 2,3,4 for +X,Y,Z,  and 5,6,7 for -X,-Y,-Z
//
void NavigationEventRouter::AlignView(int axis)
{
    float axes[3][3] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};

    double dirvec[3] = {0.0, 0.0, 0.0};
    double upvec[3] = {0.0, 0.0, 0.0};
    upvec[1] = 1.;
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();
    if (axis == 1) {    // Special case to align to closest axis.
        // determine the closest view direction and up vector to the current viewpoint.
        // Check the dot product with all axes
        float  maxVDot = -1.f;
        int    bestVDir = 0;
        float  maxUDot = -1.f;
        int    bestUDir = 0;
        double curViewDir[3], curUpVec[3];
        vpParams->GetCameraViewDir(curViewDir);
        vpParams->GetCameraUpVec(curUpVec);
        for (int i = 0; i < 3; i++) {
            double dotVProd = 0.;
            double dotUProd = 0.;
            for (int j = 0; j < 3; j++) {
                dotUProd += (axes[i][j] * curViewDir[j]);
                dotVProd += (axes[i][j] * curUpVec[j]);
            }
            if (abs(dotVProd) > maxVDot) {
                maxVDot = abs(dotVProd);
                bestVDir = i + 1;
                if (dotVProd < 0.f) bestVDir = -i - 1;
            }
            if (abs(dotUProd) > maxUDot) {
                maxUDot = abs(dotUProd);
                bestUDir = i + 1;
                if (dotUProd < 0.f) bestUDir = -i - 1;
            }
        }
        for (int i = 0; i < 3; i++) {
            if (bestUDir > 0)
                dirvec[i] = axes[bestUDir - 1][i];
            else
                dirvec[i] = -axes[-1 - bestUDir][i];

            if (bestVDir > 0)
                upvec[i] = axes[bestVDir - 1][i];
            else
                upvec[i] = -axes[-1 - bestVDir][i];
        }
    } else {
        // establish view direction, up vector:
        switch (axis) {
        case (2): dirvec[0] = 1.f; break;
        case (3):
            dirvec[1] = 1.f;
            upvec[1] = 0.f;
            upvec[0] = 1.f;
            break;
        case (4): dirvec[2] = 1.f; break;
        case (5): dirvec[0] = -1.f; break;
        case (6):
            dirvec[1] = -1.f;
            upvec[1] = 0.f;
            upvec[0] = 1.f;
            break;
        case (7): dirvec[2] = -1.f; break;
        default: assert(0);
        }
    }

    vector<double> stretch = vpParams->GetStretchFactors();

    // Determine distance from center to camera, in stretched coordinates
    double posvec[3], center[3];

    vpParams->GetCameraPos(posvec);
    vpParams->GetRotationCenter(center);

    // determine the relative position in stretched coords:
    vsub(posvec, center, posvec);
    float viewDist = vlength(posvec);
    // Position the camera the same distance from the center but down the -axis direction
    for (int i = 0; i < 3; i++) {
        dirvec[i] = dirvec[i] * viewDist;
        posvec[i] = (center[i] - dirvec[i]);
    }

    _vizMgr->SetTrackBall(posvec, dirvec, upvec, center, true);
}

// Reset the center of view.  Leave the camera where it is
void NavigationEventRouter::SetCenter(const double *coords)
{
#ifdef DEAD
    double           vdir[3];
    vector<double>   nvdir;
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();
    vector<double>   stretch = _dataStatus->getStretchFactors();

    // Determine the new viewDir in stretched world coords

    vcopy(coords, vdir);
    // Stretch the new view center coords
    for (int i = 0; i < 3; i++) vdir[i] *= stretch[i];
    double campos[3];
    vpParams->getStretchedCamPosLocal(campos);
    vsub(vdir, campos, vdir);

    vnormal(vdir);
    vector<double> vvdir;
    #ifdef DEAD
    Command *cmd = Command::CaptureStart(vpParams, "re-center view");
    #endif
    for (int i = 0; i < 3; i++) vvdir.push_back(vdir[i]);
    vpParams->setViewDir(vvdir);
    vector<double> rotCtr;
    for (int i = 0; i < 3; i++) { rotCtr.push_back(coords[i]); }
    vpParams->setRotationCenterLocal(rotCtr);
    #ifdef DEAD
    Command::CaptureEnd(cmd, vpParams);
    #endif
    updateTab();

#endif
}

void NavigationEventRouter::SetHomeViewpoint()
{
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();
    vpParams->SetCurrentVPToHome();
}

void NavigationEventRouter::UseHomeViewpoint()
{
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    Viewpoint *homeVP = vpParams->GetHomeViewpoint();
    vpParams->SetCurrentViewpoint(homeVP);

    double posvec[3], dirvec[3], upvec[3], center[3];
    vpParams->GetCameraPos(posvec);
    vpParams->GetCameraViewDir(dirvec);
    vpParams->GetCameraUpVec(upvec);
    vpParams->GetRotationCenter(center);

    _vizMgr->SetTrackBall(posvec, dirvec, upvec, center, true);
}

void NavigationEventRouter::ViewAll()
{
    DataStatus *dataStatus = _controlExec->GetDataStatus();
    ParamsMgr * paramsMgr = _controlExec->GetParamsMgr();
    size_t      ts = GetCurrentTimeStep();

    vector<double> minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);
    assert(minExts.size() == 3);
    assert(maxExts.size() == 3);

    double maxSide = max(maxExts[2] - minExts[2], max(maxExts[1] - minExts[1], maxExts[0] - minExts[0]));

    // calculate the camera position: center - 1.5*dirvec*maxSide;
    // Position the camera 1.5*maxSide units away from the center, aimed
    // at the center.
    //

    // Make sure the dirvec is normalized:
    double dirvec[] = {0.0, 0.0, -1.0};
    vnormal(dirvec);

    double upvec[] = {0.0, 1.0, 0.0};

    double posvec[3], center[3];
    for (int i = 0; i < 3; i++) {
        center[i] = 0.5f * (maxExts[i] + minExts[i]);
        posvec[i] = center[i] - 1.5 * maxSide * dirvec[i];
    }

    _vizMgr->SetTrackBall(posvec, dirvec, upvec, center, true);
}

VAPoR::ParamsBase *NavigationEventRouter::GetActiveParams() const
{
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    return (paramsMgr->GetViewpointParams(vizName));
}
