#pragma once

#include "VSection.h"

namespace VAPoR {
class ControlExec;
}
class PTransformWidget;
class VLabel;
class QTreeWidget;
class VMetadataTree;

class DatasetInspector : public QTabWidget {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    QWidget *_dataTab, *_metaTab;
    PTransformWidget *_tw;
    VLabel *_name, *_type, *_path;
    VMetadataTree *_metaAttrs, *_metaDims, *_metaVars, *_metaCoords;

public:
    DatasetInspector(VAPoR::ControlExec *ce);
    void Update();

    static std::string DatasetTypeDescriptiveName(std::string type);
};
