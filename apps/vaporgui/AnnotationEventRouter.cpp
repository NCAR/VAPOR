//************************************************************************
//															*
//			 Copyright (C)  2015										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		AnnotationEventRouter.cpp
//
//	Author:	Scott Pearse
//			Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the AnnotationEventRouter class.
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
#include <vapor/AnnotationParams.h>
#include <vapor/DataStatus.h>
#include <vapor/DataMgrUtils.h>

#include "qcolordialog.h"
#include <qlabel.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ErrorReporter.h"
#include "AnnotationEventRouter.h"
#include "vapor/ControlExecutive.h"
#include "EventRouter.h"
#include "PWidgets.h"

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

AnnotationEventRouter::AnnotationEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), Ui_AnnotationGUI(), EventRouter(ce, AnnotationParams::GetClassType())
{
    setupUi(this);

    _textSizeCombo = new Combo(axisTextSizeEdit, axisTextSizeSlider, true);
    _digitsCombo = new Combo(axisDigitsEdit, axisDigitsSlider, true);
    _ticWidthCombo = new Combo(ticWidthEdit, ticWidthSlider);
    _annotationVaporTable = new VaporTable(axisAnnotationTable);
    _annotationVaporTable->Reinit((VaporTable::DOUBLE), (VaporTable::MUTABLE), (VaporTable::HighlightFlags)(0));

    connectAnnotationWidgets();

    _animConnected = false;
    _ap = NULL;

    // clang-format off
    _axisArrowGroup = new PGroup({
        new PSection("Orientation Arrows", {
            (new PCheckbox(AnnotationParams::AxisArrowEnabledTag, "Show arrows (XYZ->RGB)")),
            (new PDoubleSliderEdit(AnnotationParams::AxisArrowSizeTag, "Size"))->SetRange(0.f, 1.f)->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(AnnotationParams::AxisArrowXPosTag, "X Position"))->SetRange(0.f, 1.f)->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(AnnotationParams::AxisArrowYPosTag, "Y Position"))->SetRange(0.f, 1.f)->EnableDynamicUpdate()
        })
    });
    layout()->addWidget(_axisArrowGroup);
    
    
    _timeSlidersGroup = new PGroup({
        new PLabel("Lower-left coordinates:"),
        new PDoubleSliderEdit(AnnotationParams::_timeLLXTag, "X"),
        new PDoubleSliderEdit(AnnotationParams::_timeLLYTag, "Y"),
    });
    auto l = (QVBoxLayout*)tab_4->layout();
    l->insertWidget(l->indexOf(verticalLayout_9), _timeSlidersGroup);
    verticalLayout_9->hide();
    // clang-format on
}

AnnotationEventRouter::~AnnotationEventRouter() {}

void AnnotationEventRouter::connectAnnotationWidgets()
{
    connect(_axisAnnotationEnabledCheckbox, SIGNAL(toggled(bool)), this, SLOT(setAxisAnnotation(bool)));
    connect(_latLonAnnotationCheckbox, SIGNAL(toggled(bool)), this, SLOT(setLatLonAnnotation(bool)));
    connect(_textSizeCombo, SIGNAL(valueChanged(int)), this, SLOT(setAxisTextSize(int)));
    connect(_digitsCombo, SIGNAL(valueChanged(int)), this, SLOT(setAxisDigits(int)));
    connect(_ticWidthCombo, SIGNAL(valueChanged(double)), this, SLOT(setAxisTicWidth(double)));
    connect(axisColorButton, SIGNAL(pressed()), this, SLOT(setAxisColor()));
    connect(axisBackgroundColorButton, SIGNAL(pressed()), this, SLOT(setAxisBackgroundColor()));
    connect(_annotationVaporTable, SIGNAL(returnPressed()), this, SLOT(axisAnnotationTableChanged()));
    connect(xTicOrientationCombo, SIGNAL(activated(int)), this, SLOT(setXTicOrientation(int)));
    connect(yTicOrientationCombo, SIGNAL(activated(int)), this, SLOT(setYTicOrientation(int)));
    connect(zTicOrientationCombo, SIGNAL(activated(int)), this, SLOT(setZTicOrientation(int)));
    connect(copyRegionButton, SIGNAL(pressed()), this, SLOT(copyRegionFromRenderer()));
    connect(_timeCombo, SIGNAL(activated(int)), this, SLOT(timeAnnotationChanged()));
    connect(_timeSizeEdit, SIGNAL(returnPressed()), this, SLOT(timeSizeChanged()));
    connect(_timeColorButton, SIGNAL(clicked()), this, SLOT(setTimeColor()));

    connect(backgroundColorButton, SIGNAL(clicked()), this, SLOT(setBackgroundColor()));
    connect(domainColorButton, SIGNAL(clicked()), this, SLOT(setDomainColor()));
    connect(domainFrameCheckbox, SIGNAL(clicked()), this, SLOT(setDomainFrameEnabled()));
    connect(regionColorButton, SIGNAL(clicked()), this, SLOT(setRegionColor()));
}

void AnnotationEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Overview of the VizFeature tab", "http://www.vapor.ucar.edu/docs/vapor-gui-help/vizfeature-tab#VizFeatureOverview"));
}

void AnnotationEventRouter::_updateTab()
{
    updateRegionColor();
    updateDomainColor();
    updateBackgroundColor();
    updateTimePanel();
    updateAxisAnnotations();

    AnnotationParams *vParams = (AnnotationParams *)GetActiveParams();

    domainFrameCheckbox->setChecked(vParams->GetUseDomainFrame());

    _axisArrowGroup->Update(vParams);
    _timeSlidersGroup->Update(vParams);

    return;
}

void AnnotationEventRouter::copyRegionFromRenderer()
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
    VAssert(copyParams);

    Box *               copyBox = copyParams->GetBox();
    std::vector<double> minExtents, maxExtents;
    copyBox->GetExtents(minExtents, maxExtents);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    if (minExtents.size() < 3) {    // copyBox->IsPlanar()
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

void AnnotationEventRouter::addRendererToCombo(string visName, string typeName, string visAbb, string dataSetName)
{
    // Abbreviate Params names by removing 'Params" from them.
    // Then store them in a map for later reference.
    //
    string typeAbb = typeName;
    int    pos = typeAbb.find("Params");
    typeAbb.erase(pos, 6);
    _renTypeNames[typeAbb] = typeName;

    std::vector<string> renNames;
    ParamsMgr *         paramsMgr = _controlExec->GetParamsMgr();
    renNames = paramsMgr->GetRenderParamInstances(visName, dataSetName, typeName);

    for (int k = 0; k < renNames.size(); k++) {
        string  displayName = visAbb + ":" + dataSetName + ":" + typeAbb + ":" + renNames[k];
        QString qDisplayName = QString::fromStdString(displayName);
        copyRegionCombo->addItem(qDisplayName);
    }
}

void AnnotationEventRouter::updateCopyRegionCombo()
{
    copyRegionCombo->clear();

    AnnotationParams *vParams = (AnnotationParams *)GetActiveParams();
    std::string       dataSetName = vParams->GetCurrentAxisDataMgrName();

    _visNames.clear();

    ParamsMgr *              paramsMgr = _controlExec->GetParamsMgr();
    std::vector<std::string> visNames = paramsMgr->GetVisualizerNames();
    DataStatus *             dataStatus = _controlExec->GetDataStatus();

    for (int i = 0; i < visNames.size(); i++) {
        string visName = visNames[i];

        // Create a mapping of abreviated visualizer names to their
        // actual string values.
        //
        string visAbb = "Vis" + std::to_string(i);
        _visNames[visAbb] = visName;

        std::vector<string> typeNames;
        typeNames = paramsMgr->GetRenderParamsClassNames(visName);

        for (int j = 0; j < typeNames.size(); j++) {
            string typeName = typeNames[j];

            std::vector<string> dmNames = dataStatus->GetDataMgrNames();
            for (int k = 0; k < dmNames.size(); k++) {
                string dataSetName = dmNames[k];
                addRendererToCombo(visName, typeName, visAbb, dataSetName);
            }
        }
    }
}

void AnnotationEventRouter::updateAxisEnabledCheckbox()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            annotationEnabled = aa->GetAxisAnnotationEnabled();
    if (annotationEnabled)
        _axisAnnotationEnabledCheckbox->setCheckState(Qt::Checked);
    else
        _axisAnnotationEnabledCheckbox->setCheckState(Qt::Unchecked);
}

