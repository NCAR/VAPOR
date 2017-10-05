//************************************************************************
//															*
//			 Copyright (C)  2015										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		VizFeatureEventRouter.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the VizFeatureEventRouter class.
//		This class supports routing messages from the gui to the params
//		This is derived from the vizfeature Widget
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning(disable : 4100 4996)
#endif
#include <qdesktopwidget.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include "GL/glew.h"
#include "vapor/VizFeatureParams.h"
#include "ui_vizFeaturesTab.h"
#include "qcolordialog.h"

#include <qlabel.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "VizFeatureEventRouter.h"
#include "vapor/ControlExecutive.h"
#include "EventRouter.h"

using namespace VAPoR;

VizFeatureEventRouter::VizFeatureEventRouter(
    QWidget *parent, ControlExec *ce) : QWidget(parent),
                                        Ui_vizFeaturesTab(),
                                        EventRouter(ce, VizFeatureParams::GetClassType()) {

    setupUi(this);

    cout << "VizFeatureEventRouter constructor" << endl;

    _animConnected = false;
    _ap = NULL;
}

VizFeatureEventRouter::~VizFeatureEventRouter() {
}
/**********************************************************
 * Whenever a new vizfeaturetab is created it must be hooked up here
 ************************************************************/
void VizFeatureEventRouter::hookUpTab() {
    cout << "Hooking up VizFeatureEventRouter" << endl;
    connect(stretch0Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(stretch0Edit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(stretch1Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(stretch1Edit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(stretch2Edit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(stretch2Edit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));

    connect(backgroundColorButton, SIGNAL(clicked()), this, SLOT(setBackgroundColor()));
    connect(domainColorButton, SIGNAL(clicked()), this, SLOT(setDomainColor()));
    connect(domainFrameCheckbox, SIGNAL(toggled(bool)), this, SLOT(setUseDomainFrame(bool)));
    connect(regionFrameCheckbox, SIGNAL(toggled(bool)), this, SLOT(setUseRegionFrame(bool)));
    connect(regionColorButton, SIGNAL(clicked()), this, SLOT(setRegionColor()));
    connect(axisAnnotationCheckbox, SIGNAL(clicked()), this, SLOT(annotationChanged()));
    connect(axisArrowCheckbox, SIGNAL(toggled(bool)), this, SLOT(setUseAxisArrows(bool)));
    connect(arrowXEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(arrowYEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(arrowZEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    //Slots associated with axis annotation:
    connect(xMinTicEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(yMinTicEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(zMinTicEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(xMaxTicEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(yMaxTicEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(zMaxTicEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(xNumTicsEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(yNumTicsEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(zNumTicsEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(xTicSizeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(yTicSizeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(zTicSizeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(axisOriginYEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(axisOriginZEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(axisOriginXEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(latLonCheckbox, SIGNAL(toggled(bool)), this, SLOT(setLatLonAnnot(bool)));
    connect(xTicOrientCombo, SIGNAL(activated(int)), this, SLOT(setXTicOrient(int)));
    connect(yTicOrientCombo, SIGNAL(activated(int)), this, SLOT(setYTicOrient(int)));
    connect(zTicOrientCombo, SIGNAL(activated(int)), this, SLOT(setZTicOrient(int)));
    connect(labelHeightEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(labelDigitsEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(ticWidthEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));

    connect(xMinTicEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(yMinTicEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(zMinTicEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(xMaxTicEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(yMaxTicEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(zMaxTicEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(xNumTicsEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(yNumTicsEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(zNumTicsEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(xTicSizeEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(yTicSizeEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(zTicSizeEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(axisOriginYEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(axisOriginZEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(axisOriginXEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(labelHeightEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(labelDigitsEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(ticWidthEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(arrowXEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(arrowYEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(arrowZEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));

    connect(axisColorButton, SIGNAL(clicked()), this, SLOT(selectAxisColor()));

    connect(timeCombo, SIGNAL(activated(int)), this, SLOT(timeAnnotationChanged()));
    connect(timeLLXEdit, SIGNAL(returnPressed()), this, SLOT(timeLLXChanged()));
    connect(timeLLYEdit, SIGNAL(returnPressed()), this, SLOT(timeLLYChanged()));
    connect(timeSizeEdit, SIGNAL(returnPressed()), this, SLOT(timeSizeChanged()));
    connect(timeColorButton, SIGNAL(pressed()), this, SLOT(timeColorChanged()));
}

void VizFeatureEventRouter::GetWebHelp(
    vector<pair<string, string>> &help) const {
    help.clear();

    help.push_back(make_pair(
        "Overview of the VizFeature tab",
        "http://www.vapor.ucar.edu/docs/vapor-gui-help/vizfeature-tab#VizFeatureOverview"));
}

/*********************************************************************************
 * Slots associated with VizFeatureTab:
 *********************************************************************************/

void VizFeatureEventRouter::
    setVizFeatureTextChanged(const QString &) {
    SetTextChanged(true);
}
void VizFeatureEventRouter::confirmText() {
    if (!_textChangedFlag)
        return;
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();

#ifdef DEAD
    VizFeatureParams *defParams = (VizFeatureParams *)_paramsMgr->GetDefaultParams(Params::_visualizerFeaturesParamsTag);
    assert(defParams);
    Command *cmd = Command::CaptureStart(vParams, "vizfeature text edit", VizFeatureParams::UndoRedo, defParams);

#endif
    _confirmText();

    SetTextChanged(false);

#ifdef DEAD
    vParams->Validate(2);

    Command::CaptureEnd(cmd, vParams, defParams);
#endif
}

void VizFeatureEventRouter::_confirmText() {
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();

    vector<double> stretch;
    stretch.push_back(stretch0Edit->text().toDouble());
    stretch.push_back(stretch1Edit->text().toDouble());
    stretch.push_back(stretch2Edit->text().toDouble());
    //Force them to have minimum = 1:
    double minfactor = 1.e32;
    for (int i = 0; i < 3; i++) {
        if (stretch[i] == 0.)
            stretch[i] = 1.;
        if (stretch[i] < 0.)
            stretch[i] = -stretch[i];
        if (stretch[i] < minfactor)
            minfactor = stretch[i];
    }
    //Make the smallest one be one.
    if (minfactor != 1.0) {
        for (int i = 0; i < 3; i++)
            stretch[i] /= minfactor;
    }

    vector<double> dvals(3, 0.);
    vector<long> lvals(3, 0);
    dvals[0] = xMinTicEdit->text().toDouble();
    dvals[1] = yMinTicEdit->text().toDouble();
    dvals[2] = zMinTicEdit->text().toDouble();
    vParams->SetMinTics(dvals);
    dvals[0] = xMaxTicEdit->text().toDouble();
    dvals[1] = yMaxTicEdit->text().toDouble();
    dvals[2] = zMaxTicEdit->text().toDouble();
    vParams->SetMaxTics(dvals);
    dvals[0] = xTicSizeEdit->text().toDouble();
    dvals[1] = yTicSizeEdit->text().toDouble();
    dvals[2] = zTicSizeEdit->text().toDouble();
    vParams->SetTicSize(dvals);
    dvals[0] = axisOriginXEdit->text().toDouble();
    dvals[1] = axisOriginYEdit->text().toDouble();
    dvals[2] = axisOriginZEdit->text().toDouble();
    vParams->SetAxisOrigin(dvals);
    lvals[0] = xNumTicsEdit->text().toInt();
    lvals[1] = yNumTicsEdit->text().toInt();
    lvals[2] = zNumTicsEdit->text().toInt();
    vParams->SetNumTics(lvals);
    vParams->SetAxisTextHeight(labelHeightEdit->text().toInt());
    vParams->SetAxisDigits(labelDigitsEdit->text().toInt());
    vParams->SetTicWidth(ticWidthEdit->text().toDouble());

    dvals[0] = arrowXEdit->text().toDouble();
    dvals[1] = arrowYEdit->text().toDouble();
    dvals[2] = arrowZEdit->text().toDouble();
    vParams->SetAxisArrowCoords(dvals);
    invalidateText();
}

void VizFeatureEventRouter::
    vizfeatureReturnPressed(void) {
    confirmText();
}

//Insert values from params into tab panel
//
void VizFeatureEventRouter::_updateTab() {
    cout << "VizFeatureEventRouter::_updateTab() BLOCKED" << endl;
    return;

    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();

    QPalette pal(regionColorEdit->palette());
    double clr[3];
    vParams->GetRegionColor(clr);
    QColor newColor = QColor((int)(clr[0] * 255), (int)(clr[1] * 255), (int)(clr[2] * 255));
    pal.setColor(QPalette::Base, newColor);
    regionColorEdit->setPalette(pal);
    QPalette pal1(domainColorEdit->palette());
    vParams->GetDomainColor(clr);
    newColor = QColor((int)(clr[0] * 255), (int)(clr[1] * 255), (int)(clr[2] * 255));
    pal1.setColor(QPalette::Base, newColor);
    domainColorEdit->setPalette(pal1);
    QPalette pal2(backgroundColorEdit->palette());
    vParams->GetBackgroundColor(clr);
    newColor = QColor((int)(clr[0] * 255), (int)(clr[1] * 255), (int)(clr[2] * 255));
    pal2.setColor(QPalette::Base, newColor);
    backgroundColorEdit->setPalette(pal2);

    domainFrameCheckbox->setChecked(vParams->GetUseDomainFrame());
    regionFrameCheckbox->setChecked(vParams->GetUseRegionFrame());

#ifdef DEAD
    string projString = _controlExec->GetDataMgr()->GetMapProjection();
#else
    string projString;
#endif
    if (projString.size() == 0)
        latLonCheckbox->setEnabled(false);
    else {
        latLonCheckbox->setEnabled(true);
        latLonCheckbox->setChecked(vParams->GetLatLonAxes());
    }
    vector<long> ticDir = vParams->GetTicDirs();
    xTicOrientCombo->setCurrentIndex(ticDir[0] - 1);
    yTicOrientCombo->setCurrentIndex(ticDir[1] / 2);
    zTicOrientCombo->setCurrentIndex(ticDir[2]);
    bool axisAnnot = vParams->GetAxisAnnotation();
    axisAnnotationCheckbox->setChecked(axisAnnot);
    if (axisAnnot) {
        axisAnnotationFrame->show();
    } else {
        axisAnnotationFrame->hide();
    }
    vector<long> numtics = vParams->GetNumTics();
    vector<double> mintics = vParams->GetMinTics();
    vector<double> maxtics = vParams->GetMaxTics();
    vector<double> ticSizes = vParams->GetTicSize();

    vector<double> orig = vParams->GetAxisOrigin();
    axisOriginXEdit->setText(QString::number(orig[0]));
    axisOriginYEdit->setText(QString::number(orig[1]));
    axisOriginZEdit->setText(QString::number(orig[2]));
    xMinTicEdit->setText(QString::number(mintics[0]));
    yMinTicEdit->setText(QString::number(mintics[1]));
    zMinTicEdit->setText(QString::number(mintics[2]));
    xMaxTicEdit->setText(QString::number(maxtics[0]));
    yMaxTicEdit->setText(QString::number(maxtics[1]));
    zMaxTicEdit->setText(QString::number(maxtics[2]));
    xTicSizeEdit->setText(QString::number(ticSizes[0]));
    yTicSizeEdit->setText(QString::number(ticSizes[1]));
    zTicSizeEdit->setText(QString::number(ticSizes[2]));
    xNumTicsEdit->setText(QString::number(numtics[0]));
    yNumTicsEdit->setText(QString::number(numtics[1]));
    zNumTicsEdit->setText(QString::number(numtics[2]));
    labelHeightEdit->setText(QString::number(vParams->GetAxisTextHeight()));
    labelDigitsEdit->setText(QString::number(vParams->GetAxisDigits()));
    ticWidthEdit->setText(QString::number(vParams->GetTicWidth()));
    QPalette pal3(axisColorEdit->palette());
    vParams->GetAxisColor(clr);
    newColor = QColor((int)(clr[0] * 255), (int)(clr[1] * 255), (int)(clr[2] * 255));
    pal3.setColor(QPalette::Base, newColor);
    axisColorEdit->setPalette(pal3);

    //Axis arrows:
    vector<double> axisArrowCoords = vParams->GetAxisArrowCoords();
    arrowXEdit->setText(QString::number(axisArrowCoords[0]));
    arrowYEdit->setText(QString::number(axisArrowCoords[1]));
    arrowZEdit->setText(QString::number(axisArrowCoords[2]));
    axisArrowCheckbox->setChecked(vParams->ShowAxisArrows());

    adjustSize();
}

void VizFeatureEventRouter::setRegionColor() {

    QPalette pal(regionColorEdit->palette());
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
    if (!newColor.isValid())
        return;
    pal.setColor(QPalette::Base, newColor);
    regionColorEdit->setPalette(pal);
    vector<double> rgb;
    rgb.push_back((double)newColor.red() / 256.);
    rgb.push_back((double)newColor.green() / 256.);
    rgb.push_back((double)newColor.blue() / 256.);
    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetRegionColor(rgb);
}
void VizFeatureEventRouter::setDomainColor() {
    QPalette pal(domainColorEdit->palette());
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
    if (!newColor.isValid())
        return;
    pal.setColor(QPalette::Base, newColor);
    domainColorEdit->setPalette(pal);
    vector<double> rgb;
    rgb.push_back((double)newColor.red() / 256.);
    rgb.push_back((double)newColor.green() / 256.);
    rgb.push_back((double)newColor.blue() / 256.);
    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetDomainColor(rgb);
}
void VizFeatureEventRouter::setBackgroundColor() {
    QPalette pal(backgroundColorEdit->palette());
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
    if (!newColor.isValid())
        return;
    pal.setColor(QPalette::Base, newColor);
    backgroundColorEdit->setPalette(pal);
    vector<double> rgb;
    rgb.push_back((double)newColor.red() / 256.);
    rgb.push_back((double)newColor.green() / 256.);
    rgb.push_back((double)newColor.blue() / 256.);
    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetBackgroundColor(rgb);
    invalidateText();
}
void VizFeatureEventRouter::selectAxisColor() {
    QPalette pal(axisColorEdit->palette());
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
    if (!newColor.isValid())
        return;
    pal.setColor(QPalette::Base, newColor);
    axisColorEdit->setPalette(pal);
    vector<double> rgb;
    rgb.push_back((double)newColor.red() / 256.);
    rgb.push_back((double)newColor.green() / 256.);
    rgb.push_back((double)newColor.blue() / 256.);
    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetAxisColor(rgb);
    invalidateText();
}

void VizFeatureEventRouter::timeAnnotationChanged() {
    if (_animConnected == false) {
        _ap = GetAnimationParams();
        bool v = connect(_ap, SIGNAL(timestepChanged()), this, SLOT(timeAnnotationChanged()));
        _animConnected = true;
    }

    MiscParams *miscParams = GetMiscParams();

    int index = timeCombo->currentIndex();
    if (index == 1) {
        miscParams->SetTimeStep(true);
        miscParams->SetTimeStamp(false);
        _controlExec->ClearText();
        drawTimeStep();
    } else if (index == 2) {
        miscParams->SetTimeStamp(true);
        miscParams->SetTimeStep(false);
        _controlExec->ClearText();
        drawTimeStamp();
    } else {
        miscParams->SetTimeStamp(false);
        miscParams->SetTimeStep(false);
        _controlExec->ClearText();
    }
}

void VizFeatureEventRouter::timeLLXChanged() {
    MiscParams *miscParams = GetMiscParams();
    float llx = timeLLXEdit->text().toFloat();

    miscParams->SetTimeAnnotLLX(llx);
    drawTimeStamp();
}

void VizFeatureEventRouter::timeLLYChanged() {
    MiscParams *miscParams = GetMiscParams();
    float lly = timeLLYEdit->text().toFloat();

    miscParams->SetTimeAnnotLLY(lly);
    drawTimeStamp();
}

void VizFeatureEventRouter::timeSizeChanged() {
    MiscParams *miscParams = GetMiscParams();
    float size = timeSizeEdit->text().toFloat();
    miscParams->SetTimeAnnotSize(size);
    drawTimeStamp();
}

void VizFeatureEventRouter::timeColorChanged() {
    MiscParams *miscParams = GetMiscParams();

    QPalette pal(timeColorEdit->palette());
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);
    if (!newColor.isValid())
        return;
    pal.setColor(QPalette::Base, newColor);
    timeColorEdit->setPalette(pal);
    //vector<float> rgb;
    float rgb[3];
    rgb[0] = ((float)newColor.red() / 256.);
    rgb[1] = ((float)newColor.green() / 256.);
    rgb[2] = ((float)newColor.blue() / 256.);
    miscParams->SetTimeAnnotColor(rgb);
    drawTimeStamp();
}

void VizFeatureEventRouter::drawTimeStep(string myString) {
    _controlExec->ClearText();

    if (myString == "") {
        myString = "Timestep: " + std::to_string(GetCurrentTimeStep());
    }

    MiscParams *mp = GetMiscParams();
    int x = mp->GetTimeAnnotLLX();
    int y = mp->GetTimeAnnotLLY();
    int size = mp->GetTimeAnnotSize();
    float color[3];
    mp->GetTimeAnnotColor(color);

    _controlExec->DrawText(myString, x, y, size, color, 1);
}

void VizFeatureEventRouter::drawTimeStamp() {
    MiscParams *mp = GetMiscParams();
    if (mp->GetTimeStep() == true) {
        drawTimeStep();
        return;
    }

    size_t ts = GetCurrentTimeStep();
    DataStatus *ds = _controlExec->getDataStatus();
    vector<double> timeCoords = ds->GetTimeCoordinates();

    double myTime = timeCoords[ts];
    std::ostringstream ss;
    ss << myTime;
    std::string myString = ss.str();
    drawTimeStep(myString);
}

void VizFeatureEventRouter::setXTicOrient(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<long> ticDir = vfParams->GetTicDirs();
    ticDir[0] = xTicOrientCombo->currentIndex() + 1;
    vfParams->SetTicDirs(ticDir);
}
void VizFeatureEventRouter::setYTicOrient(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<long> ticDir = vfParams->GetTicDirs();
    ticDir[1] = yTicOrientCombo->currentIndex() * 2;
    vfParams->SetTicDirs(ticDir);
}
void VizFeatureEventRouter::setZTicOrient(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<long> ticDir = vfParams->GetTicDirs();
    ticDir[2] = zTicOrientCombo->currentIndex();
    vfParams->SetTicDirs(ticDir);
}

void VizFeatureEventRouter::setLatLonAnnot(bool val) {
    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetLatLonAxes(val);
    invalidateText();
}
void VizFeatureEventRouter::
    setUseDomainFrame(bool val) {

    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();

    vfParams->SetUseDomainFrame(val);
}
void VizFeatureEventRouter::
    setUseRegionFrame(bool val) {

    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();

    vfParams->SetUseRegionFrame(val);
}

// Response to a click on axisAnnotation checkbox:
void VizFeatureEventRouter::annotationChanged() {
    bool annotate = axisAnnotationCheckbox->isChecked();
    if (annotate) {
        axisAnnotationFrame->show();
    } else {
        axisAnnotationFrame->hide();
    }
    invalidateText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetAxisAnnotation(annotate);
}
void VizFeatureEventRouter::setUseAxisArrows(bool val) {
    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetShowAxisArrows(val);
}

void VizFeatureEventRouter::invalidateText() {
#ifdef DEAD
    VizFeatureRenderer *vfRender = GetActiveVisualizer()->getVizFeatureRenderer();
    if (vfRender)
        vfRender->invalidateCache();
#endif
}
