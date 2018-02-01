//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		VizFeatureEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June, 2015
//
//	Description:	Defines the VizFeatureEventRouter class.
//		This class handles events for the VizFeature params
//
#ifndef VIZFEATUREEVENTROUTER_H
#define VIZFEATUREEVENTROUTER_H

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_vizFeaturesTab.h"
#include "RangeCombos.h"
#include "VaporTable.h"
#include <vapor/AxisAnnotation.h>

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class VizFeatureEventRouter : public QWidget, public Ui_vizFeaturesTab, public EventRouter {
    Q_OBJECT

public:
    VizFeatureEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    virtual ~VizFeatureEventRouter();

    //! For the VizFeatureEventRouter, we must override confirmText method on base class,
    //! so that text changes issue Command::CaptureStart and Command::CaptureEnd,
    //! supplying a special UndoRedo helper method
    //!
    virtual void confirmText();

    //! Connect signals and slots from tab
    virtual void hookUpTab();

    virtual void GetWebHelp(std::vector<std::pair<string, string>> &help) const;

    //! Ignore wheel event in tab (to avoid confusion)
    virtual void wheelEvent(QWheelEvent *) {}

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("VizFeature"); }
    string        GetType() const { return GetClassType(); }

protected slots:
    void setAxisDataMgr(int);
    void setAxisAnnotation(bool);
    void setLatLonAnnotation(bool);
    void setAxisTextSize(int);
    void setAxisDigits(int);
    void setAxisTicWidth(double);
    void setAxisColor();
    void axisAnnotationTableChanged();
    void setXTicOrientation(int);
    void setYTicOrientation(int);
    void setZTicOrientation(int);

    void setVizFeatureTextChanged(const QString &qs);
    void vizfeatureReturnPressed();
    void setDomainColor();
    void setRegionColor();
    void setBackgroundColor();
    void setUseRegionFrame();
    void setUseDomainFrame();
    void setTimeColor();
    void setLatLonAnnot(bool);
    void setUseAxisArrows();
    void timeAnnotationChanged();
    void timeLLXChanged();
    void timeLLYChanged();
    void timeSizeChanged();
    void setCurrentDataMgr(int);

private:
    Combo *     _textSizeCombo;
    Combo *     _digitsCombo;
    Combo *     _ticWidthCombo;
    VaporTable *_annotationVaporTable;

    vector<double> getTableRow(int row);

    void connectAnnotationWidgets();

    VizFeatureEventRouter() {}

    void setColorHelper(QWidget *w, vector<double> &rgb);

    void updateColorHelper(const vector<double> &rgb, QWidget *w);

    void updateRegionColor();
    void updateDomainColor();
    void updateBackgroundColor();
    void updateAxisColor();
    void updateTimeColor();
    void updateAxisAnnotations();
    void updateDataMgrCombo();
    void updateAnnotationCheckbox();
    void updateLatLonCheckbox();
    void updateAnnotationTable();
    void updateTicOrientationCombos();

    AxisAnnotation *_getCurrentAxisAnnotation();

    void getActiveExtents(vector<double> &minExts, vector<double> &maxExts);
    void initializeAnnotations();
    void initializeAnnotationExtents();
    void initializeTicSizes();

    virtual void _confirmText();
    virtual void _updateTab();

    void drawTimeStamp();
    void drawTimeUser();
    void drawTimeStep(string text = "");

    AnimationParams *_ap;
    bool             _animConnected;
    bool             _annotationsInitialized;
};

#endif    // VIZFEATUREEVENTROUTER_H
