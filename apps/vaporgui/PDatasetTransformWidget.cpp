#include "PDatasetTransformWidget.h"
#include <vapor/ControlExecutive.h>
#include <vapor/STLUtils.h>
#include "GUIStateParams.h"
#include "VSection.h"
#include "PStringDropdown.h"
#include "PTransformWidget.h"

using namespace VAPoR;


PDatasetTransformWidget::PDatasetTransformWidget(VAPoR::ControlExec *ce)
: PWidget("", _section = new VSectionGroup("Transform",
                                           {
                                               _twDataset = new PStringDropdown("transformWidgetDatasetTag", {}, "Dataset"),
                                               _tw = new PTransformWidget,
                                           })),
  _ce(ce)
{
}


void PDatasetTransformWidget::updateGUI() const
{
    ParamsMgr *      pm = _ce->GetParamsMgr();
    auto             stateParams = ((GUIStateParams *)pm->GetParams(GUIStateParams::GetClassType()));
    auto             activeViz = stateParams->GetActiveVizName();
    ViewpointParams *vp = pm->GetViewpointParams(activeViz);

    DataStatus *   dataStatus = _ce->GetDataStatus();
    vector<string> datasets = dataStatus->GetDataMgrNames();
    if (datasets.empty()) {
        _section->setEnabled(false);
        return;
    } else {
        _section->setEnabled(true);
    }

    _twDataset->SetItems(datasets);
    _twDataset->Update(vp);

    string dataset = vp->GetValueString(SelectedDatasetTag, datasets[0]);
    if (!STLUtils::Contains(datasets, dataset)) dataset = datasets[0];
    Transform *transform = vp->GetTransform(dataset);
    _tw->Update(transform);
}
