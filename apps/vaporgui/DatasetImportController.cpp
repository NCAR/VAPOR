#include "DatasetImportController.h"
#include "ErrorReporter.h"
#include "DatasetTypeLookup.h"

#include "vapor/ControlExecutive.h"
#include "vapor/DataStatus.h"
#include "vapor/GUIStateParams.h"
#include "vapor/AnimationParams.h"
#include "vapor/NavigationUtils.h"
#include "vapor/FileUtils.h"

#include <QInputDialog>

using namespace std;
using namespace VAPoR;

std::string DatasetImportController::GetDatasetName(ParamsMgr *pm, std::string file, DatasetExistsAction existsAction) {
    auto names = pm->GetDataMgrNames();
    if (names.empty() || existsAction == DatasetExistsAction::AddNew)
        return ControlExec::MakeStringConformant(FileUtils::Basename(file));
    if (existsAction == DatasetExistsAction::ReplaceFirst)
        return names[0];

    QStringList items;
    items << "New Dataset";
    for (auto &n : names) items << QString::fromStdString(n);

    bool ok;
    QString choice = QInputDialog::getItem(nullptr, "Load Data", "Load as new dataset or replace existing", items, 0, false, &ok);
    if (!ok || choice.isEmpty()) return "";

    std::string name = choice.toStdString();
    if (name == "New Dataset")
        return ControlExec::MakeStringConformant(FileUtils::Basename(file));
    return name;
}

bool DatasetImportController::ImportDataset(ControlExec *ce, const vector<string> &files, std::string format, DatasetExistsAction action) {
    ParamsMgr *pm = ce->GetParamsMgr();
    pm->BeginSaveStateGroup("Import Dataset");

    std::string name = GetDatasetName(pm, files[0], action);
    if (name.empty()) {
        pm->EndSaveStateGroup();
        return false;
    }

    if (ce->OpenData(files, name, format) < 0) {
        pm->EndSaveStateGroup();
        MSG_ERR("Failed to load data");
        return false;
    }

    auto gsp = ce->GetParams<GUIStateParams>();
    gsp->InsertOpenDataSet(name, format, files);

    auto ds = ce->GetDataStatus();
    auto ap = ce->GetParams<AnimationParams>();
    ap->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);

    if (gsp->GetValueLong(GUIStateParams::SessionNewTag, false)) {
        NavigationUtils::ViewAll(ce);
        NavigationUtils::SetHomeViewpoint(ce);
        gsp->SetProjectionString(ds->GetMapProjection());
    }

    gsp->SetValueLong(GUIStateParams::SessionNewTag, "Reset SessionNewTag to false after data import", false);

    pm->EndSaveStateGroup();
    emit datasetImported();
    return true;
}
