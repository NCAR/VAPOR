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

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class VizFeatureEventRouter : public QWidget, public Ui_vizFeaturesTab, public EventRouter {

    Q_OBJECT

  public:
    VizFeatureEventRouter(
        QWidget *parent, VAPoR::ControlExec *ce);

    virtual ~VizFeatureEventRouter();

    //! For the VizFeatureEventRouter, we must override confirmText method on base class,
    //! so that text changes issue Command::CaptureStart and Command::CaptureEnd,
    //! supplying a special UndoRedo helper method
    //!
    virtual void confirmText();

    //! Connect signals and slots from tab
    virtual void hookUpTab();

    virtual void GetWebHelp(
        std::vector<std::pair<string, string>> &help) const;

    //! Ignore wheel event in tab (to avoid confusion)
    virtual void wheelEvent(QWheelEvent *) {}

    // Get static string identifier for this router class
    //
    static string GetClassType() {
        return ("VizFeature");
    }
    string GetType() const { return GetClassType(); }

  protected slots:

    void setVizFeatureTextChanged(const QString &qs);
    void vizfeatureReturnPressed();
    void setDomainColor();
    void setRegionColor();
    void setBackgroundColor();
    void setUseRegionFrame(bool);
    void setUseDomainFrame(bool);
    void annotationChanged();
    void selectAxisColor();
    void setXTicOrient(int);
    void setYTicOrient(int);
    void setZTicOrient(int);
    void setLatLonAnnot(bool);
    void setUseAxisArrows(bool);
    void timeAnnotationChanged();
    void timeLLXChanged();
    void timeLLYChanged();
    void timeSizeChanged();
    void timeColorChanged();

  private:
    VizFeatureEventRouter() {}

    void invalidateText();

    virtual void _confirmText();
    virtual void _updateTab();

    void drawTimeStamp();
    void drawTimeStep(string text = "");

    AnimationParams *_ap;
    bool _animConnected;
};

#endif //VIZFEATUREEVENTROUTER_H
