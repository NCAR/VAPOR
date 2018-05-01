//************************************************************************
//															*
//			 Copyright (C)  2015										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		AnnotationsEventRouter.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the AnnotationsEventRouter class.
//		This class supports routing messages from the gui to the params
//		This is derived from the vizfeature Widget
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100 4996)
#endif
#include <qdesktopwidget.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include "GL/glew.h"
#include <vapor/AnnotationsParams.h>

#include "ui_AnnotationsGUI.h"
#include "qcolordialog.h"

#include <qlabel.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "AnnotationsEventRouter.h"
#include "vapor/ControlExecutive.h"
#include "EventRouter.h"

using namespace VAPoR;

namespace {

template<typename Out> void split(const std::string &s, char delim, Out result)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) { *(result++) = item; }
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

}    // namespace

AnnotationsEventRouter::AnnotationsEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), Ui_AnnotationsGUI(), EventRouter(ce, AnnotationsParams::GetClassType())
{
    setupUi(this);

    _textSizeCombo = new Combo(axisTextSizeEdit, axisTextSizeSlider, true);
    _digitsCombo = new Combo(axisDigitsEdit, axisDigitsSlider, true);
    _ticWidthCombo = new Combo(ticWidthEdit, ticWidthSlider);
    _annotationVaporTable = new VaporTable(axisAnnotationTable);
    _annotationVaporTable->Reinit((VaporTable::DOUBLE), (VaporTable::MUTABLE), (VaporTable::HighlightFlags)(0));

    connectAnnotationWidgets();

    setCurrentAxisDataMgr(0);

    _animConnected = false;
    _ap = NULL;
}

AnnotationsEventRouter::~AnnotationsEventRouter() {}

void AnnotationsEventRouter::connectAnnotationWidgets()
{
    connect(axisAnnotationEnabledCheckbox, SIGNAL(toggled(bool)), this, SLOT(setAxisAnnotation(bool)));
    connect(latLonAnnotationCheckbox, SIGNAL(toggled(bool)), this, SLOT(setLatLonAnnotation(bool)));
    connect(_textSizeCombo, SIGNAL(valueChanged(int)), this, SLOT(setAxisTextSize(int)));
    connect(_digitsCombo, SIGNAL(valueChanged(int)), this, SLOT(setAxisDigits(int)));
    connect(_ticWidthCombo, SIGNAL(valueChanged(double)), this, SLOT(setAxisTicWidth(double)));
    connect(axisColorButton, SIGNAL(pressed()), this, SLOT(setAxisColor()));
    connect(axisBackgroundColorButton, SIGNAL(pressed()), this, SLOT(setAxisBackgroundColor()));
    connect(_annotationVaporTable, SIGNAL(returnPressed()), this, SLOT(axisAnnotationTableChanged()));
    connect(xTicOrientationCombo, SIGNAL(activated(int)), this, SLOT(setXTicOrientation(int)));
    connect(yTicOrientationCombo, SIGNAL(activated(int)), this, SLOT(setYTicOrientation(int)));
    connect(zTicOrientationCombo, SIGNAL(activated(int)), this, SLOT(setZTicOrientation(int)));
    connect(dataMgrSelectorCombo, SIGNAL(activated(int)), this, SLOT(setCurrentAxisDataMgr(int)));
    connect(copyRegionButton, SIGNAL(pressed()), this, SLOT(copyRegionFromRenderer()));
}

/**********************************************************
 * Whenever a new vizfeaturetab is created it must be hooked up here
 ************************************************************/
