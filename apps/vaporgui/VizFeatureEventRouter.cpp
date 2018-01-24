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

    _textSizeCombo = new Combo(axisTextSizeEdit, axisTextSizeSlider, true);
    _digitsCombo = new Combo(axisDigitsEdit, axisDigitsSlider, true);
    _ticWidthCombo = new Combo(ticWidthEdit, ticWidthSlider);
    _annotationVaporTable = new VaporTable(axisAnnotationTable);
    _annotationVaporTable->Reinit((VaporTable::DOUBLE),
                                  (VaporTable::MUTABLE),
                                  (VaporTable::HighlightFlags)(0));

    connectAnnotationWidgets();

    // Disabled for now. Need to add support in VizFeatureRenderer
    //
    /*xMinTicEdit->setEnabled(false);
	yMinTicEdit->setEnabled(false);
	zMinTicEdit->setEnabled(false);
	xMaxTicEdit->setEnabled(false);
	yMaxTicEdit->setEnabled(false);
	zMaxTicEdit->setEnabled(false);
	xNumTicsEdit->setEnabled(false);
	yNumTicsEdit->setEnabled(false);
	zNumTicsEdit->setEnabled(false);
	xTicSizeEdit->setEnabled(false);
	yTicSizeEdit->setEnabled(false);
	zTicSizeEdit->setEnabled(false);
	axisOriginXEdit->setEnabled(false);
	axisOriginYEdit->setEnabled(false);
	axisOriginZEdit->setEnabled(false);
	xTicOrientCombo->setEnabled(false);
	yTicOrientCombo->setEnabled(false);
	zTicOrientCombo->setEnabled(false);
	labelHeightEdit->setEnabled(false);
	labelDigitsEdit->setEnabled(false);
	ticWidthEdit->setEnabled(false);*/

    _annotationsInitialized = false;
    _animConnected = false;
    _ap = NULL;
}

VizFeatureEventRouter::~VizFeatureEventRouter() {
}

void VizFeatureEventRouter::connectAnnotationWidgets() {
    connect(axisAnnotationEnabledCheckbox, SIGNAL(toggled(bool)),
            this, SLOT(setAxisAnnotation(bool)));
    connect(latLonAnnotationCheckbox, SIGNAL(toggled(bool)),
            this, SLOT(setLatLonAnnotation(bool)));
    connect(_textSizeCombo, SIGNAL(valueChanged(int)),
            this, SLOT(setAxisTextSize(int)));
    connect(_digitsCombo, SIGNAL(valueChanged(int)),
            this, SLOT(setAxisDigits(int)));
    connect(_ticWidthCombo, SIGNAL(valueChanged(double)),
            this, SLOT(setAxisTicWidth(double)));
    connect(axisColorButton, SIGNAL(pressed()),
            this, SLOT(setAxisColor()));
    connect(_annotationVaporTable, SIGNAL(valueChanged(int, int)),
            this, SLOT(axisAnnotationTableChanged()));
    connect(xTicOrientationCombo, SIGNAL(activated(int)),
            this, SLOT(setXTicOrientation(int)));
    connect(yTicOrientationCombo, SIGNAL(activated(int)),
            this, SLOT(setYTicOrientation(int)));
    connect(zTicOrientationCombo, SIGNAL(activated(int)),
            this, SLOT(setZTicOrientation(int)));
    connect(_annotationVaporTable, SIGNAL(valueChanged(int, int)),
            this, SLOT(axisAnnotationTableChanged()));
}

/**********************************************************
 * Whenever a new vizfeaturetab is created it must be hooked up here
 ************************************************************/
