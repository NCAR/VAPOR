#pragma once

#include "VaporFwd.h"
#include "PWidgetsFwd.h"
#include <QWidget>
#include "Updatable.h"

class ViewpointTab : public QWidget, public Updatable {
    Q_OBJECT

    ControlExec *_controlExec;
    PWidget *_pg = nullptr;
    PWidget *_movingDomainSection = nullptr;

public:
    ViewpointTab(VAPoR::ControlExec *ce);
    void Update() override;
};
