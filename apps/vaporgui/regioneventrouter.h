//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		regioneventrouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Defines the RegionEventRouter class.
//		This class handles events for the region params
//
#ifndef REGIONEVENTROUTER_H
#define REGIONEVENTROUTER_H

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_regionTab.h"
#include "TabManager.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class RegionEventRouter : public QWidget, public Ui_RegionTab, public EventRouter {
    Q_OBJECT

public:
    RegionEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    virtual ~RegionEventRouter();

    // Connect signals and slots from tab
    virtual void hookUpTab();

    virtual void GetWebHelp(std::vector<std::pair<string, string>> &help) const;

    virtual void relabel();

    //! set the center of the region, shrink it if necessary
    //! \param[in] newCenter Point that will become center.
    void setCenter(const double newCenter[3]);

    // Start to slide a region face.  Need to save direction vector
    //
    virtual void captureMouseDown(int button);
    // When the mouse goes up, save the face displacement into the region.
    void captureMouseUp();

    virtual void wheelEvent(QWheelEvent *) {}

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("Region"); }
    string        GetType() const { return GetClassType(); }

protected slots:
    void changeExtents();

    void loadRegionExtents();
    void saveRegionExtents();
    void adjustExtents();
    void setRegionTabTextChanged(const QString &qs);
    void regionReturnPressed();
    void setMaxSize();

    void CopyBox();
    void setDomainVars();
    void removeDomainVar();
    void addDomainVar(int);

protected:
    virtual void _confirmText();
    virtual void _updateTab();

private:
    RegionEventRouter() {}

    //! Avoids updating of the slider text and other widgets that
    //! could interfere with interactive updating.
    //! \param[in] doIgnore true to start ignoring, false to stop.
    void setIgnoreBoxSliderEvents(bool doIgnore) { _ignoreBoxSliderEvents = doIgnore; }
    //! Method indicates whether box slider events are being ignored.
    //! \retval true if the box slider events are being ignored.
    bool doIgnoreBoxSliderEvents() { return _ignoreBoxSliderEvents; }

    static const char *_webHelpText[];
    static const char *_webHelpURL[];

    // Map combo indices to Params tags
    vector<string> _boxMapping;
    bool           _ignoreBoxSliderEvents;
};

#endif    // REGIONEVENTROUTER_H
