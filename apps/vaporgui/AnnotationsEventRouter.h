//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnnotationsEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June, 2015
//
//	Description:	Defines the AnnotationsEventRouter class.
//		This class handles events for the VizFeature params
//
#ifndef VIZFEATUREEVENTROUTER_H
#define VIZFEATUREEVENTROUTER_H


#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_AnnotationsGUI.h"
#include "RangeCombos.h"
#include "VaporTable.h"
#include <vapor/AxisAnnotation.h>

QT_USE_NAMESPACE

namespace VAPoR {
	class ControlExec;
}



class AnnotationsEventRouter : public QWidget, public Ui_AnnotationsGUI, public EventRouter {

	Q_OBJECT

public: 

    AnnotationsEventRouter(
        QWidget *parent, VAPoR::ControlExec *ce
    );

	virtual ~AnnotationsEventRouter();

	//! For the AnnotationsEventRouter, we must override confirmText method on base class,
	//! so that text changes issue Command::CaptureStart and Command::CaptureEnd,
	//! supplying a special UndoRedo helper method
	//!
	virtual void confirmText();

	//! Connect signals and slots from tab
	virtual void hookUpTab();

    virtual void GetWebHelp(
        std::vector <std::pair <string, string>> &help
    ) const;
	
	//! Ignore wheel event in tab (to avoid confusion)
	virtual void wheelEvent(QWheelEvent*) {}

 // Get static string identifier for this router class
 //
 static string GetClassType() {
	return("Annotations");
 }
 string GetType() const {return GetClassType(); }


	
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
	
	void setVizFeatureTextChanged(const QString& qs);
	void vizfeatureReturnPressed();
	void setDomainColor();
	void setRegionColor();
	void setBackgroundColor();
	void setUseDomainFrame();
	void setTimeColor();
	void setLatLonAnnot(bool);
	void setUseAxisArrows();
	void timeAnnotationChanged();
	void timeLLXChanged();
	void timeLLYChanged();
	void timeSizeChanged();
	void setCurrentAxisDataMgr(int);
	void copyRegionFromRenderer();

private:

	Combo* _textSizeCombo;
	Combo* _digitsCombo;
	Combo* _ticWidthCombo;
	VaporTable* _annotationVaporTable;

	std::map<std::string, std::string> _visNames;
	std::map<std::string, std::string> _renTypeNames;

	vector<double> getTableRow(int row);

	void connectAnnotationWidgets();

	AnnotationsEventRouter() {}

	void setColorHelper(
		QWidget *w, vector <double> &rgb
	);

	void updateColorHelper(
		const vector <double> &rgb, QWidget *w
	);

	void updateRegionColor();
	void updateDomainColor();
	void updateBackgroundColor();
	void updateAxisColor();
	void updateAxisBackgroundColor();
	void updateTimeColor();
	void updateAxisAnnotations();
	void updateDataMgrCombo();
	void updateCopyRegionCombo();
	void updateAnnotationCheckbox();
	void updateLatLonCheckbox();
	void updateAnnotationTable();
	void updateTicOrientationCombos();

	VAPoR::AxisAnnotation* _getCurrentAxisAnnotation();

	std::vector<double> getDomainExtents() const;
	void scaleNormalizedCoordsToWorld(std::vector<double> &coords);
	void scaleWorldCoordsToNormalized(std::vector<double> &coords);
	void convertPCSToLon(double &xCoord);
	void convertPCSToLat(double &yCoord);
	void convertPCSToLonLat(double &xCoord, double &yCoord);
	void convertLonLatToPCS(double &xCoord, double &yCoord);
	void convertLonToPCS(double &xCoord);
	void convertLatToPCS(double &yCoord);

	void initializeAnnotation(VAPoR::AxisAnnotation* aa);
	void initializeAnnotationExtents(VAPoR::AxisAnnotation* aa);
	void initializeTicSizes(VAPoR::AxisAnnotation* aa);

	virtual void _confirmText();
	virtual void _updateTab();

	void drawTimeStamp();
	void drawTimeUser();
	void drawTimeStep(string text="");

	AnimationParams* _ap;
	bool _animConnected;
};

#endif //VIZFEATUREEVENTROUTER_H 
