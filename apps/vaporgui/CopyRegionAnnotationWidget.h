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

    QString name() const { return "CopyRegionAnnotationWidget"; }
    QString includeFile() const { return "CopyRegionAnnotationWidget.h"; }
    QString group() const { return tr("A widget for copying one renderer's region to axis annotations"); }
    QString toolTip() const
    {
        return tr("A widget for copying one renderer's "
                  "region to asix annotations");
    }
    QString whatsThis() const
    {
        return tr("This widget contains all widgets "
                  "necessary for copying renderer regions "
                  "into a set of axis annotations");
    }

    virtual void Update(VAPoR::ParamsMgr *pm);

protected slots:
    void copyRegion() override;

private:
    void _scaleWorldCoordsToNormalized(std::vector<double> &minExts, std::vector<double> &maxExts, int timeStep);

    VAPoR::ControlExec *_controlExec;
};
