#pragma once

#include <QObject>
#include <vector>
#include <string>

namespace VAPoR {
    class ControlExec;
}

class DatasetImportController : public QObject {
    Q_OBJECT

public:
    enum class DatasetExistsAction { Prompt, ReplaceFirst, AddNew };
    bool ImportDataset(VAPoR::ControlExec *ce, const std::vector<std::string> &files, std::string format, DatasetExistsAction action);

signals:
    void datasetImported();
};
