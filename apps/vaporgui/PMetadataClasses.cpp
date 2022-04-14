#include <QTreeWidget>
#include <QHeaderView>
#include <sstream>

#include "ErrorReporter.h"
#include "PMetadataClasses.h"
#include "PSection.h"
#include "PStringDropdown.h"
#include "PSliderEdit.h"

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/DC.h>
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>
#include <vapor/STLUtils.h>

namespace {
    std::vector<std::string> xtypeLookup = { "INVALID", "FLOAT", "DOUBLE", "UINT8", "INT8", "INT32", "INT64", "TEXT" };
    std::vector<std::string> axisLookup  = { "X", "Y", "Z", "T"};
}

const std::string PMetadataSection::SelectedDatasetTag  = "metadataDatasetTag";
const std::string PMetadataSection::MetadataTimestepTag = "metadataTimestepTag";

POpenVariableMetadataWidget::POpenVariableMetadataWidget()
: PWidget("", _varTree = new VOpenVariableMetadataTree())
{}

void POpenVariableMetadataWidget::updateGUI() const {
    _varTree->Update(getParams(),nullptr,getDataMgr());
}

PMetadataSection::PMetadataSection(VAPoR::ControlExec* ce)
: PWidget("", _section = new VSectionGroup("Dataset Metadata",
                                           {
                                                _metadataDataset = new PStringDropdown(SelectedDatasetTag, {}, "Dataset"),
                                                (_metadataTimestep = new PIntegerSliderEdit(MetadataTimestepTag, "Generate metadata\nfor timestep #"))->AllowUserRange(false),
                                                _dimTree = new VDimensionMetadataTree(),
                                                _varTree = new VVariableMetadataTree(),
                                                _coordTree = new VCoordinateVariableMetadataTree(),
                                                _globalTree = new VGlobalAttributeMetadataTree(),
                                            })
) {
    VAssert(ce != nullptr);
    _ce = ce;

    connect(_varTree, &VVariableMetadataTree::_timestepRejected, this, &PMetadataSection::updateGUI);
}

void PMetadataSection::updateGUI() const {

    VAPoR::ParamsMgr *      pm = _ce->GetParamsMgr();
    auto                    stateParams = ((GUIStateParams*)pm->GetParams(GUIStateParams::GetClassType()));
    auto                    activeViz = stateParams->GetActiveVizName();
    VAPoR::ViewpointParams *vp = pm->GetViewpointParams(activeViz);
    if (!vp) return;

    VAPoR::DataStatus *   dataStatus = _ce->GetDataStatus();
    vector<string> datasets = dataStatus->GetDataMgrNames();
    if (datasets.empty()) {
        _metadataDataset->setEnabled(false);
        _metadataTimestep->setEnabled(false);
        _varTree->setEnabled(false);
        _coordTree->setEnabled(false);
        return;
    } else {
        _metadataDataset->setEnabled(true);
        _metadataTimestep->setEnabled(true);
        _varTree->setEnabled(true);
        _coordTree->setEnabled(true);
    }

    _metadataDataset->SetItems(datasets);
    _metadataDataset->SetItems(datasets);
    _metadataDataset->Update(vp);

    VAPoR::DataMgr* dm;
    string dataset = vp->GetValueString(SelectedDatasetTag, datasets[0]);
    if (!STLUtils::Contains(datasets, dataset)) dataset = datasets[0];
    dm = _ce->GetDataStatus()->GetDataMgr(dataset);

    _dimTree->Update(getParams(), nullptr, dm);
    _varTree->Update(getParams(), nullptr, dm);
    _coordTree->Update(getParams(), nullptr, dm);
    _globalTree->Update(getParams(), nullptr, dm);
    
    _metadataTimestep->SetRange(0,dm->GetNumTimeSteps()-1);
    _metadataTimestep->Update(getParams());
}