void VizFeatureEventRouter::hookUpTab() {
    connect(
        backgroundColorButton, SIGNAL(clicked()),
        this, SLOT(setBackgroundColor()));
    connect(
        domainColorButton, SIGNAL(clicked()),
        this, SLOT(setDomainColor()));
    connect(
        domainFrameCheckbox, SIGNAL(clicked()),
        this, SLOT(setUseDomainFrame()));
    connect(
        regionFrameCheckbox, SIGNAL(clicked()),
        this, SLOT(setUseRegionFrame()));
    connect(
        regionColorButton, SIGNAL(clicked()),
        this, SLOT(setRegionColor()));
    connect(
        axisAnnotationCheckbox, SIGNAL(clicked()),
        this, SLOT(setAxisAnnotation2()));
    connect(
        axisArrowCheckbox, SIGNAL(clicked()),
        this, SLOT(setUseAxisArrows()));
    connect(
        arrowXEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        arrowYEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        arrowZEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    //Slots associated with axis annotation:
    connect(
        xMinTicEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        yMinTicEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        zMinTicEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        xMaxTicEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        yMaxTicEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        zMaxTicEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        xNumTicsEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        yNumTicsEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        zNumTicsEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        xTicSizeEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        yTicSizeEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        zTicSizeEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        axisOriginYEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        axisOriginZEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        axisOriginXEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        latLonCheckbox, SIGNAL(toggled(bool)),
        this, SLOT(setLatLonAnnot(bool)));
    connect(
        xTicOrientCombo, SIGNAL(activated(int)),
        this, SLOT(setXTicOrient(int)));
    connect(
        yTicOrientCombo, SIGNAL(activated(int)),
        this, SLOT(setYTicOrient(int)));
    connect(
        zTicOrientCombo, SIGNAL(activated(int)),
        this, SLOT(setZTicOrient(int)));
    connect(
        labelHeightEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        labelDigitsEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(
        ticWidthEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(setVizFeatureTextChanged(const QString &)));

    connect(
        xMinTicEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        yMinTicEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        zMinTicEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        xMaxTicEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        yMaxTicEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        zMaxTicEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        xNumTicsEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        yNumTicsEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        zNumTicsEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        xTicSizeEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        yTicSizeEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        zTicSizeEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        axisOriginYEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        axisOriginZEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        axisOriginXEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        labelHeightEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        labelDigitsEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        ticWidthEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        arrowXEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        arrowYEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));
    connect(
        arrowZEdit, SIGNAL(returnPressed()),
        this, SLOT(vizfeatureReturnPressed()));

    connect(
        axisColorButton, SIGNAL(clicked()),
        this, SLOT(setAxisColor()));

    connect(
        timeCombo, SIGNAL(activated(int)),
        this, SLOT(timeAnnotationChanged()));
    connect(
        timeLLXEdit, SIGNAL(returnPressed()),
        this, SLOT(timeLLXChanged()));
    connect(
        timeLLYEdit, SIGNAL(returnPressed()),
        this, SLOT(timeLLYChanged()));
    connect(
        timeSizeEdit, SIGNAL(returnPressed()),
        this, SLOT(timeSizeChanged()));
    connect(
        timeColorButton, SIGNAL(clicked()),
        this, SLOT(setTimeColor()));
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
#ifdef DEAD
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();

    vector<double> stretch;
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
#endif
}

void VizFeatureEventRouter::
    vizfeatureReturnPressed(void) {
    confirmText();
}

//Insert values from params into tab panel
//
void VizFeatureEventRouter::_updateTab() {
    updateRegionColor();
    updateDomainColor();
    updateBackgroundColor();
    updateTimeColor();
    updateAxisAnnotations();

    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();

    domainFrameCheckbox->setChecked(vParams->GetUseDomainFrame());
    regionFrameCheckbox->setChecked(vParams->GetUseRegionFrame());

    bool axisAnnot = vParams->GetAxisAnnotation();
    axisAnnotationCheckbox->setChecked(axisAnnot);
    if (axisAnnot) {
        axisAnnotationFrame->show();
    } else {
        axisAnnotationFrame->hide();
    }

    // Axis arrows:
    //
    vector<double> axisArrowCoords = vParams->GetAxisArrowCoords();
    arrowXEdit->setText(QString::number(axisArrowCoords[0]));
    arrowYEdit->setText(QString::number(axisArrowCoords[1]));
    arrowZEdit->setText(QString::number(axisArrowCoords[2]));
    axisArrowCheckbox->setChecked(vParams->GetShowAxisArrows());

    return;

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

    adjustSize();
}

void VizFeatureEventRouter::updateAnnotationCheckbox() {
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();
    bool annotationEnabled = vParams->GetAxisAnnotation();
    if (annotationEnabled)
        axisAnnotationEnabledCheckbox->setCheckState(Qt::Checked);
    else
        axisAnnotationEnabledCheckbox->setCheckState(Qt::Unchecked);
}

void VizFeatureEventRouter::updateLatLonCheckbox() {
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();
    bool annotateLatLon = vParams->GetLatLonAxes();
    if (annotateLatLon)
        latLonAnnotationCheckbox->setCheckState(Qt::Checked);
    else
        latLonAnnotationCheckbox->setCheckState(Qt::Unchecked);
}

void VizFeatureEventRouter::updateTicOrientationCombos() {
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();
    vector<double> ticDir = vParams->GetTicDirs();
    xTicOrientationCombo->setCurrentIndex(ticDir[0] - 1);
    yTicOrientationCombo->setCurrentIndex(ticDir[1] / 2);
    zTicOrientationCombo->setCurrentIndex(ticDir[2]);
}

void VizFeatureEventRouter::updateAnnotationTable() {
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();

    vector<double> values;
    vector<double> numtics = vParams->GetNumTics();
    values.insert(values.end(), numtics.begin(), numtics.end());
    vector<double> ticSizes = vParams->GetTicSize();
    values.insert(values.end(), ticSizes.begin(), ticSizes.end());
    vector<double> minTics = vParams->GetMinTics();
    values.insert(values.end(), minTics.begin(), minTics.end());
    vector<double> maxTics = vParams->GetMaxTics();
    values.insert(values.end(), maxTics.begin(), maxTics.end());
    vector<double> orig = vParams->GetAxisOrigin();
    values.insert(values.end(), orig.begin(), orig.end());

    vector<string> rowHeaders;
    rowHeaders.push_back("# Tics          ");
    rowHeaders.push_back("Size            ");
    rowHeaders.push_back("Min             ");
    rowHeaders.push_back("Max             ");
    rowHeaders.push_back("Origin          ");

    vector<string> colHeaders;
    colHeaders.push_back("X");
    colHeaders.push_back("Y");
    colHeaders.push_back("Z");

    _annotationVaporTable->Update(5, 3, values, rowHeaders, colHeaders);
}

void VizFeatureEventRouter::updateOldGui() {
    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();
    vector<double> values;
    vector<double> numtics = vParams->GetNumTics();
    values.insert(values.end(), numtics.begin(), numtics.end());
    vector<double> ticSizes = vParams->GetTicSize();
    values.insert(values.end(), ticSizes.begin(), ticSizes.end());
    vector<double> minTics = vParams->GetMinTics();
    values.insert(values.end(), minTics.begin(), minTics.end());
    vector<double> maxTics = vParams->GetMaxTics();
    values.insert(values.end(), maxTics.begin(), maxTics.end());
    vector<double> orig = vParams->GetAxisOrigin();
    values.insert(values.end(), orig.begin(), orig.end());
    vector<double> mintics = vParams->GetMinTics();
    vector<double> maxtics = vParams->GetMaxTics();

    double clr[3];
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
    QColor newColor = QColor((int)(clr[0] * 255), (int)(clr[1] * 255), (int)(clr[2] * 255));
    pal3.setColor(QPalette::Base, newColor);
    axisColorEdit->setPalette(pal3);
}

void VizFeatureEventRouter::getActiveExtents(
    vector<double> &minExts, vector<double> &maxExts) {
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    size_t ts = GetCurrentTimeStep();
    DataStatus *dataStatus = _controlExec->GetDataStatus();
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);
}

