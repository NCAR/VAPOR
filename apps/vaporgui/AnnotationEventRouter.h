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
#include "RangeCombos.h"
#include "VaporTable.h"
#include <vapor/AxisAnnotation.h>

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class PGroup;
class PSection;
class VComboBox;
class VPushButton;

class AnnotationEventRouter : public QWidget, public EventRouter {
    Q_OBJECT

public:
    AnnotationEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    virtual ~AnnotationEventRouter();

    virtual void hookUpTab(){};

    //! Ignore wheel event in tab (to avoid confusion)
    virtual void wheelEvent(QWheelEvent *) {}

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("Annotation"); }
    string        GetType() const { return GetClassType(); }

    virtual void _confirmText(){};

protected slots:
    void axisAnnotationTableChanged();

private:
    Combo *_textSizeCombo;
    Combo *_digitsCombo;
    Combo *_ticWidthCombo;

    std::map<std::string, std::string> _visNames;
    std::map<std::string, std::string> _renTypeNames;

    vector<double> getTableRow(int row);

    AnnotationEventRouter() {}

    void setColorHelper(QWidget *w, vector<double> &rgb);
    void updateColorHelper(const vector<double> &rgb, QWidget *w);

    void updateAxisAnnotations();
    void updateAxisTable();

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

    virtual void _updateTab();

    AnimationParams *_ap;
    bool             _animConnected;

    VaporTable *_annotationVaporTable;

    std::vector<PGroup *> _groups;
    std::vector<PGroup *> _axisGroups;
};

#endif    // ANNOTATIONEVENTROUTER_H
