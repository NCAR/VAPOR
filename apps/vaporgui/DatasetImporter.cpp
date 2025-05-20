#include "DatasetImporter.h"
#include "ErrorReporter.h"
#include "DatasetTypeLookup.h"

#include "vapor/ControlExecutive.h"
#include "vapor/DataStatus.h"
#include "vapor/GUIStateParams.h"
#include "vapor/AnimationParams.h"
#include "vapor/SettingsParams.h"
#include "vapor/NavigationUtils.h"
#include "vapor/FileUtils.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>

DatasetImporter::DatasetImporter(VAPoR::ControlExec *ce) : _ce(ce), _pm(_ce->GetParamsMgr()) {}

bool DatasetImporter::ImportDataset(const std::vector<string> &files, string format, DatasetExistsAction existsAction, string name) {
    std::cout << "DatasetImporter::ImportDataset" << std::endl;
    _pm->BeginSaveStateGroup("Import Dataset");
    if (name.empty()) name = _getDataSetName(files[0], existsAction);
    int rc = _ce->OpenData(files, name, format);
    if (rc < 0) {
        _pm->EndSaveStateGroup();
        MSG_ERR("Failed to load data");
        std::cout << "failed" << std::endl;
        return false;
    }

    auto gsp = _ce->GetParams<GUIStateParams>();
    gsp->InsertOpenDataSet(name, format, files);

    VAPoR::DataStatus *ds = _ce->GetDataStatus();
    AnimationParams* ap = (AnimationParams*)_pm->GetParams(AnimationParams::GetClassType());
    ap->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);

    if (_sessionNewFlag) {
        NavigationUtils::ViewAll(_ce);
        NavigationUtils::SetHomeViewpoint(_ce);
        gsp->SetProjectionString(ds->GetMapProjection());
    }

    _pm->EndSaveStateGroup();

    std::cout << "name, format, files " << name << " " << format << " " << files.size() << std::endl;
    emit datasetImported();
    std::cout << "emitted" << std::endl;

    return true;
}

void DatasetImporter::SetSessionNewFlag(bool flag) { 
    _sessionNewFlag = flag; 
};

string DatasetImporter::_getDataSetName(string file, DatasetExistsAction existsAction) {
    vector<string> names = _pm->GetDataMgrNames();
    if (names.empty() || existsAction == DatasetExistsAction::AddNew)
        return ControlExec::MakeStringConformant(FileUtils::Basename(file));
    else if (existsAction == DatasetExistsAction::ReplaceFirst)
        return names[0];

    string newSession = "New Dataset";

    QStringList items;
    items << tr(newSession.c_str());
    for (int i = 0; i < names.size(); i++)
        items << tr(names[i].c_str());

    bool    ok;
    QString item = QInputDialog::getItem(nullptr, tr("Load Data"), tr("Load as new dataset or replace existing"), items, 0, false, &ok);
    if (!ok || item.isEmpty())
        return "";

    string dataSetName = item.toStdString();

    if (dataSetName == newSession)
        dataSetName = ControlExec::MakeStringConformant(FileUtils::Basename(file));

    std::cout << "datasetName " << dataSetName << std::endl;
    return dataSetName;
}