void AnnotationEventRouter::updateLatLonCheckbox()
{
    string projString = getProjString();
    if (projString.size() == 0) {
        _latLonAnnotationCheckbox->setEnabled(false);
        return;
    } else
        _latLonAnnotationCheckbox->setEnabled(true);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            annotateLatLon = aa->GetLatLonAxesEnabled();
    if (annotateLatLon)
        _latLonAnnotationCheckbox->setCheckState(Qt::Checked);
    else
        _latLonAnnotationCheckbox->setCheckState(Qt::Unchecked);
}

void AnnotationEventRouter::updateTicOrientationCombos()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();

    xTicOrientationCombo->setCurrentIndex(ticDir[0] - 1);
    yTicOrientationCombo->setCurrentIndex(ticDir[1] / 2);
    zTicOrientationCombo->setCurrentIndex(ticDir[2]);
}

void AnnotationEventRouter::scaleNormalizedCoordsToWorld(std::vector<double> &coords)
{
    std::vector<double> extents = getDomainExtents();
    int                 size = extents.size() / 2;
    for (int i = 0; i < size; i++) {
        double offset = coords[i] * (extents[i + 3] - extents[i]);
        double minimum = extents[i];
        coords[i] = offset + minimum;
    }
}

void AnnotationEventRouter::scaleWorldCoordsToNormalized(std::vector<double> &coords)
{
    std::vector<double> extents = getDomainExtents();
    int                 size = extents.size() / 2;
    for (int i = 0; i < size; i++) {
        double point = coords[i] - extents[i];
        double magnitude = extents[i + 3] - extents[i];
        coords[i] = point / magnitude;
    }
}

void AnnotationEventRouter::updateAxisTable()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            latLonEnabled = aa->GetLatLonAxesEnabled();

    vector<double> tableValues;

    vector<double> numtics = aa->GetNumTics();
    tableValues.insert(tableValues.end(), numtics.begin(), numtics.end());

    vector<double> ticSizes = aa->GetTicSize();
    tableValues.insert(tableValues.end(), ticSizes.begin(), ticSizes.end());

    vector<double> minTics = aa->GetMinTics();
    vector<double> maxTics = aa->GetMaxTics();
    scaleNormalizedCoordsToWorld(minTics);
    scaleNormalizedCoordsToWorld(maxTics);
    if (latLonEnabled) {
        // converPCSToLonLat() modifies inputs by reference, so we need dummy
        // variables that can be discarded in order to do use these values
        // multiple times.
        //
        double minX = minTics[0];
        double minY = minTics[1];
        convertPCSToLonLat(minTics[0], minTics[1]);    // min X, min Y
        convertPCSToLonLat(minX, maxTics[1]);          // min X, max Y
        convertPCSToLonLat(maxTics[0], minY);          // max X, min Y
    }
    tableValues.insert(tableValues.end(), minTics.begin(), minTics.end());
    tableValues.insert(tableValues.end(), maxTics.begin(), maxTics.end());

    vector<double> origin = aa->GetAxisOrigin();
    scaleNormalizedCoordsToWorld(origin);
    if (latLonEnabled) { convertPCSToLonLat(origin[0], origin[1]); }
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

string AnnotationEventRouter::getProjString()
{
    DataStatus *dataStatus = _controlExec->GetDataStatus();
    string      projString = dataStatus->GetMapProjection();
    return projString;
}

void AnnotationEventRouter::convertPCSToLon(double &xCoord)
{
    double dummy = 0.;
    convertPCSToLonLat(xCoord, dummy);
}

void AnnotationEventRouter::convertPCSToLat(double &yCoord)
{
    double dummy = 0.;
    convertPCSToLonLat(dummy, yCoord);
}

void AnnotationEventRouter::convertPCSToLonLat(double &xCoord, double &yCoord)
{
    string projString = getProjString();
    double coords[2] = {xCoord, yCoord};
    double coordsForError[2] = {coords[0], coords[1]};

    int rc = DataMgrUtils::ConvertPCSToLonLat(projString, coords, 1);
    if (rc < 0) {
        MyBase::SetErrMsg("Could not convert point %f, %f to Lon/Lat", coordsForError[0], coordsForError[1]);
        MSG_ERR("Error converting PCS to Lat-Lon coordinates");
    }

    xCoord = coords[0];
    yCoord = coords[1];
}

