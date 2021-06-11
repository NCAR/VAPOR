#pragma once

#include "CopyRegionWidget.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class CopyRegionAnnotationWidget : public CopyRegionWidget {
    Q_OBJECT

public:
    CopyRegionAnnotationWidget(VAPoR::ControlExec *ce);

    QString toolTip() const
    {
        return tr("A widget for copying one renderer's "
                  "region to axis annotations");
    }

    void Update();

protected slots:
    void copyRegion() override;

private:
    void _scaleWorldCoordsToNormalized(std::vector<double> &minExts, std::vector<double> &maxExts, int timeStep);

    VAPoR::ControlExec *_controlExec;
};
