//************************************************************************
//															*
//		     Copyright (C)  2006										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		ViewpointEventRouter.cpp
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

#include <qcombobox.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "ViewpointEventRouter.h"
#include "vapor/ViewpointParams.h"
#include "vapor/ControlExecutive.h"
#include "vizTab.h"
#include "VizWinMgr.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "MainForm.h"

using namespace VAPoR;

ViewpointEventRouter::ViewpointEventRouter(QWidget *parent, VizWinMgr *vizMgr, ControlExec *ce) : QWidget(parent), Ui_VizTab(), EventRouter(ce, ViewpointParams::GetClassType())
{
    setupUi(this);

    _vizMgr = vizMgr;

    _panChanged = false;
    for (int i = 0; i < 3; i++) _lastCamPos[i] = 0.f;
}

ViewpointEventRouter::~ViewpointEventRouter() {}
/**********************************************************
 * Whenever a new viztab is created it must be hooked up here
 ************************************************************/
void ViewpointEventRouter::hookUpTab()
{
    // connect (stereoCombo, SIGNAL (activated(int)), this, SLOT (SetStereoMode(int)));
    // connect (latLonCheckbox, SIGNAL (toggled(bool)), this, SLOT(ToggleLatLon(bool)));
    connect(camPosLat, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(camPosLon, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(rotCenterLat, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(rotCenterLon, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(numLights, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos00, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos01, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos02, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos10, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos11, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos12, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos20, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos21, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightPos22, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(camPos0, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(camPos1, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(camPos2, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(viewDir0, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(viewDir1, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(viewDir2, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(upVec0, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(upVec1, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(upVec2, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(rotCenter0, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(rotCenter1, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(rotCenter2, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightDiff0, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightDiff1, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightDiff2, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightSpec0, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightSpec1, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(lightSpec2, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(shininessEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(ambientEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));
    connect(stereoSeparationEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVtabTextChanged(const QString &)));

    // Connect all the returnPressed signals, these will update the visualizer.
    connect(camPosLat, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(camPosLon, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(rotCenterLat, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(rotCenterLon, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos00, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos01, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos02, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos10, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos11, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos12, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos20, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos21, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightPos22, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightDiff0, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightDiff1, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightDiff2, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightSpec0, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightSpec1, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(lightSpec2, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(shininessEdit, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(ambientEdit, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));

    connect(camPos0, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(camPos1, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(camPos2, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(viewDir0, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(viewDir1, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(viewDir2, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(upVec0, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(upVec1, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(upVec2, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(rotCenter0, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(rotCenter1, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(rotCenter2, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(numLights, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));
    connect(stereoSeparationEdit, SIGNAL(returnPressed()), this, SLOT(viewpointReturnPressed()));

    connect(LocalGlobal, SIGNAL(activated(int)), VizWinMgr::getInstance(), SLOT(setVpLocalGlobal(int)));
    connect(VizWinMgr::getInstance(), SIGNAL(enableMultiViz(bool)), LocalGlobal, SLOT(setEnabled(bool)));
}

void ViewpointEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
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

void ViewpointEventRouter::setVtabTextChanged(const QString &) { SetTextChanged(true); }
// Put all text changes into the params
void ViewpointEventRouter::_confirmText()
{
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    size_t timeStep = GetCurrentTimeStep();

    vector<double> minExts, maxExts;

    DataStatus *dataStatus = _controlExec->getDataStatus();
    dataStatus->GetExtents(timeStep, minExts, maxExts);

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

void ViewpointEventRouter::viewpointReturnPressed(void) { confirmText(); }

// Insert values from params into tab panel
//
void ViewpointEventRouter::_updateTab()
{
    cout << "ViewpointEventRouter::_updateTab() BLOCKED" << endl;
    return;

    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    QString strng;

#ifdef DEAD
    if (vpParams->IsLocal())
        LocalGlobal->setCurrentIndex(1);
    else
#endif
        LocalGlobal->setCurrentIndex(0);

    int nLights = vpParams->getNumLights();
    numLights->setText(strng.setNum(nLights));

    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    size_t     ts = paramsMgr->GetAnimationParams()->GetCurrentTimestep();

    vector<double> minExts, maxExts;

    DataStatus *dataStatus = _controlExec->getDataStatus();
    dataStatus->GetExtents(ts, minExts, maxExts);

    latLonFrame->hide();
    // Always display the current values of the campos and rotcenter

    double posvec[3], dirvec[3], upvec[3], center[3];
    vpParams->GetCameraPos(posvec);
    vpParams->GetCameraViewDir(dirvec);
    vpParams->GetCameraUpVec(upvec);
    vpParams->GetRotationCenter(center);

    camPos0->setText(strng.setNum(posvec[0], 'g', 3));
    camPos1->setText(strng.setNum(posvec[1], 'g', 3));
    camPos2->setText(strng.setNum(posvec[2], 'g', 3));

    viewDir0->setText(strng.setNum(dirvec[0], 'g', 3));
    viewDir1->setText(strng.setNum(dirvec[1], 'g', 3));
    viewDir2->setText(strng.setNum(dirvec[2], 'g', 3));

    upVec0->setText(strng.setNum(upvec[0], 'g', 3));
    upVec1->setText(strng.setNum(upvec[1], 'g', 3));
    upVec2->setText(strng.setNum(upvec[2], 'g', 3));

    // perspectiveCombo->setCurrentIndex(currentViewpoint->hasPerspective());
    rotCenter0->setText(strng.setNum(center[0], 'g', 3));
    rotCenter1->setText(strng.setNum(center[1], 'g', 3));
    rotCenter2->setText(strng.setNum(center[2], 'g', 3));

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
void ViewpointEventRouter::CenterSubRegion(RegionParams *rParams)
{
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
#ifdef DEAD
    Command *cmd = Command::CaptureStart(vpParams, "Center viewpoint on subregion");
#endif

    double         posvec[3], center[3];
    vector<double> rotCtr;
    for (int i = 0; i < 3; i++) {
        center[i] = 0.5 * (minExts[i] + maxExts[i]);
        posvec[i] = center[i] - (1.5 * maxProj * dirvec[i] / stretch[i]);
    }

    _vizMgr->SetTrackBall(posvec, dirvec, upvec, center, true);

#ifdef DEAD
    Command::CaptureEnd(cmd, vpParams);
#endif

#ifdef DEAD
    // modify near/far distance as needed:
    VizWinMgr::getInstance()->resetViews(vpParams);
#endif

    updateTab();
}

// Align the view direction to one of the axes.
// axis is 2,3,4 for +X,Y,Z,  and 5,6,7 for -X,-Y,-Z
//
void ViewpointEventRouter::AlignView(int axis)
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

#ifdef DEAD
    Command *cmd = Command::CaptureStart(vpParams, "axis-align view");
#endif

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

#ifdef DEAD
    Command::CaptureEnd(cmd, vpParams);
    vpParams->VPSetChanged(true);
#endif
    updateTab();
}

// Reset the center of view.  Leave the camera where it is
void ViewpointEventRouter::SetCenter(const double *coords)
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

void ViewpointEventRouter::setHomeViewpoint()
{
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();
    vpParams->setCurrentVPToHome();
    updateTab();
}
void ViewpointEventRouter::useHomeViewpoint()
{
#ifdef DEAD
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    Viewpoint *homeViewpoint = vpParams->getHomeViewpoint();
    Viewpoint *newViewpoint = (Viewpoint *)homeViewpoint->deepCopy();

    vpParams->setCurrentViewpoint(newViewpoint);
#endif

    updateTab();
}
#ifdef DEAD
void ViewpointEventRouter::captureMouseUp()
{
    // Update the tab:
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();

    if (_panChanged) {
        // Apply the translation to the rotation
        float  trans;
        double newRot[3];
        double camPos[3];
        vpParams->GetCameraPos(camPos);

        double rotCenter[3];
        vpParams->GetRotationCenter(rotCenter);

        for (int i = 0; i < 3; i++) {
            trans = camPos[i] - _lastCamPos[i];
            newRot[i] = rotCenter[i] + trans;
        }
        vpParams->SetRotationCenter(newRot);
        _panChanged = false;
    }
    updateTab();

    // Just rerender:
    #ifdef DEAD
    VizWinMgr::getInstance()->refreshViewpoint(vpParams);
    #endif
}
// If the mouse drag resulted in a spin, the event is modified when
// the spin is terminated:
void ViewpointEventRouter::endSpin() { updateTab(); }
#endif

#ifdef DEAD
// Reinitialize Viewpoint tab settings, session has changed.
// Note that this is called after the globalViewpointParams are set up, but before
// any of the localViewpointParams are setup.
void ViewpointEventRouter::_reinitTab(bool doOverride)
{
    if (VizWinMgr::getInstance()->getNumVisualizers() > 1)
        LocalGlobal->setEnabled(true);
    else
        LocalGlobal->setEnabled(false);
    setEnabled(true);
}
#endif

#ifdef DEAD
// Save undo/redo state when user grabs a rake handle
//
void ViewpointEventRouter::captureMouseDown(int button)
{
    // If text has changed, will ignore it-- don't call confirmText()!
    //
    ViewpointParams *vpParams = (ViewpointParams *)GetActiveParams();
    SetTextChanged(false);

    if (button == 2) {    // panning
        // save current camera position
        double camPos[3];
        vpParams->GetCameraPos(camPos);
        for (int i = 0; i < 3; i++) _lastCamPos[i] = camPos[i];
        _panChanged = true;
    }
}

#endif
