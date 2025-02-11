#include "ImportPanel.h"
#include "VLabel.h"
#include "VSection.h"
#include "PImportData.h"
#include <vapor/GUIStateParams.h>
#include <vapor/ControlExecutive.h>
#include <QVBoxLayout>

using namespace VAPoR;

ImportPanel::ImportPanel(VAPoR::ControlExec *ce) : _ce(ce)
{

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);

    l->addWidget(new VSectionGroup("Import Data", {_importer = new PImportData(_ce)}));

    VSectionGroup *sg = new VSectionGroup("Tips", {
        new VHyperlink(
            "How to convert Non-Compliant NetCDF Files?",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/dataFormatRequirements/netCDF/nonCompliantNetCDF.html"
        ),
        new VHyperlink(
            "Download example Datasets",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/downloads/sampleData.html"
        ),
        new VHyperlink(
            "Additional information on supported data formats",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/dataFormatRequirements.html"
        ),
        new VHyperlink(
            "Get Help",
            "https://vapor.discourse.group/"
        ),
    });
    sg->setEnabled(true);
    l->addWidget(sg);

    l->addStretch(1);
    setLayout(l);
}

void ImportPanel::Update()
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    _importer->Update(guiParams);
}