VMetadataTree::VMetadataTree()
: VSectionGroup(""),
  Updateable(),
  _tree(new QTreeWidget),
  _dm(nullptr),
  _ts(-1),
  _topLevelBranches({})
{
    _vgroup->AddM({_tree});
    _tree->setHeaderHidden(true);
    _tree->header()->setStretchLastSection(false);
    _tree->setColumnCount(2);
    _tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(_tree, &QTreeWidget::itemExpanded, this, &VMetadataTree::_generateMetadata);
}

void VMetadataTree::Update(VAPoR::ParamsBase* p, VAPoR::ParamsMgr* pm, VAPoR::DataMgr* dm) {
    // Check if we need to update our tree.
    // Note: This function also generates the _topLevelBranches variable list, who's metadata will be shown.
    if (!_checkNeedUpdate(p, dm)) return;

    QStringList qbranches;
    for (auto branch : _topLevelBranches)
        qbranches << QString::fromStdString(branch);
    qbranches.removeDuplicates();
    qbranches.removeAll(QString(""));

    _tree->clear();
    for (auto qbranch : qbranches){
        QTreeWidgetItem* varItem = new QTreeWidgetItem(_tree, {qbranch});
        // Add a blank "leaf" on each branch so they can be expanded (and therefore populated)
        QTreeWidgetItem* leaf = new QTreeWidgetItem(varItem, {""});
        (void)leaf; // Silence unused variable warning
        _tree->insertTopLevelItem(0,varItem);
    }
}

void VVariableMetadataTree::_generateMetadata(QTreeWidgetItem* item) const {
    if (item->childCount() > 1) return; // This branch contains more than an empty leaf, and has already been computed

    QTreeWidgetItem* leaf = item->takeChild(0);
    if (leaf != 0) delete leaf;

    QString qvar = item->text(0);
    std::vector<double> range;
    int rc = _dm->GetDataRange(_ts, qvar.toStdString(), -1, -1, range);
    if (rc < 0) return;

    QTreeWidgetItem* varItem = item;

    new QTreeWidgetItem(varItem, {"Min:", QString::number(range[0])});
    new QTreeWidgetItem(varItem, {"Max:", QString::number(range[1])});

    std::vector<size_t> dims;
    _dm->GetDimLensAtLevel(qvar.toStdString(), -1, dims, _ts);
    QString qDims = QString::number(dims[0]);
    if (dims.size()>1) 
        qDims = qDims + ":" + QString::number(dims[1]);
    if (dims.size()>2) 
        qDims = qDims + ":" + QString::number(dims[2]);
    new QTreeWidgetItem(varItem, {"Dims (XYZ):", qDims});

    VAPoR::DC::Mesh mesh;
    QTreeWidgetItem* coords = new QTreeWidgetItem(varItem, {"Coordinates"});

    VAPoR::DC::DataVar dataVar;
    _dm->GetDataVarInfo(qvar.toStdString(), dataVar);
    _dm->GetMesh(dataVar.GetMeshName(), mesh);
    std::vector<std::string> coordVars = mesh.GetCoordVars();
    QStringList qCoordVars;
    for (auto coordVar : coordVars)
        qCoordVars << QString::fromStdString(coordVar);
    qCoordVars.removeDuplicates();
    qCoordVars.removeAll(QString(""));
    for (auto qCoordVar : qCoordVars )
        _generateCoordVarInfo(coords, qCoordVar);

    _generateAttributeInfo(varItem, dataVar);
}

bool VMetadataTree::_checkNeedUpdate(VAPoR::ParamsBase* p, VAPoR::DataMgr* dm) {
    bool needsUpdate = false;
   
    if (dm != _dm ) {
        _dm = dm;
        needsUpdate = true;
    }
    
    VAPoR::RenderParams* rp = dynamic_cast<VAPoR::RenderParams*>(p);
    size_t ts;
    if (rp != nullptr) {
        ts = rp->GetCurrentTimestep();
    }
    else {
        ts = p->GetValueLong(PMetadataSection::MetadataTimestepTag, 0);
    }
    if (ts != _ts ) {
        _ts=ts;
        needsUpdate = true;
    }
    if (ts > _dm->GetNumTimeSteps()-1) {
        _ts = _dm->GetNumTimeSteps()-1;
        needsUpdate = true;
    }
    
    std::vector<std::string> topLevelBranches;
    _gatherBranches(topLevelBranches, p);
    if (topLevelBranches!= _topLevelBranches) {
        _topLevelBranches = topLevelBranches;
        needsUpdate = true;
    }
  
    return needsUpdate;
}

