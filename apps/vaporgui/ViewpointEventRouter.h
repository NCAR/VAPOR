//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		ViewpointEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Defines the ViewpointEventRouter class.
//		This class handles events for the viewpoint params
//
#ifndef VIEWPOINTEVENTROUTER_H
#define VIEWPOINTEVENTROUTER_H

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_vizTab.h"

namespace VAPoR {
class ControlExec;
class RegionParams;
}    // namespace VAPoR

class VizWinMgr;

class ViewpointEventRouter : public QWidget, public Ui_VizTab, public EventRouter {
    Q_OBJECT

public:
    ViewpointEventRouter(QWidget *parent, VizWinMgr *vizMgr, VAPoR::ControlExec *ce);
    virtual ~ViewpointEventRouter();
    // Connect signals and slots from tab
    virtual void hookUpTab();

    virtual void GetWebHelp(std::vector<std::pair<string, string>> &help) const;

    // Following methods are for undo/redo support:
    // Methods to capture state at start and end of mouse moves:
    //

    // Methods to handle home viewpoint
    void SetHomeViewpoint();
    void UseHomeViewpoint();

    void ViewAll();
    // Following are only accessible from main menu
    void CenterSubRegion();
    void AlignView(int axis);

    // Set from probe:
    void SetCenter(const double *centerCoords);

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("Viewpoint"); }
    string        GetType() const { return GetClassType(); }

    virtual void updateTab();

protected:
    virtual void _confirmText(){};
    virtual void _updateTab();

private:
    ViewpointEventRouter() {}

    virtual void wheelEvent(QWheelEvent *) {}

    VizWinMgr *_vizMgr;

    void updateTransforms();
    void updateCameraChanged();
    void updateLightChanged();

    VAPoR::ParamsBase *GetActiveParams() const;

private slots:
    void setCameraChanged();
    void setCameraLatLonChanged();
    void setLightChanged();
    void notImplemented();
};

#endif    // VIEWPOINTEVENTROUTER_H