void AnnotationsEventRouter::hookUpTab()
{
    connect(backgroundColorButton, SIGNAL(clicked()), this, SLOT(setBackgroundColor()));
    connect(domainColorButton, SIGNAL(clicked()), this, SLOT(setDomainColor()));
    connect(domainFrameCheckbox, SIGNAL(clicked()), this, SLOT(setUseDomainFrame()));
    connect(regionColorButton, SIGNAL(clicked()), this, SLOT(setRegionColor()));
    connect(axisArrowCheckbox, SIGNAL(clicked()), this, SLOT(setUseAxisArrows()));
    connect(arrowXEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(arrowYEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(arrowZEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setVizFeatureTextChanged(const QString &)));
    connect(arrowXEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(arrowYEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));
    connect(arrowZEdit, SIGNAL(returnPressed()), this, SLOT(vizfeatureReturnPressed()));

    connect(timeCombo, SIGNAL(activated(int)), this, SLOT(timeAnnotationChanged()));
    connect(timeLLXEdit, SIGNAL(returnPressed()), this, SLOT(timeLLXChanged()));
    connect(timeLLYEdit, SIGNAL(returnPressed()), this, SLOT(timeLLYChanged()));
    connect(timeSizeEdit, SIGNAL(returnPressed()), this, SLOT(timeSizeChanged()));
    connect(timeColorButton, SIGNAL(clicked()), this, SLOT(setTimeColor()));
}

void AnnotationsEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Overview of the VizFeature tab", "http://www.vapor.ucar.edu/docs/vapor-gui-help/vizfeature-tab#VizFeatureOverview"));
}

/*********************************************************************************
 * Slots associated with VizFeatureTab:
 *********************************************************************************/

void AnnotationsEventRouter::setVizFeatureTextChanged(const QString &) { SetTextChanged(true); }
void AnnotationsEventRouter::confirmText()
{
    if (!_textChangedFlag) return;
    AnnotationsParams *vParams = (AnnotationsParams *)GetActiveParams();

#ifdef VAPOR3_0_0_ALPHA
    AnnotationsParams *defParams = (AnnotationsParams *)_paramsMgr->GetDefaultParams(Params::_visualizerFeaturesParamsTag);
    assert(defParams);
    Command *cmd = Command::CaptureStart(vParams, "vizfeature text edit", AnnotationsParams::UndoRedo, defParams);

#endif
    _confirmText();

    SetTextChanged(false);

#ifdef VAPOR3_0_0_ALPHA
    vParams->Validate(2);

    Command::CaptureEnd(cmd, vParams, defParams);
#endif
}

void AnnotationsEventRouter::_confirmText()
{
#ifdef VAPOR3_0_0_ALPHA
    AnnotationsParams *vParams = (AnnotationsParams *)GetActiveParams();

    vector<double> stretch;
    // Force them to have minimum = 1:
    double minfactor = 1.e32;
    for (int i = 0; i < 3; i++) {
        if (stretch[i] == 0.) stretch[i] = 1.;
        if (stretch[i] < 0.) stretch[i] = -stretch[i];
        if (stretch[i] < minfactor) minfactor = stretch[i];
    }
    // Make the smallest one be one.
    if (minfactor != 1.0) {
        for (int i = 0; i < 3; i++) stretch[i] /= minfactor;
    }

    vector<double> dvals(3, 0.);
    vector<long>   lvals(3, 0);
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
#endif
}

void AnnotationsEventRouter::vizfeatureReturnPressed(void) { confirmText(); }

// Insert values from params into tab panel
//
void AnnotationsEventRouter::_updateTab()
{
    updateRegionColor();
    updateDomainColor();
    updateBackgroundColor();
    updateTimeColor();
    updateAxisAnnotations();

    AnnotationsParams *vParams = (AnnotationsParams *)GetActiveParams();

    domainFrameCheckbox->setChecked(vParams->GetUseDomainFrame());

    // Axis arrows:
    //
    vector<double> axisArrowCoords = vParams->GetAxisArrowCoords();
    arrowXEdit->setText(QString::number(axisArrowCoords[0]));
    arrowYEdit->setText(QString::number(axisArrowCoords[1]));
    arrowZEdit->setText(QString::number(axisArrowCoords[2]));
    axisArrowCheckbox->setChecked(vParams->GetShowAxisArrows());

    return;

    QPalette pal(regionColorEdit->palette());
    double   clr[3];
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

    adjustSize();
}

