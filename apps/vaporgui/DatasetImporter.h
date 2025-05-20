#pragma once

#include "DatasetExistsAction.h"
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
    bool _sessionNewFlag = true;

    std::string _getDataSetName(std::string file, DatasetExistsAction existsAction);

public:
    DatasetImporter(VAPoR::ControlExec *ce);
    bool ImportDataset(
        const std::vector<std::string> &files, 
        std::string format, 
        DatasetExistsAction existsAction = DatasetExistsAction::Prompt, 
        std::string name = ""
    );
    void SetSessionNewFlag(bool flag);

signals:
    void datasetImported();
};