void VizFeatureEventRouter::initializeTicSizes() {
    vector<double> ticSizes, minExts, maxExts;
    getActiveExtents(minExts, maxExts);
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();

    // Initialize tic length to 2.5% of the domain that they're oriented on.
    // X starts oriented on Y,
    // Y starts oriented on X,
    // Z starts oriented on X
    double xTicSize = (maxExts[1] - minExts[1]) * .025;
    double yzTicSize = (maxExts[0] - minExts[0]) * .025;
    ticSizes.push_back(xTicSize);
    ticSizes.push_back(yzTicSize);
    ticSizes.push_back(yzTicSize);
    vfParams->SetTicSize(ticSizes);
}

void VizFeatureEventRouter::initializeAnnotationExtents() {
    vector<double> minExts, maxExts;
    getActiveExtents(minExts, maxExts);

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetMinTics(minExts);
    vfParams->SetMaxTics(maxExts);
    vfParams->SetAxisOrigin(minExts);
}

void VizFeatureEventRouter::initializeAnnotations() {
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->BeginSaveStateGroup("Initialize axis annotation table");

    initializeAnnotationExtents();
    initializeTicSizes();

    paramsMgr->EndSaveStateGroup();

    _annotationsInitialized = true;
}

void VizFeatureEventRouter::updateAxisAnnotations() {
    if (!_annotationsInitialized)
        initializeAnnotations();

    updateAnnotationCheckbox();
    updateLatLonCheckbox();
    updateTicOrientationCombos();
    updateAnnotationTable();
    updateAxisColor();

    VizFeatureParams *vParams = (VizFeatureParams *)GetActiveParams();

    int textSize = vParams->GetAxisFontSize();
    _textSizeCombo->Update(4, 50, textSize);

    int numDigits = vParams->GetAxisDigits();
    _digitsCombo->Update(1, 12, numDigits);

    GLdouble minMax[2];
    glGetDoublev(GL_ALIASED_LINE_WIDTH_RANGE, minMax);
    double ticWidth = vParams->GetTicWidth();
    _ticWidthCombo->Update(minMax[0], minMax[1], ticWidth);
}