void AnnotationsEventRouter::updateDataMgrCombo()
{
    // Save current selection
    AnnotationsParams *vParams = (AnnotationsParams *)GetActiveParams();
    string             currentSelection = vParams->GetCurrentAxisDataMgrName();

    // Repopulate the combo's entries
    ParamsMgr *    pMgr = _controlExec->GetParamsMgr();
    vector<string> names = pMgr->GetDataMgrNames();
    dataMgrSelectorCombo->clear();
    for (int i = 0; i < names.size(); i++) {
        QString name = QString::fromStdString(names[i]);
        dataMgrSelectorCombo->addItem(name);
    }

    // Reset the saved selection
    QString qCurrentSelection = QString::fromStdString(currentSelection);
    int     index = dataMgrSelectorCombo->findText(qCurrentSelection);
    if (index > 0)
        dataMgrSelectorCombo->setCurrentIndex(index);
    else {
        dataMgrSelectorCombo->setCurrentIndex(0);
        setCurrentAxisDataMgr(0);
    }
}

void AnnotationsEventRouter::copyRegionFromRenderer()
{
    string copyString = copyRegionCombo->currentText().toStdString();
    if (copyString == "") return;

    std::vector<std::string> elems = split(copyString, ':');
    string                   visualizer = _visNames[elems[0]];
    string                   dataSetName = elems[1];
    string                   renType = _renTypeNames[elems[2]];
    string                   renderer = elems[3];

    ParamsMgr *   paramsMgr = _controlExec->GetParamsMgr();
    RenderParams *copyParams = paramsMgr->GetRenderParams(visualizer, dataSetName, renType, renderer);
    assert(copyParams);

    Box *               copyBox = copyParams->GetBox();
    std::vector<double> minExtents, maxExtents;
    copyBox->GetExtents(minExtents, maxExtents);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    if (copyBox->IsPlanar()) {
        std::vector<double> currentMin = aa->GetMinTics();
        std::vector<double> currentMax = aa->GetMaxTics();
        scaleNormalizedCoordsToWorld(currentMin);
        scaleNormalizedCoordsToWorld(currentMax);
        minExtents.push_back(currentMin[2]);
        maxExtents.push_back(currentMax[2]);
    }

    scaleWorldCoordsToNormalized(minExtents);
    scaleWorldCoordsToNormalized(maxExtents);

    paramsMgr->BeginSaveStateGroup("Copying extents from renderer to"
                                   " AxisAnnotation");
    aa->SetAxisOrigin(minExtents);
    aa->SetMinTics(minExtents);
    aa->SetMaxTics(maxExtents);
    paramsMgr->EndSaveStateGroup();
}

void AnnotationsEventRouter::updateCopyRegionCombo()
{
    copyRegionCombo->clear();

    AnnotationsParams *vParams = (AnnotationsParams *)GetActiveParams();
    std::string        dataSetName = vParams->GetCurrentAxisDataMgrName();

    _visNames.clear();

    ParamsMgr *              paramsMgr = _controlExec->GetParamsMgr();
    std::vector<std::string> visNames = paramsMgr->GetVisualizerNames();
    for (int i = 0; i < visNames.size(); i++) {
        // Create a mapping of abreviated visualizer names to their
        // actual string values.
        //
        string visAbb = "Vis" + std::to_string(i);
        _visNames[visAbb] = visNames[i];

        std::vector<string> typeNames;
        typeNames = paramsMgr->GetRenderParamsClassNames(visNames[i]);

        for (int j = 0; j < typeNames.size(); j++) {
            // Abbreviate Params names by removing 'Params" from them.
            // Then store them in a map for later reference.
            //
            string typeAbb = typeNames[j];
            int    pos = typeAbb.find("Params");
            typeAbb.erase(pos, 6);
            _renTypeNames[typeAbb] = typeNames[j];

            std::vector<string> renNames;
            renNames = paramsMgr->GetRenderParamInstances(visNames[i], typeNames[j]);

            for (int k = 0; k < renNames.size(); k++) {
                string  displayName = visAbb + ":" + dataSetName + ":" + typeAbb + ":" + renNames[k];
                QString qDisplayName = QString::fromStdString(displayName);
                copyRegionCombo->addItem(qDisplayName);
            }
        }
    }
}

