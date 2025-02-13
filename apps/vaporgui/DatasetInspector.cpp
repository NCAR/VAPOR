#include "DatasetInspector.h"
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>
#include "PTransformWidget.h"
#include "VLabel.h"
#include <QTreeWidget>
#include "PMetadataClasses.h"
#include "PProjectionStringSection.h"
#include "VScrollGroup.h"

using namespace VAPoR;

DatasetInspector::DatasetInspector(VAPoR::ControlExec *ce)
: _ce(ce) {
    VScrollGroup *g = new VScrollGroup;
    g->Add(new VSectionGroup("Info", {
        _name = new VLabel,
        _type = new VLabel,
        _path = new VLabel
    }));
    g->Add(new VSectionGroup("Transform", {_tw = new PTransformWidget}));

    addTab(_dataTab = g, "Dataset");
    addTab(_metaTab = new VScrollGroup({
               _metaAttrs = new VGlobalAttributeMetadataTree,
               _metaDims = new VDimensionMetadataTree,
               _metaVars = new VVariableMetadataTree,
               _metaCoords = new VCoordinateVariableMetadataTree,
           }), "Metadata");
    g->Add(_projectionSection = new PProjectionStringSection(_ce));

    connect(this, &QTabWidget::currentChanged, this, &DatasetInspector::Update);
}

void DatasetInspector::Update()
{
    ParamsMgr *pm = _ce->GetParamsMgr();
    GUIStateParams *guiParams = (GUIStateParams *)pm->GetParams(GUIStateParams::GetClassType());
    ViewpointParams *vp = pm->GetViewpointParams(guiParams->GetActiveVizName());
    DataStatus *dataStatus = _ce->GetDataStatus();
    string dataset = guiParams->GetActiveDataset();
    if (!vp || dataset.empty()) return;

    if (currentWidget() == _dataTab) {
        _name->SetText("Name: " + dataset);
        _type->SetText("Type: " + DatasetTypeDescriptiveName(guiParams->GetOpenDataSetFormat(dataset)));
        _path->SetText("Path: " + guiParams->GetOpenDataSetPaths(dataset)[0]);
        Transform *transform = vp->GetTransform(dataset);
        _tw->Update(transform);
        _projectionSection->Update(guiParams);
    }
    else if (currentWidget() == _metaTab) {
        auto dataMgr = dataStatus->GetDataMgr(dataset);
        _metaAttrs->Update(guiParams, pm, dataMgr);
        _metaDims->Update(guiParams, pm, dataMgr);
        _metaVars->Update(guiParams, pm, dataMgr);
        _metaCoords->Update(guiParams, pm, dataMgr);
    }
}

std::string DatasetInspector::DatasetTypeDescriptiveName(std::string type)
{
    if (type == "vdc") return "VDC";
    if (type == "wrf") return "WRF-ARW";
    if (type == "cf") return "NetCDF-CF";
    if (type == "mpas") return "MPAS";
    if (type == "bov") return "Brick of Values (BOV)";
    if (type == "dcp") return "Data Collection Particles (DCP)";
    if (type == "ugrid") return "Unstructured Grid (UGRID)";
    return type;
}
