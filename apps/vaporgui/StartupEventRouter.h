//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		StartupEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November, 2015
//
//	Description:	Defines the StartupEventRouter class.
//		This class handles events for the Startup params
//
#ifndef STARTUPEVENTROUTER_H
#define STARTUPEVENTROUTER_H

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_startupTab.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class StartupParams;
class StartupEventRouter : public QWidget, public Ui_startupTab, public EventRouter {

    Q_OBJECT

  public:
    StartupEventRouter(
        QWidget *parent, VAPoR::ControlExec *ce);

    virtual ~StartupEventRouter();

    //! For the StartupEventRouter, we must override confirmText method on base class,
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
        return ("Startup");
    }
    string GetType() const { return GetClassType(); }

  private slots:

    void setStartupTextChanged(const QString &qs);
    void startupReturnPressed();
    void chooseSessionPath();
    void chooseMetadataPath();
    void chooseImagePath();
    void chooseTFPath();
    void chooseFlowPath();
    void choosePythonPath();
    void copyLatestSession();
    void copyLatestMetadata();
    void copyLatestTF();
    void copyLatestImage();
    void copyLatestFlow();
    void copyLatestPython();
    void changeTextureSize(bool val);
    void winLockChanged(bool val);
    void setAutoStretch(bool val);
    void apply();
    void restoreDefaults();
    void tabChanged(int topIndex, int subIndex);

  private:
    StartupEventRouter() {}

    void invalidateText();
    virtual void _confirmText();
    virtual void _updateTab();
    void setSettingsChanged(bool);

    bool _settingsChanged;
    StartupParams *_savedStartupParams;
};

#endif //STARTUPEVENTROUTER_H
