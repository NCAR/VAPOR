#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}
class VSection;
class PStringDropdown;
class PTransformWidget;


class PDatasetTransformWidget : public PWidget {
    VAPoR::ControlExec *     _ce;
    VSection *               _section;
    PStringDropdown *        _twDataset;
    PTransformWidget *       _tw;
    static const std::string SelectedDatasetTag;

public:
    PDatasetTransformWidget(VAPoR::ControlExec *ce);

    void updateGUI() const override;
};
