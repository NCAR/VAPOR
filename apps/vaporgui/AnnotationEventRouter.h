#pragma once

#include "VaporFwd.h"
#include <QWidget>
#include "Updatable.h"

class PGroup;

class AnnotationEventRouter : public QWidget, public Updatable {
    Q_OBJECT

    ControlExec *_controlExec;
    std::vector<PGroup *> _groups;
    std::vector<PGroup *> _axisGroups;

public:
    AnnotationEventRouter(VAPoR::ControlExec *ce);
    void Update() override;
};
