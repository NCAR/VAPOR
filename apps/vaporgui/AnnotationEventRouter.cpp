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
#include "PAxisAnnotationWidget.h"

using namespace VAPoR;

AnnotationEventRouter::AnnotationEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), EventRouter(ce, AnnotationParams::GetClassType())
{
    // clang-format off
    setLayout(new QVBoxLayout);

    VSection *axisAnnotationTab = new VSection("Axis Annotations");
    PGroup *  axisAnnotationGroup1 = new PGroup(
        {new PCheckbox(AxisAnnotation::_annotationEnabledTag, "Axis Annotations Enabled"), 
         new PCheckbox(AxisAnnotation::_latLonAxesTag, "Annotate with lat/lon"), 
         new PAxisAnnotationWidget(ce)
    });
    axisAnnotationTab->layout()->addWidget(axisAnnotationGroup1);
    _axisGroups.push_back(axisAnnotationGroup1);

    PGroup *axisAnnotationGroup2 = new PGroup({
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

    PGroup* copyRegionGroup = new PGroup({
        new PCopyRegionAnnotationWidget(ce)
    });
    layout()->addWidget(copyRegionGroup);
    _groups.push_back(copyRegionGroup);

    PGroup *timeAnnotationGroup = new PGroup({new PSection(
        "Time Annotation", {new PEnumDropdown(AnnotationParams::_timeTypeTag, {"No annotation", "Time step number", "User time", "Formatted date/time"}, {0, 1, 2, 3}, "Annotation type"),
                            (new PIntegerSliderEdit(VAPoR::AnnotationParams::_timeSizeTag, "Font Size"))->SetRange(24, 100)->EnableDynamicUpdate(),
                            (new PDoubleSliderEdit(AnnotationParams::_timeLLXTag, "X Position"))->EnableDynamicUpdate(),
                            (new PDoubleSliderEdit(AnnotationParams::_timeLLYTag, "Y Position"))->EnableDynamicUpdate(), 
                            new PColorSelector(AnnotationParams::_timeColorTag, "Text Color")})
    });
    layout()->addWidget(timeAnnotationGroup);
    _groups.push_back(timeAnnotationGroup);

    PGroup *axisArrowGroup = new PGroup({new PSection("Orientation Arrows", {(new PCheckbox(AnnotationParams::AxisArrowEnabledTag, "Show arrows (XYZ->RGB)")),
                                                                             (new PDoubleSliderEdit(AnnotationParams::AxisArrowSizeTag, "Size"))->SetRange(0.f, 1.f)->EnableDynamicUpdate(),
                                                                             (new PDoubleSliderEdit(AnnotationParams::AxisArrowXPosTag, "X Position"))->SetRange(0.f, 1.f)->EnableDynamicUpdate(),
                                                                             (new PDoubleSliderEdit(AnnotationParams::AxisArrowYPosTag, "Y Position"))->SetRange(0.f, 1.f)->EnableDynamicUpdate()})});
    layout()->addWidget(axisArrowGroup);
    _groups.push_back(axisArrowGroup);

    PGroup *ThreeDGeometryGroup = new PGroup(
        {new PSection("3D Geometry", {new PCheckbox(AnnotationParams::_domainFrameTag, "Display Domain Bounds"), 
                                      new PColorSelector(AnnotationParams::_domainColorTag, "Domain Frame Color"),
                                      new PColorSelector(AnnotationParams::_backgroundColorTag, "Background Color"),
                                         // new PColorSelector(AnnotationParams::_regionColorTag, "Region Frame Color")  Broken.  See #1742
                                     })});
    layout()->addWidget(ThreeDGeometryGroup);
    _groups.push_back(ThreeDGeometryGroup);
    // clang-format on
}

AnnotationEventRouter::~AnnotationEventRouter() {}

void AnnotationEventRouter::_updateTab()
{
    AnnotationParams *aParams = (AnnotationParams *)GetActiveParams();
    AxisAnnotation *  aa = aParams->GetAxisAnnotation();
    AnnotationParams *vParams = (AnnotationParams *)GetActiveParams();

    for (PGroup *group : _groups) group->Update(vParams);
    for (PGroup *group : _axisGroups) group->Update(aa, _controlExec->GetParamsMgr());

    return;
}
