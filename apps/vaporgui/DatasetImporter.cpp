#include "DatasetImporter.h"
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>
#include "PTransformWidget.h"
#include "VLabel.h"
#include <QTreeWidget>
#include "PMetadataClasses.h"
#include "VScrollGroup.h"
#include "DatasetTypeLookup.h"
#include "PDatasetImportTypeSelector.h"

using namespace VAPoR;

DatasetImporter::DatasetImporter(VAPoR::ControlExec *ce)
: _ce(ce) {
    VScrollGroup *g = new VScrollGroup;

    std::vector<std::string> datasetTypes = GetDatsetTypeDescriptions();
    g->Add(new PDatasetImportTypeSelector(datsetTypes));

    g->Add(new VSectionGroup("Info fjioesajfo;mweo;ji", {
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

    connect(this, &QTabWidget::currentChanged, this, &DatasetImporter::Update);
}

void DatasetImporter::Update()
{
    ParamsMgr *pm = _ce->GetParamsMgr();
    GUIStateParams *guiParams = (GUIStateParams *)pm->GetParams(GUIStateParams::GetClassType());
    ViewpointParams *vp = pm->GetViewpointParams(guiParams->GetActiveVizName());
    DataStatus *dataStatus = _ce->GetDataStatus();
    string dataset = guiParams->GetActiveDataset();
    if (!vp || dataset.empty()) return;

    if (currentWidget() == _dataTab) {
        _name->SetText("Name: " + dataset);
        //_type->SetText("Type: " + DatasetTypeDescriptiveName(guiParams->GetOpenDataSetFormat(dataset)));
        _type->SetText("Type: " + DatasetTypeDescriptiveName(guiParams->GetOpenDataSetFormat(dataset)));
        _path->SetText("Path: " + guiParams->GetOpenDataSetPaths(dataset)[0]);
        Transform *transform = vp->GetTransform(dataset);
        _tw->Update(transform);
    }
    else if (currentWidget() == _metaTab) {
        auto dataMgr = dataStatus->GetDataMgr(dataset);
        _metaAttrs->Update(guiParams, pm, dataMgr);
        _metaDims->Update(guiParams, pm, dataMgr);
        _metaVars->Update(guiParams, pm, dataMgr);
        _metaCoords->Update(guiParams, pm, dataMgr);
    }
}

//std::string DatasetImporter::DatasetTypeDescriptiveName(std::string type)
//{
//    if (type == "vdc") return "VDC";
//    if (type == "wrf") return "WRF-ARW";
//    if (type == "cf") return "NetCDF-CF";
//    if (type == "mpas") return "MPAS";
//    if (type == "bov") return "Brick of Values (BOV)";
//    if (type == "dcp") return "Data Collection Particles (DCP)";
//    if (type == "ugrid") return "Unstructured Grid (UGRID)";
//    return type;
//}
