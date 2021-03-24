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
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <qdesktopwidget.h>
#include <qrect.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <QTextEdit>
#include <QScrollArea>
#include <cfloat>
#include "PCheckbox.h"
#include "PIntegerInput.h"

#include <qcombobox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "vapor/ViewpointParams.h"
#include "vapor/ControlExecutive.h"
#include <vapor/DataStatus.h>
#include <vapor/DataMgrUtils.h>
#include "ui_NavigationTab.h"
#include "ErrorReporter.h"
#include "TrackBall.h"
#include "NavigationEventRouter.h"
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>

#include "VSection.h"

using namespace VAPoR;

namespace {
string HomeModelViewMatrixTag = "HomeModelViewMatrix";
string HomeRotationCenterTag = "HomeRotationCenter";
}    // namespace

NavigationEventRouter::NavigationEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), Ui_NavigationTab(), EventRouter(ce, ViewpointParams::GetClassType())
{
    setupUi(this);

    futureFeaturesTab->hide();

    // Not implemented
    //
    camPosLat->setEnabled(false);
    camPosLon->setEnabled(false);
    rotCenterLat->setEnabled(false);
    rotCenterLon->setEnabled(false);
    stereoCombo->setEnabled(false);
    latLonCheckbox->setEnabled(false);
    stereoSeparationEdit->setEnabled(false);
    adjustSize();

    verticalLayout->setSpacing(15);

    VSection *framebufferSection = new VSection("Framebuffer Settings");
    verticalLayout->insertWidget(verticalLayout->count() - 1, framebufferSection);

    framebufferSection->layout()->addWidget(_useCustomFramebufferCheckbox = new PCheckbox(ViewpointParams::UseCustomFramebufferTag, "Use Custom Output Size"));
    framebufferSection->layout()->addWidget(_customFramebufferWidth = new PIntegerInput(ViewpointParams::CustomFramebufferWidthTag, "Output Width (px)"));
    framebufferSection->layout()->addWidget(_customFramebufferHeight = new PIntegerInput(ViewpointParams::CustomFramebufferHeightTag, "Output Height (px)"));
    _customFramebufferWidth->SetRange(1, 16384);
    _customFramebufferHeight->SetRange(1, 16384);
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

    connect(_projectionCombo, SIGNAL(activated(const QString &)), this, SLOT(projectionComboBoxChanged(const QString &)));
}

void NavigationEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("View Tab Overview", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting"));

    help.push_back(make_pair("Controlling the Viewpoint in VAPOR GUI", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting#ControlView"));

    help.push_back(make_pair("Viewpoint Settings", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting#ViewpointSettings"));

    help.push_back(make_pair("Lighting Settings", "http://www.vapor.ucar.edu/docs/vapor-gui-help/viewpoint-and-lighting#LightingControl"));
}

void NavigationEventRouter::_performAutoStretching(string dataSetName)
{
    DataStatus *ds = _controlExec->GetDataStatus();

    ParamsMgr *    paramsMgr = _controlExec->GetParamsMgr();
    vector<string> winNames = paramsMgr->GetVisualizerNames();

    vector<double> minExt, maxExt;

    for (int i = 0; i < winNames.size(); i++) {
        ViewpointParams *   vpParams = paramsMgr->GetViewpointParams(winNames[i]);
        Transform *         transform = vpParams->GetTransform(dataSetName);
        std::vector<double> scales = transform->GetScales();
        int                 xDimension = 0;
        int                 yDimension = 1;
        int                 zDimension = 2;

        // If a dimension's scale is not 1.f, the user has saved a session with
        // a non-default value.  Don't modify it.
        if (scales[xDimension] != 1.f) continue;
        if (scales[yDimension] != 1.f) continue;
        if (scales[zDimension] != 1.f) continue;

        size_t ts = GetCurrentTimeStep();
        ds->GetActiveExtents(paramsMgr, winNames[i], dataSetName, ts, minExt, maxExt);

        vector<float> range;
        float         maxRange = 0.0;
        for (int i = 0; i < minExt.size(); i++) {
            float r = fabs(maxExt[i] - minExt[i]);
            if (maxRange < r) { maxRange = r; }
            range.push_back(r);
        }

        if (fabs(maxRange) <= FLT_EPSILON) maxRange = 1.0;

        vector<double> scale(range.size(), 1.0);
        for (int i = 0; i < range.size(); i++) {
            if (range[i] < (maxRange / 10.0) && fabs(range[i]) > FLT_EPSILON) { scale[i] = maxRange / (10.0 * range[i]); }
        }

        transform->SetScales(scale);
    }
}

void NavigationEventRouter::LoadDataNotify(string dataSetName)
{
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    SettingsParams *sP = (SettingsParams *)paramsMgr->GetParams(SettingsParams::GetClassType());

    bool autoStretchingEnabled = sP->GetAutoStretchEnabled();
    if (autoStretchingEnabled) { _performAutoStretching(dataSetName); }
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

    _setViewpointParams(center, posvec, dirvec, upvec);
}

void NavigationEventRouter::setCameraLatLonChanged() { cout << "Not implemented" << endl; }

void NavigationEventRouter::notImplemented() { cout << "Not implemented" << endl; }

void NavigationEventRouter::updateCameraChanged()
{
    double posvec[3], dirvec[3], upvec[3], center[3];

    bool status = _getViewpointParams(center, posvec, dirvec, upvec);
    if (!status) return;

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
    ViewpointParams *vpParams = _getActiveParams();

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
    ViewpointParams *vpParams = _getActiveParams();

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

        DataStatus *   dataStatus = _controlExec->GetDataStatus();
        vector<string> names = dataStatus->GetDataMgrNames();

        for (int j = 0; j < names.size(); j++) {
            Transform *t = vParams->GetTransform(names[j]);

            if (!t->IsOriginInitialized()) {
                size_t         ts = GetCurrentTimeStep();
                vector<double> minExts;
                vector<double> maxExts;
                vector<double> origin;

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

    int numDataMgrs = dataMgrs.size();

    datasetProjectionTable->clear();
    datasetProjectionTable->setRowCount(numDataMgrs + 1);

    // Set up table with dataset projection strings
    //
    string dataSetName, projString;
    bool   usingDSProj = false;
    for (int i = 0; i < numDataMgrs; i++) {
        dataSetName = dataMgrs[i];
        projString = dataStatus->GetMapProjectionDefault(dataSetName);
        createProjCell(i, projString, true);
        createProjCheckBox(i, projString == currentProj);
        if (projString == currentProj) { usingDSProj = true; }
    }

    // Apply the user's custom proj string if needed
    //
    if (!usingDSProj && !currentProj.empty()) {
        createProjCell(numDataMgrs, currentProj, false);
        createProjCheckBox(numDataMgrs, true);
    } else {
        createProjCell(numDataMgrs, "Custom", false);
        createProjCheckBox(numDataMgrs, false);
    }

    resizeProjTable();
}

void NavigationEventRouter::resizeProjTable()
{
    datasetProjectionTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    datasetProjectionTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    datasetProjectionTable->verticalHeader()->hide();
    datasetProjectionTable->resizeRowsToContents();

    int height = datasetProjectionTable->horizontalHeader()->height();
    int rows = datasetProjectionTable->rowCount();
    datasetProjectionTable->setMaximumHeight(height * rows * 3);
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

    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(projCheckboxChanged()));
}

void NavigationEventRouter::createProjCell(int row, string projString, bool ro)
{
    QTextEdit *textEdit = new QTextEdit(datasetProjectionTable);
    textEdit->setText(QString::fromStdString(projString));
    textEdit->setAlignment(Qt::AlignCenter);
    textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
    textEdit->setReadOnly(ro);

    // If this is the last row, the item should be editable for custom
    // proj strings from the user
    //
    datasetProjectionTable->setCellWidget(row, 0, textEdit);
}

void NavigationEventRouter::projCheckboxChanged()
{
    QCheckBox *checkBox = (QCheckBox *)sender();
    int        row = checkBox->parentWidget()->property("row").toInt();

    QTextEdit *textEdit;
    textEdit = qobject_cast<QTextEdit *>(datasetProjectionTable->cellWidget(row, 0));
    string proj = textEdit->toPlainText().toStdString();

    GUIStateParams *params = GetStateParams();
    if (checkBox->checkState() == 0 || proj == "Custom") { proj = ""; }

    params->SetProjectionString(proj);
    emit Proj4StringChanged(proj);
}

void NavigationEventRouter::projectionComboBoxChanged(const QString &s)
{
    ViewpointParams *               vpParams = _getActiveParams();
    ViewpointParams::ProjectionType type;

    if (s == "Perspective") type = ViewpointParams::Perspective;
    if (s == "Map Orthographic") type = ViewpointParams::MapOrthographic;

    vpParams->SetProjectionType(type);
    emit ProjectionTypeChanged(type);
}

// Insert values from params into tab panel
//
void NavigationEventRouter::_updateTab()
{
    if (!_getActiveParams()) return;

    updateCameraChanged();
    updateLightChanged();
    updateTransforms();
    updateProjections();

    VAPoR::ViewpointParams *vp = _getActiveParams();
    _useCustomFramebufferCheckbox->Update(vp);
    _customFramebufferWidth->Update(vp);
    _customFramebufferHeight->Update(vp);
    _customFramebufferWidth->setEnabled(vp->GetValueLong(ViewpointParams::UseCustomFramebufferTag, 0));
    _customFramebufferHeight->setEnabled(vp->GetValueLong(ViewpointParams::UseCustomFramebufferTag, 0));
}

void NavigationEventRouter::CenterSubRegion()
{
    cout << "NavigationEventRouter::CenterSubRegion not implemented" << endl;

#ifdef VAPOR3_0_0_ALPHA

    ViewpointParams *vpParams = _getActiveParams();
    if (!vpParams) return;

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

    _setViewpointParams(center, posvec, dirvec, upvec);

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
    ViewpointParams *vpParams = _getActiveParams();
    if (!vpParams) return;

    double curPosVec[3], curViewDir[3], curUpVec[3], curCenter[3];
    bool   status = _getViewpointParams(curCenter, curPosVec, curViewDir, curUpVec);
    if (!status) return;

    if (axis == 1) {    // Special case to align to closest axis.
        // determine the closest view direction and up vector to the current viewpoint.
        // Check the dot product with all axes
        float maxVDot = -1.f;
        int   bestVDir = 0;
        float maxUDot = -1.f;
        int   bestUDir = 0;
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
        default: return;
        }
    }

    vector<double> stretch = vpParams->GetStretchFactors();

    // Determine distance from center to camera, in stretched coordinates

    // determine the relative position in stretched coords:
    vsub(curPosVec, curCenter, curPosVec);
    float viewDist = vlength(curPosVec);
    // Position the camera the same distance from the center but down the -axis direction
    for (int i = 0; i < 3; i++) {
        dirvec[i] = dirvec[i] * viewDist;
        curPosVec[i] = (curCenter[i] - dirvec[i]);
    }

    _setViewpointParams(curCenter, curPosVec, dirvec, upvec);
}

// Reset the center of view.  Leave the camera where it is
void NavigationEventRouter::SetCenter(const double *coords)
{
#ifdef VAPOR3_0_0_ALPHA
    double           vdir[3];
    vector<double>   nvdir;
    ViewpointParams *vpParams = _getActiveParams();
    if (!vpParams) return;
    vector<double> stretch = _dataStatus->getStretchFactors();

    // Determine the new viewDir in stretched world coords

    vcopy(coords, vdir);
    // Stretch the new view center coords
    for (int i = 0; i < 3; i++) vdir[i] *= stretch[i];
    double campos[3];
    vpParams->getStretchedCamPosLocal(campos);
    vsub(vdir, campos, vdir);

    vnormal(vdir);
    vector<double> vvdir;
    #ifdef VAPOR3_0_0_ALPHA
    Command *cmd = Command::CaptureStart(vpParams, "re-center view");
    #endif
    for (int i = 0; i < 3; i++) vvdir.push_back(vdir[i]);
    vpParams->setViewDir(vvdir);
    vector<double> rotCtr;
    for (int i = 0; i < 3; i++) { rotCtr.push_back(coords[i]); }
    vpParams->setRotationCenterLocal(rotCtr);
    #ifdef VAPOR3_0_0_ALPHA
    Command::CaptureEnd(cmd, vpParams);
    #endif
    updateTab();

#endif
}

void NavigationEventRouter::SetHomeViewpoint()
{
    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vpParams = _getActiveParams();
    GUIStateParams * guiParams = GetStateParams();

    // Get the current model view matrix and it home
    //
    vector<double> m = vpParams->GetModelViewMatrix();
    vector<double> c = vpParams->GetRotationCenter();

    paramsMgr->BeginSaveStateGroup("Set home viewpoint");

    guiParams->SetValueDoubleVec(HomeModelViewMatrixTag, "Modelview matrix", m);
    guiParams->SetValueDoubleVec(HomeRotationCenterTag, "Camera rotation center", c);
    paramsMgr->EndSaveStateGroup();
}

void NavigationEventRouter::UseHomeViewpoint()
{
    GUIStateParams *guiParams = GetStateParams();

    // Get the home matrix and make it the current model view matrix
    //
    vector<double> defaultV(16, 0.0);
    defaultV[0] = defaultV[5] = defaultV[10] = defaultV[15] = 1.0;

    vector<double> m = guiParams->GetValueDoubleVec(HomeModelViewMatrixTag, defaultV);

    vector<double> defaultC(3, 0.0);
    vector<double> c = guiParams->GetValueDoubleVec(HomeRotationCenterTag, defaultC);

    _setViewpointParams(m, c);
}

void NavigationEventRouter::ViewAll()
{
    DataStatus *dataStatus = _controlExec->GetDataStatus();
    ParamsMgr * paramsMgr = _controlExec->GetParamsMgr();
    size_t      ts = GetCurrentTimeStep();

    vector<double> minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);
    VAssert(minExts.size() == 3);
    VAssert(maxExts.size() == 3);

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

    _setViewpointParams(center, posvec, dirvec, upvec);
}

VAPoR::ViewpointParams *NavigationEventRouter::_getActiveParams() const
{
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    if (vizName.empty()) return (NULL);

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    return (paramsMgr->GetViewpointParams(vizName));
}

void NavigationEventRouter::_setViewpointParams(const vector<double> &modelview, const vector<double> &center) const
{
    VAssert(modelview.size() == 16);
    VAssert(center.size() == 3);

    // Set modelview and rotation center for *all* visualizers
    //
    ParamsMgr *    paramsMgr = _controlExec->GetParamsMgr();
    vector<string> winNames = paramsMgr->GetVisualizerNames();

    vector<double> minExt, maxExt;

    paramsMgr->BeginSaveStateGroup("Move camera");

    for (int i = 0; i < winNames.size(); i++) {
        ViewpointParams *vpParams = paramsMgr->GetViewpointParams(winNames[i]);

        vpParams->SetModelViewMatrix(modelview);
        vpParams->SetRotationCenter(center);
    }

    paramsMgr->EndSaveStateGroup();
}

void NavigationEventRouter::_setViewpointParams(const double center[3], const double posvec[3], const double dirvec[3], const double upvec[3])
{
    std::vector<double> modelview, centerv;

    // Ugh. Use trackball to convert viewing vectors into a model view
    // matrix
    //
    Trackball trackball;
    bool      rc = trackball.setFromFrame(posvec, dirvec, upvec, center, true);
    if (rc == false) {    // If trackball fails
        MSG_ERR("Invalid camera settings");
        updateCameraChanged();
        return;
    } else {    // else use the trackball's model view matrix
        trackball.TrackballSetMatrix();
        const double *m = trackball.GetModelViewMatrix();
        modelview.assign(m, m + 16);
        centerv.assign(center, center + 3);
    }

    _setViewpointParams(modelview, centerv);
}

bool NavigationEventRouter::_getViewpointParams(double center[3], double posvec[3], double dirvec[3], double upvec[3]) const
{
    // Get camera parameters from ViewpointParams
    //
    ViewpointParams *vpParams = _getActiveParams();
    double           m[16];
    vpParams->GetModelViewMatrix(m);

    bool status = vpParams->ReconstructCamera(m, posvec, upvec, dirvec);
    if (!status) {
        MSG_ERR("Failed to get camera parameters");
        return (false);
    }

    vpParams->GetRotationCenter(center);

    return (true);
}
