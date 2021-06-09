#pragma once

#include "EventRouter.h"

class PGroup;
class PIntegerInput;
namespace VAPoR {
class ControlExec;
}


class AnimationTab : public QWidget, public EventRouter {
    Q_OBJECT
    
    PGroup *_g;
    PIntegerInput *_startTS;
    PIntegerInput *_stopTS;
    
public:
    AnimationTab(QWidget *parent, VAPoR::ControlExec *ce);
    ~AnimationTab() {}
    static string GetClassType() { return ("Animation"); }
    string GetType() const { return GetClassType(); }

protected:
    virtual void _updateTab();
};