void VVariableMetadataTree::_generateCoordVarInfo(QTreeWidgetItem* parent, const QString& qCoordVar) const {
    VAPoR::DC::CoordVar coordVar;
    if (!_dm->GetCoordVarInfo(qCoordVar.toStdString(), coordVar)) return;

    QTreeWidgetItem* coordItem;
    if (parent->text(0) == qCoordVar)
        coordItem = parent;
    else
        coordItem = new QTreeWidgetItem(parent, {qCoordVar});

    std::vector<std::string> dimNames = coordVar.GetDimNames();
    QString qDimNames;
    for (auto dimName : dimNames)
        qDimNames += QString::fromStdString(dimName) + " ";
    new QTreeWidgetItem(coordItem, {"Dimension names:", qDimNames});
    
    std::vector<size_t> dims = {1};
    _dm->GetDimLensAtLevel(qCoordVar.toStdString(), -1, dims, _ts);
    QString qDims = QString::number(dims[0]);
    if (dims.size()>1) 
        qDims = qDims + ":" + QString::number(dims[1]);
    if (dims.size()>2) 
        qDims = qDims + ":" + QString::number(dims[2]);
    new QTreeWidgetItem(coordItem, {"Dimension sizes:", qDims});

    new QTreeWidgetItem(coordItem, {"Time dim:", QString::fromStdString(coordVar.GetTimeDimName())});
    new QTreeWidgetItem(coordItem, {"Axis:", QString::fromStdString(axisLookup[coordVar.GetAxis()])});
    new QTreeWidgetItem(coordItem, {"Data type:", QString::fromStdString(xtypeLookup[coordVar.GetXType()+1])});
    new QTreeWidgetItem(coordItem, {"Units:", QString::fromStdString(coordVar.GetUnits())});
}

void VVariableMetadataTree::_generateAttributeInfo(QTreeWidgetItem* parent, const VAPoR::DC::BaseVar baseVar) const {
    QTreeWidgetItem* attrs = new QTreeWidgetItem(parent, {"Attributes"});
    std::map<std::string, VAPoR::DC::Attribute> attributes = baseVar.GetAttributes();
    std::map<std::string, VAPoR::DC::Attribute>::iterator it;
    std::stringstream s;
    for(auto const& attribute : attributes) {
        s.str(std::string());
        VAPoR::DC::XType xType = attribute.second.GetXType();
        if ( xType == VAPoR::DC::XType::INVALID ) continue;
        else if ( xType == VAPoR::DC::XType::TEXT ) {
            std::string values;
            attribute.second.GetValues(values);
            s.str(values);
        }
        else if (xType == VAPoR::DC::XType::FLOAT ||
                 xType == VAPoR::DC::XType::DOUBLE ) {
            std::vector<double> values;
            attribute.second.GetValues(values);
            copy(values.begin(), values.end(), ostream_iterator<int>(s," "));
        }
        else {
            std::vector<long> values;
            attribute.second.GetValues(values);
            copy(values.begin(), values.end(), ostream_iterator<int>(s," "));
        }
            
        new QTreeWidgetItem(attrs, {QString::fromStdString(attribute.first), QString::fromStdString(s.str())});
    }
}

VOpenVariableMetadataTree::VOpenVariableMetadataTree() : VVariableMetadataTree() {
    setTabText(0,"Open Variable Metadata");
}

void VOpenVariableMetadataTree::_gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* p) const {
    VAPoR::RenderParams* rp = dynamic_cast<VAPoR::RenderParams*>(p);
    vars.clear();
    vars.push_back( rp->GetVariableName() );
    vars.push_back( rp->GetXFieldVariableName() );
    vars.push_back( rp->GetYFieldVariableName() );
    vars.push_back( rp->GetZFieldVariableName() );
    vars.push_back( rp->GetHeightVariableName() );
    vars.push_back( rp->GetColorMapVariableName() );
    std::vector<std::string> auxs = rp->GetAuxVariableNames();
    vars.insert(vars.end(), auxs.begin(), auxs.end());
}

