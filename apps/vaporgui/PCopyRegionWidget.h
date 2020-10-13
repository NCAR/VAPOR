#pragma once

#include "PWidget.h"

class CopyRegionWidget;

class PCopyRegionWidget : public PWidget {
    CopyRegionWidget *_widget;
public:
    PCopyRegionWidget();
protected:
    void updateGUI() const override;
    bool requireParamsMgr() const override { return true; }
};
