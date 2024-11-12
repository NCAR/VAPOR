#pragma once

#include "VaporFwd.h"
#include <QWidget>
#include "Updatable.h"

class PGroup;
class PIntegerInput;


class AnimationTab : public QWidget, public Updatable {
    Q_OBJECT

    ControlExec *_controlExec;
    PGroup *       _g;
    PIntegerInput *_startTS;
    PIntegerInput *_stopTS;

public:
    AnimationTab(VAPoR::ControlExec *ce);
    void Update() override;
};
