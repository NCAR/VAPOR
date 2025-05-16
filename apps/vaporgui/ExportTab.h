#pragma once

#include "VaporFwd.h"
#include "PWidgetsFwd.h"
#include "Updatable.h"
#include "VGroup.h"

class MainForm;
class CaptureController;

class ExportTab : public VGroup, public Updatable {
    Q_OBJECT

    ControlExec *_ce;
    PWidget *_pg = nullptr;
    PWidget *_movingDomainSection = nullptr;

public:
    ExportTab(VAPoR::ControlExec *ce, CaptureController *cc, MainForm *mf);
    void Update() override;
};
