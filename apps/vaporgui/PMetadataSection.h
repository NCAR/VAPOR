#pragma once

#include "PSection.h"
#include <vapor/DC.h>

class QTreeWidget;
class QTreeWidgetItem;

namespace VAPoR {
    class RenderParams;
    class DataMgr;
}

class PMetadataSection : public PSection {
    Q_OBJECT

public:
    PMetadataSection();

protected:
    bool requireDataMgr() const override { return true; }
};

class PMetadataTree : public PWidget {
    Q_OBJECT

    QTreeWidget* _tree;

    void _generateCoordVarInfo(QTreeWidgetItem* parent, const VAPoR::DC::DataVar dataVar) const;
    void _generateAttributeInfo(QTreeWidgetItem* parent, const VAPoR::DC::BaseVar baseVar) const;

public:
    PMetadataTree();

protected:
    void updateGUI() const override;
    bool requireDataMgr() const override { return true; }
};
