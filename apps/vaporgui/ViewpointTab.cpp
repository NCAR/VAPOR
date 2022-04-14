#include "ViewpointTab.h"
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>
#include <QHBoxLayout>
#include "PWidgets.h"
#include "PDatasetTransformWidget.h"
#include "PProjectionStringSection.h"
#include "PCameraControlsSection.h"
#include "PFramebufferSettingsSection.h"
#include "PMetadataClasses.h"
#include "PTimestepSliderEdit.h"

using namespace VAPoR;


ViewpointTab::ViewpointTab(ControlExec *ce) : EventRouter(ce, ViewpointParams::GetClassType())
{
    PProjectionStringSection *proj;
    _pg = new PGroup({
        new PDatasetTransformWidget(_controlExec),
        new PCameraControlsSection(_controlExec),
        new PFramebufferSettingsSection(_controlExec),
        proj = new PProjectionStringSection(_controlExec),
//        new PMetadataSection(_controlExec),
    });

    connect(proj, &PProjectionStringSection::Proj4StringChanged, this, &ViewpointTab::Proj4StringChanged);

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    layout()->addWidget(_pg);
    l->addStretch();
}

void ViewpointTab::_updateTab()
{
    auto vp = NavigationUtils::GetActiveViewpointParams(_controlExec);
    if (isEnabled() && vp)
        if (_pg) _pg->Update(GetStateParams());
}
