//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AppSettingsEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November, 2015
//
//	Description:	Defines the AppSettingsEventRouter class.
//		This class handles events for the AppSettings params
//
#ifndef APPSETTINGSEVENTROUTER_H
#define APPSETTINGSEVENTROUTER_H

#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "appSettingsTab.h"

namespace VAPoR {
class ControlExec;
}

QT_USE_NAMESPACE

class AppSettingsEventRouter : public QWidget, public Ui_appSettingsTab, public EventRouter {

    Q_OBJECT

  public:
    AppSettingsEventRouter(
        QWidget *parent, VAPoR::ControlExec *ce);

    virtual ~AppSettingsEventRouter();

    //! For the AppSettingsEventRouter, we must override confirmText method on base class,
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
        return ("AppSettings");
    }

    string GetType() const { return GetClassType(); }

  protected slots:

    void setAppSettingsTextChanged(const QString &qs);
    void appSettingsReturnPressed();
    void setNoCitation(bool);
    void warningChanged(bool);
    void trackMouseChanged(bool);
    void lowerAccuracyChanged(bool);
    void setAutoSave(bool);
    void save();
    void restoreDefaults();
    void chooseLogFilePath();
    void chooseAutoSaveFilename();
    void silenceAllMessages(bool);
    void unsilenceMessages();
    void enableLogfile(bool);

  private:
    AppSettingsEventRouter() {}

    void invalidateText();
    virtual void _confirmText();
    virtual void _updateTab();

    bool _settingsChanged;
};

#endif //APPSETTINGSEVENTROUTER_H