void AnnotationsEventRouter::updateAnnotationCheckbox()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            annotationEnabled = aa->GetAxisAnnotationEnabled();
    if (annotationEnabled)
        axisAnnotationEnabledCheckbox->setCheckState(Qt::Checked);
    else
        axisAnnotationEnabledCheckbox->setCheckState(Qt::Unchecked);
}

void AnnotationsEventRouter::updateLatLonCheckbox()
{
    string      dmName = dataMgrSelectorCombo->currentText().toStdString();
    DataStatus *dataStatus = _controlExec->GetDataStatus();
    DataMgr *   dataMgr = dataStatus->GetDataMgr(dmName);
    assert(dataMgr);
    string projString = dataMgr->GetMapProjection();

    if (projString.size() == 0) {
        latLonAnnotationCheckbox->setEnabled(false);
        return;
    } else
        latLonAnnotationCheckbox->setEnabled(true);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            annotateLatLon = aa->GetLatLonAxesEnabled();
    if (annotateLatLon)
        latLonAnnotationCheckbox->setCheckState(Qt::Checked);
    else
        latLonAnnotationCheckbox->setCheckState(Qt::Unchecked);
}

void AnnotationsEventRouter::updateTicOrientationCombos()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();

    xTicOrientationCombo->setCurrentIndex(ticDir[0] - 1);
    yTicOrientationCombo->setCurrentIndex(ticDir[1] / 2);
    zTicOrientationCombo->setCurrentIndex(ticDir[2]);
}

void AnnotationsEventRouter::scaleNormalizedCoordsToWorld(std::vector<double> &coords)
{
    std::vector<double> extents = getDomainExtents();
    for (int i = 0; i < 3; i++) {
        double offset = coords[i] * (extents[i + 3] - extents[i]);
        double minimum = extents[i];
        coords[i] = offset + minimum;
    }
}

void AnnotationsEventRouter::scaleWorldCoordsToNormalized(std::vector<double> &coords)
{
    std::vector<double> extents = getDomainExtents();
    for (int i = 0; i < 3; i++) {
        double point = coords[i] - extents[i];
        double magnitude = extents[i + 3] - extents[i];
        coords[i] = point / magnitude;
    }
}

void AnnotationsEventRouter::updateAnnotationTable()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            annotationEnabled = aa->GetLatLonAxesEnabled();

    vector<double> tableValues;

    vector<double> numtics = aa->GetNumTics();
    tableValues.insert(tableValues.end(), numtics.begin(), numtics.end());

    vector<double> ticSizes = aa->GetTicSize();
    tableValues.insert(tableValues.end(), ticSizes.begin(), ticSizes.end());

    vector<double> minTics = aa->GetMinTics();
    scaleNormalizedCoordsToWorld(minTics);
    if (annotationEnabled) {
        convertPCSToLon(minTics[0]);
        convertPCSToLat(minTics[1]);
    }
    tableValues.insert(tableValues.end(), minTics.begin(), minTics.end());

    vector<double> maxTics = aa->GetMaxTics();
    scaleNormalizedCoordsToWorld(maxTics);
    if (annotationEnabled) {
        convertPCSToLon(maxTics[0]);
        convertPCSToLat(maxTics[1]);
    }
    tableValues.insert(tableValues.end(), maxTics.begin(), maxTics.end());

    vector<double> origin = aa->GetAxisOrigin();
    scaleNormalizedCoordsToWorld(origin);
    if (annotationEnabled) {
        convertPCSToLon(origin[0]);
        convertPCSToLat(origin[1]);
    }
    tableValues.insert(tableValues.end(), origin.begin(), origin.end());

    vector<string> rowHeaders;
    rowHeaders.push_back("# Tics		  ");
    rowHeaders.push_back("Size			");
    rowHeaders.push_back("Min			 ");
    rowHeaders.push_back("Max			 ");
    rowHeaders.push_back("Origin		  ");

    vector<string> colHeaders;
    colHeaders.push_back("X");
    colHeaders.push_back("Y");
    colHeaders.push_back("Z");

    _annotationVaporTable->Update(5, 3, tableValues, rowHeaders, colHeaders);
}

