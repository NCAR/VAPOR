#pragma once

#include <QObject>

namespace VAPoR {
class ControlExec;
class ParamsMgr;
}

//! \class DatasetImporter
//! \brief Decouples dataset import logic that was previously in MainForm so it can be
//! performed outside of the MainForm

class DatasetImporter : public QObject {
    Q_OBJECT
    VAPoR::ControlExec *_ce;
    VAPoR::ParamsMgr   *_pm;

    enum DatasetExistsAction { Prompt, AddNew, ReplaceFirst };

    void showImportDatasetGUI(std::string format);
    std::string _getDataSetName(std::string file, DatasetExistsAction existsAction);
    std::vector<std::string> _getUserFileSelection(std::string prompt, std::string dir, std::string filter, bool multi);
    int checkQStringContainsNonASCIICharacter(const QString &s);
    bool doesQStringContainNonASCIICharacter(const QString &s);

public:
    DatasetImporter(VAPoR::ControlExec *ce);
    void ImportDataset(const std::vector<std::string> &files, std::string format, DatasetExistsAction existsAction, std::string name = "");

signals:
};
