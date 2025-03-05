#pragma once

#include "VaporFwd.h"
#include "PWidgetsFwd.h"
#include <QWidget>
#include "Updatable.h"

class MainForm;

class ExportTab : public QWidget, public Updatable {
    Q_OBJECT

    ControlExec *_ce;
    PWidget *_pg = nullptr;
    PWidget *_movingDomainSection = nullptr;

public:
    ExportTab(VAPoR::ControlExec *ce, MainForm *mf);
    void Update() override;
};