void AnnotationsEventRouter::convertPCSToLon(double &xCoord)
{
    double dummy = 0.;
    convertPCSToLonLat(xCoord, dummy);
}

void AnnotationsEventRouter::convertPCSToLat(double &yCoord)
{
    double dummy = 0.;
    convertPCSToLonLat(dummy, yCoord);
}

void AnnotationsEventRouter::convertPCSToLonLat(double &xCoord, double &yCoord)
{
    // ParamsMgr * paramsMgr= _controlExec->GetParamsMgr();
    // vector<string> names = paramsMgr->GetDataMgrNames();
    // DataStatus *dataStatus = _controlExec->GetDataStatus();
    // DataMgr* dataMgr = dataStatus->GetDataMgr(names[0]);

    DataStatus *       dataStatus = _controlExec->GetDataStatus();
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    string             dataMgrName = vfParams->GetCurrentAxisDataMgrName();
    DataMgr *          dataMgr = dataStatus->GetDataMgr(dataMgrName);

    double coords[2] = {xCoord, yCoord};
    double coordsForError[2] = {coords[0], coords[1]};

    int rc = DataMgrUtils::ConvertPCSToLonLat(dataMgr, coords, 1);
    if (!rc) { MyBase::SetErrMsg("Could not convert point %f, %f to Lon/Lat", coordsForError[0], coordsForError[1]); }

    xCoord = coords[0];
    yCoord = coords[1];
}

void AnnotationsEventRouter::convertLonToPCS(double &xCoord)
{
    double dummy = 0.;
    convertLonLatToPCS(xCoord, dummy);
}

void AnnotationsEventRouter::convertLatToPCS(double &yCoord)
{
    double dummy = 0.;
    convertLonLatToPCS(dummy, yCoord);
}

void AnnotationsEventRouter::convertLonLatToPCS(double &xCoord, double &yCoord)
{
    // ParamsMgr * paramsMgr= _controlExec->GetParamsMgr();
    // vector<string> names = paramsMgr->GetDataMgrNames();
    // DataStatus *dataStatus = _controlExec->GetDataStatus();
    // DataMgr* dataMgr = dataStatus->GetDataMgr(names[0]);
    DataStatus *       dataStatus = _controlExec->GetDataStatus();
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    string             dataMgrName = vfParams->GetCurrentAxisDataMgrName();
    DataMgr *          dataMgr = dataStatus->GetDataMgr(dataMgrName);

    double coords[2] = {xCoord, yCoord};
    double coordsForError[2] = {coords[0], coords[1]};

    int rc = DataMgrUtils::ConvertLonLatToPCS(dataMgr, coords, 1);
    if (!rc) { MyBase::SetErrMsg("Could not convert point %f, %f to PCS", coordsForError[0], coordsForError[1]); }

    xCoord = coords[0];
    yCoord = coords[1];
}

AxisAnnotation *AnnotationsEventRouter::_getCurrentAxisAnnotation()
{
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    string             dataMgr = vfParams->GetCurrentAxisDataMgrName();
    AxisAnnotation *   aa = vfParams->GetAxisAnnotation(dataMgr);

    bool initialized = aa->GetAxisAnnotationInitialized();
    if (!initialized) initializeAnnotation(aa);

    return aa;
}

