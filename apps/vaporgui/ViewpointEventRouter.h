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

#ifdef DEAD
    virtual void captureMouseDown(int button);
    // When the mouse goes up, save the face displacement into the region.
    virtual void captureMouseUp();
    // When the spin is ended, it replaces captureMouseUp:
    void endSpin();
#endif

    // Methods to handle home viewpoint
    void setHomeViewpoint();
    void useHomeViewpoint();
    // Following are only accessible from main menu
    void CenterFullDomain();
    void CenterSubRegion(VAPoR::RegionParams *rParams);
    void AlignView(int axis);

    // Set from probe:
    void SetCenter(const double *centerCoords);

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("Viewpoint"); }
    string        GetType() const { return GetClassType(); }

protected:
    virtual void _confirmText();
    virtual void _updateTab();

private:
    ViewpointEventRouter() {}

    virtual void wheelEvent(QWheelEvent *) {}

    float      _lastCamPos[3];
    VizWinMgr *_vizMgr;
    bool       _panChanged;

private slots:

    void viewpointReturnPressed();
    void setVtabTextChanged(const QString &qs);
};

#endif    // VIEWPOINTEVENTROUTER_H