VVariableMetadataTree::VVariableMetadataTree() : VMetadataTree() {
    setTabText(0,"Variable Metadata");
}

void VVariableMetadataTree::_gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* p) const {
    vars.clear();
    std::vector<std::string> v = _dm->GetDataVarNames();
    vars.insert(vars.end(),v.begin(),v.end());
}

VCoordinateVariableMetadataTree::VCoordinateVariableMetadataTree() : VVariableMetadataTree() {
    setTabText(0,"Coordinate Variable Metadata");
}

void VCoordinateVariableMetadataTree::_generateMetadata(QTreeWidgetItem* item) const {
    if (item->childCount() > 1) return; // This branch has already been computed
    QTreeWidgetItem* leaf = item->takeChild(0);
    if (leaf != 0) delete leaf;
    QString qvar = item->text(0);
    _generateCoordVarInfo(item, qvar);
}

void VCoordinateVariableMetadataTree::_gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* p) const {
    vars.clear();
    std::vector<std::string> v = _dm->GetCoordVarNames();
    vars.insert(vars.end(),v.begin(),v.end());
}

VGlobalAttributeMetadataTree::VGlobalAttributeMetadataTree() : VMetadataTree() {
    setTabText(0,"Global Attributes");
}

void VGlobalAttributeMetadataTree::_generateMetadata(QTreeWidgetItem* item) const {
    if (item->childCount() > 1) return; // This branch contains more than an empty leaf, and has already been computed

    QTreeWidgetItem* leaf = item->takeChild(0);
    if (leaf != 0) delete leaf;

    QString qattribute = item->text(0);

    VAPoR::DC::XType xType = _dm->GetAttType("",qattribute.toStdString());
    QString qvalue;
    if (xType == VAPoR::DC::XType::INVALID) return;
    if (xType == VAPoR::DC::XType::TEXT) {
        std::string values;
        if (!_dm->GetAtt("", qattribute.toStdString(), values)) return;
        qvalue = QString::fromStdString(values);
    }
    else if (xType == VAPoR::DC::XType::FLOAT ||
        xType == VAPoR::DC::XType::DOUBLE ) {
        std::vector<double> values;
        if (!_dm->GetAtt("", qattribute.toStdString(), values)) return;
        for (auto value : values)
            qvalue += QString::number(value) + " ";
    }
    else {  // UINT8, INT8, INT32, INT64
        std::vector<long> values;
        if (!_dm->GetAtt("", qattribute.toStdString(), values)) return;
        for (auto value : values)
            qvalue += QString::number(value) + " ";
    }

    new QTreeWidgetItem(item, {"Value:", qvalue});
    new QTreeWidgetItem(item, {"Type:", QString::fromStdString(xtypeLookup[_dm->GetAttType("",qattribute.toStdString())+1])});
}

void VGlobalAttributeMetadataTree::_gatherBranches(std::vector<std::string> &vars, VAPoR::ParamsBase* p) const {
    vars.clear();
    std::vector<std::string> v = _dm->GetAttNames("");
    vars.insert(vars.end(),v.begin(),v.end());
}

VDimensionMetadataTree::VDimensionMetadataTree() : VMetadataTree() {
    setTabText(0,"Dimension Metadata");
}

void VDimensionMetadataTree::_generateMetadata(QTreeWidgetItem* item) const {
    std::vector<size_t> dims;
    _dm->GetDimLens(item->text(0).toStdString(), dims, _ts);
    QString dimLens;
    for (auto dim : dims) {
        dimLens = dimLens + QString::number(dim) + ":";
    }
    item->setText(1,dimLens);
}

void VDimensionMetadataTree::_gatherBranches(std::vector<std::string> &dims, VAPoR::ParamsBase* p) const {
    dims.clear();
    std::vector<std::string> d = _dm->GetDimensionNames();
    dims.insert(dims.end(),d.begin(),d.end());
}