std::vector<double> AnnotationsEventRouter::getDomainExtents() const
{
    ParamsMgr *         paramsMgr = _controlExec->GetParamsMgr();
    size_t              ts = GetCurrentTimeStep();
    DataStatus *        dataStatus = _controlExec->GetDataStatus();
    std::vector<double> minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);

    std::vector<double> extents = {minExts[0], minExts[1], minExts[2], maxExts[0], maxExts[1], maxExts[2]};
    return extents;
}

// Initialize tic length to 2.5% of the domain that they're oriented on.
void AnnotationsEventRouter::initializeTicSizes(AxisAnnotation *aa)
{
    vector<double> ticSizes(3, .025);
    aa->SetTicSize(ticSizes);
}

void AnnotationsEventRouter::initializeAnnotationExtents(AxisAnnotation *aa)
{
    vector<double> minExts(3, 0.0);
    vector<double> maxExts(3, 1.0);

    aa->SetMinTics(minExts);
    aa->SetMaxTics(maxExts);
    aa->SetAxisOrigin(minExts);

    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    string             dataMgr = vfParams->GetCurrentAxisDataMgrName();
    aa->SetDataMgrName(dataMgr);
}

void AnnotationsEventRouter::initializeAnnotation(AxisAnnotation *aa)
{
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->BeginSaveStateGroup("Initialize axis annotation table");

    initializeAnnotationExtents(aa);
    initializeTicSizes(aa);

    paramsMgr->EndSaveStateGroup();

    aa->SetAxisAnnotationInitialized(true);
}

void AnnotationsEventRouter::updateAxisAnnotations()
{
    updateDataMgrCombo();
    updateCopyRegionCombo();
    updateAnnotationCheckbox();
    updateLatLonCheckbox();
    updateTicOrientationCombos();
    updateAnnotationTable();
    updateAxisColor();
    updateAxisBackgroundColor();

    AxisAnnotation *aa = _getCurrentAxisAnnotation();

    int textSize = aa->GetAxisFontSize();
    _textSizeCombo->Update(4, 50, textSize);

    int numDigits = aa->GetAxisDigits();
    _digitsCombo->Update(1, 12, numDigits);

    GLdouble minMax[2];
    glGetDoublev(GL_ALIASED_LINE_WIDTH_RANGE, minMax);
    double ticWidth = aa->GetTicWidth();
    _ticWidthCombo->Update(minMax[0], minMax[1], ticWidth);
}

void AnnotationsEventRouter::axisAnnotationTableChanged()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            annotateLatLon = aa->GetLatLonAxesEnabled();

    std::vector<double> numTics = getTableRow(0);
    for (int i = 0; i < numTics.size(); i++) { numTics[i] = round(numTics[i]); }
    aa->SetNumTics(numTics);

    std::vector<double> ticSizes = getTableRow(1);
    aa->SetTicSize(ticSizes);

    std::vector<double> minTics = getTableRow(2);
    if (annotateLatLon) {
        convertLonToPCS(minTics[0]);
        convertLatToPCS(minTics[1]);
    }
    scaleWorldCoordsToNormalized(minTics);
    aa->SetMinTics(minTics);

    std::vector<double> maxTics = getTableRow(3);
    if (annotateLatLon) {
        convertLonToPCS(maxTics[0]);
        convertLatToPCS(maxTics[1]);
    }
    scaleWorldCoordsToNormalized(maxTics);
    aa->SetMaxTics(maxTics);

    std::vector<double> origins = getTableRow(4);
    if (annotateLatLon) {
        convertLonToPCS(origins[0]);
        convertLatToPCS(origins[1]);
    }
    scaleWorldCoordsToNormalized(origins);
    aa->SetAxisOrigin(origins);
}

void AnnotationsEventRouter::setCurrentAxisDataMgr(int index)
{
    QString qDataMgr = dataMgrSelectorCombo->itemText(index);
    string  dataMgr = qDataMgr.toStdString();

    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vfParams->SetCurrentAxisDataMgrName(dataMgr);
}