void VizFeatureEventRouter::axisAnnotationTableChanged() {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();

    vector<double> values;
    values = getTableRow(0);
    vfParams->SetNumTics(values);

    /*vector<double> foo;
	_annotationVaporTable->GetRow(0, foo);

	for (int i=0; i<3; i++) {
		cout << "v&foo " << i << " " << values[i] << " " << foo[i] << endl;
	}*/

    values = getTableRow(1);
    vfParams->SetTicSize(values);

    values = getTableRow(2);
    vfParams->SetMinTics(values);

    values = getTableRow(3);
    vfParams->SetMaxTics(values);

    values = getTableRow(4);
    vfParams->SetAxisOrigin(values);
}

vector<double> VizFeatureEventRouter::getTableRow(int row) {
    vector<double> contents;
    cout << row << endl;
    for (int col = 0; col < 3; col++) {
        double val = _annotationVaporTable->GetValue(row, col);
        contents.push_back(val);
    }
    return contents;
}

void VizFeatureEventRouter::setColorHelper(
    QWidget *w, vector<double> &rgb) {
    rgb.clear();

    QPalette pal(w->palette());
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);

    rgb.push_back(newColor.red() / 255.0);
    rgb.push_back(newColor.green() / 255.0);
    rgb.push_back(newColor.blue() / 255.0);
}

void VizFeatureEventRouter::updateColorHelper(
    const vector<double> &rgb, QWidget *w) {
    assert(rgb.size() == 3);

    QColor color((int)(rgb[0] * 255.0), (int)(rgb[1] * 255.0), (int)(rgb[2] * 255.0));

    QPalette pal;
    pal.setColor(QPalette::Base, color);
    w->setPalette(pal);
}

void VizFeatureEventRouter::setRegionColor() {

    vector<double> rgb;

    setColorHelper(regionColorEdit, rgb);
    if (rgb.size() != 3)
        return;

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetRegionColor(rgb);
}

void VizFeatureEventRouter::updateRegionColor() {

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> rgb;
    vfParams->GetRegionColor(rgb);

    updateColorHelper(rgb, regionColorEdit);
}

void VizFeatureEventRouter::setDomainColor() {
    vector<double> rgb;

    setColorHelper(domainColorEdit, rgb);
    if (rgb.size() != 3)
        return;

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetDomainColor(rgb);
}

void VizFeatureEventRouter::updateDomainColor() {

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> rgb;
    vfParams->GetDomainColor(rgb);

    updateColorHelper(rgb, domainColorEdit);
}

void VizFeatureEventRouter::setBackgroundColor() {
    vector<double> rgb;

    setColorHelper(backgroundColorEdit, rgb);
    if (rgb.size() != 3)
        return;

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetBackgroundColor(rgb);
}

void VizFeatureEventRouter::updateBackgroundColor() {

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> rgb;
    vfParams->GetBackgroundColor(rgb);

    updateColorHelper(rgb, backgroundColorEdit);
}

void VizFeatureEventRouter::setAxisColor() {
    vector<double> rgb;

    setColorHelper(axisColorEdit, rgb);
    if (rgb.size() != 3)
        return;

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetAxisColor(rgb);
}

void VizFeatureEventRouter::updateAxisColor() {

    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> rgb;
    vfParams->GetAxisColor(rgb);

    updateColorHelper(rgb, oldAxisColorEdit);
    updateColorHelper(rgb, axisColorEdit);
}

void VizFeatureEventRouter::setTimeColor() {
    vector<double> rgb;

    setColorHelper(timeColorEdit, rgb);
    if (rgb.size() != 3)
        return;

    MiscParams *miscParams = GetMiscParams();
    miscParams->SetTimeAnnotColor(rgb);
}

