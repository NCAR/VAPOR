#pragma once

#include "EventRouter.h"
#include "PWidgetsFwd.h"

namespace VAPoR {
class ControlExec;
}

class ViewpointTab : public QWidget, public EventRouter {
    Q_OBJECT

    PWidget *_pg = nullptr;

public:
    ViewpointTab(VAPoR::ControlExec *ce);
    virtual ~ViewpointTab() {}

    static string GetClassType() { return ("Viewpoint"); }
    string        GetType() const { return GetClassType(); }

    virtual void updateTab() { _updateTab(); }
    virtual void _updateTab();

signals:
    void Proj4StringChanged(string proj4String);
};