vector<double> AnnotationsEventRouter::getTableRow(int row)
{
    vector<double> contents;
    for (int col = 0; col < 3; col++) {
        double val = _annotationVaporTable->GetValue(row, col);
        contents.push_back(val);
    }
    return contents;
}

void AnnotationsEventRouter::setColorHelper(QWidget *w, vector<double> &rgb)
{
    rgb.clear();

    QPalette pal(w->palette());
    QColor   newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);

    rgb.push_back(newColor.red() / 255.0);
    rgb.push_back(newColor.green() / 255.0);
    rgb.push_back(newColor.blue() / 255.0);
}

void AnnotationsEventRouter::updateColorHelper(const vector<double> &rgb, QWidget *w)
{
    QColor color((int)(rgb[0] * 255.0), (int)(rgb[1] * 255.0), (int)(rgb[2] * 255.0));

    QPalette pal;
    pal.setColor(QPalette::Base, color);
    w->setPalette(pal);
}

void AnnotationsEventRouter::setRegionColor()
{
    vector<double> rgb;

    setColorHelper(regionColorEdit, rgb);
    if (rgb.size() != 3) return;

    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vfParams->SetRegionColor(rgb);
}

void AnnotationsEventRouter::updateRegionColor()
{
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vector<double>     rgb;
    vfParams->GetRegionColor(rgb);

    updateColorHelper(rgb, regionColorEdit);
}

void AnnotationsEventRouter::setDomainColor()
{
    vector<double> rgb;

    setColorHelper(domainColorEdit, rgb);
    if (rgb.size() != 3) return;

    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vfParams->SetDomainColor(rgb);
}

void AnnotationsEventRouter::updateDomainColor()
{
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vector<double>     rgb;
    vfParams->GetDomainColor(rgb);

    updateColorHelper(rgb, domainColorEdit);
}

void AnnotationsEventRouter::setBackgroundColor()
{
    vector<double> rgb;

    setColorHelper(backgroundColorEdit, rgb);
    if (rgb.size() != 3) return;

    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vfParams->SetBackgroundColor(rgb);
}

void AnnotationsEventRouter::updateBackgroundColor()
{
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vector<double>     rgb;
    vfParams->GetBackgroundColor(rgb);

    updateColorHelper(rgb, backgroundColorEdit);
}

void AnnotationsEventRouter::setAxisColor()
{
    vector<double> rgb;

    setColorHelper(axisColorEdit, rgb);
    if (rgb.size() == 3) rgb.push_back(1.f);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisColor(rgb);
}

void AnnotationsEventRouter::setAxisBackgroundColor()
{
    vector<double> rgb;

    setColorHelper(axisBackgroundColorEdit, rgb);
    if (rgb.size() == 3) rgb.push_back(1.f);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisBackgroundColor(rgb);
}

void AnnotationsEventRouter::updateAxisColor()
{
    vector<double>  rgb;
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    rgb = aa->GetAxisColor();

    updateColorHelper(rgb, axisColorEdit);
}

void AnnotationsEventRouter::updateAxisBackgroundColor()
{
    vector<double>  rgb;
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    rgb = aa->GetAxisBackgroundColor();

    updateColorHelper(rgb, axisBackgroundColorEdit);
}

void AnnotationsEventRouter::setTimeColor()
{
    vector<double> rgb;

    setColorHelper(timeColorEdit, rgb);
    if (rgb.size() != 3) return;

    MiscParams *miscParams = GetMiscParams();
    miscParams->SetTimeAnnotColor(rgb);
}

void AnnotationsEventRouter::updateTimeColor()
{
    MiscParams *   miscParams = GetMiscParams();
    vector<double> rgb;
    miscParams->GetTimeAnnotColor(rgb);

    updateColorHelper(rgb, timeColorEdit);
}

