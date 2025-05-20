#include "ImportTab.h"
#include "PImportDataWidget.h"
#include "PProjectionStringWidget.h"
#include "VHyperlink.h"
#include "VSection.h"
#include "DatasetImporter.h"

#include <vapor/GUIStateParams.h>
#include <vapor/ControlExecutive.h>
#include <QVBoxLayout>

using namespace VAPoR;

ImportTab::ImportTab(VAPoR::ControlExec *ce, DatasetImporter *di) : _ce(ce)
{
    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);

    l->addWidget(new VSectionGroup("Import Data", {_importer = new PImportDataWidget(_ce, di)}));

    VSectionGroup *sg = new VSectionGroup("Tips", {
        new VHyperlink(
            "How to convert Non-Compliant NetCDF Files?",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/dataFormatRequirements/netCDF/nonCompliantNetCDF.html",
            true
        ),
        new VHyperlink(
            "Download example Datasets",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/downloads/sampleData.html",
            true
        ),
        new VHyperlink(
            "Additional information on supported data formats",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/dataFormatRequirements.html",
            true
        ),
        new VHyperlink(
            "Get Help",
            "https://vapor.discourse.group/",
            true
        ),
    });
    sg->setEnabled(true);
    l->addWidget(sg);

    l->addWidget(_projectionWidget = new PProjectionStringWidget(_ce));

    l->addStretch(1);
    setLayout(l);
}

void ImportTab::Update()
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    _importer->Update(guiParams);

    bool noDatasetLoaded = guiParams->GetOpenDataSetNames().empty();
    if (noDatasetLoaded) _projectionWidget->hide();
    else _projectionWidget->Update(guiParams);
}
