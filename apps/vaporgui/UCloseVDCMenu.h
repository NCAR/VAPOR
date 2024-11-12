#pragma once
#include <QMenu>
#include "Updatable.h"
#include "common.h"

class UCloseVDCMenu : public QMenu, public Updatable{
    Q_OBJECT
    ControlExec * const _ce;

public:
    UCloseVDCMenu(QMenu *parent, ControlExec *ce);
    void Update() override;
};
