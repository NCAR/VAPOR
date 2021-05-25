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
#include "VSection.h"
#include "PWidgets.h"
#include "PEnumDropdownHLI.h"
#include "PSliderEditHLI.h"
#include "VComboBox.h"
#include "VPushButton.h"
#include "Updateable.h"
#include "PCopyRegionAnnotationWidget.h"

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

AnnotationEventRouter::AnnotationEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), EventRouter(ce, AnnotationParams::GetClassType())
{
    setLayout(new QVBoxLayout);
    _animConnected = false;
    _ap = NULL;

    VSection *axisAnnotationTab = new VSection("Axis Annotations");
    PGroup *  axisAnnotationGroup1 = new PGroup({
        new PCheckbox(AxisAnnotation::_annotationEnabledTag, "Axis Annotations Enabled"),
        new PCheckbox(AxisAnnotation::_latLonAxesTag, "Annotate with lat/lon"),
    });
    axisAnnotationTab->layout()->addWidget(axisAnnotationGroup1);
    _axisGroups.push_back(axisAnnotationGroup1);

    QTableWidget *annotationTable = new QTableWidget;
    _annotationVaporTable = new VaporTable(annotationTable);
    connect(_annotationVaporTable, SIGNAL(valueChanged(int, int)), this, SLOT(axisAnnotationTableChanged()));
    _annotationVaporTable->Reinit((VaporTable::DOUBLE), (VaporTable::MUTABLE), (VaporTable::HighlightFlags)(0));
    axisAnnotationTab->layout()->addWidget(annotationTable);

    PGroup *axisAnnotationGroup2 = new PGroup({
        new PCopyRegionAnnotationWidget(ce),
        new PColorSelector(AxisAnnotation::_colorTag, "Axis Text Color"),
        new PColorSelector(AxisAnnotation::_backgroundColorTag, "Text Background Color"),
        (new PIntegerSliderEditHLI<AxisAnnotation>("Font Size", &AxisAnnotation::GetAxisFontSize, &AxisAnnotation::SetAxisFontSize))->SetRange(2, 48)->EnableDynamicUpdate(),
        (new PIntegerSliderEdit(AxisAnnotation::_digitsTag, "Digits"))->SetRange(0, 8)->EnableDynamicUpdate(),
        //(new PIntegerSliderEdit(AxisAnnotation::_ticWidthTag, "Tic Width"))->SetRange(0, 10)->EnableDynamicUpdate(), // Broken, see 2711
        new PEnumDropdownHLI<AxisAnnotation>("X Tickmark Orientation", {"Y axis", "Z axis"}, {1, 2}, &AxisAnnotation::GetXTicDir, &AxisAnnotation::SetXTicDir),
        new PEnumDropdownHLI<AxisAnnotation>("Y Tickmark Orientation", {"X axis", "Z axis"}, {0, 2}, &AxisAnnotation::GetYTicDir, &AxisAnnotation::SetYTicDir),
        new PEnumDropdownHLI<AxisAnnotation>("Z Tickmark Orientation", {"X axis", "Y axis"}, {0, 1}, &AxisAnnotation::GetZTicDir, &AxisAnnotation::SetZTicDir),
    });
    axisAnnotationTab->layout()->addWidget(axisAnnotationGroup2);
    _axisGroups.push_back(axisAnnotationGroup2);

    layout()->addWidget(axisAnnotationTab);

    PGroup *timeAnnotationGroup = new PGroup({new PSection(
        "Time Annotation", {new PEnumDropdown(AnnotationParams::_timeTypeTag, {"No annotation", "Time step number", "User time", "Formatted date/time"}, {0, 1, 2, 3}, "Annotation type"),
                            new PIntegerInput(AnnotationParams::_timeSizeTag, "Font Size"), (new PDoubleSliderEdit(AnnotationParams::_timeLLXTag, "X Position"))->EnableDynamicUpdate(),
                            (new PDoubleSliderEdit(AnnotationParams::_timeLLYTag, "Y Position"))->EnableDynamicUpdate(), new PColorSelector(AnnotationParams::_timeColorTag, "Text Color")})});
    layout()->addWidget(timeAnnotationGroup);
    _groups.push_back(timeAnnotationGroup);

    PGroup *axisArrowGroup = new PGroup({new PSection("Orientation Arrows", {(new PCheckbox(AnnotationParams::AxisArrowEnabledTag, "Show arrows (XYZ->RGB)")),
                                                                             (new PDoubleSliderEdit(AnnotationParams::AxisArrowSizeTag, "Size"))->SetRange(0.f, 1.f)->EnableDynamicUpdate(),
                                                                             (new PDoubleSliderEdit(AnnotationParams::AxisArrowXPosTag, "X Position"))->SetRange(0.f, 1.f)->EnableDynamicUpdate(),
                                                                             (new PDoubleSliderEdit(AnnotationParams::AxisArrowYPosTag, "Y Position"))->SetRange(0.f, 1.f)->EnableDynamicUpdate()})});
    layout()->addWidget(axisArrowGroup);
    _groups.push_back(axisArrowGroup);

    PGroup *ThreeDGeometryGroup = new PGroup(
        {new PSection("3D Geometry", {
                                         new PCheckbox(AnnotationParams::_domainFrameTag, "Display Domain Bounds"), new PColorSelector(AnnotationParams::_domainColorTag, "Domain Frame Color"),
                                         new PColorSelector(AnnotationParams::_backgroundColorTag, "Background Color"),
                                         // new PColorSelector(AnnotationParams::_regionColorTag, "Region Frame Color")  Broken.  See #1742
                                     })});
    layout()->addWidget(ThreeDGeometryGroup);
    _groups.push_back(ThreeDGeometryGroup);
}

AnnotationEventRouter::~AnnotationEventRouter() {}

void AnnotationEventRouter::_updateTab()
{
    updateAxisTable();

    AxisAnnotation *  a = _getCurrentAxisAnnotation();
    AnnotationParams *vParams = (AnnotationParams *)GetActiveParams();

    for (PGroup *group : _groups) group->Update(vParams);
    for (PGroup *group : _axisGroups) group->Update(a, _controlExec->GetParamsMgr());

    return;
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