void AnnotationEventRouter::convertLonToPCS(double &xCoord)
{
    double dummy = 0.;
    convertLonLatToPCS(xCoord, dummy);
}

void AnnotationEventRouter::convertLatToPCS(double &yCoord)
{
    double dummy = 0.;
    convertLonLatToPCS(dummy, yCoord);
}

void AnnotationEventRouter::convertLonLatToPCS(double &xCoord, double &yCoord)
{
    string projString = getProjString();
    double coords[2] = {xCoord, yCoord};
    double coordsForError[2] = {coords[0], coords[1]};

    int rc = DataMgrUtils::ConvertLonLatToPCS(projString, coords, 1);
    if (rc < 0) {
        MyBase::SetErrMsg("Could not convert point %f, %f to PCS", coordsForError[0], coordsForError[1]);
        MSG_ERR("Error converting from Lat-Lon to PCS coordinates");
    }

    xCoord = coords[0];
    yCoord = coords[1];
}

AxisAnnotation *AnnotationEventRouter::_getCurrentAxisAnnotation()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    AxisAnnotation *  aa = aParams->GetAxisAnnotation();

    bool initialized = aa->GetAxisAnnotationInitialized();
    if (!initialized) initializeAnnotation(aa);

    return aa;
}

std::vector<double> AnnotationEventRouter::getDomainExtents() const
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
void AnnotationEventRouter::initializeTicSizes(AxisAnnotation *aa)
{
    vector<double> ticSizes(3, .025);
    aa->SetTicSize(ticSizes);
}

void AnnotationEventRouter::initializeAnnotationExtents(AxisAnnotation *aa)
{
    vector<double> minExts(3, 0.0);
    vector<double> maxExts(3, 1.0);

    aa->SetMinTics(minExts);
    aa->SetMaxTics(maxExts);
    aa->SetAxisOrigin(minExts);
}

void AnnotationEventRouter::initializeAnnotation(AxisAnnotation *aa)
{
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->BeginSaveStateGroup("Initialize axis annotation table");

    initializeAnnotationExtents(aa);
    initializeTicSizes(aa);

    paramsMgr->EndSaveStateGroup();

    aa->SetAxisAnnotationInitialized(true);
}

void AnnotationEventRouter::updateAxisAnnotations()
{
    updateCopyRegionCombo();
    updateAxisEnabledCheckbox();
    updateLatLonCheckbox();
    updateTicOrientationCombos();
    updateAxisTable();
    updateAxisColor();
    updateAxisBackgroundColor();

    AxisAnnotation *aa = _getCurrentAxisAnnotation();

    int textSize = aa->GetAxisFontSize();
    _textSizeCombo->Update(4, 50, textSize);

    int numDigits = aa->GetAxisDigits();
    _digitsCombo->Update(1, 12, numDigits);

    // GLdouble minMax[2];
    // glGetDoublev(GL_ALIASED_LINE_WIDTH_RANGE, minMax);
    double ticWidth = aa->GetTicWidth();
    _ticWidthCombo->Update(0, 7, ticWidth);    // minMax[0], minMax[1], ticWidth);
}

