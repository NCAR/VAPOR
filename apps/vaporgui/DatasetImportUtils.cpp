#include "DatasetImportUtils.h"
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

namespace DatasetImportUtils {

static string GetDataSetName(ParamsMgr *pm, string file, DatasetExistsAction existsAction) {
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

    string name = choice.toStdString();
    if (name == "New Dataset")
        return ControlExec::MakeStringConformant(FileUtils::Basename(file));
    return name;
}

bool ImportDataset(ControlExec *ce, const vector<string> &files, string format, DatasetExistsAction action, bool sessionNewFlag, function<void()> onImport) {
    ParamsMgr *pm = ce->GetParamsMgr();
    pm->BeginSaveStateGroup("Import Dataset");

    string name = GetDataSetName(pm, files[0], action);
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

    if (sessionNewFlag) {
        NavigationUtils::ViewAll(ce);
        NavigationUtils::SetHomeViewpoint(ce);
        gsp->SetProjectionString(ds->GetMapProjection());
    }

    pm->EndSaveStateGroup();

    if (onImport) onImport();
    return true;
}

}  // namespace DatasetImportUtils

