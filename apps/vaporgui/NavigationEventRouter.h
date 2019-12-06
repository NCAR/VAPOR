//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		NavigationEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Defines the NavigationEventRouter class.
//		This class handles events for the viewpoint params
//
#ifndef VIEWPOINTEVENTROUTER_H
#define VIEWPOINTEVENTROUTER_H

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_NavigationTab.h"
#include "ParamsWidgets.h"

namespace VAPoR {
class ControlExec;
class RegionParams;
}    // namespace VAPoR

class NavigationEventRouter : public QWidget, public Ui_NavigationTab, public EventRouter {
    Q_OBJECT

public:
    NavigationEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    virtual ~NavigationEventRouter();
    // Connect signals and slots from tab
    virtual void hookUpTab();

    virtual void GetWebHelp(std::vector<std::pair<string, string>> &help) const;

    // Following methods are for undo/redo support:
    // Methods to capture state at start and end of mouse moves:
    //

    // Set from probe:
    void SetCenter(const double *centerCoords);

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("Viewpoint"); }
    string        GetType() const { return GetClassType(); }

    virtual bool Supports2DVariables() const { return true; }
    virtual bool Supports3DVariables() const { return false; }

    virtual void updateTab();

    void LoadDataNotify(string dataSetName);

signals:
    void Proj4StringChanged(string proj4String);
    void ProjectionTypeChanged(int);

protected:
    virtual void _confirmText(){};
    virtual void _updateTab();

private:
    ParamsWidgetCheckbox *_useCustomFramebufferCheckbox;
    ParamsWidgetNumber *  _customFramebufferWidth;
    ParamsWidgetNumber *  _customFramebufferHeight;

    NavigationEventRouter() {}

    virtual void wheelEvent(QWheelEvent *) {}

    void updateTransforms();
    void updateProjections();
    // void appendProjTable(int row, string projString, bool usingCurrentProj);
    void createProjCell(int row, string projString, bool ro);
    void createProjCheckBox(int row, bool usingCurrentProj);
    void resizeProjTable();
    void updateCameraChanged();
    void updateLightChanged();

    VAPoR::ViewpointParams *_getActiveParams() const;

    void _setViewpointParams(const vector<double> &modelview, const vector<double> &center) const;

    void _setViewpointParams(const double center[3], const double posvec[3], const double dirvec[3], const double upvec[3]) const;

    bool _getViewpointParams(double center[3], double posvec[3], double dirvec[3], double upvec[3]) const;

    void _performAutoStretching(string dataSetName);

public slots:
    void UseHomeViewpoint();
    void ViewAll();
    void SetHomeViewpoint();
    void AlignView(int axis);
    void CenterSubRegion();

private slots:
    void setCameraChanged();
    void setCameraLatLonChanged();
    void setLightChanged();
    void notImplemented();
    void projCheckboxChanged();
    void projectionComboBoxChanged(const QString &);
};

#endif    // VIEWPOINTEVENTROUTER_H