void AnnotationEventRouter::axisAnnotationTableChanged()
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    bool            annotateLatLon = aa->GetLatLonAxesEnabled();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->BeginSaveStateGroup("Annotation table changed");
    std::vector<double> numTics = getTableRow(0);
    for (int i = 0; i < numTics.size(); i++) { numTics[i] = round(numTics[i]); }
    aa->SetNumTics(numTics);

    std::vector<double> ticSizes = getTableRow(1);
    aa->SetTicSize(ticSizes);

    std::vector<double> minTics = getTableRow(2);
    std::vector<double> maxTics = getTableRow(3);
    if (annotateLatLon) {
        // converLonLatToPCS() modifies inputs by reference, so we need dummy
        // variables that can be discarded in order to do use these values
        // multiple times.
        //
        double minLon = minTics[0];
        double minLat = minTics[1];
        convertLonLatToPCS(minTics[0], minTics[1]);    // min lon, min lat
        convertLonLatToPCS(maxTics[0], minLat);        // max lon, min lat
        convertLonLatToPCS(minLon, maxTics[1]);        // min lon, max lat
    }
    scaleWorldCoordsToNormalized(minTics);
    scaleWorldCoordsToNormalized(maxTics);
    aa->SetMinTics(minTics);
    aa->SetMaxTics(maxTics);

    std::vector<double> origins = getTableRow(4);
    if (annotateLatLon) { convertLonLatToPCS(origins[0], origins[1]); }
    scaleWorldCoordsToNormalized(origins);
    aa->SetAxisOrigin(origins);

    paramsMgr->EndSaveStateGroup();
}

vector<double> AnnotationEventRouter::getTableRow(int row)
{
    vector<double> contents;
    for (int col = 0; col < 3; col++) {
        double val = _annotationVaporTable->GetValue(row, col);
        contents.push_back(val);
    }
    return contents;
}

void AnnotationEventRouter::setColorHelper(QWidget *w, vector<double> &rgb)
{
    rgb.clear();

    QPalette pal(w->palette());
    QColor   newColor = QColorDialog::getColor(pal.color(QPalette::Base), this);

    rgb.push_back(newColor.red() / 255.0);
    rgb.push_back(newColor.green() / 255.0);
    rgb.push_back(newColor.blue() / 255.0);
}

void AnnotationEventRouter::updateColorHelper(const vector<double> &rgb, QWidget *w)
{
    QColor color((int)(rgb[0] * 255.0), (int)(rgb[1] * 255.0), (int)(rgb[2] * 255.0));

    QPalette pal;
    pal.setColor(QPalette::Base, color);
    w->setPalette(pal);
}

void AnnotationEventRouter::setRegionColor()
{
    vector<double> rgb;

    setColorHelper(regionColorEdit, rgb);
    if (rgb.size() != 3) return;

    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    aParams->SetRegionColor(rgb);
}

void AnnotationEventRouter::updateRegionColor()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    vector<double>    rgb;
    aParams->GetRegionColor(rgb);

    updateColorHelper(rgb, regionColorEdit);
}

void AnnotationEventRouter::setDomainColor()
{
    vector<double> rgb;

    setColorHelper(domainColorEdit, rgb);
    if (rgb.size() != 3) return;

    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    aParams->SetDomainColor(rgb);
}

void AnnotationEventRouter::updateDomainColor()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    vector<double>    rgb;
    aParams->GetDomainColor(rgb);

    updateColorHelper(rgb, domainColorEdit);
}

void AnnotationEventRouter::setBackgroundColor()
{
    vector<double> rgb;

    setColorHelper(backgroundColorEdit, rgb);
    if (rgb.size() != 3) return;

    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    aParams->SetBackgroundColor(rgb);
}

void AnnotationEventRouter::updateBackgroundColor()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    vector<double>    rgb;
    aParams->GetBackgroundColor(rgb);

    updateColorHelper(rgb, backgroundColorEdit);
}

void AnnotationEventRouter::setAxisColor()
{
    vector<double> rgb;

    setColorHelper(axisColorEdit, rgb);
    if (rgb.size() == 3) rgb.push_back(1.f);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisColor(rgb);
}

void AnnotationEventRouter::setAxisBackgroundColor()
{
    vector<double> rgb;

    setColorHelper(axisBackgroundColorEdit, rgb);
    if (rgb.size() == 3) rgb.push_back(1.f);

    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisBackgroundColor(rgb);
}

void AnnotationEventRouter::updateAxisColor()
{
    vector<double>  rgb;
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    rgb = aa->GetAxisColor();

    updateColorHelper(rgb, axisColorEdit);
}

void AnnotationEventRouter::updateAxisBackgroundColor()
{
    vector<double>  rgb;
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    rgb = aa->GetAxisBackgroundColor();

    updateColorHelper(rgb, axisBackgroundColorEdit);
}

