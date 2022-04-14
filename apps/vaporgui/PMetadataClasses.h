#pragma once

#include "PSection.h"
#include "VContainer.h"
#include "VSection.h"
#include <vapor/DC.h>

class QTreeWidget;
class QTreeWidgetItem;
class PStringDropdown;
class PIntegerSliderEdit;
class VVariableMetadataTree;
class VOpenVariableMetadataTree;
class VCoordinateVariableMetadataTree;
class VGlobalAttributeMetadataTree;

namespace VAPoR {
    class ControlExec;
    class RenderParams;
    class DataMgr;
}

//! \class POpenVariableMetadataWidget
//! Allows the user view variable metadata that is currently
//! in use by an instantiated renderer.
class POpenVariableMetadataWidget : public PWidget {
    Q_OBJECT

    VOpenVariableMetadataTree*    _varTree;

public:
    POpenVariableMetadataWidget();

protected:
    bool requireDataMgr() const override { return true; }
    void updateGUI() const override;
};

//! \class PMetadataSection
//! Allows the user view all variable metadata that is in the
//! dataset selected by a drop-down menu.  It also displays
//! coordinate variable metadata and global attributes for the
//! given dataset, at the given timestep.
class PMetadataSection : public PWidget {
    Q_OBJECT

    VAPoR::ControlExec*              _ce;
    VSection*                        _section;
    VVariableMetadataTree*   _varTree;
    VCoordinateVariableMetadataTree* _coordTree;
    VGlobalAttributeMetadataTree*    _globalTree;
    PStringDropdown*                 _metadataDataset;
    PIntegerSliderEdit*              _metadataTimestep;

public:
    PMetadataSection(VAPoR::ControlExec* ce);

    static const std::string SelectedDatasetTag;
    static const std::string MetadataTimestepTag;

protected:
    void updateGUI() const override;
};

//! \class VMetadataTree
//! Abstract base class for VWidgets that wrap QTreeWidgets.
//! Only updates QTreeWidgets when the user's selected DataMgr,
//! timestep, or variable list change.
class VMetadataTree : public VSectionGroup, public Updateable {
    Q_OBJECT

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

    virtual void _gatherBranches(std::vector<std::string> &branches, VAPoR::ParamsBase* rp = nullptr) const = 0;
    virtual void _generateMetadata(QTreeWidgetItem* item) const = 0;
    bool _checkNeedUpdate(VAPoR::ParamsBase* p, VAPoR::DataMgr* dm);
};

//! \class VVariableMetadataTree
//! Displays metadata for all variables in a given DataMgr instance.
class VVariableMetadataTree : public VMetadataTree {
    Q_OBJECT

public:
    VVariableMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &branches, VAPoR::ParamsBase* rp = nullptr) const override;
    void _generateMetadata(QTreeWidgetItem* item) const override;

    void _generateCoordVarInfo(QTreeWidgetItem* parent, const QString& qCoordVar) const;
    void _generateAttributeInfo(QTreeWidgetItem* parent, const VAPoR::DC::BaseVar baseVar) const;
};

//! \class VVariableMetadataTree
//! Displays metadata for all variables open in a given RenderParams instance.
class VOpenVariableMetadataTree : public VVariableMetadataTree {
    Q_OBJECT

public:
    VOpenVariableMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &branches, VAPoR::ParamsBase* p = nullptr) const override;
};

//! \class VVariableMetadataTree
//! Displays metadata for all coordinate variables in a given DataMgr instance.
class VCoordinateVariableMetadataTree : public VVariableMetadataTree {
    Q_OBJECT

public:
    VCoordinateVariableMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &branches, VAPoR::ParamsBase* rp) const override;
    void _generateMetadata(QTreeWidgetItem* item) const override;
};

//! \class VVariableMetadataTree
//! Displays global attribute metadata for a given DataMgr instance.
class VGlobalAttributeMetadataTree : public VMetadataTree {
    Q_OBJECT

public:
    VGlobalAttributeMetadataTree();

protected:
    void _gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* rp) const override;
    void _generateMetadata(QTreeWidgetItem* item) const override;
};
