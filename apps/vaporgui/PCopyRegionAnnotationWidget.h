#pragma once

#include "PWidget.h"
//#include "vapor/ControlExecutive.h"
//#include "ControlExecWidget.h"

namespace VAPoR {
class ControlExec;
}

class CopyRegionAnnotationWidget;

//! \class PPCopyRegionAnnotationWidget
//! \brief PWidget wrapper for the PCopyRegionAnnotationWidget
//! \author Scott Pearse

class PCopyRegionAnnotationWidget : public PWidget {
    CopyRegionAnnotationWidget *_widget;

public:
    PCopyRegionAnnotationWidget(VAPoR::ControlExec *ce);

private:
    void updateGUI() const override;
};
