#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}

class CopyRegionAnnotationWidget;

//! \class PPCopyRegionAnnotationWidget
//! \brief PWidget wrapper for the PCopyRegionAnnotationWidget

class PCopyRegionAnnotationWidget : public PWidget {
    CopyRegionAnnotationWidget *_widget;

public:
    PCopyRegionAnnotationWidget(VAPoR::ControlExec *ce);

private:
    void updateGUI() const override;
};
