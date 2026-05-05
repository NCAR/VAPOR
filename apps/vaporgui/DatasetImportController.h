#pragma once

#include <QObject>
#include <vector>
#include <string>

namespace VAPoR {
    class ControlExec;
    class ParamsMgr;
}

class DatasetImportController : public QObject {
    Q_OBJECT

public:
    enum class DatasetExistsAction { Prompt, ReplaceFirst, AddNew };
    bool ImportDataset(VAPoR::ControlExec *ce, const std::vector<std::string> &files, std::string format, DatasetExistsAction action);
    std::string GetDatasetName(VAPoR::ParamsMgr *pm, std::string file, DatasetExistsAction existsAction);


signals:
    void datasetImported();
};
