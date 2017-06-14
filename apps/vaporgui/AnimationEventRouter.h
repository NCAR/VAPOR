//************************************************************************
//															*
//		     Copyright (C)  2006										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		AnimationEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Defines the AnimationEventRouter class.
//		This class handles events for the Animation params
//
#ifndef ANIMATIONEVENTROUTER_H
#define ANIMATIONEVENTROUTER_H

#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "animationTab.h"
#include "TabManager.h"

namespace VAPoR {
class AnimationParams;
class ControlExec;
}    // namespace VAPoR

class QTableWidget;
class QTimer;
class Combo;

QT_USE_NAMESPACE

class AnimationEventRouter : public QWidget, public Ui_AnimationTab, public EventRouter {
    Q_OBJECT

public:
    AnimationEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    ~AnimationEventRouter();

    // Connect signals and slots from tab
    virtual void hookUpTab();

    virtual void GetWebHelp(std::vector<std::pair<string, string>> &help) const;

    // Get static string identifier for this router class
    //
    static string GetClassType() { return ("Animation"); }

    string GetType() const { return GetClassType(); }

public slots:
    // Animation slots:
    //
    void AnimationPause();
    void AnimationPlayReverse();
    void AnimationPlayForward();
    void AnimationStepForward();
    void AnimationStepReverse();
    void AnimationReplay();
    void AnimationGoToBegin();
    void AnimationGoToEnd();

    void SetTimeStep(int ts);
    void SetFrameStep(int step);
    void SetFrameRate(int step);

signals:

    // Emitted when animation is turned on (true) or off (false)
    //
    void AnimationOnOffChanged(bool onOff);

    // Emitted when the client should draw a frame during animation. Only
    // Emitted if AnimationOnOffChanged() was most recently called with
    // onOff == true;
    //
    void AnimationDraw();

protected:
    virtual void _confirmText(){};
    virtual void _updateTab();

private:
    AnimationEventRouter() {}

    void setPlay(int direction);

    void enableWidgets(bool on);

    // actions on main window:
    //
    QAction *_mainPlayForwardAction;
    QAction *_mainPlayBackwardAction;
    QAction *_mainPauseAction;

    Combo * _timestepSelectCombo;
    Combo * _frameStepCombo;
    Combo * _frameRateCombo;
    QTimer *_myTimer;
    int     _direction;

private slots:

    void setStart();
    void setEnd();
    void playNextFrame();
};

#endif    // ANIMATIONEVENTROUTER_H
