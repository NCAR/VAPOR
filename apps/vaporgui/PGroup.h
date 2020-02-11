#pragma once

#include "PWidget.h"
#include <vector>

class PGroup : public PWidget {
    Q_OBJECT
    
    QWidget *_widget;
    std::vector<PWidget *> _children;
    
public:
    PGroup();
    PGroup *Add(PWidget *pw);

protected:
    void updateGUI() const override;
};
