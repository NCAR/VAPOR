//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnnotationEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June, 2015
//
//	Description:	Defines the AnnotationEventRouter class.
//		This class handles events for the Annotation params
//
#ifndef ANNOTATIONEVENTROUTER_H
#define ANNOTATIONEVENTROUTER_H

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_AnnotationGUI.h"
#include "RangeCombos.h"
#include "VaporTable.h"
#include <vapor/AxisAnnotation.h>

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class PGroup;

class AnnotationEventRouter : public QWidget, public Ui_AnnotationGUI, public EventRouter {
    Q_OBJECT

public:
    AnnotationEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    virtual ~AnnotationEventRouter();

    virtual void hookUpTab(){};

    virtual void GetWebHelp(std::vector<std::pair<string, string>> &help) const;

    //! Ignore wheel event in tab (to avoid confusion)
    virtual void wheelEvent(QWheelEvent *) {}

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("Annotation"); }
    string        GetType() const { return GetClassType(); }

    virtual void _confirmText(){};

protected slots:
    void setAxisAnnotation(bool);
    void setLatLonAnnotation(bool);
    void setAxisTextSize(int);
    void setAxisDigits(int);
    void setAxisTicWidth(double);
    void setAxisColor();
    void setAxisBackgroundColor();
    void axisAnnotationTableChanged();
    void setXTicOrientation(int);
    void setYTicOrientation(int);
    void setZTicOrientation(int);

    void setDomainColor();
    void setRegionColor();
    void setBackgroundColor();
    void setDomainFrameEnabled();
    void setTimeColor();
    void setLatLonAnnot(bool);
    void timeAnnotationChanged();
    void timeSizeChanged();
    void copyRegionFromRenderer();

private:
    Combo *     _textSizeCombo;
    Combo *     _digitsCombo;
    Combo *     _ticWidthCombo;
    VaporTable *_annotationVaporTable;

    std::map<std::string, std::string> _visNames;
    std::map<std::string, std::string> _renTypeNames;

    vector<double> getTableRow(int row);

    void connectAnnotationWidgets();

    AnnotationEventRouter() {}

    void setColorHelper(QWidget *w, vector<double> &rgb);

    void updateColorHelper(const vector<double> &rgb, QWidget *w);

    void updateRegionColor();
    void updateDomainColor();
    void updateBackgroundColor();

    void updateAxisAnnotations();
    void updateAxisColor();
    void updateAxisBackgroundColor();
    void updateAxisTable();
    void updateAxisEnabledCheckbox();
    void updateLatLonCheckbox();
    void updateCopyRegionCombo();
    void updateTicOrientationCombos();
    void addRendererToCombo(string, string, string, string);

    void updateTimePanel();
    void updateTimeColor();
    void updateTimeType();
    void updateTimeSize();

    void   updateDataMgrCombo();
    string getProjString();

    VAPoR::AxisAnnotation *_getCurrentAxisAnnotation();

    std::vector<double> getDomainExtents() const;
    void                scaleNormalizedCoordsToWorld(std::vector<double> &coords);
    void                scaleWorldCoordsToNormalized(std::vector<double> &coords);
    void                convertPCSToLon(double &xCoord);
    void                convertPCSToLat(double &yCoord);
    void                convertPCSToLonLat(double &xCoord, double &yCoord);
    void                convertLonLatToPCS(double &xCoord, double &yCoord);
    void                convertLonToPCS(double &xCoord);
    void                convertLatToPCS(double &yCoord);

    void initializeAnnotation(VAPoR::AxisAnnotation *aa);
    void initializeAnnotationExtents(VAPoR::AxisAnnotation *aa);
    void initializeTicSizes(VAPoR::AxisAnnotation *aa);

    virtual void _updateTab();

    void drawTimeStamp();
    void drawTimeUser();
    void drawTimeStep(string text = "");

    AnimationParams *_ap;
    bool             _animConnected;

    PGroup *_axisArrowGroup;
    PGroup *_timeSlidersGroup;
};

#endif    // ANNOTATIONEVENTROUTER_H
