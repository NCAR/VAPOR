#pragma once

#include "PSection.h"
#include "VContainer.h"
#include "VSection.h"
#include <vapor/DC.h>

class QTreeWidget;
class QTreeWidgetItem;
class PStringDropdown;
class PIntegerSliderEdit;

namespace VAPoR {
    class ControlExec;
    class RenderParams;
    class DataMgr;
}

class POpenVariableMetadataTree;
class POpenVariableMetadataWidget : public PWidget {
    Q_OBJECT

    POpenVariableMetadataTree*    _varTree;

public:
    POpenVariableMetadataWidget();

protected:
    bool requireDataMgr() const override { return true; }
    void updateGUI() const override;
};

class VFullDataVariableMetadataTree;
class VCoordinateVariableMetadataTree;
class VGlobalAttributeMetadataTree;
class PMetadataSection : public PWidget {
    Q_OBJECT

    VAPoR::ControlExec*              _ce;
    VSection*                        _section;
    VFullDataVariableMetadataTree*   _varTree;
    VCoordinateVariableMetadataTree* _coordTree;
    VGlobalAttributeMetadataTree*    _globalTree;
    PStringDropdown*                 _metadataDataset;
    PIntegerSliderEdit*              _metadataTimestep;

public:
    PMetadataSection(VAPoR::ControlExec* ce);

    static const std::string SelectedDatasetTag;
    static const std::string MetadataRLevelTag;
    static const std::string MetadataCLevelTag;
    static const std::string MetadataTimestepTag;

protected:
    void updateGUI() const override;
};

class VMetadataTree : public VSectionGroup, public Updateable {
    Q_OBJECT

    void _generateAttributeInfo(QTreeWidgetItem* parent, const VAPoR::DC::BaseVar baseVar) const;

public:
    VMetadataTree();
    virtual void Update(VAPoR::ParamsBase* p, VAPoR::ParamsMgr* pm, VAPoR::DataMgr* dm) override;

signals:
    void _timestepRejected();

protected:

    QTreeWidget*    _tree;
    VAPoR::DataMgr* _dm;
    size_t          _ts;
    bool            _needUpdate;
    std::vector<std::string> _topLevelBranches;

    virtual void _gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* rp = nullptr) const = 0;
    virtual void _generateMetadata(const QString& qvar) const;
    void _generateCoordVarInfo(QTreeWidgetItem* parent, const QString& qCoordVar) const;
    bool _checkNeedUpdate(VAPoR::ParamsBase* p, VAPoR::DataMgr* dm);
};

class POpenVariableMetadataTree : public VMetadataTree {
    Q_OBJECT

public:
    POpenVariableMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* p = nullptr) const override;
};

class VFullDataVariableMetadataTree : public VMetadataTree {
    Q_OBJECT

public:
    VFullDataVariableMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* rp = nullptr) const override;
};

class VCoordinateVariableMetadataTree : public VMetadataTree {
    Q_OBJECT

public:
    VCoordinateVariableMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* rp) const override;
    virtual void _generateMetadata(const QString& qvar) const override;
};

class VGlobalAttributeMetadataTree : public VMetadataTree {
    Q_OBJECT

public:
    VGlobalAttributeMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* rp) const override;
    virtual void _generateMetadata(const QString& qvar) const override;
};