void AnnotationsEventRouter::timeAnnotationChanged()
{
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

void AnnotationsEventRouter::timeLLXChanged()
{
    MiscParams *miscParams = GetMiscParams();
    float       llx = timeLLXEdit->text().toFloat();

    miscParams->SetTimeAnnotLLX(llx);
    drawTimeStamp();
}

void AnnotationsEventRouter::timeLLYChanged()
{
    MiscParams *miscParams = GetMiscParams();
    float       lly = timeLLYEdit->text().toFloat();

    miscParams->SetTimeAnnotLLY(lly);
    drawTimeStamp();
}

void AnnotationsEventRouter::timeSizeChanged()
{
    MiscParams *miscParams = GetMiscParams();
    float       size = timeSizeEdit->text().toFloat();
    miscParams->SetTimeAnnotSize(size);
    drawTimeStamp();
}

void AnnotationsEventRouter::drawTimeStep(string myString)
{
    _controlExec->ClearText();

    if (myString == "") { myString = "Timestep: " + std::to_string(GetCurrentTimeStep()); }

    MiscParams *mp = GetMiscParams();
    int         x = mp->GetTimeAnnotLLX();
    int         y = mp->GetTimeAnnotLLY();
    int         size = mp->GetTimeAnnotSize();
    float       color[3];
    mp->GetTimeAnnotColor(color);

    _controlExec->DrawText(myString, x, y, size, color, 1);
}

void AnnotationsEventRouter::drawTimeUser()
{
    MiscParams *mp = GetMiscParams();
    if (mp->GetTimeStep() == true) {
        drawTimeStep();
        return;
    }

    size_t         ts = GetCurrentTimeStep();
    DataStatus *   ds = _controlExec->GetDataStatus();
    vector<double> timeCoords = ds->GetTimeCoordinates();

    double             myTime = timeCoords[ts];
    std::ostringstream ss;
    ss << myTime;
    std::string myString = ss.str();
    drawTimeStep(myString);
}

void AnnotationsEventRouter::drawTimeStamp()
{
    MiscParams *mp = GetMiscParams();
    if (mp->GetTimeStep() == true) {
        drawTimeStep();
        return;
    }

    size_t      ts = GetCurrentTimeStep();
    DataStatus *ds = _controlExec->GetDataStatus();
    drawTimeStep(ds->GetTimeCoordsFormatted()[ts]);
}

void AnnotationsEventRouter::setAxisDigits(int digits)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisDigits(digits);
}

void AnnotationsEventRouter::setAxisTicWidth(double width)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetTicWidth(width);
}

void AnnotationsEventRouter::setXTicOrientation(int)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();
    ticDir[0] = xTicOrientationCombo->currentIndex() + 1;    // Y(1) or Z(2)
    aa->SetTicDirs(ticDir);
}
void AnnotationsEventRouter::setYTicOrientation(int)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();
    ticDir[1] = yTicOrientationCombo->currentIndex() * 2;    // X(0) or Z(2)
    aa->SetTicDirs(ticDir);
}
void AnnotationsEventRouter::setZTicOrientation(int)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();
    ticDir[2] = zTicOrientationCombo->currentIndex();    // X(0) or Y(1)
    aa->SetTicDirs(ticDir);
}

void AnnotationsEventRouter::setLatLonAnnot(bool val)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetLatLonAxesEnabled(val);
}

void AnnotationsEventRouter::setUseDomainFrame()
{
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vfParams->SetUseDomainFrame(domainFrameCheckbox->isChecked());
}

void AnnotationsEventRouter::setAxisAnnotation(bool toggled)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisAnnotationEnabled(toggled);
}

void AnnotationsEventRouter::setLatLonAnnotation(bool val)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetLatLonAxesEnabled(val);
}

void AnnotationsEventRouter::setAxisTextSize(int size)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisFontSize(size);
}

void AnnotationsEventRouter::setUseAxisArrows()
{
    AnnotationsParams *vfParams = (AnnotationsParams *)GetActiveParams();
    vfParams->SetShowAxisArrows(axisArrowCheckbox->isChecked());
}
