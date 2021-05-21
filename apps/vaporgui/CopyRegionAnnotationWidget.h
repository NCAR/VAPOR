#pragma once

#include "CopyRegionWidget.h"
#include "ControlExecWidget.h"

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}

class CopyRegionAnnotationWidget : public CopyRegionWidget, ControlExecWidget {
    Q_OBJECT

public:
    CopyRegionAnnotationWidget( VAPoR::ControlExecutive* ce );

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

    virtual void Update(VAPoR::ParamsMgr* pm);

protected slots:
    void copyRegion() override;
};
