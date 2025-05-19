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

void DatasetImporter::ImportDataset(const std::vector<string> &files, string format, DatasetExistsAction existsAction, string name) {
    _pm->BeginSaveStateGroup("Import Dataset");
    //if (name.empty()) name = _getDataSetName(files[0], existsAction);
    int rc = _ce->OpenData(files, name, format);
    if (rc < 0) {
        _pm->EndSaveStateGroup();
        MSG_ERR("Failed to load data");
        return;
    }

    auto gsp = _ce->GetParams<GUIStateParams>();
    gsp->InsertOpenDataSet(name, format, files);

    VAPoR::DataStatus *ds = _ce->GetDataStatus();
    AnimationParams* ap = (AnimationParams*)_pm->GetParams(AnimationParams::GetClassType());
    ap->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);

    //if (_sessionNewFlag) {
        NavigationUtils::ViewAll(_ce);
        NavigationUtils::SetHomeViewpoint(_ce);
        gsp->SetProjectionString(ds->GetMapProjection());
    //}

    //_sessionNewFlag = false;
    _pm->EndSaveStateGroup();

    //_leftPanel->GoToRendererTab();
}

void DatasetImporter::showImportDatasetGUI(string format) {
    static vector<pair<string, string>> prompts = GetDatasets();

    string defaultPath;
    GUIStateParams *gsp = _ce->GetParams<GUIStateParams>();
    auto openDatasets = gsp->GetOpenDataSetNames();
    if (!openDatasets.empty())
        defaultPath = gsp->GetOpenDataSetPaths(openDatasets.back())[0];
    else
        defaultPath = _ce->GetParams<SettingsParams>()->GetMetadataDir();
        //defaultPath = GetSettingsParams()->GetMetadataDir();

    auto files = _getUserFileSelection(DatasetTypeDescriptiveName(format), defaultPath, "", format!="vdc");
    if (files.empty()) return;

    ImportDataset(files, format, DatasetExistsAction::Prompt);
}

std::vector<std::string> DatasetImporter::_getUserFileSelection(std::string prompt, std::string dir, std::string filter, bool multi) {
    QString qPrompt(prompt.c_str());
    QString qDir(dir.c_str());
    QString qFilter(filter.c_str());

    vector<string> files;
    if (multi) {
        QStringList           fileNames = QFileDialog::getOpenFileNames(nullptr, qPrompt, qDir, qFilter);
        QStringList           list = fileNames;
        QStringList::Iterator it = list.begin();
        while (it != list.end()) {
            if (!it->isNull()) {
                if (checkQStringContainsNonASCIICharacter(*it) < 0)
                    return {};
                files.push_back((*it).toStdString());
            }
            ++it;
        }
    } else {
        QString fileName = QFileDialog::getOpenFileName(nullptr, qPrompt, qDir, qFilter);
        if (!fileName.isNull()) {
            if (checkQStringContainsNonASCIICharacter(fileName) < 0)
                return {};
            files.push_back(fileName.toStdString());
        }
    }

    for (int i = 0; i < files.size(); i++) {
        QFileInfo fInfo(files[i].c_str());
        if (!fInfo.isReadable() || !fInfo.isFile()) {
            MyBase::SetErrMsg("File %s not readable", files[i].c_str());
            MSG_ERR("Invalid file selection");
            return {};
        }
    }
    return (files);
}

string DatasetImporter::_getDataSetName(string file, DatasetExistsAction existsAction) {
    vector<string> names = _pm->GetDataMgrNames();
    //if (names.empty() || existsAction == AddNew)
    //    return ControlExec::MakeStringConformant(FileUtils::Basename(file));
    //else if (existsAction == ReplaceFirst)
    //    return names[0];

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

    return dataSetName;
}

int DatasetImporter::checkQStringContainsNonASCIICharacter(const QString &s)
{
    if (doesQStringContainNonASCIICharacter(s)) {
#ifdef WIN32
        MyBase::SetErrMsg("Windows will convert a colon (common in WRF timestamps) to a non-ASCII dot character. This needs to be renamed.\n");
#endif
        MyBase::SetErrMsg("Vapor does not support paths with non-ASCII characters.\n");
        MSG_ERR("Non ASCII Character in path");
        return -1;
    }
    return 0;
}

bool DatasetImporter::doesQStringContainNonASCIICharacter(const QString &s)
{
    for (int i = 0; i < s.length(); i++)
        if (s.at(i).unicode() > 127) return true;
    return false;
}