void AnnotationEventRouter::setTimeColor()
{
    vector<double> rgb;

    setColorHelper(_timeColorEdit, rgb);
    if (rgb.size() != 3) return;

    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    aParams->SetTimeColor(rgb);
}

void AnnotationEventRouter::updateTimePanel()
{
    updateTimeColor();
    updateTimeType();
    updateTimeSize();
    timeAnnotationChanged();
}

void AnnotationEventRouter::updateTimeColor()
{
    AnnotationParams *  aParams = (AnnotationParams *)GetActiveParams();
    std::vector<double> rgb = aParams->GetTimeColor();

    updateColorHelper(rgb, _timeColorEdit);
}

void AnnotationEventRouter::updateTimeType()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    int               type = aParams->GetTimeType();
    _timeCombo->setCurrentIndex(type);
}

void AnnotationEventRouter::updateTimeSize()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    int               size = aParams->GetTimeSize();
    _timeSizeEdit->setText(QString::number(size));
}

void AnnotationEventRouter::timeAnnotationChanged()
{
    int               index = _timeCombo->currentIndex();
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    aParams->SetTimeType(index);

    switch (index) {
    case 0: _controlExec->ClearText(); break;
    case 1: drawTimeStep(); break;
    case 2: drawTimeUser(); break;
    case 3: drawTimeStamp(); break;
    }
}

void AnnotationEventRouter::timeSizeChanged()
{
    float             size = _timeSizeEdit->text().toFloat();
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    aParams->SetTimeSize(size);
}

void AnnotationEventRouter::drawTimeStep(string myString)
{
    _controlExec->ClearText();

    if (myString == "") { myString = "Timestep: " + std::to_string(GetCurrentTimeStep()); }

    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    float             x = aParams->GetTimeLLX();
    float             y = aParams->GetTimeLLY();
    int               size = aParams->GetTimeSize();
    float             color[] = {0., 0., 0.};
    aParams->GetTimeColor(color);

    _controlExec->DrawTextNormalizedCoords(myString, x, y, size, color, 1);
}

void AnnotationEventRouter::drawTimeUser()
{
    size_t         ts = GetCurrentTimeStep();
    DataStatus *   ds = _controlExec->GetDataStatus();
    vector<double> timeCoords = ds->GetTimeCoordinates();

    double             myTime = timeCoords[ts];
    std::ostringstream ss;
    ss << myTime;
    std::string myString = ss.str();
    drawTimeStep(myString);
}

void AnnotationEventRouter::drawTimeStamp()
{
    size_t      ts = GetCurrentTimeStep();
    DataStatus *ds = _controlExec->GetDataStatus();
    drawTimeStep(ds->GetTimeCoordsFormatted()[ts]);
}

void AnnotationEventRouter::setAxisDigits(int digits)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisDigits(digits);
}

void AnnotationEventRouter::setAxisTicWidth(double width)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetTicWidth(width);
}

void AnnotationEventRouter::setXTicOrientation(int)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();
    ticDir[0] = xTicOrientationCombo->currentIndex() + 1;    // Y(1) or Z(2)
    aa->SetTicDirs(ticDir);
}
void AnnotationEventRouter::setYTicOrientation(int)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();
    ticDir[1] = yTicOrientationCombo->currentIndex() * 2;    // X(0) or Z(2)
    aa->SetTicDirs(ticDir);
}
void AnnotationEventRouter::setZTicOrientation(int)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    vector<double>  ticDir = aa->GetTicDirs();
    ticDir[2] = zTicOrientationCombo->currentIndex();    // X(0) or Y(1)
    aa->SetTicDirs(ticDir);
}

void AnnotationEventRouter::setLatLonAnnot(bool val)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetLatLonAxesEnabled(val);
}

void AnnotationEventRouter::setDomainFrameEnabled()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    aParams->SetUseDomainFrame(domainFrameCheckbox->isChecked());
}

void AnnotationEventRouter::setAxisAnnotation(bool toggled)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisAnnotationEnabled(toggled);
}

void AnnotationEventRouter::setLatLonAnnotation(bool val)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetLatLonAxesEnabled(val);
}

void AnnotationEventRouter::setAxisTextSize(int size)
{
    AxisAnnotation *aa = _getCurrentAxisAnnotation();
    aa->SetAxisFontSize(size);
}