void VizFeatureEventRouter::updateTimeColor() {

    MiscParams *miscParams = GetMiscParams();
    vector<double> rgb;
    miscParams->GetTimeAnnotColor(rgb);

    updateColorHelper(rgb, timeColorEdit);
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
        drawTimeUser();
    } else if (index == 3) {
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

    cout << "Drawing timestep" << endl;
    _controlExec->DrawText(myString, x, y, size, color, 1);
}

void VizFeatureEventRouter::drawTimeUser() {
    MiscParams *mp = GetMiscParams();
    if (mp->GetTimeStep() == true) {
        drawTimeStep();
        return;
    }

    size_t ts = GetCurrentTimeStep();
    DataStatus *ds = _controlExec->GetDataStatus();
    vector<double> timeCoords = ds->GetTimeCoordinates();

    double myTime = timeCoords[ts];
    std::ostringstream ss;
    ss << myTime;
    std::string myString = ss.str();
    drawTimeStep(myString);
}

void VizFeatureEventRouter::drawTimeStamp() {
    MiscParams *mp = GetMiscParams();
    if (mp->GetTimeStep() == true) {
        drawTimeStep();
        return;
    }

    size_t ts = GetCurrentTimeStep();
    DataStatus *ds = _controlExec->GetDataStatus();
    drawTimeStep(ds->GetTimeCoordsFormatted()[ts]);
}

void VizFeatureEventRouter::setAxisDigits(int digits) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetAxisDigits(digits);
}

void VizFeatureEventRouter::setAxisTicWidth(double width) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetTicWidth(width);
}

void VizFeatureEventRouter::setXTicOrientation(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> ticDir = vfParams->GetTicDirs();
    ticDir[0] = xTicOrientationCombo->currentIndex() + 1; // Y(1) or Z(2)
    vfParams->SetTicDirs(ticDir);
}
void VizFeatureEventRouter::setYTicOrientation(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> ticDir = vfParams->GetTicDirs();
    ticDir[1] = yTicOrientationCombo->currentIndex() * 2; // X(0) or Z(2)
    vfParams->SetTicDirs(ticDir);
}
void VizFeatureEventRouter::setZTicOrientation(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> ticDir = vfParams->GetTicDirs();
    ticDir[2] = zTicOrientationCombo->currentIndex(); // X(0) or Y(1)
    vfParams->SetTicDirs(ticDir);
}

void VizFeatureEventRouter::setXTicOrient(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> ticDir = vfParams->GetTicDirs();
    ticDir[0] = xTicOrientCombo->currentIndex() + 1;
    vfParams->SetTicDirs(ticDir);
}
void VizFeatureEventRouter::setYTicOrient(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> ticDir = vfParams->GetTicDirs();
    ticDir[1] = yTicOrientCombo->currentIndex() * 2;
    vfParams->SetTicDirs(ticDir);
}
void VizFeatureEventRouter::setZTicOrient(int) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vector<double> ticDir = vfParams->GetTicDirs();
    ticDir[2] = zTicOrientCombo->currentIndex();
    vfParams->SetTicDirs(ticDir);
}

void VizFeatureEventRouter::setLatLonAnnot(bool val) {
    confirmText();
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetLatLonAxes(val);
    invalidateText();
}

void VizFeatureEventRouter::setUseDomainFrame() {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetUseDomainFrame(domainFrameCheckbox->isChecked());
}

void VizFeatureEventRouter::setUseRegionFrame() {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetUseRegionFrame(regionFrameCheckbox->isChecked());
}

void VizFeatureEventRouter::setAxisAnnotation(bool toggled) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetAxisAnnotation(toggled);
}

void VizFeatureEventRouter::setLatLonAnnotation(bool val) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetLatLonAxes(val);
}

void VizFeatureEventRouter::setAxisTextSize(int size) {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetAxisFontSize(size);
}

// Response to a click on axisAnnotation checkbox:
void VizFeatureEventRouter::setAxisAnnotation2() {
    bool annotate = axisAnnotationCheckbox->isChecked();
    if (annotate) {
        axisAnnotationFrame->show();
    } else {
        axisAnnotationFrame->hide();
    }
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetAxisAnnotation(annotate);
}

void VizFeatureEventRouter::setUseAxisArrows() {
    VizFeatureParams *vfParams = (VizFeatureParams *)GetActiveParams();
    vfParams->SetShowAxisArrows(axisArrowCheckbox->isChecked());
}

void VizFeatureEventRouter::invalidateText() {
#ifdef DEAD
    VizFeatureRenderer *vfRender = GetActiveVisualizer()->getVizFeatureRenderer();
    if (vfRender)
        vfRender->invalidateCache();
#endif
}
