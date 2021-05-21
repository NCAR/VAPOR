#pragma once

#include "PWidget.h"
#include "ControlExecWidget.h"

class CopyRegionAnnotationWidget;

//! \class PPCopyRegionAnnotationWidget
//! \brief PWidget wrapper for the PCopyRegionAnnotationWidget
//! \author Scott Pearse

class PCopyRegionAnnotationWidget : public PWidget {
    CopyRegionAnnotationWidget *_widget;

public:
    PCopyRegionAnnotationWidget( VAPoR::ControlExecutive* ce);

private:
    void updateGUI() const override;
};
